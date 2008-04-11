#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.3 $
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
TARGET=genconstidl
PRJ=..
PRJNAME=oovbaapi

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

.IF "$(ENABLE_VBA)"!="YES"
dummy:
        @echo "not building vba..."
.ENDIF

# symbol files of Microsoft API
MYSYMFILES = access.api vba.api adodb.api msforms.api word.api dao.api powerpoint.api excel.api stdole.api

# ------------------------------------------------------------------

MY_GEN_IDL_PATH=$(MISC)$/idl

MYDONEFILES += $(foreach,i,$(MYSYMFILES) $(MISC)$/$(i:b).done)

# --- Targets ------------------------------------------------------

ALLTAR: GENIDL

GENIDL : $(MY_GEN_IDL_PATH) $(MYDONEFILES)

GENRDB : GENIDL $(MYURDFILES)

$(MISC)$/%.done : %.api
    @echo Processing $?
    $(PERL) api-to-idl.pl $? $(MY_GEN_IDL_PATH)
    @$(TOUCH) $@

$(MY_GEN_IDL_PATH) : 
    @@-$(MKDIR) $@

CLEAN :
    @$(RM) $(MY_GEN_IDL_PATH)$/*.idl
    @$(RM) $(foreach,i,$(MYSYMFILES) $(MISC)$/$(i:b).done)

