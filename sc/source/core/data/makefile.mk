#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.23 $
#
#   last change: $Author: kz $ $Date: 2008-03-07 11:13:34 $
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

SLOFILES =  \
    $(SLO)$/attarray.obj \
    $(SLO)$/attrib.obj \
    $(SLO)$/autonamecache.obj \
    $(SLO)$/bcaslot.obj \
    $(SLO)$/cell.obj \
    $(SLO)$/cell2.obj \
    $(SLO)$/column.obj \
    $(SLO)$/column2.obj \
    $(SLO)$/column3.obj \
    $(SLO)$/compressedarray.obj \
    $(SLO)$/conditio.obj \
    $(SLO)$/dbdocutl.obj \
    $(SLO)$/dociter.obj \
    $(SLO)$/docpool.obj \
    $(SLO)$/documen2.obj \
    $(SLO)$/documen3.obj \
    $(SLO)$/documen4.obj \
    $(SLO)$/documen5.obj \
    $(SLO)$/documen6.obj \
    $(SLO)$/documen7.obj \
    $(SLO)$/documen8.obj \
    $(SLO)$/documen9.obj \
    $(SLO)$/document.obj \
        $(SLO)$/dpdimsave.obj \
        $(SLO)$/dpgroup.obj \
    $(SLO)$/dpobject.obj \
    $(SLO)$/dpoutput.obj \
    $(SLO)$/dpsave.obj \
    $(SLO)$/dpsdbtab.obj \
    $(SLO)$/dpshttab.obj \
    $(SLO)$/dptabdat.obj \
    $(SLO)$/dptabres.obj \
    $(SLO)$/dptabsrc.obj \
    $(SLO)$/drawpage.obj \
    $(SLO)$/drwlayer.obj \
    $(SLO)$/fillinfo.obj \
    $(SLO)$/global.obj \
    $(SLO)$/global2.obj \
    $(SLO)$/globalx.obj \
    $(SLO)$/markarr.obj \
    $(SLO)$/markdata.obj \
    $(SLO)$/olinetab.obj \
    $(SLO)$/pagepar.obj \
    $(SLO)$/patattr.obj \
    $(SLO)$/pivot.obj \
    $(SLO)$/pivot2.obj \
    $(SLO)$/poolhelp.obj \
    $(SLO)$/scimpexpmsg.obj \
    $(SLO)$/sortparam.obj \
    $(SLO)$/stlpool.obj \
    $(SLO)$/stlsheet.obj \
    $(SLO)$/table1.obj \
    $(SLO)$/table2.obj \
    $(SLO)$/table3.obj \
    $(SLO)$/table4.obj \
    $(SLO)$/table5.obj \
    $(SLO)$/table6.obj \
    $(SLO)$/userdat.obj \
    $(SLO)$/validat.obj \
    $(SLO)$/postit.obj

EXCEPTIONSFILES= \
    $(SLO)$/autonamecache.obj \
    $(SLO)$/bcaslot.obj \
    $(SLO)$/cell2.obj \
    $(SLO)$/column.obj \
    $(SLO)$/documen2.obj \
    $(SLO)$/document.obj \
    $(SLO)$/dpdimsave.obj \
    $(SLO)$/dpgroup.obj \
    $(SLO)$/dptabres.obj \
    $(SLO)$/table1.obj \
    $(SLO)$/table3.obj \
    $(SLO)$/postit.obj \
    $(SLO)$/documen3.obj \
    $(SLO)$/documen5.obj \
    $(SLO)$/documen6.obj \
    $(SLO)$/documen9.obj \
    $(SLO)$/dpsdbtab.obj \
    $(SLO)$/dpobject.obj \
    $(SLO)$/dpoutput.obj \
    $(SLO)$/dpsave.obj \
    $(SLO)$/dbdocutl.obj \
    $(SLO)$/dptabsrc.obj \
    $(SLO)$/drwlayer.obj \
    $(SLO)$/globalx.obj

.IF "$(OS)$(COM)$(CPUNAME)"=="LINUXGCCSPARC"
NOOPTFILES= \
        $(SLO)$/column2.obj \
        $(SLO)$/column3.obj \
        $(SLO)$/table3.obj \
        $(SLO)$/table4.obj  \
        $(SLO)$/documen4.obj \
        $(SLO)$/conditio.obj \
        $(SLO)$/validat.obj
EXCEPTIONSNOOPTFILES= \
        $(SLO)$/cell.obj
.ELSE
EXCEPTIONSFILES+= \
    $(SLO)$/cell.obj \
    $(SLO)$/global.obj
.ENDIF

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

