#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.10 $
#
#   last change: $Author: os $ $Date: 2001-04-05 13:07:25 $
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
        optasian.cxx		\
        optpath.cxx		\
        optdict.cxx		\
        optitems.cxx	\
        optgenrl.cxx	\
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

SRCFILES =  \
        optasian.src	\
        optjsearch.src	\
        optgenrl.src	\
        optdict.src		\
        optsave.src		\
        optpath.src		\
        optlingu.src	\
        optgrid.src		\
        optinet2.src	\
        multipat.src    \
        optextbr.src

SLOFILES=	\
        $(SLO)$/asiancfg.obj	\
        $(SLO)$/optasian.obj	\
        $(SLO)$/optjsearch.obj	\
        $(SLO)$/optpath.obj	\
        $(SLO)$/optdict.obj	\
        $(SLO)$/optitems.obj	\
        $(SLO)$/optgenrl.obj	\
        $(SLO)$/optsave.obj	\
        $(SLO)$/adritem.obj	\
        $(SLO)$/optlingu.obj \
        $(SLO)$/optgrid.obj	\
        $(SLO)$/optinet2.obj	\
        $(SLO)$/multipat.obj    \
        $(SLO)$/multifil.obj    \
        $(SLO)$/optextbr.obj    \
        $(SLO)$/srchcfg.obj

SVXLIGHTOBJFILES= \
        $(OBJ)$/adritem.obj

.IF "$(GUI)"=="WIN"
SLOFILES+=	\
        $(SLO)$/optdll.obj
.ENDIF
.ENDIF

EXCEPTIONSFILES= \
        $(SLO)$/optasian.obj \
        $(SLO)$/optdict.obj \
        $(SLO)$/optlingu.obj \
        $(SLO)$/optsave.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

