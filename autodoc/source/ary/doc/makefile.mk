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
TARGET=ary_doc



# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=true
PRJINC=$(PRJ)$/source

.INCLUDE :  settings.mk
.INCLUDE : $(PRJ)$/source$/mkinc$/fullcpp.mk



# --- Files --------------------------------------------------------


OBJFILES= \
    $(OBJ)$/d_boolean.obj           \
    $(OBJ)$/d_docu.obj              \
    $(OBJ)$/d_hypertext.obj         \
    $(OBJ)$/d_namedlist.obj         \
    $(OBJ)$/d_node.obj              \
    $(OBJ)$/d_oldcppdocu.obj        \
    $(OBJ)$/d_oldidldocu.obj        \
    $(OBJ)$/d_summary.obj           \
    $(OBJ)$/d_struct.obj            \
    $(OBJ)$/d_unit.obj              \
    $(OBJ)$/dht_component.obj       \
    $(OBJ)$/dht_helper.obj          \
    $(OBJ)$/dht_html.obj            \
    $(OBJ)$/dht_link2ce.obj         \
    $(OBJ)$/dht_link2param.obj      \
    $(OBJ)$/dht_link2unknown.obj    \
    $(OBJ)$/dht_link2url.obj        \
    $(OBJ)$/dht_plain.obj           \
    $(OBJ)$/dht_stream.obj          \
    $(OBJ)$/dht_white.obj



# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
