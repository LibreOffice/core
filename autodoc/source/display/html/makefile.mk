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
TARGET=display_html


# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=true
PRJINC=$(PRJ)$/source

.INCLUDE :  settings.mk
.INCLUDE : $(PRJ)$/source$/mkinc$/fullcpp.mk


# --- Files --------------------------------------------------------

OBJFILES= \
    $(OBJ)$/aryattrs.obj    \
    $(OBJ)$/cfrstd.obj	    \
    $(OBJ)$/chd_udk2.obj	\
    $(OBJ)$/cre_link.obj	\
    $(OBJ)$/dsply_cl.obj	\
    $(OBJ)$/dsply_da.obj    \
    $(OBJ)$/dsply_op.obj	\
    $(OBJ)$/easywri.obj	    \
    $(OBJ)$/hd_chlst.obj  	\
    $(OBJ)$/hd_docu.obj  	\
    $(OBJ)$/hdimpl.obj  	\
    $(OBJ)$/html_kit.obj  	\
    $(OBJ)$/nav_main.obj  	\
    $(OBJ)$/navibar.obj  	\
    $(OBJ)$/outfile.obj     \
    $(OBJ)$/opageenv.obj    \
    $(OBJ)$/pagemake.obj    \
    $(OBJ)$/pm_aldef.obj    \
    $(OBJ)$/pm_base.obj     \
    $(OBJ)$/pm_class.obj    \
    $(OBJ)$/pm_help.obj     \
    $(OBJ)$/pm_index.obj    \
    $(OBJ)$/pm_namsp.obj    \
    $(OBJ)$/pm_start.obj    \
    $(OBJ)$/protarea.obj


# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk



