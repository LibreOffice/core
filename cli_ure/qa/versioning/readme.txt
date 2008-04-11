/*************************************************************************
 *
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: readme.txt,v $
#
# $Revision: 1.4 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
 ************************************************************************/



WINDOWS ONLY, no other platforms


This test checks if the cli assemblies and their policy files are installed in the GAC and if the policy files are correct. The office must be installed with complete system integration. That is, one does not use setup /a for starting the installation.

In the sub directory version_libs are libraries which are all using (referencing)
cli_cppuhelper
cli_basetypes
cli_ure
cli_types.

They are named version_2_0_1 etc, where the name in indicate the version of the office for which they were initially build. 
If there are only the assemblies for example OOo2.0.2 installed and a client was build with the assemblies from OOo2 then the policy assemblies must make the correct redirection. 

When calling dmake in qa/versioning then an executable runtest.exe and version.dll are build. When runtest runs then it loads all dlls from the directory and tries to run a particular test code which is only available in the version_xxx.dll s. The code in the version_xxx.dlls uses the default bootstrap mechanism to find the office installation and start the office. When running runtest in a build environment then it may use the wrong libraries for bootstrapping in which case the test fails. Then one has to set PATH which must point to the program directory of the office installation. For example:
set PATH=d:\office\program

runtest stops when a test failed to run. It writes error information on the console which shows which version_xxx.dll failed to run and if this was due to a referenced assembly that could not be loaded. This would be the case if such an assembly  or the policy assembly  is not properly installed or the redirection in the policy assembly is wrong. 

The version_current.dll contains the test. It is build with references to the  assemblies in bin directory of this project. That is, it references always the current versions of the assemblies, with "current" meaning the current build environment. version_current.dll is also executed when runtest is started.

The test framework is started with
dmake run office=d:\office

The parameter office must be a system path to the office installation directory. The java code calls runtest and also sets PATH so that the test works in the build environment. If the test says that it failed, then one should run runtest directly because it puts out more information.



Creating a new version_xxx.dll
==============================
When a version of our assemblies changes then one should provide a new version dll which test exactly the assemblies with the changed version. This is easily done:
1. set the build environment in which the new versions are effectiv.
2  call: dmake name=version_xxx.dll
3. copy the new version dll from the output tree into qa/versioning/version_libs
4. commit the new version.dll using the -kb switch (only when creating the new file : cvs new -kb version_2_0_2.dll)
Notice: Avoid naming the dll version.dll. This caused an exception but I could not make out why.

