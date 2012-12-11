#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

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
            $(LB)$/ary_kernel.lib $(LB)$/ary_idl.lib  \
            $(LB)$/ary_info.lib \
            $(LB)$/parser_kernel.lib $(LB)$/parser_tokens.lib \
            $(LB)$/display_kernel.lib $(LB)$/display_html.lib $(LB)$/display_idl.lib \
            $(LB)$/display_toolkit.lib $(LB)$/parser2_tokens.lib  \
            $(LB)$/parser2_s2_luidl.lib $(LB)$/parser2_s2_dsapi.lib \
            $(LB)$/ary2_cinfo.lib $(LB)$/ary_doc.lib



APP1TARGET=	$(TARGET)
APP1STACK=	1000000
APP1OBJS=   $(OBJ)$/main.obj

APP1RPATH=SDKBIN

.IF "$(OS)"=="WNT"
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
            $(LB)$/ary_kernel.lib $(LB)$/ary_idl.lib  \
            $(LB)$/ary_info.lib \
            $(LB)$/parser_kernel.lib $(LB)$/parser_tokens.lib \
            $(LB)$/display_kernel.lib $(LB)$/display_html.lib $(LB)$/display_idl.lib \
            $(LB)$/display_toolkit.lib $(LB)$/parser2_tokens.lib  \
            $(LB)$/parser2_s2_luidl.lib $(LB)$/parser2_s2_dsapi.lib \
            $(LB)$/ary2_cinfo.lib $(LB)$/ary_doc.lib


.INCLUDE :  target.mk
