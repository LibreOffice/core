#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: gt $ $Date: 2001-02-14 09:29:47 $
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
TARGET=data

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
    bclist.cxx \
    bcaslot.cxx \
    docpool.cxx \
    poolhelp.cxx \
    column.cxx \
    column2.cxx \
    column3.cxx \
    document.cxx \
    documen2.cxx \
    documen3.cxx \
    documen4.cxx \
    documen5.cxx \
    documen6.cxx \
    documen7.cxx \
    documen8.cxx \
    documen9.cxx \
    fillinfo.cxx \
    table1.cxx \
    table2.cxx \
    table3.cxx \
    table4.cxx \
    table5.cxx \
    table6.cxx \
    olinetab.cxx \
    patattr.cxx \
    cell.cxx \
    cell2.cxx \
    attarray.cxx \
    attrib.cxx \
    global.cxx \
    global2.cxx \
    globalx.cxx \
    markarr.cxx \
    markdata.cxx \
    dociter.cxx \
    drwlayer.cxx \
    userdat.cxx \
    drawpage.cxx \
    stlsheet.cxx \
    stlpool.cxx \
    dptabsrc.cxx \
    dptabres.cxx \
    dptabdat.cxx \
    dpshttab.cxx \
    dpsdbtab.cxx \
    dpoutput.cxx \
    dpobject.cxx \
    dpsave.cxx \
    pivot.cxx \
    pivot2.cxx \
    dbdocutl.cxx \
    pagepar.cxx \
    conditio.cxx \
    validat.cxx \
    scimpexpmsg.cxx

SLOFILES =  \
    $(SLO)$/bclist.obj \
    $(SLO)$/bcaslot.obj \
    $(SLO)$/docpool.obj \
    $(SLO)$/poolhelp.obj \
    $(SLO)$/column.obj \
    $(SLO)$/column2.obj \
    $(SLO)$/column3.obj \
    $(SLO)$/document.obj \
    $(SLO)$/documen2.obj \
    $(SLO)$/documen3.obj \
    $(SLO)$/documen4.obj \
    $(SLO)$/documen5.obj \
    $(SLO)$/documen6.obj \
    $(SLO)$/documen7.obj \
    $(SLO)$/documen8.obj \
    $(SLO)$/documen9.obj \
    $(SLO)$/fillinfo.obj \
    $(SLO)$/table1.obj \
    $(SLO)$/table2.obj \
    $(SLO)$/table3.obj \
    $(SLO)$/table4.obj \
    $(SLO)$/table5.obj \
    $(SLO)$/table6.obj \
    $(SLO)$/olinetab.obj \
    $(SLO)$/patattr.obj \
    $(SLO)$/cell.obj \
    $(SLO)$/cell2.obj \
    $(SLO)$/attarray.obj \
    $(SLO)$/attrib.obj \
    $(SLO)$/global.obj \
    $(SLO)$/global2.obj \
    $(SLO)$/globalx.obj \
    $(SLO)$/markarr.obj \
    $(SLO)$/markdata.obj \
    $(SLO)$/dociter.obj \
    $(SLO)$/drwlayer.obj \
    $(SLO)$/userdat.obj \
    $(SLO)$/drawpage.obj \
    $(SLO)$/stlsheet.obj \
    $(SLO)$/stlpool.obj \
    $(SLO)$/dptabsrc.obj \
    $(SLO)$/dptabres.obj \
    $(SLO)$/dptabdat.obj \
    $(SLO)$/dpshttab.obj \
    $(SLO)$/dpsdbtab.obj \
    $(SLO)$/dpoutput.obj \
    $(SLO)$/dpobject.obj \
    $(SLO)$/dpsave.obj \
    $(SLO)$/pivot.obj \
    $(SLO)$/pivot2.obj \
    $(SLO)$/dbdocutl.obj \
    $(SLO)$/pagepar.obj \
    $(SLO)$/conditio.obj \
    $(SLO)$/validat.obj \
    $(SLO)$/scimpexpmsg.obj

EXCEPTIONSFILES= \
    $(SLO)$/documen6.obj \
    $(SLO)$/dpsdbtab.obj \
    $(SLO)$/dpobject.obj \
    $(SLO)$/dpoutput.obj \
    $(SLO)$/dpsave.obj	\
    $(SLO)$/dbdocutl.obj \
        $(SLO)$/dptabsrc.obj \
    $(SLO)$/globalx.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

