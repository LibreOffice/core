#! /bin/bash

source/src2xml.py --include-path=$(tr '\n' ':' < include-sw.lst) --dry-run $(cat src-sw.lst) "$@"
