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

SLOFILES = \
    $(SLO)$/namespacemap.obj \
    $(SLO)$/propertynames.obj \
    $(SLO)$/tokenmap.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

GENHEADERPATH = $(INCCOM)$/oox$/token

$(MISC)$/tokenhash.gperf $(INCCOM)$/tokennames.inc $(GENHEADERPATH)$/tokens.hxx $(INCCOM)$/namespacenames.inc $(MISC)$/namespaces.txt $(GENHEADERPATH)$/namespaces.hxx $(INCCOM)$/propertynames.inc $(GENHEADERPATH)$/properties.hxx :
    @@noop $(assign do_phony:=.PHONY)

$(SLO)$/tokenmap.obj : $(INCCOM)$/tokenhash.inc $(INCCOM)$/tokennames.inc $(GENHEADERPATH)$/tokens.hxx $(MISC)$/do_tokens

$(INCCOM)$/tokenhash.inc : $(MISC)$/tokenhash.gperf $(MISC)$/do_tokens
    $(AUGMENT_LIBRARY_PATH) gperf --compare-strncmp $(MISC)$/tokenhash.gperf | $(SED) -e "s/(char\*)0/(char\*)0, 0/g" | $(GREP) -v "^#line" >$(INCCOM)$/tokenhash.inc

$(MISC)$/do_tokens $(do_phony) : tokens.txt tokens.pl tokens.hxx.head tokens.hxx.tail $(GENHEADERPATH)$/tokens.hxx $(INCCOM)$/tokennames.inc $(MISC)$/tokenhash.gperf
    @@-$(RM) $@
    $(MKDIRHIER) $(GENHEADERPATH)
    $(PERL) tokens.pl tokens.txt $(MISC)$/tokenids.inc $(INCCOM)$/tokennames.inc $(MISC)$/tokenhash.gperf && $(TYPE) tokens.hxx.head $(MISC)$/tokenids.inc tokens.hxx.tail > $(GENHEADERPATH)$/tokens.hxx && $(TOUCH) $@

$(SLO)$/namespacemap.obj : $(INCCOM)$/namespacenames.inc $(MISC)$/namespaces.txt $(GENHEADERPATH)$/namespaces.hxx $(MISC)$/do_namespaces

$(MISC)$/do_namespaces $(do_phony) : namespaces.txt namespaces.pl namespaces.hxx.head namespaces.hxx.tail $(INCCOM)$/namespacenames.inc $(MISC)$/namespaces.txt $(GENHEADERPATH)$/namespaces.hxx
    @@-$(RM) $@
    $(MKDIRHIER) $(GENHEADERPATH)
    $(PERL) namespaces.pl namespaces.txt $(MISC)$/namespaceids.inc $(INCCOM)$/namespacenames.inc $(MISC)$/namespaces.txt && $(TYPE) namespaces.hxx.head $(MISC)$/namespaceids.inc namespaces.hxx.tail > $(GENHEADERPATH)$/namespaces.hxx && $(TOUCH) $@

$(SLO)$/propertynames.obj : $(INCCOM)$/propertynames.inc $(GENHEADERPATH)$/properties.hxx $(MISC)$/do_properties

$(MISC)$/do_properties $(do_phony) : properties.txt properties.pl properties.hxx.head properties.hxx.tail $(INCCOM)$/propertynames.inc $(GENHEADERPATH)$/properties.hxx
    @@-$(RM) $@
    $(MKDIRHIER) $(GENHEADERPATH)
    $(PERL) properties.pl properties.txt $(MISC)$/propertyids.inc $(INCCOM)$/propertynames.inc && $(TYPE) properties.hxx.head $(MISC)$/propertyids.inc properties.hxx.tail > $(GENHEADERPATH)$/properties.hxx && $(TOUCH) $@
