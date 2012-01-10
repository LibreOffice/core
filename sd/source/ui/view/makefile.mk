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




PRJ=..$/..$/..

PROJECTPCH=sd
PROJECTPCHSOURCE=$(PRJ)$/util$/sd
PRJNAME=sd
TARGET=view

AUTOSEG=true
PRJINC=..$/slidesorter
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES =	\
        DocumentRenderer.src

SLOFILES =	\
        $(SLO)$/sdview.obj	\
        $(SLO)$/sdview2.obj	\
        $(SLO)$/sdview3.obj	\
        $(SLO)$/sdview4.obj	\
        $(SLO)$/sdview5.obj	\
        $(SLO)$/viewshel.obj \
        $(SLO)$/viewshe2.obj \
        $(SLO)$/viewshe3.obj \
        $(SLO)$/sdwindow.obj \
        $(SLO)$/drviewsh.obj \
        $(SLO)$/drviews1.obj \
        $(SLO)$/drviews2.obj \
        $(SLO)$/drviews3.obj \
        $(SLO)$/drviews4.obj \
        $(SLO)$/drviews5.obj \
        $(SLO)$/drviews6.obj \
        $(SLO)$/drviews7.obj \
        $(SLO)$/drviews8.obj \
        $(SLO)$/drviews9.obj \
        $(SLO)$/drviewsa.obj \
        $(SLO)$/drviewsb.obj \
        $(SLO)$/drviewsc.obj \
        $(SLO)$/drviewsd.obj \
        $(SLO)$/drviewse.obj \
        $(SLO)$/drviewsf.obj \
        $(SLO)$/drviewsg.obj \
        $(SLO)$/drviewsi.obj \
        $(SLO)$/drviewsj.obj \
        $(SLO)$/drvwshrg.obj \
        $(SLO)$/drawview.obj \
        $(SLO)$/grviewsh.obj \
        $(SLO)$/outlnvsh.obj \
        $(SLO)$/outlnvs2.obj \
        $(SLO)$/presvish.obj \
        $(SLO)$/outlview.obj \
        $(SLO)$/tabcontr.obj \
        $(SLO)$/bmcache.obj	\
        $(SLO)$/drbezob.obj	\
        $(SLO)$/drtxtob.obj	\
        $(SLO)$/drtxtob1.obj	\
        $(SLO)$/sdruler.obj	\
        $(SLO)$/frmview.obj	\
        $(SLO)$/clview.obj  \
        $(SLO)$/zoomlist.obj	\
        $(SLO)$/unmodpg.obj	\
        $(SLO)$/DocumentRenderer.obj			\
        $(SLO)$/FormShellManager.obj			\
        $(SLO)$/GraphicObjectBar.obj			\
        $(SLO)$/GraphicViewShellBase.obj		\
        $(SLO)$/ImpressViewShellBase.obj		\
        $(SLO)$/MediaObjectBar.obj				\
        $(SLO)$/Outliner.obj					\
        $(SLO)$/OutlinerIterator.obj			\
        $(SLO)$/OutlineViewShellBase.obj		\
        $(SLO)$/SlideSorterViewShellBase.obj	\
        $(SLO)$/PresentationViewShellBase.obj	\
        $(SLO)$/ToolBarManager.obj				\
        $(SLO)$/UpdateLockManager.obj			\
        $(SLO)$/ViewClipboard.obj				\
        $(SLO)$/ViewShellBase.obj				\
        $(SLO)$/ViewShellImplementation.obj		\
        $(SLO)$/ViewShellManager.obj			\
        $(SLO)$/ViewShellHint.obj				\
        $(SLO)$/ViewTabBar.obj					\
        $(SLO)$/WindowUpdater.obj				\
        $(SLO)$/viewoverlaymanager.obj


# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

