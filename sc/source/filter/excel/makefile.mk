#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.20 $
#
#   last change: $Author: hjs $ $Date: 2003-08-19 11:35:43 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
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
        $(SLO)$/xltools.obj					\
        $(SLO)$/xlroot.obj					\
        $(SLO)$/xlstyle.obj					\
        $(SLO)$/xlformula.obj				\
        $(SLO)$/xlocx.obj					\
        $(SLO)$/xltracer.obj				\
        $(SLO)$/xistream.obj				\
        $(SLO)$/xiroot.obj					\
        $(SLO)$/xihelper.obj				\
        $(SLO)$/xilink.obj					\
        $(SLO)$/xicontent.obj				\
        $(SLO)$/xistyle.obj					\
        $(SLO)$/xiescher.obj				\
        $(SLO)$/xestream.obj				\
        $(SLO)$/xerecord.obj				\
        $(SLO)$/xeroot.obj					\
        $(SLO)$/xehelper.obj				\
        $(SLO)$/xelink.obj					\
        $(SLO)$/xecontent.obj				\
        $(SLO)$/xestyle.obj					\
        $(SLO)$/XclChartsApiHelper.obj		\
        $(SLO)$/XclImpCharts.obj			\
        $(SLO)$/XclImpChartsApi.obj			\
        $(SLO)$/XclExpCharts.obj			\
        $(SLO)$/read.obj					\
        $(SLO)$/impop.obj					\
        $(SLO)$/excimp8.obj					\
        $(SLO)$/excel.obj					\
        $(SLO)$/excform.obj					\
        $(SLO)$/excform8.obj				\
        $(SLO)$/fontbuff.obj				\
        $(SLO)$/tokstack.obj				\
        $(SLO)$/namebuff.obj				\
        $(SLO)$/colrowst.obj				\
        $(SLO)$/expop2.obj					\
        $(SLO)$/excrecds.obj				\
        $(SLO)$/excdoc.obj					\
        $(SLO)$/exctools.obj				\
        $(SLO)$/excobj.obj					\
        $(SLO)$/excchart.obj				\
        $(SLO)$/functab.obj					\
        $(SLO)$/frmbase.obj					\
        $(SLO)$/exccomp.obj					\
        $(SLO)$/biffdump.obj

EXCEPTIONSFILES = \
        $(SLO)$/XclImpChartsApi.obj			\
        $(SLO)$/XclExpCharts.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

