# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

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

# vim: set noet sw=4 ts=4:
