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

PRJNAME=sc
TARGET=data

PROJECTPCH4DLL=TRUE
PROJECTPCH=core_pch
PROJECTPCHSOURCE=..\pch\core_pch

AUTOSEG=true

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =  \
    $(EXCEPTIONSFILES) \
    $(EXCEPTIONSNOOPTFILES) \
    $(SLO)$/attrib.obj \
    $(SLO)$/compressedarray.obj \
    $(SLO)$/docparam.obj \
    $(SLO)$/markarr.obj \
    $(SLO)$/markdata.obj \
    $(SLO)$/pagepar.obj \
    $(SLO)$/sheetevents.obj \
    $(SLO)$/sortparam.obj

EXCEPTIONSFILES= \
    $(SLO)$/attarray.obj \
    $(SLO)$/autonamecache.obj \
    $(SLO)$/bcaslot.obj \
    $(SLO)$/cell2.obj \
    $(SLO)$/clipparam.obj \
    $(SLO)$/column.obj \
    $(SLO)$/dbdocutl.obj \
    $(SLO)$/dociter.obj \
    $(SLO)$/docpool.obj \
    $(SLO)$/documen2.obj \
    $(SLO)$/documen3.obj \
    $(SLO)$/documen5.obj \
    $(SLO)$/documen6.obj \
    $(SLO)$/documen7.obj \
    $(SLO)$/documen8.obj \
    $(SLO)$/documen9.obj \
    $(SLO)$/document.obj \
    $(SLO)$/dpcachetable.obj \
    $(SLO)$/dpdimsave.obj \
    $(SLO)$/dpglobal.obj \
    $(SLO)$/dpgroup.obj \
    $(SLO)$/dpobject.obj \
    $(SLO)$/dpoutput.obj \
    $(SLO)$/dpoutputgeometry.obj \
    $(SLO)$/dpsave.obj \
    $(SLO)$/dpsdbtab.obj \
    $(SLO)$/dpshttab.obj \
    $(SLO)$/dptabdat.obj \
    $(SLO)$/dptablecache.obj \
    $(SLO)$/dptabres.obj \
    $(SLO)$/dptabsrc.obj \
    $(SLO)$/drawpage.obj \
    $(SLO)$/drwlayer.obj \
    $(SLO)$/fillinfo.obj \
    $(SLO)$/funcdesc.obj \
    $(SLO)$/global.obj \
    $(SLO)$/global2.obj \
    $(SLO)$/globalx.obj \
    $(SLO)$/olinetab.obj \
    $(SLO)$/patattr.obj \
    $(SLO)$/pivot2.obj \
    $(SLO)$/poolhelp.obj \
    $(SLO)$/postit.obj \
    $(SLO)$/scdpoutputimpl.obj \
    $(SLO)$/segmenttree.obj \
    $(SLO)$/stlpool.obj \
    $(SLO)$/stlsheet.obj \
    $(SLO)$/subtotalparam.obj \
    $(SLO)$/tabbgcolor.obj \
    $(SLO)$/table1.obj \
    $(SLO)$/table2.obj \
    $(SLO)$/table5.obj \
    $(SLO)$/table6.obj \
    $(SLO)$/tabprotection.obj \
    $(SLO)$/userdat.obj

.IF "$(OS)$(COM)$(CPUNAME)"=="LINUXGCCSPARC"
EXCEPTIONSNOOPTFILES= \
    $(SLO)$/cell.obj \
    $(SLO)$/column2.obj \
    $(SLO)$/column3.obj \
    $(SLO)$/conditio.obj \
    $(SLO)$/documen4.obj \
    $(SLO)$/table3.obj \
    $(SLO)$/table4.obj \
    $(SLO)$/validat.obj

.ELSE
EXCEPTIONSFILES+= \
    $(SLO)$/cell.obj \
    $(SLO)$/column2.obj \
    $(SLO)$/column3.obj \
    $(SLO)$/conditio.obj \
    $(SLO)$/documen4.obj \
    $(SLO)$/table3.obj \
    $(SLO)$/table4.obj \
    $(SLO)$/validat.obj
.ENDIF

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

