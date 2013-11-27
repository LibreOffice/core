#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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
