#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

Adapting version after a release of an office
=============================================
After a release the entries in cli_ure/version/version.txt must be changed to reflect the versions
of the assemblies at the time of the release. Please refer to the document at 
http://udk.openoffice.org/common/man/spec/assemblyversioning.html
for more information about versioning of assemblies. 


Step 1: Remove old office installations. 

To do this, deinstall all offices, 
and make sure that there are no assemblies installed in the Global Assembly Cache (GAC). The GAC 
is contained in  C:\Windows\assembly. Change C:\Windows according to your windows installation.
Use the Windows Explorer to view the contents of the GAC. Are there any of the following assemblies
installed:

cli_uretypes.dll
cli_basetypes.dll
cli_cppuhelper.dll
cli_ure.dll
cli_oootypes.dll (build in unoil)

policy.x.y.cli_uretypes.dll
policy.x.y.cli_basetypes.dll
policy.x.y.cli_ure.dll
policy.x.y.cli_cppuhelper.dll
policy.x.y.cli_oootypes.dll (build in unoil)

The "x" and "y" in the names of the policy assemblies are to be replaces by version numbers. At the 
time of writing the real names are:

policy.1.0.cli_uretypes.dll
policy.1.0.cli_basetypes.dll
policy.1.0.cli_ure.dll
policy.1.0.cli_cppuhelper.dll
policy.1.0.cli_ootypes.dll (build in unoil)

After deinstalling the offices, there should none of them remain in the GAC. If there are some, then 
try to remove them by clicking on them an choose uninstall from the context menu of the mouse.

Step 2: Install the office of the last release (respin when using staroffice)

Step 3: Determine the versions of the assemblies

Use the Windows Explorer to view the contents of the Windows\assembly directory. Locate the assemblies
and policy assemblies. See step 1 for the names of those assemblies. Take down the version number as 
can be found in the version column, which is usually right next to the name column.

Step 4: Changing the versions in the cli_ure module.

Open the file cli_ure\version\version.txt.
The file contains name/value pairs, such as:CLI_URETYPES_NEW_VERSION=1.0.3.0.
The first part of the names represent the assemly which they are referring to. So obviously 
entries starting with CLI_URETYPES refer to the cli_uretypes.dll. Entries which contain the part "POLICY" refer
to the policy assembly. For example:
CLI_URETYPES_POLICY_VERSION refers to the policy assembly for cli_uretypes which is named 
policy.1.0.cli_uretypes.dll

The versions may already have been incremented because someone has changed code after the 
last release. So if a version from version.txt is greater then the one of the respective 
assembly in the GAC then leave it at that.

The values have to be adjusted as follows:

XYZ_NEW_VERSION : increse the value of the version 
GAC. "XYZ_" would be "CLI_URETYPES", "CLI_URE", etc.
XYZ_OLD_VERSION : must be changes so that the right version part is one less than 
XYZ_NEW_VERSION. For example

CLI_URETYPES_NEW_VERSION=1.0.[3].0
CLI_URETYPES_OLD_VERSION=1.0.0.0-1.0.[2].0

The affected part is marked by the brackets.

XYZ_POLICY_VERSION: change the version according to the version of the policy assembly from the 
GAC.

XYZ_POLICY_ASSEMBLY: remain unchanged.

Commit the changes and rebuild the project.

Step 5: Changing the versions in the unoil module

The unoil module builds the cli_oootypes.dll which contains the office types (offapi). Change the contents
of unoil/climaker/version.txt similar to the versions.txt in this module. Then rebuild the module


The automatic test in cli_ure/qa/versioning should be extended. See the readme.txt in that directory 
for more information.







How does the version mechanism works
====================================

The assemblies which are build in this project have a strong name and hence a version. The version should 
be increased whenever something was fixed or the code has changed in any way. For further information
have a look at 
http://udk.openoffice.org/common/man/spec/assemblyversioning.html

The versions of all assemblies are contained in cli_ure/version/version.txt. This is the place where
the versions are changed. This will happen under two circumstances. First, the versions are 
adjusted AFTER an office was released. The version.txt must then contain the versions at the 
time of the release. Second, when something was fixed. Then a version should be changed unless 
this has already be done for the next release. Please look at the document mentioned further
above.

If new published UNO types have been added since the last release (product update) then the 
minor version of cli_uretypes.dll should be incremented. When building the version directory, a script
is run that recognizes this and writes a cliureversion.mk file in the bin directory.
cliureversion.mk contains all the entries of version.txt. The versions have been incremented 
by the script. The script obtains the information of new types from unotype_statistics.txt
which is build in offapi.

The contents of cliureversion.mk is used when building the assemblies in this project. It 
is also delivered so that instset_native (or instsetoo_native) can use it when building 
the installation sets.


The contents of version.txt
===========================
The entries in version.txt are needed for building the assemblies and for building the 
installation set (msi database).

For every assembly exist four 
entries. For example:

CLI_URETYPES_NEW_VERSION=1.0.3.0
CLI_URETYPES_OLD_VERSION=1.0.0.0-1.0.2.0
CLI_URETYPES_POLICY_VERSION=3.0.0.0
CLI_URETYPES_POLICY_ASSEMBLY=policy.1.0.cli_uretypes

The meaning of these entries is the following:

CLI_URETYPES_NEW_VERSION represents the current version of the assembly.

