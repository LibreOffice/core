#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 16:37:08 $
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
TARGET=ary_cpp



# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=true
PRJINC=$(PRJ)$/source

.INCLUDE :  settings.mk
.INCLUDE : $(PRJ)$/source$/mkinc$/fullcpp.mk


# --- Files --------------------------------------------------------


OBJFILES= \
    $(OBJ)$/c_class.obj		\
    $(OBJ)$/c_define.obj	\
    $(OBJ)$/c_enum.obj		\
    $(OBJ)$/c_enuval.obj	\
    $(OBJ)$/c_funct.obj		\
    $(OBJ)$/c_gate.obj		\
    $(OBJ)$/c_macro.obj	    \
    $(OBJ)$/c_namesp.obj	\
    $(OBJ)$/c_osigna.obj	\
    $(OBJ)$/c_slots.obj		\
    $(OBJ)$/c_tydef.obj		\
    $(OBJ)$/c_vari.obj		\
    $(OBJ)$/ca_type.obj     \
    $(OBJ)$/cdf_gate.obj    \
    $(OBJ)$/cg_file.obj     \
    $(OBJ)$/cg_gate.obj     \
    $(OBJ)$/cg_proj.obj     \
    $(OBJ)$/cpp_defs.obj    \
    $(OBJ)$/cpp_disp.obj    \
    $(OBJ)$/namechai.obj	\
    $(OBJ)$/predecla.obj    \
    $(OBJ)$/prpr.obj        \
    $(OBJ)$/tplparam.obj	\
    $(OBJ)$/usedtype.obj




# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk



