#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.12 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 18:08:02 $
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
            $(LB)$/ary_info.lib $(LB)$/ary_loc.lib $(LB)$/ary_store.lib \
            $(LB)$/parser_kernel.lib $(LB)$/parser_tokens.lib $(LB)$/parser_semantic.lib \
            $(LB)$/parser_cpp.lib $(LB)$/parser_adoc.lib \
            $(LB)$/display_kernel.lib $(LB)$/display_html.lib $(LB)$/display_idl.lib \
            $(LB)$/display_toolkit.lib $(LB)$/parser2_tokens.lib  \
            $(LB)$/parser2_s2_luidl.lib $(LB)$/parser2_s2_dsapi.lib \
            $(LB)$/ary2_cinfo.lib



APP1TARGET=	$(TARGET)
APP1STACK=	1000000
APP1OBJS=   $(OBJ)$/main.obj

.IF "$(GUI)"=="WNT"
APP1STDLIBS= $(LIBSTLPORT) cosv.lib udm.lib
.ELSE
.IF "$(OS)"=="MACOSX"
APP1STDLIBS= $(LIBSTLPORT) -ludm -lcosv
.ELSE
APP1STDLIBS= -lcosv -ludm
.ENDIF
.ENDIF

APP1LIBS=$(LB)$/atdoc.lib

DEPOBJFILES += $(APP1OBJS)

APP1DEPN=   $(LB)$/$(TARGET).lib $(LB)$/autodoc_tools.lib \
            $(LB)$/ary_kernel.lib $(LB)$/ary_cpp.lib $(LB)$/ary_idl.lib  \
            $(LB)$/ary_info.lib $(LB)$/ary_loc.lib $(LB)$/ary_store.lib \
            $(LB)$/parser_kernel.lib $(LB)$/parser_tokens.lib $(LB)$/parser_semantic.lib \
            $(LB)$/parser_cpp.lib $(LB)$/parser_adoc.lib \
            $(LB)$/display_kernel.lib $(LB)$/display_html.lib $(LB)$/display_idl.lib \
            $(LB)$/display_toolkit.lib $(LB)$/parser2_tokens.lib  \
            $(LB)$/parser2_s2_luidl.lib $(LB)$/parser2_s2_dsapi.lib \
            $(LB)$/ary2_cinfo.lib


.INCLUDE :  target.mk
