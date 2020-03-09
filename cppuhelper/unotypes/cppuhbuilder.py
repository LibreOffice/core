#!/usr/bin/env python3

from pathlib import Path
import sys, os, subprocess

# FIXME: mostly a duplicate of the same script in udkapi.
# Make these use some sort of common functionality eventually.

#  command: [offbuilder,
#            cppumaker_exe,
#            offapi_urbm,
#            udkapi_urb,
#            '@OUTDIR@',
#            '@PRIVATE_DIR@',
#            meson.current_source_dir()]

cppumaker = sys.argv[1]
offapi_urb = sys.argv[2]
offapi_urb_abs = os.path.join(os.getcwd(), offapi_urb)
udkapi_urb = sys.argv[3]
udkapi_urb_abs = os.path.join(os.getcwd(), udkapi_urb)
out_dir = sys.argv[4]
out_dir_abs = os.path.join(os.getcwd(), out_dir)
private_dir = sys.argv[5]
private_dir_abs = os.path.join(os.getcwd(), private_dir)
fakeout = sys.argv[6]
source_dir = sys.argv[7]

if not os.path.exists(private_dir):
    os.mkdir(private_dir)

subprocess.check_call([cppumaker,
                       '-Gc',
                       '-C',
                       '-O' + out_dir_abs, # FIXME, should be priv_dir_abs
                       offapi_urb_abs,
                       '-X',
                       udkapi_urb_abs])

open(fakeout, 'w')
