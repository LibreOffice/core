#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************

no-description.oxt, no-dependencies.oxt, empty-dependencies.oxt effectively have
no dependencies and should thus install successfully.

broken-dependencies.oxt contains a malformed description.xml and should thus
display an error and not install.

double-dependencies.oxt contains a description.xml with two dependencies
elements.  This is not allowed by the spec but behaviour is unspecified.  In the
current implementation, it combines the two elements, and thus finds two
unsatisfied dependencies, displays the Unsatisfied Dependencies dialog and does
not install.

version21.oxt contains a dependency on OOo 2.1 (and should thus only install in
OOo 2.1 or later); version21ns.oxt is the same, but with a different way of
using XML namespaces; version21other.oxt additionally contains an unsatisfied
dependency (and should thus not install in any OOo version).  version22.oxt
contains a dependency on OOo 2.2 (and should thus only install in OOo 2.2 or
later).  version23.oxt contains a dependency on OOo 2.3 (and should thus only
install in OOo 2.3 or later).  version10000.oxt contains a dependency on the
hypothetical OOo version 10000 (and should thus not install in any OOo version).
versionempty.oxt contains an empty value attribute and versionnone.oxt lacks the
value attribute; neither is allowed by the spec, but the current implementation
treats both as pre OOo 2.1 versions (and the extensions should thus install in
OOo 2.1 or later).

maxversion30.oxt contains a maximal version dependency on OOo 3.0 (and should
thus only install in OOo 3.0 or earlier, back to OOo 2.3, thanks to the
additionally specified OpenOffice.org-minimal-version attribute).
maxversion10000.oxt contains a maximal version dependency on the hypothetical
OOo version 10000 (and should thus install in any OOo version 3.1 or later;
OpenOffice.org-maximal-version was introduced in OOo 3.1, and no OpenOffice.org-
minimal-version attribute is specified).  bad-minmaxversion.oxt contains a
minimal version dependency on OOo 3.2 and a maximal version dependency on
OOo 3.1 (and should thus not install in any OOo version).

minattr22.oxt contains a (hypothetical, most probably never satisfied)
UNSATISFIED dependency with an OpenOffice.org-minimal-version attribute of
"2.2" (and should thus install in OOo 2.3 or later); minattr23.oxt is similar,
but with an OpenOffice.org-minimal-version attribute of "2.3" (and should thus
also install in OOo 2.3 or later); minattr24.oxt is similar, but with an
OpenOffice.org-minimal-version attribute of "2.4" (and should thus only install
in OOo 2.4 or later).

All of the following testcases should result in the Unsatisfied Dependencies
dialog being displayed and the extension not being installed:

unknown-dependency.oxt contains a dependency without a name attribute, and
should thus display "Unknown" (localized).

funny-dependency.oxt, many-dependencies.oxt contain somewhat extreme input.

license-dependency.oxt contains both a license to be accepted by the user and
dependencies.  What is important here is that the Unsatisfied Dependencies
dialog is displayed, but not the license (as installation aborts as soon as
unsatisfied dependencies are found).
