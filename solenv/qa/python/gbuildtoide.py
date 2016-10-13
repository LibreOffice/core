'''
  This is file is part of the LibreOffice project.

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  This file incorporates work covered by the following license notice:

    Licensed to the Apache Software Foundation (ASF) under one or more
    contributor license agreements. See the NOTICE file distributed
    with this work for additional information regarding copyright
    ownership. The ASF licenses this file to you under the Apache
    License, Version 2.0 (the "License"); you may not use this file
    except in compliance with the License. You may obtain a copy of
    the License at http://www.apache.org/licenses/LICENSE-2.0 .
'''

import subprocess
import unittest
import json
import os
import os.path
import tempfile


class CheckGbuildToIde(unittest.TestCase):

    def test_gbuildtoide(self):
        tempwork = tempfile.mkdtemp()
        os.chdir(os.path.join(os.environ['SRCDIR'], 'solenv', 'qa', 'python', 'selftest'))
        subprocess.check_call(['make', 'gbuildtoide', 'WORKDIR=%s' % tempwork])
        jsonfiles = os.listdir(os.path.join(tempwork, 'GbuildToIde', 'Library'))
        gbuildlibs = []
        for jsonfilename in jsonfiles:
            with open(os.path.join(tempwork, 'GbuildToIde', 'Library', jsonfilename), 'r') as f:
                print('loading %s' % jsonfilename)
                gbuildlibs.append(json.load(f))
        foundlibs = set()
        for lib in gbuildlibs:
            self.assertEqual(set(lib.keys()), set(['ASMOBJECTS', 'CFLAGS', 'COBJECTS', 'CXXFLAGS', 'CXXOBJECTS', 'DEFS', 'GENCOBJECTS', 'GENCXXOBJECTS', 'ILIBTARGET', 'INCLUDE', 'LINKED_LIBS', 'LINKED_STATIC_LIBS', 'LINKTARGET', 'OBJCFLAGS', 'OBJCOBJECTS', 'OBJCXXFLAGS', 'OBJCXXOBJECTS', 'YACCOBJECTS']))
            if lib['LINKTARGET'].find('gbuildselftestdep') != -1:
                foundlibs.add('gbuildselftestdep')
            elif lib['LINKTARGET'].find('gbuildselftest') != -1:
                foundlibs.add('gbuildselftest')
                self.assertIn('-Igbuildtoidetestinclude', lib['INCLUDE'].split())
                self.assertIn('gbuildselftestdep', lib['LINKED_LIBS'].split())
                self.assertIn('solenv/qa/python/selftest/selftestobject', lib['CXXOBJECTS'].split())
                self.assertIn('-DGBUILDSELFTESTDEF', lib['DEFS'].split())
                self.assertIn('-DGBUILDSELFTESTCXXFLAG', lib['CXXFLAGS'].split())
                self.assertIn('-DGBUILDSELFTESTCFLAG', lib['CFLAGS'].split())
            else:
                self.assertTrue(False)
        self.assertEqual(foundlibs, set(['gbuildselftest', 'gbuildselftestdep']))
        self.assertEqual(len(foundlibs), 2)

if __name__ == "__main__":
    unittest.main()
