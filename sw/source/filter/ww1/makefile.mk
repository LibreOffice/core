#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: obo $ $Date: 2008-02-26 14:20:02 $
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
TARGET=ww1

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

.IF "$(mydebug)" != ""
CDEFS=$(CDEFS) -Dmydebug
.ENDIF

# --- Files --------------------------------------------------------

EXCEPTIONSFILES=	\
        $(SLO)$/w1filter.obj \
        $(SLO)$/fltshell.obj

SLOFILES =  \
        $(SLO)$/w1par.obj \
        $(SLO)$/w1class.obj \
        $(SLO)$/w1filter.obj \
        $(SLO)$/w1sprm.obj \
        $(SLO)$/fltshell.obj \


# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

run: alltar

#copy all relevant files to a backupdir
bak:
    copy ..\inc\fltshell.hxx backup
    copy *.?xx backup
    copy makefile* backup
    copy vcs.cfg backup
    copy ..\..\..\dump1\src\dump1.cxx backup
    copy ..\..\..\dump1\src\makefile backup\makefile.dmp
    copy ..\..\..\WNTMSCI\bin\makefile backup\makefile.bin
    copy ..\..\..\UTIL\makefile backup\makefile.utl
    copy s:\solenv\inc\wnt.mak backup

# remove this filter from libs to avoid annoying effects
upgrade:
    attrib -r *.?xx
    attrib -r makefile.*
    del ..\..\..\WNMSCI\LIB\ww1.lib
    del ..\..\..\WNMSCI\DBO\w1*.obj
    del ..\..\..\WNMSCI\DBO\fltshell.obj
    del ..\..\..\WNMSCI\MISC
    lib /nologo /REMOVE:..\..\..\WNTMSCI\obj\fltshell.obj /out:..\..\..\WNTMSCI\LIB\filter.lib ..\..\..\WNTMSCI\LIB\filter.lib
    lib /nologo /REMOVE:..\..\..\WNTMSCI\obj\w1sprm.obj /out:..\..\..\WNTMSCI\LIB\filter.lib ..\..\..\WNTMSCI\LIB\filter.lib
    lib /nologo /REMOVE:..\..\..\WNTMSCI\obj\w1filter.obj /out:..\..\..\WNTMSCI\LIB\filter.lib ..\..\..\WNTMSCI\LIB\filter.lib
    lib /nologo /REMOVE:..\..\..\WNTMSCI\obj\w1class.obj /out:..\..\..\WNTMSCI\LIB\filter.lib ..\..\..\WNTMSCI\LIB\filter.lib
    lib /nologo /REMOVE:..\..\..\WNTMSCI\obj\w1par.obj /out:..\..\..\WNTMSCI\LIB\filter.lib ..\..\..\WNTMSCI\LIB\filter.lib
    copy backup\makefile.bin ..\..\..\WNTMSCI\bin\makefile
    diff backup\makefile.utl ..\..\..\UTIL\makefile
    diff s:\solenv\inc\wnt.mak backup

zip: bak
    pkzip c:\temp\ww1 backup\*.*

