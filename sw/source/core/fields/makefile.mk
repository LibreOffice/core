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

PRJ=..$/..$/..

PRJNAME=sw
TARGET=fields

AUTOSEG=true

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

# --- Files --------------------------------------------------------

CXXFILES = \
        authfld.cxx \
        cellfml.cxx \
        chpfld.cxx \
        dbfld.cxx \
        ddefld.cxx \
        ddetbl.cxx \
        docufld.cxx \
        expfld.cxx \
        fldbas.cxx \
        flddat.cxx \
        flddropdown.cxx \
        scrptfld.cxx \
        macrofld.cxx \
        fldlst.cxx \
        postithelper.cxx \
        reffld.cxx \
        tblcalc.cxx \
        usrfld.cxx



SLOFILES =	\
        $(EXCEPTIONSFILES) \
        $(SLO)$/scrptfld.obj

EXCEPTIONSFILES = \
        $(SLO)$/authfld.obj \
        $(SLO)$/cellfml.obj \
        $(SLO)$/chpfld.obj \
        $(SLO)$/dbfld.obj \
        $(SLO)$/ddefld.obj \
        $(SLO)$/ddetbl.obj \
        $(SLO)$/docufld.obj \
        $(SLO)$/expfld.obj \
        $(SLO)$/fldbas.obj \
        $(SLO)$/flddat.obj \
        $(SLO)$/flddropdown.obj \
        $(SLO)$/fldlst.obj \
        $(SLO)$/macrofld.obj \
        $(SLO)$/postithelper.obj \
        $(SLO)$/reffld.obj \
        $(SLO)$/tblcalc.obj \
        $(SLO)$/textapi.obj \
        $(SLO)$/usrfld.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

