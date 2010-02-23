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
TARGET=display_idl


# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=true
PRJINC=$(PRJ)$/source

.INCLUDE :  settings.mk
.INCLUDE : $(PRJ)$/source$/mkinc$/fullcpp.mk


# --- Files --------------------------------------------------------

OBJFILES= \
    $(OBJ)$/hfi_constgroup.obj  \
    $(OBJ)$/hfi_doc.obj         \
    $(OBJ)$/hfi_enum.obj        \
    $(OBJ)$/hfi_globalindex.obj \
    $(OBJ)$/hfi_hierarchy.obj   \
    $(OBJ)$/hfi_interface.obj   \
    $(OBJ)$/hfi_method.obj      \
    $(OBJ)$/hfi_module.obj      \
    $(OBJ)$/hfi_navibar.obj     \
    $(OBJ)$/hfi_property.obj    \
    $(OBJ)$/hfi_service.obj     \
    $(OBJ)$/hfi_singleton.obj   \
    $(OBJ)$/hfi_siservice.obj   \
    $(OBJ)$/hfi_struct.obj      \
    $(OBJ)$/hfi_tag.obj         \
    $(OBJ)$/hfi_typedef.obj     \
    $(OBJ)$/hfi_typetext.obj    \
    $(OBJ)$/hfi_xrefpage.obj    \
    $(OBJ)$/hi_ary.obj        	\
    $(OBJ)$/hi_display.obj      \
    $(OBJ)$/hi_env.obj          \
    $(OBJ)$/hi_factory.obj      \
    $(OBJ)$/hi_linkhelper.obj   \
    $(OBJ)$/hi_main.obj


# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk



