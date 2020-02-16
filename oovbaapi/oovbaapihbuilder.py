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
udkapi_urb = sys.argv[2]
udkapi_urb_abs = os.path.join(os.getcwd(), udkapi_urb)
offapi_urb = sys.argv[3]
offapi_urb_abs = os.path.join(os.getcwd(), offapi_urb)
oovbaapi_urb = sys.argv[4]
oovbaapi_urb_abs = os.path.join(os.getcwd(), oovbaapi_urb)
out_dir = sys.argv[5]
out_dir_abs = os.path.join(os.getcwd(), out_dir)
private_dir = sys.argv[6]
private_dir_abs = os.path.join(os.getcwd(), private_dir)
fakeout = sys.argv[7]
source_dir = sys.argv[8]

if not os.path.exists(private_dir):
    os.mkdir(private_dir)

subprocess.check_call([cppumaker,
                       '-Gc',
                       '-L',
                       '-O' + out_dir_abs, # FIXME, should be priv_dir_abs
                       oovbaapi_urb_abs,
                       '-X',
                       offapi_urb_abs,
                       '-X',
                       udkapi_urb_abs])

open(fakeout, 'w')
