#!/usr/bin/python
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from __future__ import print_function
from optparse import OptionParser
from os import path
from sys import stderr
try:
  from subprocess import check_output
except ImportError:
  from subprocess import Popen, PIPE
  def check_output(*cmd):
    return Popen(*cmd, stdout=PIPE).communicate()[0]
opts = OptionParser()
opts.add_option('--repository', help='maven repository id')
opts.add_option('--url', help='maven repository url')
opts.add_option('-a', help='action (valid actions are: install,deploy)')
opts.add_option('-v', help='libreoffice version')
opts.add_option('-s', action='append', help='triplet of artifactId:type:path')

args, ctx = opts.parse_args()
if not args.v:
  print('version is empty', file=stderr)
  exit(1)

common = [
  '-DgroupId=org.libreoffice',
  '-Dversion=%s' % args.v,
]

self = path.dirname(path.abspath(__file__))
mvn = ['mvn', '--file', path.join(self, 'fake_pom.xml')]

if 'install' == args.a:
  cmd = mvn + ['install:install-file'] + common
elif 'deploy' == args.a:
  cmd = mvn + [
    'deploy:deploy-file',
    '-DrepositoryId=%s' % args.repository,
    '-Durl=%s' % args.url,
  ] + common
else:
  print("unknown action -a %s" % args.a, file=stderr)
  exit(1)

for spec in args.s:
  artifact, packaging_type, src = spec.split(':')
  try:
    check_output(cmd + [
      '-DartifactId=%s' % artifact,
      '-Dpackaging=%s' % packaging_type,
      '-Dfile=%s' % src,
    ])
  except Exception as e:
    print('%s command failed: %s' % (args.a, e), file=stderr)
    exit(1)
