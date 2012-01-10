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
TARGET=parser_adoc


# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=true
PRJINC=$(PRJ)$/source

.INCLUDE :  settings.mk
.INCLUDE : $(PRJ)$/source$/mkinc$/fullcpp.mk


# --- Files --------------------------------------------------------

OBJFILES= \
    $(OBJ)$/adoc_tok.obj   	\
    $(OBJ)$/a_rdocu.obj		\
    $(OBJ)$/cx_a_std.obj	\
    $(OBJ)$/cx_a_sub.obj	\
    $(OBJ)$/docu_pe.obj		\
    $(OBJ)$/prs_adoc.obj	\
    $(OBJ)$/tk_attag.obj  	\
    $(OBJ)$/tk_docw.obj



# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk



