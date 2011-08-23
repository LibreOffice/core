#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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

PRJ=..$/..

PRJNAME=sysui
TARGET=launcher

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  ..$/productversion.mk

.IF "$(PKGFORMAT)"!="$(PKGFORMAT:s/rpm//)"
RPMTARFILES=$(BIN)$/rpm$/{$(PRODUCTLIST)}-desktop-integration.tar.gz
.ENDIF

.IF "$(PKGFORMAT)"!="$(PKGFORMAT:s/deb//)"
DEBTARFILES=$(BIN)$/deb$/{$(PRODUCTLIST)}-desktop-integration.tar.gz
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

ALLTAR: $(RPMTARFILES) $(DEBTARFILES)

.IF "$(RPMTARFILES)" != ""

$(RPMTARFILES) : $(PKGDIR)
    $(MKDIRHIER) $(@:d)
    $(GNUTAR) -C $(PKGDIR:d:d) -cf - $(PKGDIR:f)$/{$(shell cd $(PKGDIR); ls $(@:b:b:s/-/ /:1)*.rpm)} | gzip > $@

.ENDIF # "$(TARFILE)" != ""

.IF "$(DEBTARFILES)" != ""

$(DEBTARFILES) : $(PKGDIR)
    $(MKDIRHIER) $(@:d)
    $(GNUTAR) -C $(PKGDIR:d:d) -cf - $(PKGDIR:f)$/{$(shell @cd $(PKGDIR); ls $(@:b:b:s/-/ /:1)*.deb)} | gzip > $@
    
.ENDIF
