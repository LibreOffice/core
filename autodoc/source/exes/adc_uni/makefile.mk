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
TARGET=autodoc
TARGETTYPE=CUI

# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=true
PRJINC=$(PRJ)$/source

.INCLUDE :  settings.mk
.INCLUDE : $(PRJ)$/source$/mkinc$/fullcpp.mk

UWINAPILIB=$(0)
LIBSALCPPRT=$(0)

# --- Files --------------------------------------------------------

OBJFILES= \
    $(OBJ)$/adc_cl.obj	        \
    $(OBJ)$/adc_cmd_parse.obj	\
    $(OBJ)$/adc_cmds.obj        \
    $(OBJ)$/adc_msg.obj	        \
    $(OBJ)$/cmd_run.obj         \
    $(OBJ)$/cmd_sincedata.obj


# --- Targets ------------------------------------------------------

LIB1TARGET=$(LB)$/atdoc.lib
LIB1FILES= \
            $(LB)$/$(TARGET).lib $(LB)$/autodoc_tools.lib \
            $(LB)$/ary_kernel.lib $(LB)$/ary_cpp.lib $(LB)$/ary_idl.lib  \
            $(LB)$/ary_info.lib $(LB)$/ary_loc.lib \
            $(LB)$/parser_kernel.lib $(LB)$/parser_tokens.lib $(LB)$/parser_semantic.lib \
            $(LB)$/parser_cpp.lib $(LB)$/parser_adoc.lib \
            $(LB)$/display_kernel.lib $(LB)$/display_html.lib $(LB)$/display_idl.lib \
            $(LB)$/display_toolkit.lib $(LB)$/parser2_tokens.lib  \
            $(LB)$/parser2_s2_luidl.lib $(LB)$/parser2_s2_dsapi.lib \
            $(LB)$/ary2_cinfo.lib $(LB)$/ary_doc.lib



APP1TARGET=	$(TARGET)
APP1STACK=	1000000
APP1OBJS=   $(OBJ)$/main.obj

APP1RPATH=SDK

.IF "$(GUI)"=="WNT"
APP1STDLIBS= $(COSVLIB) $(UDMLIB)
.ELSE
.IF "$(OS)"=="MACOSX"
# See <http://porting.openoffice.org/servlets/ReadMsg?list=mac&msgNo=6911>:
APP1STDLIBS= -Wl,-all_load -ludm -lcosv
.ELSE
APP1STDLIBS= -lcosv -ludm
.ENDIF
.ENDIF

APP1LIBS=$(LB)$/atdoc.lib

DEPOBJFILES += $(APP1OBJS)

APP1DEPN=   $(LB)$/$(TARGET).lib $(LB)$/autodoc_tools.lib \
            $(LB)$/ary_kernel.lib $(LB)$/ary_cpp.lib $(LB)$/ary_idl.lib  \
            $(LB)$/ary_info.lib $(LB)$/ary_loc.lib \
            $(LB)$/parser_kernel.lib $(LB)$/parser_tokens.lib $(LB)$/parser_semantic.lib \
            $(LB)$/parser_cpp.lib $(LB)$/parser_adoc.lib \
            $(LB)$/display_kernel.lib $(LB)$/display_html.lib $(LB)$/display_idl.lib \
            $(LB)$/display_toolkit.lib $(LB)$/parser2_tokens.lib  \
            $(LB)$/parser2_s2_luidl.lib $(LB)$/parser2_s2_dsapi.lib \
            $(LB)$/ary2_cinfo.lib $(LB)$/ary_doc.lib


.INCLUDE :  target.mk
