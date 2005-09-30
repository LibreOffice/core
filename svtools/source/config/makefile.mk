#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.55 $
#
#   last change: $Author: hr $ $Date: 2005-09-30 10:14:05 $
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

PRJNAME=svtools
TARGET=config
TARGET1=heavyconfig
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/svl.pmk

# --- Files --------------------------------------------------------

SLOFILES = \
    $(SLO)$/accelcfg.obj				\
    $(SLO)$/xmlaccelcfg.obj				\
    $(SLO)$/moduleoptions.obj           \
    $(SLO)$/pathoptions.obj             \
    $(SLO)$/saveopt.obj					\
    $(SLO)$/helpopt.obj                 \
    $(SLO)$/javaoptions.obj             \
    $(SLO)$/securityoptions.obj			\
    $(SLO)$/localisationoptions.obj		\
    $(SLO)$/workingsetoptions.obj		\
    $(SLO)$/internaloptions.obj			\
    $(SLO)$/startoptions.obj			\
    $(SLO)$/historyoptions.obj			\
    $(SLO)$/inetoptions.obj				\
    $(SLO)$/menuoptions.obj				\
    $(SLO)$/dynamicmenuoptions.obj		\
    $(SLO)$/options3d.obj				\
    $(SLO)$/fontoptions.obj				\
    $(SLO)$/addxmltostorageoptions.obj	\
    $(SLO)$/miscopt.obj					\
    $(SLO)$/defaultoptions.obj			\
    $(SLO)$/searchopt.obj				\
    $(SLO)$/printwarningoptions.obj		\
    $(SLO)$/printoptions.obj			\
    $(SLO)$/cacheoptions.obj            \
    $(SLO)$/regoptions.obj				\
    $(SLO)$/cmdoptions.obj				\
    $(SLO)$/extendedsecurityoptions.obj \
    $(SLO)$/sourceviewconfig.obj		\
    $(SLO)$/compatibility.obj           \
    $(SLO)$/eventcfg.obj    \
    $(SLO)$/accessibilityoptions.obj	\
    $(SLO)$/fontsubstconfig.obj				\
    $(SLO)$/misccfg.obj				\
    $(SLO)$/apearcfg.obj				\
    $(SLO)$/fltrcfg.obj				\
        $(SLO)$/languageoptions.obj     \
        $(SLO)$/ctloptions.obj          \
        $(SLO)$/cjkoptions.obj          \
        $(SLO)$/colorcfg.obj            \
        $(SLO)$/undoopt.obj                 \
        $(SLO)$/useroptions.obj      \
    $(SLO)$/syslocaleoptions.obj \
    $(SLO)$/lingucfg.obj    \
    $(SLO)$/itemholder1.obj    \
    $(SLO)$/itemholder2.obj

EXCEPTIONSFILES = \
    $(SLO)$/accelcfg.obj				\
    $(SLO)$/saveopt.obj					\
    $(SLO)$/lingucfg.obj                \
    $(SLO)$/inetoptions.obj				\
    $(SLO)$/xmlaccelcfg.obj             \
    $(SLO)$/moduleoptions.obj           \
    $(SLO)$/pathoptions.obj         \
    $(SLO)$/eventcfg.obj    \
    $(SLO)$/itemholder1.obj    \
    $(SLO)$/itemholder2.obj	\
    $(SLO)$/viewoptions.obj
    

# --- config.lib Files --------------------------------------------------------
LIB1TARGET= $(SLB)$/$(TARGET).lib

LIB1OBJFILES=  \
    $(SLO)$/accelcfg.obj				\
    $(SLO)$/fltrcfg.obj				\
    $(SLO)$/xmlaccelcfg.obj				\
    $(SLO)$/moduleoptions.obj           \
    $(SLO)$/pathoptions.obj             \
    $(SLO)$/saveopt.obj					\
    $(SLO)$/lingucfg.obj                \
    $(SLO)$/javaoptions.obj             \
    $(SLO)$/securityoptions.obj			\
    $(SLO)$/localisationoptions.obj		\
    $(SLO)$/workingsetoptions.obj		\
    $(SLO)$/viewoptions.obj				\
    $(SLO)$/internaloptions.obj			\
    $(SLO)$/startoptions.obj			\
    $(SLO)$/historyoptions.obj			\
    $(SLO)$/inetoptions.obj				\
    $(SLO)$/menuoptions.obj				\
    $(SLO)$/dynamicmenuoptions.obj		\
    $(SLO)$/options3d.obj				\
    $(SLO)$/fontoptions.obj				\
    $(SLO)$/addxmltostorageoptions.obj	\
    $(SLO)$/miscopt.obj					\
    $(SLO)$/defaultoptions.obj			\
    $(SLO)$/searchopt.obj				\
    $(SLO)$/printwarningoptions.obj		\
    $(SLO)$/cacheoptions.obj            \
    $(SLO)$/regoptions.obj				\
    $(SLO)$/cmdoptions.obj				\
    $(SLO)$/extendedsecurityoptions.obj \
    $(SLO)$/sourceviewconfig.obj		\
    $(SLO)$/compatibility.obj           \
    $(SLO)$/eventcfg.obj                \
    $(SLO)$/itemholder1.obj
    
# --- heavyconfig.lib Files --------------------------------------------------------
LIB2TARGET= $(SLB)$/$(TARGET1).lib

LIB2OBJFILES=  \
    $(SLO)$/accessibilityoptions.obj	\
    $(SLO)$/fontsubstconfig.obj				\
    $(SLO)$/misccfg.obj				\
    $(SLO)$/apearcfg.obj				\
    $(SLO)$/helpopt.obj                 \
    $(SLO)$/printoptions.obj			\
        $(SLO)$/languageoptions.obj     \
        $(SLO)$/ctloptions.obj          \
        $(SLO)$/cjkoptions.obj          \
        $(SLO)$/colorcfg.obj            \
        $(SLO)$/undoopt.obj                 \
        $(SLO)$/useroptions.obj      \
    $(SLO)$/syslocaleoptions.obj \
    $(SLO)$/itemholder2.obj

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

