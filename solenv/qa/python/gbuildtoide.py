'''
  This file is part of the LibreOffice project.

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

'''

import subprocess
import unittest
import json
import os
import os.path
import tempfile


class CheckGbuildToIde(unittest.TestCase):
    def setUp(self):
        self.tempwork = tempfile.mkdtemp()

    def tearDown(self):
        subprocess.check_call(['rm', '-rf', self.tempwork])

    def test_gbuildtoide(self):
        os.chdir(os.path.join(os.environ['SRCDIR'], 'solenv', 'qa', 'python', 'selftest'))
        make = os.environ['MAKE']
        subprocess.check_call([make, 'gbuildtoide', 'WORKDIR=%s' % self.tempwork])
        jsonfiles = os.listdir(os.path.join(self.tempwork, 'GbuildToIde', 'Library'))
        gbuildlibs = []
        for jsonfilename in jsonfiles:
            with open(os.path.join(self.tempwork, 'GbuildToIde', 'Library', jsonfilename), 'r') as f:
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
        jsonfiles = os.listdir(os.path.join(self.tempwork, 'GbuildToIde', 'Executable'))
        gbuildexes = []
        for jsonfilename in jsonfiles:
            with open(os.path.join(self.tempwork, 'GbuildToIde', 'Executable', jsonfilename), 'r') as f:
                gbuildexes.append(json.load(f))
        foundexes = set()
        for exe in gbuildexes:
            if exe['LINKTARGET'].find('gbuildselftestexe') != -1:
                foundexes.add('gbuildselftestexe')
            else:
                self.assertTrue(False)
        self.assertEqual(foundexes, set(['gbuildselftestexe']))
        self.assertEqual(len(foundexes), 1)

if __name__ == "__main__":
    unittest.main()
