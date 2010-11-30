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
        $(SLO)$/fltshell.obj \
        $(SLO)$/w1filter.obj \
        $(SLO)$/w1par.obj \
        $(SLO)$/w1sprm.obj

SLOFILES =  \
        $(EXCEPTIONSFILES) \
        $(SLO)$/w1class.obj


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

