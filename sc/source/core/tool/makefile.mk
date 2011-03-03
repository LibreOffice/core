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
TARGET=tool

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
        $(SLO)$/addinhelpid.obj \
        $(SLO)$/detdata.obj  \
        $(SLO)$/filtopt.obj \
        $(SLO)$/hints.obj \
        $(SLO)$/inputopt.obj \
        $(SLO)$/navicfg.obj \
        $(SLO)$/printopt.obj \
        $(SLO)$/rechead.obj  \
        $(SLO)$/refdata.obj \
        $(SLO)$/refreshtimer.obj

EXCEPTIONSFILES= \
        $(SLO)$/addincfg.obj \
        $(SLO)$/addincol.obj \
        $(SLO)$/addinlis.obj \
        $(SLO)$/address.obj \
        $(SLO)$/adiasync.obj \
        $(SLO)$/appoptio.obj \
        $(SLO)$/autoform.obj \
        $(SLO)$/callform.obj \
        $(SLO)$/cellform.obj \
        $(SLO)$/cellkeytranslator.obj \
        $(SLO)$/chartarr.obj \
        $(SLO)$/charthelper.obj \
        $(SLO)$/chartlis.obj \
        $(SLO)$/chartlock.obj \
        $(SLO)$/chartpos.obj \
        $(SLO)$/chgtrack.obj \
        $(SLO)$/chgviset.obj \
        $(SLO)$/collect.obj  \
        $(SLO)$/compiler.obj \
        $(SLO)$/consoli.obj  \
        $(SLO)$/dbcolect.obj \
        $(SLO)$/ddelink.obj \
        $(SLO)$/detfunc.obj \
        $(SLO)$/docoptio.obj \
        $(SLO)$/doubleref.obj \
        $(SLO)$/editutil.obj \
        $(SLO)$/formulaparserpool.obj \
        $(SLO)$/interpr1.obj \
        $(SLO)$/interpr2.obj \
        $(SLO)$/interpr3.obj \
        $(SLO)$/interpr4.obj \
        $(SLO)$/interpr5.obj \
        $(SLO)$/interpr6.obj \
        $(SLO)$/interpr7.obj \
        $(SLO)$/lookupcache.obj \
        $(SLO)$/odffmap.obj \
        $(SLO)$/optutil.obj \
        $(SLO)$/parclass.obj \
        $(SLO)$/prnsave.obj \
        $(SLO)$/progress.obj \
        $(SLO)$/queryparam.obj \
        $(SLO)$/rangelst.obj \
        $(SLO)$/rangenam.obj \
        $(SLO)$/rangeseq.obj \
        $(SLO)$/rangeutl.obj \
        $(SLO)$/reffind.obj \
        $(SLO)$/reftokenhelper.obj \
        $(SLO)$/scmatrix.obj \
        $(SLO)$/refupdat.obj \
        $(SLO)$/simplerangelist.obj \
        $(SLO)$/stringutil.obj \
        $(SLO)$/subtotal.obj \
        $(SLO)$/token.obj \
        $(SLO)$/unitconv.obj \
        $(SLO)$/userlist.obj \
        $(SLO)$/viewopti.obj \
        $(SLO)$/zforauto.obj

# [kh] POWERPC compiler problem
.IF "$(OS)$(COM)$(CPUNAME)"=="LINUXGCCPOWERPC"
NOOPTFILES= \
                $(SLO)$/subtotal.obj
.ENDIF

.IF "$(OS)$(COM)$(CPUNAME)"=="LINUXGCCSPARC"
NOOPTFILES= \
        $(SLO)$/interpr2.obj \
        $(SLO)$/interpr4.obj \
        $(SLO)$/token.obj    \
        $(SLO)$/chartarr.obj
.ENDIF

.IF "$(GUI)"=="OS2"
NOOPTFILES= \
        $(SLO)$/interpr6.obj
.ENDIF

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

# avoid quotung problems
$(INCCOM)$/osversiondef.hxx :
    @@-$(RM) $@
    @$(TYPE) $(mktmp #define SC_INFO_OSVERSION "$(OS)") > $@

$(SLO)$/interpr5.obj : $(INCCOM)$/osversiondef.hxx

