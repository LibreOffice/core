#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: readme.txt,v $
#
# $Revision: 1.7 $
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
#*************************************************************************

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
