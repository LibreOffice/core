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
# $Revision: 1.32 $
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

PRJNAME=vcl
TARGET=gdi

.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.INCLUDE :  $(PRJ)$/util$/makefile2.pmk

.IF "$(COM)"=="ICC"
CDEFS+=-D_STD_NO_NAMESPACE -D_VOS_NO_NAMESPACE -D_UNO_NO_NAMESPACE
.ENDIF

# --- Files --------------------------------------------------------

SLOFILES=	$(SLO)$/salmisc.obj 	\
            $(SLO)$/animate.obj 	\
            $(SLO)$/impanmvw.obj	\
            $(SLO)$/bitmap.obj		\
            $(SLO)$/bitmap2.obj 	\
            $(SLO)$/bitmap3.obj 	\
            $(SLO)$/bitmap4.obj 	\
            $(SLO)$/alpha.obj		\
            $(SLO)$/bitmapex.obj	\
            $(SLO)$/imgcons.obj 	\
            $(SLO)$/bmpacc.obj		\
            $(SLO)$/bmpacc2.obj 	\
            $(SLO)$/bmpacc3.obj 	\
            $(SLO)$/bmpfast.obj	\
            $(SLO)$/cvtsvm.obj		\
            $(SLO)$/cvtgrf.obj		\
            $(SLO)$/font.obj		\
            $(SLO)$/gdimtf.obj		\
            $(SLO)$/gfxlink.obj 	\
            $(SLO)$/gradient.obj	\
            $(SLO)$/hatch.obj		\
            $(SLO)$/graph.obj		\
            $(SLO)$/image.obj		\
            $(SLO)$/impimage.obj		\
            $(SLO)$/impbmp.obj		\
            $(SLO)$/impgraph.obj	\
            $(SLO)$/impimagetree.obj \
            $(SLO)$/imagerepository.obj   \
            $(SLO)$/impprn.obj		\
            $(SLO)$/impvect.obj 	\
            $(SLO)$/implncvt.obj	\
            $(SLO)$/jobset.obj		\
            $(SLO)$/lineinfo.obj	\
            $(SLO)$/mapmod.obj		\
            $(SLO)$/metaact.obj 	\
            $(SLO)$/metric.obj		\
            $(SLO)$/octree.obj		\
            $(SLO)$/outmap.obj		\
            $(SLO)$/outdev.obj		\
            $(SLO)$/outdev2.obj 	\
            $(SLO)$/outdev3.obj 	\
            $(SLO)$/outdev4.obj 	\
            $(SLO)$/outdev5.obj 	\
            $(SLO)$/outdev6.obj 	\
            $(SLO)$/virdev.obj      \
            $(SLO)$/fontcvt.obj		\
            $(SLO)$/print.obj		\
            $(SLO)$/print2.obj		\
            $(SLO)$/regband.obj 	\
            $(SLO)$/region.obj		\
            $(SLO)$/wall.obj		\
            $(SLO)$/opengl.obj		\
            $(SLO)$/fontcfg.obj		\
            $(SLO)$/base14.obj		\
            $(SLO)$/pdfwriter.obj	\
            $(SLO)$/pdfwriter_impl.obj	\
            $(SLO)$/pdffontcache.obj\
            $(SLO)$/sallayout.obj		\
            $(SLO)$/salgdilayout.obj	\
            $(SLO)$/extoutdevdata.obj	\
            $(SLO)$/pdfextoutdevdata.obj	\
            $(SLO)$/salnativewidgets-none.obj	\
            $(SLO)$/bmpconv.obj		\
            $(SLO)$/pngread.obj		\
            $(SLO)$/pngwrite.obj	\
            $(SLO)$/graphictools.obj

EXCEPTIONSFILES=	$(SLO)$/outdev.obj		\
                    $(SLO)$/outdev3.obj 	\
                    $(SLO)$/gfxlink.obj		\
                    $(SLO)$/print.obj		\
                    $(SLO)$/print2.obj		\
                    $(SLO)$/sallayout.obj		\
                    $(SLO)$/image.obj		\
                    $(SLO)$/impimage.obj		\
                    $(SLO)$/impgraph.obj	\
                    $(SLO)$/metric.obj		\
                    $(SLO)$/pdfwriter_impl.obj	\
                    $(SLO)$/pdffontcache.obj\
                    $(SLO)$/fontcfg.obj		\
                    $(SLO)$/bmpconv.obj		\
                    $(SLO)$/pdfextoutdevdata.obj	\
                    $(SLO)$/fontcvt.obj		\
                    $(SLO)$/jobset.obj		\
                    $(SLO)$/impimagetree.obj		\
                    $(SLO)$/pngread.obj		\
                    $(SLO)$/pngwrite.obj    \
                    $(SLO)$/virdev.obj \
                    $(SLO)$/impprn.obj \
                    $(SLO)$/graphictools.obj


# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
