#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.27 $
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

PRJ=..$/..

PRJNAME=svx
TARGET=svdraw
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

LIB1TARGET= $(SLB)$/$(TARGET)-core.lib
LIB1OBJFILES= \
         $(SLO)$/svdxcgv.obj		 \
         $(SLO)$/svdmodel.obj		\
         $(SLO)$/svdpage.obj         \
         $(SLO)$/svdobj.obj          \
         $(SLO)$/svdedtv1.obj        \
         $(SLO)$/svdcrtv.obj         \
         $(SLO)$/svdograf.obj        \
         $(SLO)$/svdoole2.obj        \
         $(SLO)$/svdhdl.obj          \
         $(SLO)$/svdmrkv.obj         \
         $(SLO)$/svdogrp.obj         \
         $(SLO)$/svdotxln.obj        \
         $(SLO)$/svdotextdecomposition.obj		\
         $(SLO)$/svdotextpathdecomposition.obj	\
         $(SLO)$/svdouno.obj         \
         $(SLO)$/svdpagv.obj		 \
         $(SLO)$/svddrgmt.obj        \
         $(SLO)$/svdpntv.obj         \
         $(SLO)$/svdsnpv.obj         \
         $(SLO)$/svdview.obj         \
         $(SLO)$/svdtext.obj		\
         $(SLO)$/svdoashp.obj		 \
         $(SLO)$/svdoattr.obj        \
         $(SLO)$/svdotext.obj        \
         $(SLO)$/svdotxat.obj        \
         $(SLO)$/svdotxdr.obj        \
         $(SLO)$/svdotxed.obj        \
         $(SLO)$/svdotxfl.obj        \
         $(SLO)$/svdotxtr.obj        \
         $(SLO)$/svdorect.obj        \
         $(SLO)$/svdoedge.obj        \
         $(SLO)$/svdomeas.obj        \
         $(SLO)$/svdopath.obj        \
         $(SLO)$/svdocapt.obj        \
         $(SLO)$/svdocirc.obj        \
         $(SLO)$/svdopage.obj        \
         $(SLO)$/svdoutl.obj         \
         $(SLO)$/svdovirt.obj        \
         $(SLO)$/svdoutlinercache.obj \
         $(SLO)$/gradtrns.obj         \
         $(SLO)$/svdattr.obj         \
         $(SLO)$/svddrag.obj         \
         $(SLO)$/svddrgv.obj         \
         $(SLO)$/svdedtv2.obj        \
         $(SLO)$/svdedxv.obj         \
         $(SLO)$/svdetc.obj          \
         $(SLO)$/sdrhittesthelper.obj          \
         $(SLO)$/svdfmtf.obj		\
         $(SLO)$/svdglev.obj         \
         $(SLO)$/svdglue.obj         \
         $(SLO)$/svdhlpln.obj        \
         $(SLO)$/svditer.obj         \
         $(SLO)$/svdlayer.obj        \
         $(SLO)$/svdmark.obj         \
         $(SLO)$/svdmrkv1.obj        \
         $(SLO)$/sdrcomment.obj         \
         $(SLO)$/sdrmasterpagedescriptor.obj         \
         $(SLO)$/sdrpagewindow.obj         \
         $(SLO)$/sdrpaintwindow.obj         \
         $(SLO)$/svdpoev.obj         \
         $(SLO)$/svdtrans.obj        \
         $(SLO)$/svdundo.obj         \
         $(SLO)$/svdviter.obj        \
         $(SLO)$/clonelist.obj		 \
         $(SLO)$/svdedtv.obj         \
         $(SLO)$/selectioncontroller.obj \
         $(SLO)$/polypolygoneditor.obj \
         $(SLO)$/svdibrow.obj        \
         $(SLO)$/svdomedia.obj

LIB2TARGET= $(SLB)$/$(TARGET).lib
LIB2OBJFILES= \
         $(SLO)$/svdoimp.obj         \
         $(SLO)$/svdscrol.obj        \
         $(SLO)$/ActionDescriptionProvider.obj \
         $(SLO)$/impgrfll.obj

SLOFILES = $(LIB1OBJFILES) $(LIB2OBJFILES) 

SRS1NAME=svdstr
SRC1FILES= svdstr.src

.INCLUDE :  target.mk
