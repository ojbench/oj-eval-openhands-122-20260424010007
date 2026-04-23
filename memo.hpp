#ifndef MEMO_HPP
#define MEMO_HPP

#include "event.h"

#include <iostream>

inline std::string CustomNotifyLateEvent::GetNotification(int n) const {
  return NotifyLateEvent::GetNotification(n) + generator_(n);
}

class Memo {
 private:
  struct Node {
    const Event *event;
    int n; // Number of times notified
    Node *next;
    Node(const Event *e, int count = 0) : event(e), n(count), next(nullptr) {}
  };

  Node **heads;
  Node **tails;
  int duration;
  int current_tick;

  void add_to_list(int time, const Event *event, int n) {
    if (time > duration) return;
    Node *newNode = new Node(event, n);
    if (tails[time] == nullptr) {
      heads[time] = tails[time] = newNode;
    } else {
      tails[time]->next = newNode;
      tails[time] = newNode;
    }
  }

 public:
  Memo() = delete;

  Memo(int duration) : duration(duration), current_tick(0) {
    heads = new Node*[duration + 1];
    tails = new Node*[duration + 1];
    for (int i = 0; i <= duration; ++i) {
      heads[i] = nullptr;
      tails[i] = nullptr;
    }
  }

  ~Memo() {
    for (int i = 0; i <= duration; ++i) {
      Node *curr = heads[i];
      while (curr) {
        Node *next = curr->next;
        delete curr;
        curr = next;
      }
    }
    delete[] heads;
    delete[] tails;
  }

  void AddEvent(const Event *event) {
    if (const NormalEvent *ne = dynamic_cast<const NormalEvent*>(event)) {
      add_to_list(ne->GetDeadline(), event, 0);
    } else if (const NotifyBeforeEvent *nbe = dynamic_cast<const NotifyBeforeEvent*>(event)) {
      add_to_list(nbe->GetNotifyTime(), event, 0);
    } else if (const NotifyLateEvent *nle = dynamic_cast<const NotifyLateEvent*>(event)) {
      add_to_list(nle->GetDeadline(), event, 0);
    }
  }

  void Tick() {
    current_tick++;
    if (current_tick > duration) return;

    Node *curr = heads[current_tick];
    heads[current_tick] = nullptr;
    tails[current_tick] = nullptr;

    while (curr) {
      if (!curr->event->IsComplete()) {
        std::cout << curr->event->GetNotification(curr->n) << std::endl;
        
        if (const NotifyBeforeEvent *nbe = dynamic_cast<const NotifyBeforeEvent*>(curr->event)) {
          if (curr->n == 0) {
            add_to_list(nbe->GetDeadline(), curr->event, 1);
          }
        } else if (const NotifyLateEvent *nle = dynamic_cast<const NotifyLateEvent*>(curr->event)) {
          add_to_list(current_tick + nle->GetFrequency(), curr->event, curr->n + 1);
        }
      }
      Node *next = curr->next;
      delete curr;
      curr = next;
    }
  }
};
#endif