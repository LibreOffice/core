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
TARGET=ary_idl


# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=true
PRJINC=$(PRJ)$/source

.INCLUDE :  settings.mk
.INCLUDE : $(PRJ)$/source$/mkinc$/fullcpp.mk


# --- Files --------------------------------------------------------


OBJFILES= \
    $(OBJ)$/i_attribute.obj     \
    $(OBJ)$/i_ce.obj            \
    $(OBJ)$/i_ce2s.obj          \
    $(OBJ)$/i_comrela.obj       \
    $(OBJ)$/i_constant.obj      \
    $(OBJ)$/i_constgroup.obj    \
    $(OBJ)$/i_enum.obj          \
    $(OBJ)$/i_enumvalue.obj     \
    $(OBJ)$/i_exception.obj     \
    $(OBJ)$/i_function.obj      \
    $(OBJ)$/i_interface.obj     \
    $(OBJ)$/i_module.obj        \
    $(OBJ)$/i_namelookup.obj    \
    $(OBJ)$/i_param.obj         \
    $(OBJ)$/i_property.obj      \
    $(OBJ)$/i_reposypart.obj    \
    $(OBJ)$/i_service.obj       \
    $(OBJ)$/i_singleton.obj     \
    $(OBJ)$/i_siservice.obj     \
    $(OBJ)$/i_sisingleton.obj   \
    $(OBJ)$/i_struct.obj        \
    $(OBJ)$/i_structelem.obj    \
    $(OBJ)$/i_traits.obj        \
    $(OBJ)$/i_typedef.obj       \
    $(OBJ)$/i2s_calculator.obj  \
    $(OBJ)$/ia_ce.obj           \
    $(OBJ)$/ia_type.obj         \
    $(OBJ)$/is_ce.obj           \
    $(OBJ)$/is_type.obj         \
    $(OBJ)$/it_builtin.obj      \
    $(OBJ)$/it_ce.obj           \
    $(OBJ)$/it_explicit.obj     \
    $(OBJ)$/it_sequence.obj     \
    $(OBJ)$/it_tplparam.obj     \
    $(OBJ)$/it_xnameroom.obj



# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
