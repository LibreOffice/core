#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.30 $
#
#   last change: $Author: kz $ $Date: 2003-08-25 15:28:09 $
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

.INCLUDE :  settings.mk

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

SLOFILES =  \
    $(SLO)$/imagemgr.obj\
    $(SLO)$/appuno.obj \
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
    $(SLO)$/sfxdll.obj \
    $(SLO)$/module.obj \
    $(SLO)$/appsys.obj \
    $(SLO)$/loadenv.obj \
    $(SLO)$/dlgcont.obj \
    $(SLO)$/namecont.obj \
    $(SLO)$/scriptcont.obj \
    $(SLO)$/newhelp.obj \
    $(SLO)$/helpinterceptor.obj \
    $(SLO)$/shutdownicon.obj \
    $(SLO)$/shutdowniconw32.obj \
    $(SLO)$/sfxpicklist.obj \
    $(SLO)$/helpdispatch.obj \
    $(SLO)$/imestatuswindow.obj \
    $(SLO)$/accelinfo.obj

EXCEPTIONSFILES=\
    $(SLO)$/imagemgr.obj		\
    $(SLO)$/appopen.obj \
    $(SLO)$/appmain.obj			\
    $(SLO)$/appmisc.obj			\
    $(SLO)$/frstinit.obj		\
    $(SLO)$/appinit.obj			\
    $(SLO)$/appcfg.obj			\
    $(SLO)$/helpinterceptor.obj	\
    $(SLO)$/newhelp.obj			\
    $(SLO)$/sfxhelp.obj			\
    $(SLO)$/shutdownicon.obj	\
    $(SLO)$/shutdowniconw32.obj \
    $(SLO)$/sfxpicklist.obj		\
    $(SLO)$/helpdispatch.obj

.IF "$(GUI)" == "MAC"
SLOFILES +=\
        $(SLO)$/appmac.obj
.ENDIF

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

