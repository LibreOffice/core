#!/bin/bash
# Restore all deleted files from the problematic commit
git diff --name-only --diff-filter=D 6dfa8cf^ 6dfa8cf | while read p; do
  git checkout 6dfa8cf^ -- "$p"
done
