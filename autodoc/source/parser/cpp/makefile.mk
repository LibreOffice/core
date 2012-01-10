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



