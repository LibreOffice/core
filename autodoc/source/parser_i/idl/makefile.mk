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



