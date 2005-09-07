#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 18:21:47 $
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



