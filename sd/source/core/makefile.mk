#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.13 $
#
#   last change: $Author: rt $ $Date: 2008-03-12 11:27:04 $
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

PRJ=..$/..

PROJECTPCH=sd
PROJECTPCHSOURCE=$(PRJ)$/util$/sd
PRJNAME=sd
TARGET=core
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

IENV+=-I..\ui\inc

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES = glob.src

SLOFILES = $(SLO)$/stlsheet.obj  \
           $(SLO)$/stlfamily.obj \
           $(SLO)$/stlpool.obj	\
           $(SLO)$/drawdoc.obj \
           $(SLO)$/drawdoc2.obj \
           $(SLO)$/drawdoc3.obj \
           $(SLO)$/drawdoc4.obj \
           $(SLO)$/drawdoc_animations.obj\
           $(SLO)$/sdpage.obj \
           $(SLO)$/sdpage2.obj	\
           $(SLO)$/sdattr.obj \
           $(SLO)$/sdobjfac.obj \
           $(SLO)$/anminfo.obj	\
           $(SLO)$/sdiocmpt.obj	\
           $(SLO)$/typemap.obj	\
           $(SLO)$/pglink.obj   \
           $(SLO)$/cusshow.obj  \
           $(SLO)$/PageListWatcher.obj  \
           $(SLO)$/sdpage_animations.obj\
           $(SLO)$/CustomAnimationPreset.obj\
           $(SLO)$/CustomAnimationEffect.obj\
           $(SLO)$/TransitionPreset.obj\
           $(SLO)$/undoanim.obj\
           $(SLO)$/EffectMigration.obj\
           $(SLO)$/CustomAnimationCloner.obj\
           $(SLO)$/shapelist.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

