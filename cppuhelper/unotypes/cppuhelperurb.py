#!/usr/bin/env python3

from pathlib import Path
import sys, os, subprocess

idlwrite = sys.argv[1]
udkapi_urb = sys.argv[2]
udkapi_urb_abs = os.path.join(os.getcwd(), udkapi_urb)
source_dir = sys.argv[3]
priv_dir = sys.argv[4]
priv_dir_abs = os.path.join(os.getcwd(), priv_dir)
classnamefile = os.path.join(priv_dir, 'classnames')
rdbfile = os.path.join(os.getcwd(), sys.argv[5])

if not os.path.exists(priv_dir):
    os.mkdir(priv_dir)

with open(classnamefile, 'w') as of:
    fnames = Path(source_dir).glob('cppuhelper/**/*.idl')
    for fname in fnames:
        dotname = str(fname.relative_to(source_dir).with_suffix('')).replace('/', '.')
        if dotname.endswith('.modules'):
            continue
        of.write(dotname)
        of.write(' ')

subprocess.check_call([idlwrite,
                       udkapi_urb_abs,
                       source_dir,
                       '@' + classnamefile,
                       rdbfile])
