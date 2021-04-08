#!/usr/bin/env python3

import os, sys, subprocess

output_name = sys.argv[1]
output_dir = os.path.split(output_name)[0]
dattmp_name = os.path.splitext(output_name)[0] + '.tmp'
gncmtmp_name = output_name.replace('_dat.c', '_tmp.c')
priv_dir = sys.argv[2]
rsp_name = os.path.join(priv_dir, dattmp_name)

if not os.path.isdir(priv_dir):
    os.mkdir(priv_dir)

open(dattmp_name, 'w').write('''count_word.brk
dict_word_he.brk
dict_word_hu.brk
dict_word_nodash.brk
dict_word_prepostdash.brk
dict_word.brk
edit_word_he.brk
edit_word_hu.brk
edit_word.brk
line.brk
sent.brk
''')

rc = subprocess.call(['/usr/sbin/gencmn',
                      '-n',
                      'OpenOffice',
                      '-t',
                      'tmp',
                      '-S',
                      '-d',
                      output_dir,
                      '0',
                      dattmp_name])

os.unlink(dattmp_name)

if rc != 0:
    sys.exit(rc)

d = open(gncmtmp_name, 'r').read()
with open(output_name, 'w') as ofile:
    ofile.write('''#ifdef _MSC_VER
#pragma warning(disable : 4229)
#endif
''')
    ofile.write(d)

os.unlink(gncmtmp_name)
