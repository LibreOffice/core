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
# $Revision: 1.7 $
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
TARGET=ary_idl


# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=true
PRJINC=$(PRJ)$/source

.INCLUDE :  settings.mk
.INCLUDE : $(PRJ)$/source$/mkinc$/fullcpp.mk


# --- Files --------------------------------------------------------


OBJFILES= \
    $(OBJ)$/i_attribute.obj     \
    $(OBJ)$/i_ce.obj            \
    $(OBJ)$/i_ce2s.obj          \
    $(OBJ)$/i_comrela.obj       \
    $(OBJ)$/i_constant.obj      \
    $(OBJ)$/i_constgroup.obj    \
    $(OBJ)$/i_enum.obj          \
    $(OBJ)$/i_enumvalue.obj     \
    $(OBJ)$/i_exception.obj     \
    $(OBJ)$/i_function.obj      \
    $(OBJ)$/i_interface.obj     \
    $(OBJ)$/i_module.obj        \
    $(OBJ)$/i_namelookup.obj    \
    $(OBJ)$/i_param.obj         \
    $(OBJ)$/i_property.obj      \
    $(OBJ)$/i_reposypart.obj    \
    $(OBJ)$/i_service.obj       \
    $(OBJ)$/i_singleton.obj     \
    $(OBJ)$/i_siservice.obj     \
    $(OBJ)$/i_sisingleton.obj   \
    $(OBJ)$/i_strconst.obj      \
    $(OBJ)$/i_struct.obj        \
    $(OBJ)$/i_structelem.obj    \
    $(OBJ)$/i_traits.obj        \
    $(OBJ)$/i_typedef.obj       \
    $(OBJ)$/i2s_calculator.obj  \
    $(OBJ)$/ia_ce.obj           \
    $(OBJ)$/ia_type.obj         \
    $(OBJ)$/is_ce.obj           \
    $(OBJ)$/is_type.obj         \
    $(OBJ)$/it_builtin.obj      \
    $(OBJ)$/it_ce.obj           \
    $(OBJ)$/it_explicit.obj     \
    $(OBJ)$/it_sequence.obj     \
    $(OBJ)$/it_tplparam.obj     \
    $(OBJ)$/it_xnameroom.obj



# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
