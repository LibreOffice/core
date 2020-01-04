#!/usr/bin/env python

from __future__ import print_function
from optparse import OptionParser
import os.path
import re
import sys

M = {
  'juh': 'javaunohelper',
  'jurt': 'jurt',
  'officebean': 'bean',
  'ridl': 'ridljar',
  'unoil': 'unoil',
  'unoloader': 'ridljar',
}

parser = OptionParser()
_, args = parser.parse_args()

if not len(args):
  parser.error('not enough arguments')
elif len(args) > 1:
  parser.error('too many arguments')

DEST = r'\g<1>%s\g<3>' % args[0]


def replace_in_file(filename, src_pattern):
  try:
    f = open(filename, "r")
    s = f.read()
    f.close()
    s = re.sub(src_pattern, DEST, s)
    f = open(filename, "w")
    f.write(s)
    f.close()
  except IOError as err:
    print('error updating %s: %s' % (filename, err), file=sys.stderr)

src_pattern = re.compile(r'^(\s*<version>)([-.@\w]+)(</version>\s*)$',
                         re.MULTILINE)

for a in ['juh', 'jurt', 'officebean', 'ridl', 'unoil', 'unoloader']:
  replace_in_file(os.path.join(M[a], 'pom.%s.xml' % a), src_pattern)

src_pattern = re.compile(r"^(LIBREOFFICE_VERSION = ')([-.@\w]+)(')$",
                         re.MULTILINE)
replace_in_file('solenv/maven/VERSION', src_pattern)
