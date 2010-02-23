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

PRJNAME=autodoc
TARGET=ary_cpp



# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=true
PRJINC=$(PRJ)$/source

.INCLUDE :  settings.mk
.INCLUDE : $(PRJ)$/source$/mkinc$/fullcpp.mk


# --- Files --------------------------------------------------------


OBJFILES= \
    $(OBJ)$/c_builtintype.obj   \
    $(OBJ)$/c_class.obj		    \
    $(OBJ)$/c_de.obj	        \
    $(OBJ)$/c_define.obj	    \
    $(OBJ)$/c_enum.obj		    \
    $(OBJ)$/c_enuval.obj	    \
    $(OBJ)$/c_funct.obj		    \
    $(OBJ)$/c_macro.obj	        \
    $(OBJ)$/c_namesp.obj	    \
    $(OBJ)$/c_osigna.obj	    \
    $(OBJ)$/c_reposypart.obj	\
    $(OBJ)$/c_slots.obj		    \
    $(OBJ)$/c_traits.obj        \
    $(OBJ)$/c_tydef.obj		    \
    $(OBJ)$/c_vari.obj		    \
    $(OBJ)$/ca_ce.obj           \
    $(OBJ)$/ca_def.obj          \
    $(OBJ)$/ca_type.obj         \
    $(OBJ)$/cs_ce.obj           \
    $(OBJ)$/cs_def.obj          \
    $(OBJ)$/cs_type.obj         \
    $(OBJ)$/namechain.obj	    \
    $(OBJ)$/tplparam.obj	    \
    $(OBJ)$/usedtype.obj



# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk



