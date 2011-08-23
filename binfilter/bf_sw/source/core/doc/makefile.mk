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
TARGET=sw_doc

NO_HIDS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	$(PRJ)$/inc$/bf_sw$/swpre.mk
.INCLUDE :	settings.mk
.INCLUDE :	$(PRJ)$/inc$/bf_sw$/sw.mk
INC+= -I$(PRJ)$/inc$/bf_sw
.IF "$(GUI)$(COM)" == "WINMSC"
LIBFLAGS=/NOI /NOE /PAGE:512
.ENDIF

# --- Files --------------------------------------------------------

SLOFILES =	\
        $(SLO)$/sw_SwStyleNameMapper.obj \
        $(SLO)$/sw_doc.obj \
        $(SLO)$/sw_docbm.obj \
        $(SLO)$/sw_docchart.obj \
        $(SLO)$/sw_doccorr.obj \
        $(SLO)$/sw_docdde.obj \
        $(SLO)$/sw_docdesc.obj \
        $(SLO)$/sw_docdraw.obj \
        $(SLO)$/sw_docedt.obj \
        $(SLO)$/sw_docfld.obj \
        $(SLO)$/sw_docfly.obj \
        $(SLO)$/sw_docfmt.obj \
        $(SLO)$/sw_docftn.obj \
        $(SLO)$/sw_doclay.obj \
        $(SLO)$/sw_docnew.obj \
        $(SLO)$/sw_docnum.obj \
        $(SLO)$/sw_docredln.obj \
        $(SLO)$/sw_docruby.obj \
        $(SLO)$/sw_docsort.obj \
        $(SLO)$/sw_docstat.obj \
        $(SLO)$/sw_doctxm.obj \
        $(SLO)$/sw_extinput.obj \
        $(SLO)$/sw_fmtcol.obj \
        $(SLO)$/sw_ftnidx.obj \
        $(SLO)$/sw_gctable.obj \
        $(SLO)$/sw_lineinfo.obj \
        $(SLO)$/sw_notxtfrm.obj \
        $(SLO)$/sw_number.obj \
        $(SLO)$/sw_poolfmt.obj \
        $(SLO)$/sw_sortopt.obj \
        $(SLO)$/sw_swserv.obj \
        $(SLO)$/sw_swtable.obj \
        $(SLO)$/sw_tblafmt.obj \
        $(SLO)$/sw_tblrwcl.obj \
        $(SLO)$/sw_visiturl.obj \
        $(SLO)$/sw_htmltbl.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :	target.mk

