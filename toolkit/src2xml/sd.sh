#!/bin/bash

source/src2xml.py --include-path=$(tr '\n' ':' < include-sd.lst) --dry-run $(cat src-sd.lst) "$@"
