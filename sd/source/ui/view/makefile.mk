#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.31 $
#
#   last change: $Author: kz $ $Date: 2008-04-03 15:19:31 $
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
        $(SLO)$/showview.obj \
        $(SLO)$/bmcache.obj	\
        $(SLO)$/drbezob.obj	\
        $(SLO)$/drtxtob.obj	\
        $(SLO)$/drtxtob1.obj	\
        $(SLO)$/sdruler.obj	\
        $(SLO)$/frmview.obj	\
        $(SLO)$/clview.obj  \
        $(SLO)$/zoomlist.obj	\
        $(SLO)$/unmodpg.obj	\
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
        $(SLO)$/PrintManager.obj				\
        $(SLO)$/ToolBarManager.obj				\
        $(SLO)$/UpdateLockManager.obj			\
        $(SLO)$/ViewClipboard.obj				\
        $(SLO)$/ViewShellBase.obj				\
        $(SLO)$/ViewShellImplementation.obj		\
        $(SLO)$/ViewShellManager.obj			\
        $(SLO)$/ViewShellHint.obj				\
        $(SLO)$/ViewTabBar.obj					\
        $(SLO)$/WindowUpdater.obj



# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

