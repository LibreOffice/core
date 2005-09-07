#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 17:53:47 $
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



