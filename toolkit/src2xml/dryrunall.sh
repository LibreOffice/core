#! /bin/bash
source/src2xml.py --dry-run --includes-from=include.lst $(cat src.lst) "$@"
