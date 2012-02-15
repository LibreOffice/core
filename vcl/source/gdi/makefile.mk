#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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
.ENDIF

# --- Files --------------------------------------------------------

EXCEPTIONSFILES=	$(SLO)$/salmisc.obj		\
            $(SLO)$/outdev.obj		\
            $(SLO)$/outdev3.obj		\
            $(SLO)$/outdevnative.obj	\
            $(SLO)$/gfxlink.obj		\
            $(SLO)$/print.obj		\
            $(SLO)$/print2.obj		\
            $(SLO)$/print3.obj		\
            $(SLO)$/oldprintadaptor.obj	\
            $(SLO)$/configsettings.obj 	\
            $(SLO)$/sallayout.obj		\
            $(SLO)$/image.obj		\
            $(SLO)$/impimage.obj		\
            $(SLO)$/impgraph.obj		\
            $(SLO)$/metric.obj		\
            $(SLO)$/pdfwriter_impl.obj	\
            $(SLO)$/pdfwriter_impl2.obj	\
            $(SLO)$/pdffontcache.obj	\
            $(SLO)$/bmpconv.obj		\
            $(SLO)$/pdfextoutdevdata.obj	\
            $(SLO)$/jobset.obj		\
            $(SLO)$/impimagetree.obj	\
            $(SLO)$/pngread.obj		\
            $(SLO)$/pngwrite.obj		\
            $(SLO)$/virdev.obj		\
            $(SLO)$/gdimtf.obj		\
            $(SLO)$/graphictools.obj	\
            $(SLO)$/textlayout.obj		\
            $(SLO)$/lineinfo.obj		\
            $(SLO)$/svgdata.obj

SLOFILES=	$(EXCEPTIONSFILES)		\
            $(SLO)$/animate.obj 	\
            $(SLO)$/impanmvw.obj	\
            $(SLO)$/bitmap.obj	\
            $(SLO)$/bitmap2.obj 	\
            $(SLO)$/bitmap3.obj 	\
            $(SLO)$/bitmap4.obj 	\
            $(SLO)$/alpha.obj	\
            $(SLO)$/bitmapex.obj	\
            $(SLO)$/bmpacc.obj		\
            $(SLO)$/bmpacc2.obj 	\
            $(SLO)$/bmpacc3.obj 	\
            $(SLO)$/bmpfast.obj	\
            $(SLO)$/cvtsvm.obj	\
            $(SLO)$/cvtgrf.obj	\
            $(SLO)$/font.obj	\
            $(SLO)$/gradient.obj	\
            $(SLO)$/hatch.obj	\
            $(SLO)$/graph.obj	\
            $(SLO)$/impbmp.obj	\
            $(SLO)$/imagerepository.obj   \
            $(SLO)$/impvect.obj 	\
            $(SLO)$/mapmod.obj	\
            $(SLO)$/metaact.obj 	\
            $(SLO)$/octree.obj	\
            $(SLO)$/outmap.obj	\
            $(SLO)$/outdev2.obj 	\
            $(SLO)$/outdev4.obj 	\
            $(SLO)$/outdev5.obj 	\
            $(SLO)$/outdev6.obj 	\
            $(SLO)$/regband.obj 	\
            $(SLO)$/region.obj	\
            $(SLO)$/wall.obj	\
            $(SLO)$/base14.obj	\
            $(SLO)$/pdfwriter.obj	\
            $(SLO)$/salgdilayout.obj	\
            $(SLO)$/extoutdevdata.obj	\
            $(SLO)$/salnativewidgets-none.obj 

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
