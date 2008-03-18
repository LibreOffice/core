#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.22 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 15:55:36 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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
    $(SLO)$/doccomp.obj \
    $(SLO)$/docdraw.obj \
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
    $(SLO)$/tblrwcl.obj

SLOFILES =	$(EXCEPTIONSFILES) \
        $(SLO)$/docbasic.obj \
        $(SLO)$/docbm.obj \
        $(SLO)$/docchart.obj \
        $(SLO)$/doccorr.obj \
        $(SLO)$/docdde.obj \
        $(SLO)$/docdesc.obj \
        $(SLO)$/docedt.obj \
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

