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
PRJ=..$/..$/..

PRJNAME=binfilter
TARGET=config
ENABLE_EXCEPTIONS := TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk
INC+= -I$(PRJ)$/inc$/bf_svtools

# --- Files --------------------------------------------------------

SLOFILES = \
    $(SLO)$/svt_moduleoptions.obj           \
    $(SLO)$/svt_pathoptions.obj             \
    $(SLO)$/svt_saveopt.obj					\
    $(SLO)$/svt_lingucfg.obj                \
    $(SLO)$/svt_securityoptions.obj			\
    $(SLO)$/svt_localisationoptions.obj		\
    $(SLO)$/svt_workingsetoptions.obj		\
    $(SLO)$/svt_viewoptions.obj				\
    $(SLO)$/svt_internaloptions.obj			\
    $(SLO)$/svt_startoptions.obj			\
    $(SLO)$/svt_historyoptions.obj			\
    $(SLO)$/svt_inetoptions.obj				\
    $(SLO)$/svt_menuoptions.obj				\
    $(SLO)$/svt_dynamicmenuoptions.obj		\
    $(SLO)$/svt_options3d.obj				\
    $(SLO)$/svt_fontoptions.obj				\
    $(SLO)$/svt_addxmltostorageoptions.obj	\
    $(SLO)$/svt_defaultoptions.obj			\
    $(SLO)$/svt_searchopt.obj				\
    $(SLO)$/svt_printwarningoptions.obj		\
    $(SLO)$/svt_cacheoptions.obj            \
    $(SLO)$/svt_cmdoptions.obj				\
    $(SLO)$/svt_extendedsecurityoptions.obj \
    $(SLO)$/svt_sourceviewconfig.obj		\
    $(SLO)$/svt_compatibility.obj           \
    $(SLO)$/svt_optionsdlg.obj              \
    $(SLO)$/svt_itemholder1.obj \
    $(SLO)$/svt_options.obj \
    $(SLO)$/svt_misccfg.obj				\
    $(SLO)$/svt_helpopt.obj                 \
    $(SLO)$/svt_printoptions.obj			\
    $(SLO)$/svt_languageoptions.obj     \
    $(SLO)$/svt_ctloptions.obj          \
    $(SLO)$/svt_cjkoptions.obj          \
    $(SLO)$/svt_colorcfg.obj            \
    $(SLO)$/svt_undoopt.obj                 \
    $(SLO)$/svt_useroptions.obj      \
    $(SLO)$/svt_syslocaleoptions.obj \
    $(SLO)$/svt_itemholder2.obj \
    $(SLO)$/svt_miscopt.obj

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

