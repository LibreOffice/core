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
# $Revision: 1.5 $
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
TARGET=parser2_s2_luidl


# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=true
PRJINC=$(PRJ)$/source

.INCLUDE :  settings.mk
.INCLUDE : $(PRJ)$/source$/mkinc$/fullcpp.mk



# --- Files --------------------------------------------------------


OBJFILES= \
    $(OBJ)$/cx_idlco.obj		\
    $(OBJ)$/cx_sub.obj		    \
    $(OBJ)$/distrib.obj		    \
    $(OBJ)$/parsenv2.obj		\
    $(OBJ)$/pe_attri.obj	    \
    $(OBJ)$/pe_const.obj	    \
    $(OBJ)$/pe_enum2.obj    	\
    $(OBJ)$/pe_evalu.obj	    \
    $(OBJ)$/pe_excp.obj		    \
    $(OBJ)$/pe_file2.obj	    \
    $(OBJ)$/pe_func2.obj	    \
    $(OBJ)$/pe_iface.obj	    \
    $(OBJ)$/pe_property.obj	    \
    $(OBJ)$/pe_selem.obj	    \
    $(OBJ)$/pe_servi.obj	    \
    $(OBJ)$/pe_singl.obj	    \
    $(OBJ)$/pe_struc.obj 	    \
    $(OBJ)$/pe_tydf2.obj 	    \
    $(OBJ)$/pe_type2.obj 	    \
    $(OBJ)$/pe_vari2.obj 	    \
    $(OBJ)$/pestate.obj 	    \
    $(OBJ)$/semnode.obj		    \
    $(OBJ)$/tk_const.obj		\
    $(OBJ)$/tk_ident.obj		\
    $(OBJ)$/tk_keyw.obj			\
    $(OBJ)$/tk_punct.obj		\
    $(OBJ)$/tkp_uidl.obj		\
    $(OBJ)$/unoidl.obj



# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk



