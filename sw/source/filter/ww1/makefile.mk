#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 17:14:57 $
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

PRJNAME=sw
TARGET=ww1

PROJECTPCH=filt_pch
PROJECTPCHSOURCE=..\filt_1st\filt_pch

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

.IF "$(mydebug)" != ""
CDEFS=$(CDEFS) -Dmydebug
.ENDIF

# --- Files --------------------------------------------------------

CXXFILES = \
        fltshell.cxx \
        w1par.cxx \
        w1class.cxx \
        w1filter.cxx \
        w1sprm.cxx \


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

