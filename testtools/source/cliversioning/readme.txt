#*************************************************************************
# 
#  DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#  
#  Copyright 2000, 2010 Oracle and/or its affiliates.
# 
#  OpenOffice.org - a multi-platform office productivity suite
# 
#  This file is part of OpenOffice.org.
# 
#  OpenOffice.org is free software: you can redistribute it and/or modify
#  it under the terms of the GNU Lesser General Public License version 3
#  only, as published by the Free Software Foundation.
# 
#  OpenOffice.org is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Lesser General Public License version 3 for more details
#  (a copy is included in the LICENSE file that accompanied this code).
# 
#  You should have received a copy of the GNU Lesser General Public License
#  version 3 along with OpenOffice.org.  If not, see
#  <http://www.openoffice.org/license.html>
#  for a copy of the LGPLv3 License.
# 
# ************************************************************************/




WINDOWS ONLY, no other platforms




This test checks if the cli assemblies and their policy files are installed in the 
GAC and if the policy files are correct. The office must be installed with complete 
system integration. That is, one does not use setup /a for starting the 
installation.

In OOo 3.0 there was a breanking change so that the test created for previous 
version do not work anymore. To use these old tests one needs to install a 
version of OOo less than v 3.0. Then one needs to check out the cli_ure module 
for that version. The tests are contained in cli_ure/qa/versioning.

In the sub directory version_libs are libraries which are linked with (referencing)
cli_cppuhelper
cli_basetypes
cli_ure
cli_uretypes
cli_oootypes

They are named version_3_0_0 etc, where the name indicates the version of the 
office for which they were initially build. If there are only the assemblies 
for example OOo2.0.2 installed and a client was build with the assemblies from 
OOo2 then the policy assemblies must make the correct redirection. 

When calling dmake then the executable runtest.exe and version_current.dll are 
build. When runtest runs then it loads all dlls, which names start with 
"version", from the same directory and tries to run a particular test code 
which is only available in the version_xxx.dll s. The code in the 
version_xxx.dlls uses the cli default bootstrap mechanism to find the office 
installation and start the office. When running runtest in a build environment 
then it may use the wrong libraries for bootstrapping in which case the test 
fails. Then one has to set PATH which must point to the program directory of 
the office installation. For example:

set PATH=d:\office\program

runtest stops when a test failed to run. It writes error information to the 
console which shows which version_xxx.dll failed to run and if this was due to 
a referenced assembly that could not be loaded. This would be the case if such 
an assembly or the policy assembly is not properly installed or the redirection 
in the policy assembly is wrong. 

The version_current.dll references the assemblies in the current build 
environment. version_current.dll is also executed when runtest is started.

To run the whole test one can either
1. start testools/wntmsci11.pro/bin/runtests.exe in a console where no 
environment is set.
2. Call dmake run in testools/qa/cliversions

For a limited test one can call in this directory
dmake run office=d:\office
There must not be an office installed in the system (the assemblies from the GAC 
would then be used). Only the test with the version_current.dll will succeed
because there are no policy files installed. This test requires that all 
assemblies are copied next to runtests.exe - the makefile will do that.

The parameter office must be a system path to the office installation 
directory. The java code calls runtest and also sets PATH so that the test 
works in the build environment. It also sets UNO_PATH so that the office will be
found. If the test says that it failed, then one should run runtest directly 
because it puts out more information.



Creating a new version_xxx.dll
==============================
When a version of our assemblies changes then one should provide a new version dll which test exactly the assemblies with the changed version. This is easily done:
1. set the build environment in which the new versions are effectiv.
2  call: dmake name=version_xxx.dll
3. copy the new version dll from the output tree into qa/versioning/version_libs
4. commit the new version.dll using the -kb switch (only when creating the new file : cvs new -kb version_3_1_0.dll)

