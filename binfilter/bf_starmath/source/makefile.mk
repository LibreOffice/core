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

EXTERNAL_WARNINGS_NOT_ERRORS := TRUE

PROJECTPCH4DLL=TRUE

PRJ=..$/..
BFPRJ=..

PRJNAME=binfilter
TARGET=bf_starmath

NO_HIDS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(BFPRJ)$/util$/makefile.pmk
INC+= -I$(PRJ)$/inc$/bf_starmath
# --- Files --------------------------------------------------------

SMDLL=TRUE

SRS2NAME =starmath_smres
SRC2FILES = starmath_smres.src   \
            starmath_symbol.src	\
            starmath_commands.src

SLOFILES =  \
        $(SLO)$/starmath_register.obj \
        $(SLO)$/starmath_symbol.obj \
        $(SLO)$/starmath_cfgitem.obj \
        $(SLO)$/starmath_config.obj \
        $(SLO)$/starmath_document.obj \
        $(SLO)$/starmath_mathml.obj \
        $(SLO)$/starmath_format.obj \
        $(SLO)$/starmath_node.obj \
        $(SLO)$/starmath_parse.obj \
        $(SLO)$/starmath_utility.obj \
        $(SLO)$/starmath_smdll.obj \
        $(SLO)$/starmath_smmod.obj \
        $(SLO)$/starmath_rect.obj \
        $(SLO)$/starmath_unomodel.obj \
        $(SLO)$/starmath_unodoc.obj

EXCEPTIONSFILES =   \
        $(SLO)$/starmath_register.obj  \
        $(SLO)$/starmath_mathml.obj \
        $(SLO)$/starmath_viewhdl.obj \
        $(SLO)$/starmath_unomodel.obj \
        $(SLO)$/starmath_unodoc.obj


LIB2TARGET =    $(SLB)$/bf_ysm.lib
LIB2ARCHIV =    $(LB)$/libbf_ysm.a
LIB2OBJFILES  =    $(SLO)$/starmath_smlib.obj

DEPOBJFILES = $(SLO)$/starmath_smlib.obj

# --- Targets -------------------------------------------------------

.IF "$(GUI)"=="UNX" || "$(GUI)"=="MAC"
CDEFS+=-DUSE_POLYGON
.ENDIF

.INCLUDE :  target.mk

$(SLO)$/starmath_smlib.obj : $(INCCOM)$/dllname.hxx

$(INCCOM)$/dllname.hxx: makefile.mk
.IF "$(GUI)"=="UNX"
    $(RM) $@
    echo \#define DLL_NAME \"$(DLLPRE)bf_sm$(DLLPOSTFIX)$(DLLPOST)\" >$@
.ELSE
    echo $(EMQ)#define DLL_NAME $(EMQ)"$(DLLPRE)bf_sm$(DLLPOSTFIX)$(DLLPOST)$(EMQ)" >$@
.ENDIF

$(SRS)$/starmath_smres.srs: $(PRJ)$/inc$/bf_svx$/globlmn.hrc
