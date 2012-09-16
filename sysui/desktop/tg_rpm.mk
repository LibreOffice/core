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

.IF "$(PKGFORMAT)"!="$(PKGFORMAT:s/rpm//)"

ALLTAR : $(RPMFLAGFILES)

$(RPMFLAGFILES) : $(SPECFILE)
    @-$(MKDIRHIER) $(@:d)
    -$(RM) $(@:d)$(@:b:s/-/ /:1)-$(SPECFILE:b)-*
    $(RPM) -bb $< $(RPMMACROS) \
        --buildroot $(ABSLOCALOUT)$/misc$/$(@:b) \
        --define "_builddir $(shell @cd $(COMMONMISC)$/$(@:b:s/-/ /:1) && pwd)" \
        --define "productname $(PRODUCTNAME.$(@:b:s/-/ /:1))" \
        --define "pkgprefix $(@:b:s/-/ /:1)$(PRODUCTVERSION.$(@:b:s/-/ /:1))" \
        --define "unixfilename $(UNIXFILENAME.$(@:b:s/-/ /:1))" \
        --define "productversion $(PRODUCTVERSION.$(@:b:s/-/ /:1))" \
        --define "iconprefix $(ICONPREFIX.$(@:b:s/-/ /:1))" \
        --define "version $(PKGVERSION.$(@:b:s/-/ /:1))" \
        --define "release $(PKGREV)" \
        --define "__debug_install_post %nil" \
        --define "_unpackaged_files_terminate_build  0" && $(TOUCH) $@
.ENDIF
