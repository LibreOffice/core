#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: nn $ $Date: 2000-11-02 19:09:23 $
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
TARGET=tool

PROJECTPCH4DLL=TRUE
PROJECTPCH=core_pch
PROJECTPCHSOURCE=..\pch\core_pch

AUTOSEG=true

# --- Settings -----------------------------------------------------

.INCLUDE :  scpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sc.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

CXXFILES = \
        adiasync.cxx \
        appoptio.cxx \
        autoform.cxx \
        callform.cxx \
        cellform.cxx \
        chartarr.cxx \
        chartlis.cxx \
        chgtrack.cxx \
        chgviset.cxx \
        collect.cxx  \
        compiler.cxx \
        consoli.cxx  \
        dbcolect.cxx \
        ddelink.cxx \
        detfunc.cxx  \
        detdata.cxx  \
        docoptio.cxx \
        editutil.cxx \
        indexmap.cxx \
        interpr1.cxx \
        interpr2.cxx \
        interpr3.cxx \
        interpr4.cxx \
        interpr5.cxx \
        interpr6.cxx \
        linelink.cxx \
        progress.cxx \
        rangenam.cxx \
        rangelst.cxx \
        rangeutl.cxx \
        rechead.cxx  \
        refupdat.cxx \
        refdata.cxx \
        reffind.cxx \
        scdebug.cxx \
        scmatrix.cxx \
        sctictac.cxx \
        subtotal.cxx \
        token.cxx \
        unitconv.cxx \
        userlist.cxx \
        viewopti.cxx \
        inputopt.cxx \
        filtopt.cxx \
        optutil.cxx \
        zforauto.cxx \
        hints.cxx \
        prnsave.cxx \
        navicfg.cxx \
        addincol.cxx \
        addinlis.cxx \
        rangeseq.cxx


SLOFILES =  \
        $(SLO)$/adiasync.obj \
        $(SLO)$/appoptio.obj \
        $(SLO)$/autoform.obj \
        $(SLO)$/callform.obj \
        $(SLO)$/cellform.obj \
        $(SLO)$/chartarr.obj \
        $(SLO)$/chartlis.obj \
        $(SLO)$/chgtrack.obj \
        $(SLO)$/chgviset.obj \
        $(SLO)$/collect.obj  \
        $(SLO)$/compiler.obj \
        $(SLO)$/consoli.obj  \
        $(SLO)$/dbcolect.obj \
        $(SLO)$/ddelink.obj \
        $(SLO)$/detfunc.obj  \
        $(SLO)$/detdata.obj  \
        $(SLO)$/docoptio.obj \
        $(SLO)$/editutil.obj \
        $(SLO)$/indexmap.obj \
        $(SLO)$/interpr1.obj \
        $(SLO)$/interpr2.obj \
        $(SLO)$/interpr3.obj \
        $(SLO)$/interpr4.obj \
        $(SLO)$/interpr5.obj \
        $(SLO)$/interpr6.obj \
        $(SLO)$/linelink.obj \
        $(SLO)$/progress.obj \
        $(SLO)$/rangenam.obj \
        $(SLO)$/rangelst.obj \
        $(SLO)$/rangeutl.obj \
        $(SLO)$/rechead.obj  \
        $(SLO)$/refupdat.obj \
        $(SLO)$/refdata.obj \
        $(SLO)$/reffind.obj \
        $(SLO)$/scdebug.obj \
        $(SLO)$/scmatrix.obj \
        $(SLO)$/sctictac.obj \
        $(SLO)$/subtotal.obj \
        $(SLO)$/token.obj \
        $(SLO)$/unitconv.obj \
        $(SLO)$/userlist.obj \
        $(SLO)$/viewopti.obj \
        $(SLO)$/inputopt.obj \
        $(SLO)$/filtopt.obj \
        $(SLO)$/optutil.obj \
        $(SLO)$/zforauto.obj \
        $(SLO)$/hints.obj \
        $(SLO)$/prnsave.obj \
        $(SLO)$/navicfg.obj \
        $(SLO)$/addincol.obj \
        $(SLO)$/addinlis.obj \
        $(SLO)$/rangeseq.obj

EXCEPTIONSFILES= \
        $(SLO)$/addincol.obj

.IF "$(GUI)"=="OS2"
NOOPTFILES= \
        $(SLO)$/interpr6.obj
.ENDIF

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

