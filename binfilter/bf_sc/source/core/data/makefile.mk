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

EXTERNAL_WARNINGS_NOT_ERRORS := TRUE

PRJ=..$/..$/..$/..
BFPRJ=..$/..$/..

PRJNAME=binfilter
TARGET=sc_data

NO_HIDS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(BFPRJ)$/util$/makefile.pmk
INC+= -I$(PRJ)$/inc$/bf_sc
# --- Files --------------------------------------------------------

SLOFILES =  \
    $(SLO)$/sc_bclist.obj \
    $(SLO)$/sc_bcaslot.obj \
    $(SLO)$/sc_docpool.obj \
    $(SLO)$/sc_poolhelp.obj \
    $(SLO)$/sc_column.obj \
    $(SLO)$/sc_column2.obj \
    $(SLO)$/sc_column3.obj \
    $(SLO)$/sc_document.obj \
    $(SLO)$/sc_documen2.obj \
    $(SLO)$/sc_documen3.obj \
    $(SLO)$/sc_documen4.obj \
    $(SLO)$/sc_documen5.obj \
    $(SLO)$/sc_documen6.obj \
    $(SLO)$/sc_documen7.obj \
    $(SLO)$/sc_documen8.obj \
    $(SLO)$/sc_documen9.obj \
    $(SLO)$/sc_fillinfo.obj \
    $(SLO)$/sc_table1.obj \
    $(SLO)$/sc_table2.obj \
    $(SLO)$/sc_table3.obj \
    $(SLO)$/sc_table4.obj \
    $(SLO)$/sc_table5.obj \
    $(SLO)$/sc_olinetab.obj \
    $(SLO)$/sc_patattr.obj \
    $(SLO)$/sc_cell.obj \
    $(SLO)$/sc_cell2.obj \
    $(SLO)$/sc_attarray.obj \
    $(SLO)$/sc_attrib.obj \
    $(SLO)$/sc_global.obj \
    $(SLO)$/sc_global2.obj \
    $(SLO)$/sc_globalx.obj \
    $(SLO)$/sc_markarr.obj \
    $(SLO)$/sc_markdata.obj \
    $(SLO)$/sc_dociter.obj \
    $(SLO)$/sc_drwlayer.obj \
    $(SLO)$/sc_userdat.obj \
    $(SLO)$/sc_drawpage.obj \
    $(SLO)$/sc_stlsheet.obj \
    $(SLO)$/sc_stlpool.obj \
    $(SLO)$/sc_dptabsrc.obj \
    $(SLO)$/sc_dptabdat.obj \
    $(SLO)$/sc_dpshttab.obj \
    $(SLO)$/sc_dpoutput.obj \
    $(SLO)$/sc_dpobject.obj \
    $(SLO)$/sc_dpsave.obj \
    $(SLO)$/sc_pivot.obj \
    $(SLO)$/sc_pivot2.obj \
    $(SLO)$/sc_dbdocutl.obj \
    $(SLO)$/sc_pagepar.obj \
    $(SLO)$/sc_conditio.obj \
    $(SLO)$/sc_validat.obj \
    $(SLO)$/sc_sortparam.obj

EXCEPTIONSFILES= \
    $(SLO)$/sc_documen6.obj \
    $(SLO)$/sc_dpobject.obj \
    $(SLO)$/sc_dpoutput.obj \
    $(SLO)$/sc_dpsave.obj	\
    $(SLO)$/sc_dbdocutl.obj \
    $(SLO)$/sc_dptabsrc.obj \
    $(SLO)$/sc_globalx.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

