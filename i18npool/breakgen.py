#!/usr/bin/env python3

import sys, os, subprocess

ifile_name = sys.argv[1]
ofile_name = sys.argv[2]
# ICU binaries add the _brk part by themselves
# automatically so we must strip it away here.
obase = os.path.splitext(ofile_name)[0][:-4]
temp_name = ofile_name + '.tmp'
brk_name = obase + '.brk'
priv_dir = os.path.split(ofile_name)[0]

idata = open(ifile_name).readlines()
filtered = [x for x in idata if 'Prepend' not in x]
open(temp_name, 'w').writelines(filtered)

rc = subprocess.call(['genbrk',
                      '-q',
                      '-r',
                      temp_name,
                      '-o',
                      brk_name
])

if rc != 0:
    sys.exit(rc)

rc = subprocess.call(['/usr/sbin/genccode',
                      '-n',
                      'OpenOffice',
                      '-d',
                      priv_dir,
                      brk_name
                      ],
                     stdout=subprocess.DEVNULL)

sys.exit(rc)
