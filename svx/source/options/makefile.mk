#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.16 $
#
#   last change: $Author: fs $ $Date: 2002-09-03 08:15:46 $
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

PROJECTPCH4DLL=TRUE
PROJECTPCH=svxpch
PROJECTPCHSOURCE=$(PRJ)$/util\svxpch

PRJNAME=svx
TARGET=options
AUTOSEG=true

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

.IF "$(GUI)"=="MAC"
CFLAGS+=-D DG_DLL
.ELSE
CFLAGS+=-DDG_DLL
.ENDIF

# --- Files --------------------------------------------------------

.IF "$(header)" == ""

#IMGLST_SRS=$(SRS)$/options.srs
#BMP_IN=$(PRJ)$/win/res

CXXFILES = \
        asiancfg.cxx		\
        colorcfg.cxx        \
        optasian.cxx		\
        optcolor.cxx        \
        optpath.cxx     \
        optdict.cxx		\
        optitems.cxx	\
        optgenrl.cxx	\
        optaccessibility.cxx \
        optsave.cxx		\
        adritem.cxx		\
        optlingu.cxx    \
        optgrid.cxx		\
        optinet2.cxx	\
        ldapdlg.cxx		\
        multipat.cxx	\
        multifil.cxx	\
        optextbr.cxx    \
        srchcfg.cxx

.IF "$(BUILD_SOSL)" == ""
    CXXFILES += optsso.cxx
.ENDIF

SRCFILES =  \
        optasian.src	\
        optcolor.src    \
        optjsearch.src  \
        optgenrl.src	\
        optdict.src		\
        optaccessibility.src \
        optsave.src		\
        optpath.src		\
        optlingu.src	\
        optgrid.src		\
        optinet2.src	\
        multipat.src    \
        optextbr.src

.IF "$(BUILD_SOSL)" == ""
    SRCFILES += optsso.src
.ENDIF

EXCEPTIONSFILES= \
        $(SLO)$/multifil.obj    \
        $(SLO)$/optasian.obj \
        $(SLO)$/optdict.obj \
        $(SLO)$/optlingu.obj \
        $(SLO)$/optaccessibility.obj \
        $(SLO)$/optsave.obj \
        $(SLO)$/optpath.obj

SLOFILES=	$(EXCEPTIONSFILES) \
        $(SLO)$/asiancfg.obj	\
        $(SLO)$/colorcfg.obj    \
        $(SLO)$/optcolor.obj    \
        $(SLO)$/optjsearch.obj  \
        $(SLO)$/optitems.obj	\
        $(SLO)$/optgenrl.obj	\
        $(SLO)$/adritem.obj	\
        $(SLO)$/optgrid.obj	\
        $(SLO)$/optinet2.obj	\
        $(SLO)$/multipat.obj    \
        $(SLO)$/optextbr.obj    \
        $(SLO)$/srchcfg.obj

.IF "$(BUILD_SOSL)" == ""
    SLOFILES += $(SLO)$/optsso.obj
.ENDIF

SVXLIGHTOBJFILES= \
        $(OBJ)$/adritem.obj

.IF "$(GUI)"=="WIN"
SLOFILES+=	\
        $(SLO)$/optdll.obj
.ENDIF
.ENDIF

.IF "$(BUILD_SOSL)" == ""
    EXCEPTIONSFILES += $(SLO)$/optsso.obj
.ENDIF

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

