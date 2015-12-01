#!/usr/bin/python
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from __future__ import print_function
from optparse import OptionParser
from os import path, environ
from subprocess import check_output
from sys import stderr

M = {
  'juh': 'javaunohelper',
  'jurt': 'jurt',
  'officebean': 'bean',
  'ridl': 'ridljar',
  'unoil': 'unoil',
  'unoloader': 'ridljar',
}

opts = OptionParser()
opts.add_option('--repository', help='maven repository id')
opts.add_option('--url', help='maven repository url')
opts.add_option('-o')
opts.add_option('-a', help='action (valid actions are: install,deploy)')
opts.add_option('-v', help='gerrit version')
opts.add_option('-s', action='append', help='triplet of artifactId:type:path')

args, ctx = opts.parse_args()
if not args.v:
  print('version is empty', file=stderr)
  exit(1)

root = path.abspath(__file__)
while not path.exists(path.join(root, '.buckconfig')):
  root = path.dirname(root)

if 'install' == args.a:
  cmd = [
    'mvn',
    'install:install-file',
    '-Dversion=%s' % args.v,
  ]
elif 'deploy' == args.a:
  cmd = [
    'mvn',
    'gpg:sign-and-deploy-file',
    '-DrepositoryId=%s' % args.repository,
    '-Durl=%s' % args.url,
  ]
else:
  print("unknown action -a %s" % args.a, file=stderr)
  exit(1)

for spec in args.s:
  artifact, packaging_type, src = spec.split(':')
  exe = cmd + [
    '-DpomFile=%s' % path.join(root, '%s/pom.%s.xml' % (M[artifact], artifact)),
    '-Dpackaging=%s' % packaging_type,
    '-Dfile=%s' % src,
  ]
  try:
    if environ.get('VERBOSE'):
      print(' '.join(exe), file=stderr)
    check_output(exe)
  except Exception as e:
    print('%s command failed: %s' % (args.a, e), file=stderr)
    exit(1)

with open(args.o, 'w') as fd:
  if args.repository:
    print('Repository: %s' % args.repository, file=fd)
  if args.url:
    print('URL: %s' % args.url, file=fd)
  print('Version: %s' % args.v, file=fd)
