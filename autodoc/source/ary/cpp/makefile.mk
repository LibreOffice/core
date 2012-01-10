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
TARGET=ary_cpp



# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=true
PRJINC=$(PRJ)$/source

.INCLUDE :  settings.mk
.INCLUDE : $(PRJ)$/source$/mkinc$/fullcpp.mk


# --- Files --------------------------------------------------------


OBJFILES= \
    $(OBJ)$/c_builtintype.obj   \
    $(OBJ)$/c_class.obj		    \
    $(OBJ)$/c_de.obj	        \
    $(OBJ)$/c_define.obj	    \
    $(OBJ)$/c_enum.obj		    \
    $(OBJ)$/c_enuval.obj	    \
    $(OBJ)$/c_funct.obj		    \
    $(OBJ)$/c_macro.obj	        \
    $(OBJ)$/c_namesp.obj	    \
    $(OBJ)$/c_osigna.obj	    \
    $(OBJ)$/c_reposypart.obj	\
    $(OBJ)$/c_slots.obj		    \
    $(OBJ)$/c_traits.obj        \
    $(OBJ)$/c_tydef.obj		    \
    $(OBJ)$/c_vari.obj		    \
    $(OBJ)$/ca_ce.obj           \
    $(OBJ)$/ca_def.obj          \
    $(OBJ)$/ca_type.obj         \
    $(OBJ)$/cs_ce.obj           \
    $(OBJ)$/cs_def.obj          \
    $(OBJ)$/cs_type.obj         \
    $(OBJ)$/namechain.obj	    \
    $(OBJ)$/tplparam.obj	    \
    $(OBJ)$/usedtype.obj



# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk



