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
# The Initial Developer of the Original Code is Novell, Inc.
#
# Portions created by the Initial Developer are Copyright (C) 2011 Novell, 
# Inc. All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

PRJ=..
PRJNAME=external
TARGET=gdiplus

.INCLUDE :  settings.mk

.IF "$(OS)$(COM)" == "WNTGCC"

ALL:
    -$(MKDIRHIER) $(OUT)/inc/wine
    cp include/* $(OUT)/inc/wine
    for L in gdiplus msi urlmon; do $(DLLTOOL) --kill-at --input-def lib/$$L.def --dllname=$$L.dll --output-lib=$(OUT)/lib/lib$$L.dll.a; done
.ENDIF

.INCLUDE :  target.mk
