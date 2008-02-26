#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.41 $
#
#   last change: $Author: obo $ $Date: 2008-02-26 14:53:02 $
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

PRJNAME=sc
TARGET=excel

AUTOSEG=true

PROJECTPCH4DLL=TRUE
PROJECTPCH=filt_pch
PROJECTPCHSOURCE=..\pch\filt_pch

# --- Settings -----------------------------------------------------

.INCLUDE :  scpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sc.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =	\
        $(SLO)$/biffdump.obj				\
        $(SLO)$/colrowst.obj				\
        $(SLO)$/excdoc.obj					\
        $(SLO)$/excel.obj					\
        $(SLO)$/excform.obj					\
        $(SLO)$/excform8.obj				\
        $(SLO)$/excimp8.obj					\
        $(SLO)$/excobj.obj					\
        $(SLO)$/excrecds.obj				\
        $(SLO)$/exctools.obj				\
        $(SLO)$/expop2.obj					\
        $(SLO)$/fontbuff.obj				\
        $(SLO)$/frmbase.obj					\
        $(SLO)$/impop.obj					\
        $(SLO)$/namebuff.obj				\
        $(SLO)$/read.obj					\
        $(SLO)$/tokstack.obj				\
        $(SLO)$/xechart.obj					\
        $(SLO)$/xecontent.obj				\
        $(SLO)$/xeescher.obj				\
        $(SLO)$/xeformula.obj				\
        $(SLO)$/xehelper.obj				\
        $(SLO)$/xelink.obj					\
        $(SLO)$/xename.obj					\
        $(SLO)$/xepage.obj					\
        $(SLO)$/xepivot.obj					\
        $(SLO)$/xerecord.obj				\
        $(SLO)$/xeroot.obj					\
        $(SLO)$/xestream.obj				\
        $(SLO)$/xestring.obj				\
        $(SLO)$/xestyle.obj					\
        $(SLO)$/xetable.obj					\
        $(SLO)$/xeview.obj					\
        $(SLO)$/xichart.obj					\
        $(SLO)$/xicontent.obj				\
        $(SLO)$/xiescher.obj				\
        $(SLO)$/xiformula.obj				\
        $(SLO)$/xihelper.obj				\
        $(SLO)$/xilink.obj					\
        $(SLO)$/xiname.obj					\
        $(SLO)$/xipage.obj					\
        $(SLO)$/xipivot.obj					\
        $(SLO)$/xiroot.obj					\
        $(SLO)$/xistream.obj				\
        $(SLO)$/xistring.obj				\
        $(SLO)$/xistyle.obj					\
        $(SLO)$/xiview.obj					\
        $(SLO)$/xladdress.obj				\
        $(SLO)$/xlchart.obj					\
        $(SLO)$/xldumper.obj				\
        $(SLO)$/xlescher.obj				\
        $(SLO)$/xlformula.obj				\
        $(SLO)$/xlocx.obj					\
        $(SLO)$/xlpage.obj					\
        $(SLO)$/xlpivot.obj					\
        $(SLO)$/xlroot.obj					\
        $(SLO)$/xlstream.obj				\
        $(SLO)$/xlstyle.obj					\
        $(SLO)$/xltools.obj					\
        $(SLO)$/xltracer.obj				\
        $(SLO)$/xlview.obj

.IF "$(OS)$(COM)$(CPUNAME)"=="LINUXGCCSPARC"
NOOPTFILES = \
        $(SLO)$/xiescher.obj
.ENDIF

EXCEPTIONSFILES = \
        $(SLO)$/excdoc.obj					\
        $(SLO)$/excform.obj					\
        $(SLO)$/excform8.obj				\
        $(SLO)$/excimp8.obj					\
        $(SLO)$/excobj.obj					\
        $(SLO)$/excrecds.obj				\
        $(SLO)$/expop2.obj					\
        $(SLO)$/namebuff.obj				\
        $(SLO)$/xecontent.obj				\
        $(SLO)$/xeescher.obj				\
        $(SLO)$/xeformula.obj				\
        $(SLO)$/xehelper.obj				\
        $(SLO)$/xelink.obj					\
        $(SLO)$/xename.obj					\
        $(SLO)$/xepage.obj					\
        $(SLO)$/xepivot.obj					\
        $(SLO)$/xechart.obj					\
        $(SLO)$/xestring.obj				\
        $(SLO)$/xestyle.obj					\
        $(SLO)$/xetable.obj					\
        $(SLO)$/xeview.obj					\
        $(SLO)$/xichart.obj					\
        $(SLO)$/xicontent.obj				\
        $(SLO)$/xiescher.obj				\
        $(SLO)$/xihelper.obj				\
        $(SLO)$/xilink.obj					\
        $(SLO)$/xipage.obj					\
        $(SLO)$/xipivot.obj					\
        $(SLO)$/xistream.obj				\
        $(SLO)$/xistring.obj				\
        $(SLO)$/xistyle.obj					\
        $(SLO)$/xladdress.obj				\
        $(SLO)$/xiescher.obj				\
        $(SLO)$/xlchart.obj					\
        $(SLO)$/xldumper.obj				\
        $(SLO)$/xlformula.obj				\
        $(SLO)$/xlocx.obj					\
        $(SLO)$/xlpivot.obj					\
        $(SLO)$/xlstyle.obj					\
        $(SLO)$/xlview.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

