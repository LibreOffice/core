#!/usr/bin/env python3

import sys, os, subprocess

svidl = sys.argv[1]
source_root = sys.argv[2]
build_root = sys.argv[3]
build_dir = sys.argv[4]
input_file = sys.argv[5]
output_file = sys.argv[6]
dep_file = sys.argv[7]

input_abs = os.path.normpath(os.path.join(build_root, input_file))
output_abs = os.path.join(build_root, output_file)
dep_abs = os.path.join(build_root, dep_file)

args = ['-quiet',
        '-I' + os.path.join(source_root, 'svx/sdi'),
        '-I' + os.path.join(source_root, 'sfx2/sdi'),
        '-I' + os.path.join(source_root, 'basctl/sdi'),
        '-I' + os.path.join(source_root, 'include'),
        '-fx' + os.path.join(source_root, 'basctl/sdi/baside.sdi'),
        '-fs' + output_abs,
        '-fM' + dep_abs,
        '-fm' + os.path.join(build_root, build_dir),
        input_abs]

sys.exit(subprocess.call([svidl] + args))
