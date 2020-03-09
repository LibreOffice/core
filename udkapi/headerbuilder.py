#!/usr/bin/env python3

from pathlib import Path
import sys, os, subprocess

# NOTE: cppumaker is _very_ picky about paths. For
# example if you pass the output directory as relative
# it will fail with a misleading error that has _nothing_
# to do with the actual problem.

cppumaker = sys.argv[1]
rdbfile = sys.argv[2]
rdbfile_abs = os.path.join(os.getcwd(), rdbfile)
out_dir = sys.argv[3]
out_dir_abs = os.path.join(os.getcwd(), out_dir)
priv_dir = sys.argv[4]
priv_dir_abs = os.path.join(os.getcwd(), priv_dir)
fakeout = sys.argv[5]
source_dir = sys.argv[6]

subprocess.check_call([cppumaker,
                       '-Gc',
                       '-C',
                       '-O' + out_dir_abs, # FIXME, should be priv_dir_abs
                       rdbfile_abs])

open(fakeout, 'w')
