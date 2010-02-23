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
TARGET=parser_cpp


# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=true
PRJINC=$(PRJ)$/source


.INCLUDE :  settings.mk
.INCLUDE : $(PRJ)$/source$/mkinc$/fullcpp.mk



# --- Files --------------------------------------------------------


OBJFILES= \
    $(OBJ)$/all_toks.obj	\
    $(OBJ)$/c_dealer.obj	\
    $(OBJ)$/c_rcode.obj		\
    $(OBJ)$/cpp_pe.obj		\
    $(OBJ)$/cx_base.obj	    \
    $(OBJ)$/cx_c_pp.obj	    \
    $(OBJ)$/cx_c_std.obj	\
    $(OBJ)$/cx_c_sub.obj	\
    $(OBJ)$/cxt2ary.obj	    \
    $(OBJ)$/defdescr.obj	\
    $(OBJ)$/icprivow.obj	\
    $(OBJ)$/pe_base.obj		\
    $(OBJ)$/pe_class.obj	\
    $(OBJ)$/pe_defs.obj	    \
    $(OBJ)$/pe_expr.obj	    \
    $(OBJ)$/pe_enum.obj	    \
    $(OBJ)$/pe_enval.obj    \
    $(OBJ)$/pe_file.obj		\
    $(OBJ)$/pe_funct.obj	\
    $(OBJ)$/pe_ignor.obj	\
    $(OBJ)$/pe_namsp.obj	\
    $(OBJ)$/pe_param.obj	\
    $(OBJ)$/pe_tpltp.obj    \
    $(OBJ)$/pe_type.obj		\
    $(OBJ)$/pe_tydef.obj	\
    $(OBJ)$/pe_vafu.obj		\
    $(OBJ)$/pe_vari.obj		\
    $(OBJ)$/preproc.obj		\
    $(OBJ)$/prs_cpp.obj		\
    $(OBJ)$/tkp_cpp.obj



# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk



