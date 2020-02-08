#!/usr/bin/env python3

from pathlib import Path
import sys, os, subprocess

# NOTE: cppumaker is _very_ picky about paths. For
# example if you pass the output directory as relative
# it will fail with a misleading error that has _nothing_
# to do with the actual problem.

idlwrite = sys.argv[1]
cppumaker = sys.argv[2]
out_dir = sys.argv[3]
out_dir_abs = os.path.join(os.getcwd(), sys.argv[3])
priv_dir = sys.argv[4]
priv_dir_abs = os.path.join(os.getcwd(), sys.argv[4])
fakeout = sys.argv[5]
source_dir = sys.argv[6]
classnamefile = os.path.join(priv_dir, 'classnames')
rdbfile = os.path.join(os.getcwd(), priv_dir, 'udkapi.rdb')

if not os.path.exists(priv_dir):
    os.mkdir(priv_dir)

with open(classnamefile, 'w') as of:
    fnames = Path(source_dir).glob('com/**/*.idl')
    for fname in fnames:
        dotname = str(fname.relative_to(source_dir).with_suffix('')).replace('/', '.')
        if dotname == 'com.sun.star.modules':
            continue
        of.write(dotname)
        of.write(' ')

subprocess.check_call([idlwrite,
                       source_dir,
                       '@' + classnamefile,
                       rdbfile])

subprocess.check_call([cppumaker,
                       '-Gc',
                       '-O' + out_dir_abs, # FIXME, should be priv_dir_abs
                       rdbfile])
