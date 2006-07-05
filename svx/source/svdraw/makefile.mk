#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.18 $
#
#   last change: $Author: kz $ $Date: 2006-07-05 22:07:15 $
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

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES=\
         $(SLO)$/svdobj.obj          \
         $(SLO)$/svdoimp.obj         \
         $(SLO)$/svdoattr.obj        \
         $(SLO)$/svdotext.obj        \
         $(SLO)$/svdotxat.obj        \
         $(SLO)$/svdotxdr.obj        \
         $(SLO)$/svdotxed.obj        \
         $(SLO)$/svdotxfl.obj        \
         $(SLO)$/svdotxln.obj        \
         $(SLO)$/svdotxtr.obj        \
         $(SLO)$/svdorect.obj        \
         $(SLO)$/svdoedge.obj        \
         $(SLO)$/svdomeas.obj        \
         $(SLO)$/svdopath.obj        \
         $(SLO)$/svdocapt.obj        \
         $(SLO)$/svdocirc.obj        \
         $(SLO)$/svdograf.obj        \
         $(SLO)$/svdogrp.obj         \
         $(SLO)$/svdopage.obj        \
         $(SLO)$/svdouno.obj         \
         $(SLO)$/svdoutl.obj         \
         $(SLO)$/svdovirt.obj        \
         $(SLO)$/svdoutlinercache.obj \
         $(SLO)$/dialdll.obj         \
         $(SLO)$/gradtrns.obj         \
         $(SLO)$/svdattr.obj         \
         $(SLO)$/svdcrtmt.obj        \
         $(SLO)$/svdcrtv.obj         \
         $(SLO)$/svddrag.obj         \
         $(SLO)$/svddrgmt.obj        \
         $(SLO)$/svddrgv.obj         \
         $(SLO)$/svdedtv.obj         \
         $(SLO)$/svdedtv1.obj        \
         $(SLO)$/svdedtv2.obj        \
         $(SLO)$/svdedxv.obj         \
         $(SLO)$/svdetc.obj          \
         $(SLO)$/svdfmtf.obj		\
         $(SLO)$/svdfppt.obj         \
         $(SLO)$/svdglev.obj         \
         $(SLO)$/svdglue.obj         \
         $(SLO)$/svdoole2.obj        \
         $(SLO)$/svdhdl.obj          \
         $(SLO)$/svdhlpln.obj        \
         $(SLO)$/svdibrow.obj        \
         $(SLO)$/svdio.obj           \
         $(SLO)$/svditer.obj         \
         $(SLO)$/svdlayer.obj        \
         $(SLO)$/svdmark.obj         \
         $(SLO)$/svdmodel.obj        \
         $(SLO)$/svdmrkv.obj         \
         $(SLO)$/svdmrkv1.obj        \
         $(SLO)$/impgrfll.obj        \
         $(SLO)$/svdpage.obj         \
         $(SLO)$/sdrmasterpagedescriptor.obj         \
         $(SLO)$/svdpagv.obj         \
         $(SLO)$/svdpntv.obj         \
         $(SLO)$/svdpoev.obj         \
         $(SLO)$/svdscrol.obj        \
         $(SLO)$/svdsnpv.obj         \
         $(SLO)$/svdsuro.obj         \
         $(SLO)$/svdtouch.obj        \
         $(SLO)$/svdtrans.obj        \
         $(SLO)$/svdtxhdl.obj        \
         $(SLO)$/svdundo.obj         \
         $(SLO)$/svdview.obj         \
         $(SLO)$/svdviter.obj        \
         $(SLO)$/svdvmark.obj        \
         $(SLO)$/svdxcgv.obj		 \
         $(SLO)$/clonelist.obj		 \
         $(SLO)$/svdoashp.obj		 \
         $(SLO)$/svdomedia.obj		 

EXCEPTIONSFILES= \
         $(SLO)$/svdoole2.obj        \
         $(SLO)$/svdhdl.obj          \
         $(SLO)$/svdmrkv.obj         \
         $(SLO)$/svdogrp.obj         \
         $(SLO)$/svdotxln.obj        \
         $(SLO)$/svdouno.obj         \
         $(SLO)$/svdfppt.obj         \
         $(SLO)$/svdpagv.obj		 \
         $(SLO)$/svdograf.obj		\
         $(SLO)$/svdcrtv.obj         \
         $(SLO)$/svdedtv1.obj        \
         $(SLO)$/svdobj.obj          \
         $(SLO)$/svdoimp.obj         \
         $(SLO)$/svdpage.obj         \
         $(SLO)$/svdxcgv.obj		 \
         $(SLO)$/svdoashp.obj        

SRS1NAME=svdstr
SRC1FILES= svdstr.src

.INCLUDE :  target.mk
