#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: cl $ $Date: 2001-03-19 09:52:58 $
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

PROJECTPCH=sd
PROJECTPCHSOURCE=$(PRJ)$/util$/sd
PRJNAME=sd
TARGET=unoidl
ENABLE_EXCEPTIONS=TRUE
AUTOSEG=true

# --- Settings -----------------------------------------------------

.INCLUDE :  sdpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sd.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

CXXFILES = $(PRJ)$/util$/sd.cxx \
        unomodel.cxx    \
        unopage.cxx     \
        unoiview.cxx    \
        unolayer.cxx    \
        unopres.cxx     \
        unocpres.cxx    \
        unoobj.cxx		\
        unosrch.cxx		\
        unowcntr.cxx	\
        unogsfm.cxx		\
        unogstyl.cxx	\
        unopsfm.cxx		\
        unopstyl.cxx	\
        unokywds.cxx	\
        unostyls.cxx	\
        unopback.cxx	\
        unovwcrs.cxx

.IF "$(COM)"=="GCC"
NOOPTFILES= $(SLO)$/unowcntr.obj
.ENDIF

SLOFILES =      \
        $(SLO)$/unomodel.obj    \
        $(SLO)$/unopage.obj     \
        $(SLO)$/unoiview.obj    \
        $(SLO)$/unolayer.obj    \
        $(SLO)$/unopres.obj     \
        $(SLO)$/unocpres.obj    \
        $(SLO)$/unoobj.obj		\
        $(SLO)$/unosrch.obj		\
        $(SLO)$/unowcntr.obj	\
        $(SLO)$/unogsfm.obj		\
        $(SLO)$/unogstyl.obj	\
        $(SLO)$/unopsfm.obj		\
        $(SLO)$/unopstyl.obj	\
        $(SLO)$/unokywds.obj	\
        $(SLO)$/unostyls.obj	\
        $(SLO)$/unopback.obj	\
        $(SLO)$/unovwcrs.obj

SVXLIGHTOBJFILES = \
        $(OBJ)$/unomodel.obj    \
        $(OBJ)$/unopage.obj     \
        $(OBJ)$/unolayer.obj    \
        $(OBJ)$/unopres.obj     \
        $(OBJ)$/unocpres.obj    \
        $(OBJ)$/unoobj.obj		\
        $(OBJ)$/unosrch.obj		\
        $(OBJ)$/unowcntr.obj	\
        $(OBJ)$/unogsfm.obj		\
        $(OBJ)$/unogstyl.obj	\
        $(OBJ)$/unopsfm.obj		\
        $(OBJ)$/unopstyl.obj	\
        $(OBJ)$/unokywds.obj	\
        $(OBJ)$/unostyls.obj	\
        $(OBJ)$/unopback.obj	\
        $(OBJ)$/basemodel.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

