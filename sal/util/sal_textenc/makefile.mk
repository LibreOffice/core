# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# [ Copyright (C) 2011 Stephan Bergmann, Red Hat Inc. <sbergman@redhat.com>
#   (initial developer) ]
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

PRJ = ../..
PRJNAME = sal
TARGET = sal_textenc

# Should be VISIBILITY_HIDDEN=TRUE, but sal/textenc contains objects that end up
# in both sal and sal_textenc libraries, so need to use a map file here for now.

.INCLUDE: settings.mk

# Should be DLLPRE= as it is loaded dynamically, but IOS links against it.

SHL1IMPLIB = i$(SHL1TARGET)
SHL1LIBS = $(SLB)/textenc_tables.lib
SHL1RPATH = URELIB
SHL1STDLIBS = $(SALLIB)
SHL1TARGET = sal_textenc
SHL1VERSIONMAP = saltextenc.map
DEF1NAME = $(SHL1TARGET)

.INCLUDE: target.mk
