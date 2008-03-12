#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.25 $
#
#   last change: $Author: rt $ $Date: 2008-03-12 09:48:10 $
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
TARGET=svdraw
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES=\
         $(SLO)$/svdxcgv.obj		 \
         $(SLO)$/svdmodel.obj		\
         $(SLO)$/svdpage.obj         \
         $(SLO)$/svdoimp.obj         \
         $(SLO)$/svdobj.obj          \
         $(SLO)$/svdedtv1.obj        \
         $(SLO)$/svdcrtv.obj         \
         $(SLO)$/svdograf.obj        \
         $(SLO)$/svdoole2.obj        \
         $(SLO)$/chartprettypainter.obj \
         $(SLO)$/svdhdl.obj          \
         $(SLO)$/svdmrkv.obj         \
         $(SLO)$/svdogrp.obj         \
         $(SLO)$/svdotxln.obj        \
         $(SLO)$/svdouno.obj         \
         $(SLO)$/svdfppt.obj         \
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
         $(SLO)$/svdfmtf.obj		\
         $(SLO)$/svdglev.obj         \
         $(SLO)$/svdglue.obj         \
         $(SLO)$/svdhlpln.obj        \
         $(SLO)$/svdibrow.obj        \
         $(SLO)$/svditer.obj         \
         $(SLO)$/svdlayer.obj        \
         $(SLO)$/svdmark.obj         \
         $(SLO)$/svdmrkv1.obj        \
         $(SLO)$/impgrfll.obj        \
         $(SLO)$/sdrcomment.obj         \
         $(SLO)$/sdrmasterpagedescriptor.obj         \
         $(SLO)$/sdrpagewindow.obj         \
         $(SLO)$/sdrpaintwindow.obj         \
         $(SLO)$/svdpoev.obj         \
         $(SLO)$/svdscrol.obj        \
         $(SLO)$/svdtouch.obj        \
         $(SLO)$/svdtrans.obj        \
         $(SLO)$/svdtxhdl.obj        \
         $(SLO)$/svdundo.obj         \
         $(SLO)$/svdviter.obj        \
         $(SLO)$/clonelist.obj		 \
         $(SLO)$/ActionDescriptionProvider.obj \
         $(SLO)$/svdedtv.obj         \
         $(SLO)$/selectioncontroller.obj \
         $(SLO)$/polypolygoneditor.obj \
         $(SLO)$/svdomedia.obj

SRS1NAME=svdstr
SRC1FILES= svdstr.src

.INCLUDE :  target.mk
