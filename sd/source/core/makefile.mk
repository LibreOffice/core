#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-19 00:16:46 $
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

PROJECTPCH=sd
PROJECTPCHSOURCE=$(PRJ)$/util$/sd
PRJNAME=sd
TARGET=core

AUTOSEG=true

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  sv.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

IENV+=-I..\ui\inc

# --- Files --------------------------------------------------------


SRCFILES = glob.src

SLOFILES = $(SLO)$/stlsheet.obj  \
           $(SLO)$/stlpool.obj	\
           $(SLO)$/drawdoc.obj \
           $(SLO)$/drawdoc2.obj \
           $(SLO)$/drawdoc3.obj \
           $(SLO)$/drawdoc4.obj \
           $(SLO)$/sdpage.obj \
           $(SLO)$/sdattr.obj \
           $(SLO)$/sdobjfac.obj \
           $(SLO)$/anminfo.obj	\
           $(SLO)$/sdiocmpt.obj	\
           $(SLO)$/sdpage2.obj	\
           $(SLO)$/typemap.obj	\
           $(SLO)$/pglink.obj   \
           $(SLO)$/tbxitem.obj \
           $(SLO)$/cusshow.obj

EXCEPTIONSFILES= \
           $(SLO)$/drawdoc.obj \
           $(OBJ)$/sxl_drawdoc.obj

SVXLIGHTOBJFILES = \
           $(OBJ)$/stlsheet.obj  \
           $(OBJ)$/stlpool.obj	\
           $(OBJ)$/drawdoc.obj \
           $(OBJ)$/drawdoc2.obj \
           $(OBJ)$/drawdoc3.obj \
           $(OBJ)$/drawdoc4.obj \
           $(OBJ)$/sdpage.obj \
           $(OBJ)$/sdattr.obj \
           $(OBJ)$/sdobjfac.obj \
           $(OBJ)$/anminfo.obj	\
           $(OBJ)$/sdiocmpt.obj	\
           $(OBJ)$/sdpage2.obj	\
           $(OBJ)$/typemap.obj	\
           $(OBJ)$/cusshow.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

