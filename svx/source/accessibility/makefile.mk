#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.24 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 20:23:34 $
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

PRJNAME=svx
TARGET=accessibility
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =	\
    $(SLO)$/charmapacc.obj						\
    $(SLO)$/svxrectctaccessiblecontext.obj		\
    $(SLO)$/GraphCtlAccessibleContext.obj		\
    $(SLO)$/ChildrenManager.obj 				\
    $(SLO)$/ChildrenManagerImpl.obj 			\
    $(SLO)$/DescriptionGenerator.obj 			\
    $(SLO)$/AccessibleContextBase.obj			\
    $(SLO)$/AccessibleComponentBase.obj			\
    $(SLO)$/AccessibleSelectionBase.obj			\
    $(SLO)$/AccessibleShape.obj					\
    $(SLO)$/AccessibleGraphicShape.obj			\
    $(SLO)$/AccessibleOLEShape.obj				\
    $(SLO)$/AccessibleShapeInfo.obj				\
    $(SLO)$/AccessibleShapeTreeInfo.obj			\
    $(SLO)$/AccessibleTextHelper.obj			\
    $(SLO)$/AccessibleEmptyEditSource.obj		\
    $(SLO)$/AccessibleTextEventQueue.obj		\
    $(SLO)$/AccessibleStaticTextBase.obj		\
    $(SLO)$/AccessibleParaManager.obj			\
    $(SLO)$/AccessibleEditableTextPara.obj		\
    $(SLO)$/AccessibleStringWrap.obj			\
    $(SLO)$/AccessibleImageBullet.obj			\
    $(SLO)$/ShapeTypeHandler.obj				\
    $(SLO)$/SvxShapeTypes.obj					\
    $(SLO)$/AccessibleControlShape.obj			\
    $(SLO)$/DGColorNameLookUp.obj				\
    $(SLO)$/AccessibleFrameSelector.obj

SRS2NAME = accessibility
SRC2FILES = accessibility.src

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

