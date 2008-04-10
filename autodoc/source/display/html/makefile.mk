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
# $Revision: 1.3 $
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
TARGET=display_html


# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=true
PRJINC=$(PRJ)$/source

.INCLUDE :  settings.mk
.INCLUDE : $(PRJ)$/source$/mkinc$/fullcpp.mk


# --- Files --------------------------------------------------------

OBJFILES= \
    $(OBJ)$/aryattrs.obj    \
    $(OBJ)$/cfrstd.obj	    \
    $(OBJ)$/chd_udk2.obj	\
    $(OBJ)$/cre_link.obj	\
    $(OBJ)$/dsply_cl.obj	\
    $(OBJ)$/dsply_da.obj    \
    $(OBJ)$/dsply_op.obj	\
    $(OBJ)$/easywri.obj	    \
    $(OBJ)$/hd_chlst.obj  	\
    $(OBJ)$/hd_docu.obj  	\
    $(OBJ)$/hdimpl.obj  	\
    $(OBJ)$/html_kit.obj  	\
    $(OBJ)$/nav_main.obj  	\
    $(OBJ)$/navibar.obj  	\
    $(OBJ)$/outfile.obj     \
    $(OBJ)$/opageenv.obj    \
    $(OBJ)$/pagemake.obj    \
    $(OBJ)$/pm_aldef.obj    \
    $(OBJ)$/pm_base.obj     \
    $(OBJ)$/pm_class.obj    \
    $(OBJ)$/pm_help.obj     \
    $(OBJ)$/pm_index.obj    \
    $(OBJ)$/pm_namsp.obj    \
    $(OBJ)$/pm_start.obj    \
    $(OBJ)$/protarea.obj


# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk



