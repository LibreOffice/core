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
# $Revision: 1.24 $
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
TARGET=doc

# --- Settings -----------------------------------------------------

.INCLUDE :	$(PRJ)$/inc$/swpre.mk
.INCLUDE :	settings.mk
.INCLUDE :	$(PRJ)$/inc$/sw.mk

# --- Files --------------------------------------------------------

EXCEPTIONSFILES = \
    $(SLO)$/SwStyleNameMapper.obj \
    $(SLO)$/acmplwrd.obj \
    $(SLO)$/dbgoutsw.obj \
    $(SLO)$/doc.obj \
    $(SLO)$/docbm.obj \
    $(SLO)$/doccomp.obj \
    $(SLO)$/docdraw.obj \
    $(SLO)$/docedt.obj \
    $(SLO)$/docfmt.obj \
    $(SLO)$/docglbl.obj \
    $(SLO)$/docglos.obj \
    $(SLO)$/doclay.obj \
    $(SLO)$/docxforms.obj \
    $(SLO)$/docnew.obj \
    $(SLO)$/docnum.obj \
    $(SLO)$/docsort.obj \
    $(SLO)$/doctxm.obj \
    $(SLO)$/number.obj \
    $(SLO)$/swstylemanager.obj \
    $(SLO)$/tblcpy.obj \
    $(SLO)$/tblrwcl.obj \
    $(SLO)$/list.obj

SLOFILES =	$(EXCEPTIONSFILES) \
    $(SLO)$/docbasic.obj \
    $(SLO)$/docchart.obj \
    $(SLO)$/doccorr.obj \
    $(SLO)$/docdde.obj \
    $(SLO)$/docdesc.obj \
    $(SLO)$/docfld.obj \
    $(SLO)$/docfly.obj \
    $(SLO)$/docftn.obj \
    $(SLO)$/docredln.obj \
    $(SLO)$/docruby.obj \
    $(SLO)$/docstat.obj \
    $(SLO)$/extinput.obj \
    $(SLO)$/fmtcol.obj \
    $(SLO)$/ftnidx.obj \
    $(SLO)$/gctable.obj \
    $(SLO)$/lineinfo.obj \
    $(SLO)$/notxtfrm.obj \
    $(SLO)$/poolfmt.obj \
    $(SLO)$/sortopt.obj \
    $(SLO)$/swserv.obj \
    $(SLO)$/tblafmt.obj \
    $(SLO)$/visiturl.obj \
    $(SLO)$/htmltbl.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :	target.mk

