#!/bin/bash

source/src2xml.py --include-path=$(tr '\n' ':' < include-sc.lst) --dry-run $(cat src-sc.lst) "$@"
