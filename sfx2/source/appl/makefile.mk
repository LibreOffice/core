#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: mba $ $Date: 2001-02-23 11:04:54 $
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

PRJ=..$/..

PRJNAME=sfx2
TARGET=appl
ENABLE_EXCEPTIONS=TRUE
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

IMGLST_SRS=$(SRS)$/appl.srs
BMP_IN=$(PRJ)$/win/res

# w.g. compilerbugs
.IF "$(GUI)"=="WNT"
CFLAGS+=-Od
.ENDIF

# --- Files --------------------------------------------------------

SRS1NAME=appl
SRC1FILES =  \
        app.src sfx.src image.src newhelp.src

SRS2NAME=sfx
SRC2FILES =  \
        sfx.src

OBJFILES =	\
        $(OBJ)$/appctor.obj

SLOFILES =  \
        $(SLO)$/rmacceptor.obj\
        $(SLO)$/appuno.obj \
        $(SLO)$/applicat.obj \
        $(SLO)$/appmail.obj \
        $(SLO)$/appmain.obj \
        $(SLO)$/appopen.obj \
        $(SLO)$/appinit.obj \
        $(SLO)$/appmisc.obj \
        $(SLO)$/appdemo.obj \
        $(SLO)$/appreg.obj \
        $(SLO)$/appcfg.obj \
        $(SLO)$/appquit.obj \
        $(SLO)$/appchild.obj \
        $(SLO)$/appserv.obj \
        $(SLO)$/appdata.obj \
        $(SLO)$/app.obj \
        $(SLO)$/appbas.obj \
        $(SLO)$/appdde.obj \
        $(SLO)$/workwin.obj \
        $(SLO)$/sfxhelp.obj \
        $(SLO)$/childwin.obj \
        $(SLO)$/sfxdemo.obj \
        $(SLO)$/sfxdll.obj \
        $(SLO)$/module.obj \
        $(SLO)$/picklist.obj \
        $(SLO)$/exchobj.obj \
        $(SLO)$/appsys.obj \
        $(SLO)$/loadenv.obj \
        $(SLO)$/basmgr.obj \
        $(SLO)$/oinstanceprovider.obj \
        $(SLO)$/opluginframefactory.obj \
        $(SLO)$/pluginacceptthread.obj  \
        $(SLO)$/officeacceptthread.obj \
        $(SLO)$/newhelp.obj \
        $(SLO)$/helpinterceptor.obj

# SCO and MACOSX: the linker does know about weak symbols, but we can't ignore multiple defined symbols
.IF "$(OS)"=="SCO" || "$(OS)$(COM)"=="OS2GCC" || "$(OS)"=="MACOSX"
SLOFILES+=$(SLO)$/staticmbappl.obj
.ENDIF

EXCEPTIONSFILES=\
        $(SLO)$/appmain.obj		\
        $(SLO)$/appmisc.obj		\
        $(SLO)$/frstinit.obj	\
        $(SLO)$/appinit.obj		\
        $(SLO)$/applicat.obj	\
        $(SLO)$/picklist.obj	\
        $(SLO)$/rmacceptor.obj	\
        $(SLO)$/appcfg.obj		\
        $(SLO)$/helpinterceptor.obj \
                $(SLO)$/newhelp.obj

.IF "$(GUI)" == "MAC"
SLOFILES +=\
        $(SLO)$/appctor.obj		\
        $(SLO)$/appmac.obj
.ENDIF

.IF "$(GUI)" == "UNX"
SLOFILES +=\
        $(SLO)$/appctor.obj
.ENDIF

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

