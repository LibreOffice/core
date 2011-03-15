#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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
.IF "$(ENABLE_GRAPHITE)" == "TRUE"
CDEFS+=-DENABLE_GRAPHITE
.IF "$(SYSTEM_GRAPHITE)" != "YES"
CDEFS+=-DGR2_STATIC
.ENDIF
.ENDIF

# --- Files --------------------------------------------------------

EXCEPTIONSFILES=	$(SLO)$/salmisc.obj 	\
                    $(SLO)$/animate.obj 	\
                    $(SLO)$/base14.obj		\
                    $(SLO)$/bitmap.obj		\
                    $(SLO)$/bitmap2.obj 	\
                    $(SLO)$/bitmapex.obj	\
                    $(SLO)$/bmpconv.obj		\
                    $(SLO)$/configsettings.obj 	\
                    $(SLO)$/cvtgrf.obj		\
                    $(SLO)$/cvtsvm.obj		\
                    $(SLO)$/gdimtf.obj		\
                    $(SLO)$/gfxlink.obj		\
                    $(SLO)$/graph.obj		\
                    $(SLO)$/graphictools.obj \
                    $(SLO)$/image.obj		\
                    $(SLO)$/imagerepository.obj   \
                    $(SLO)$/impanmvw.obj	\
                    $(SLO)$/impgraph.obj	\
                    $(SLO)$/impimage.obj		\
                    $(SLO)$/impimagetree.obj		\
                    $(SLO)$/impvect.obj 	\
                    $(SLO)$/jobset.obj		\
                    $(SLO)$/lineinfo.obj	\
                    $(SLO)$/metaact.obj 	\
                    $(SLO)$/metric.obj		\
                    $(SLO)$/oldprintadaptor.obj		\
                    $(SLO)$/outdev.obj		\
                    $(SLO)$/outdev2.obj 	\
                    $(SLO)$/outdev3.obj 	\
                    $(SLO)$/outdev4.obj 	\
                    $(SLO)$/outdev5.obj 	\
                    $(SLO)$/outdev6.obj 	\
                    $(SLO)$/outdevnative.obj 	\
                    $(SLO)$/outmap.obj		\
                    $(SLO)$/pdfextoutdevdata.obj	\
                    $(SLO)$/pdffontcache.obj\
                    $(SLO)$/pdfwriter.obj	\
                    $(SLO)$/pdfwriter_impl.obj	\
                    $(SLO)$/pngread.obj		\
                    $(SLO)$/pngwrite.obj    \
                    $(SLO)$/print.obj		\
                    $(SLO)$/print2.obj		\
                    $(SLO)$/print3.obj		\
                    $(SLO)$/salgdilayout.obj	\
                    $(SLO)$/sallayout.obj		\
                    $(SLO)$/salnativewidgets-none.obj	\
                    $(SLO)$/textlayout.obj   \
                    $(SLO)$/virdev.obj \
                    $(SLO)$/wall.obj

SLOFILES=	$(EXCEPTIONSFILES)      \
            $(SLO)$/bitmap3.obj 	\
            $(SLO)$/bitmap4.obj 	\
            $(SLO)$/alpha.obj		\
            $(SLO)$/bmpacc.obj		\
            $(SLO)$/bmpacc2.obj 	\
            $(SLO)$/bmpacc3.obj 	\
            $(SLO)$/bmpfast.obj	\
            $(SLO)$/font.obj		\
            $(SLO)$/gradient.obj	\
            $(SLO)$/hatch.obj		\
            $(SLO)$/impbmp.obj		\
            $(SLO)$/mapmod.obj		\
            $(SLO)$/octree.obj		\
            $(SLO)$/regband.obj 	\
            $(SLO)$/region.obj		\
            $(SLO)$/extoutdevdata.obj


# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