CLI_URETYPES_OLD_VERSION represents a range of former versions (which were compatible). 
It has to be placed in the 
cli_uretypes.config XML file which is part of the policy assembly. This is done automatically.
The XYZ_OLD_VERSION and XYZ_NEW_VERSION values are used for the binding redirection of 
the policy file.

CLI_URETYPES_POLICY_VERSION represents the version of the policy file.

CLI_URETYPES_POLICY_ASSEMBLY represents the name of the policy file. 

Please refer to the document at 
http://udk.openoffice.org/common/man/spec/assemblyversioning.html
about how the versions have to look like.

When the minor version is changed, which is the third number from the left, the 
"old version" and the policy version must be changed as well. Using the former example,
an incremented version would look like this:
CLI_URETYPES_NEW_VERSION=1.0.4.0
CLI_URETYPES_OLD_VERSION=1.0.0.0-1.0.3.0
CLI_URETYPES_POLICY_VERSION=4.0.0.0
CLI_URETYPES_POLICY_ASSEMBLY=policy.1.0.cli_uretypes

If the major version changed we would have these values:
CLI_URETYPES_NEW_VERSION=2.0.0.0
CLI_URETYPES_OLD_VERSION=2.0.0.0-2.0.0.0
CLI_URETYPES_POLICY_VERSION=1.0.0.0
CLI_URETYPES_POLICY_ASSEMBLY=policy.2.0.cli_uretypes

Because a change of a major is only done if the code has changed incompatibly, we must not 
redirect old client code to the new assembly. Actually we would not need a policy file here.
The name of the policy file has changed as well so as to refer to the new version. Since
the name is new and refers to the version 2 auf cli_uretypes, we can start with the policy version
from 1.

The next compatible change would require to change the version to these:
CLI_URETYPES_NEW_VERSION=2.0.1.0
CLI_URETYPES_OLD_VERSION=2.0.0.0-2.0.1.0
CLI_URETYPES_POLICY_VERSION=2.0.0.0
CLI_URETYPES_POLICY_ASSEMBLY=policy.2.0.cli_uretypes


Automatic incrementation of the version 
=======================================
Currently switched off! See cli_ure/version/makefile.mk
The automatic incrementation of the version this is done when new published types have been 
introduce between two releases.In cli_ure/version/makefile.mk the script 
cli_ure/source/scripts/increment_version.pl
is run which creates the cliureversion.mk with the new versions. This automatism only changes
the version parts which have the meaning of a compatible change. Which versions are to be 
incremented is contained in cli_ure/version/incversions.txt. It contains, for example these entries:

CLI_URETYPES_NEW_VERSION
CLI_URETYPES_OLD_VERSION
CLI_URETYPES_POLICY_VERSION

The names are exactly the same as in version.txt. The script knows how to increase the version
of the different types:

Entries ending on _NEW_VERSION: The third number from the left is incremented.
Entries ending on _OLD_VERSION: The third number from the left of the second version is incremented.
Entries ending on _POLICY_VERSION: The first number from the left is incremented.

For example, the versions in version.txt are:
CLI_URETYPES_NEW_VERSION=1.0.4.0
CLI_URETYPES_OLD_VERSION=1.0.0.0-1.0.3.0
CLI_URETYPES_POLICY_VERSION=4.0.0.0

If new types have been added the script would create these entries in cliureversion.mk
CLI_URETYPES_NEW_VERSION=1.0.5.0
CLI_URETYPES_OLD_VERSION=1.0.0.0-1.0.4.0
CLI_URETYPES_POLICY_VERSION=5.0.0.0

As one can see from the incversions.txt, the versions of the cli_ure.dll and cli_cppuhelper.dll 
are also changed. This is because these are dependent on cli_uretypes.dll.


Further notes on increasing the assembly versions
================================
Changing a version for one assembly means changing 
at least the XYZ_NEW_VERSION, XYZ_OLD_VERSION and XYZ_POLICY_VERSION. In case of an incompatible 
change, that is one of the first two numbers of the version has been changed, the XYZ_POLICY_ASSEMBLY
must also be changed.

When changing a version of an assembly then the versions of the assemblies which depend on it should 
be changed as well. For example, when changing the version of cli_uretypes.dll, then the versions of 
cli_ure.dll and cli_cppuhelper.dll should be changed as well. One can use idlasm.exe to see which 
assemblies are referenced by an assembly. The information is contained in the assemblie's manifest.

If one would not change the versions of the dependent dlls then one would risk that an assembly
has the same old version but references  a different assembly. For example, say we have a 
cli_uretypes.dll with version 1 and a cli_ure.dll with version 1. cli_ure.dll references version 1 of
cli_uretypes.dll. Now the version of cli_uretypes.dll changes to version 2 and its policy assembly is 
adapted so that all code that uses version 1 now uses version 2. This would also allow cli_ure.dll
o run with the new cli_uretypes.dll. If now cli_ure.dll is build, then it would reference 
cli_uretypes.dll version 2, because our build environment does not keep the older assembly. The old
cli_uretypes.dll version 1 was replaced by version 2. cli_ure.dll now references cli_uretypes.dll version 2
but still has the old version.



rebasing of assemblies
=======================================================
Neither assemblies nor policy assemblies may be rebased. This would 
make the signature invalid. Therefore all assemblies must be contained
in postprocess/rebase/no_rebase.txt
