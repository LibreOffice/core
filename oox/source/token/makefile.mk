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

PRJNAME=oox
TARGET=token

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =							\
        $(SLO)$/propertylist.obj	\
        $(SLO)$/tokenmap.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

$(MISC)$/tokens.gperf $(INCCOM)$/tokenwords.inc $(INCCOM)$/tokens.hxx $(INCCOM)$/propertywords.inc $(INCCOM)$/properties.hxx $(INCCOM)$/oox$/core$/namespaces.hxx :
    @@noop $(assign do_phony:=.PHONY)

$(MISC)$/do_tokens $(do_phony) : tokens.txt gentoken.pl $(MISC)$/tokens.gperf $(INCCOM)$/tokenwords.inc $(INCCOM)$/tokens.hxx
    @@-$(RM) $@
    $(PERL) gentoken.pl tokens.txt $(INCCOM)$/tokens.hxx $(INCCOM)$/tokenwords.inc $(MISC)$/tokens.gperf && $(TOUCH) $@

$(INCCOM)$/oox:
    $(MKDIR) $(INCCOM)$/oox

$(INCCOM)$/oox$/core: $(INCCOM)$/oox
    $(MKDIR) $(INCCOM)$/oox$/core

$(MISC)$/do_namespaces $(do_phony) : namespaces.txt gennamespaces.pl
    @@-$(RM) $@
    $(MKDIRHIER) $(INCCOM)$/oox$/core
    $(PERL) gennamespaces.pl namespaces.txt $(INCCOM)$/oox$/core$/namespaces.hxx && $(TOUCH) $@

$(INCCOM)$/tokens.inc : $(MISC)$/tokens.gperf $(MISC)$/do_tokens
        $(AUGMENT_LIBRARY_PATH) $(GPERF) --compare-strncmp $(MISC)$/tokens.gperf | $(SED) -e "s/(char\*)0/(char\*)0, 0/g" | $(GREP) -v "^#line" >$(INCCOM)$/tokens.inc

$(SLO)$/tokenmap.obj : $(INCCOM)$/tokens.inc $(INCCOM)$/tokenwords.inc $(INCCOM)$/tokens.hxx $(INCCOM)$/oox$/core$/namespaces.hxx $(MISC)$/do_tokens $(MISC)$/do_namespaces

$(MISC)$/do_properties $(do_phony) : properties.txt genproperties.pl $(INCCOM)$/properties.hxx $(INCCOM)$/propertywords.inc
    @@-$(RM) $@
    $(PERL) genproperties.pl properties.txt $(INCCOM)$/properties.hxx $(INCCOM)$/propertywords.inc && $(TOUCH) $@

$(SLO)$/propertylist.obj : $(INCCOM)$/propertywords.inc $(INCCOM)$/properties.hxx $(MISC)$/do_properties

