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
TARGET=display_idl


# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=true
PRJINC=$(PRJ)$/source

.INCLUDE :  settings.mk
.INCLUDE : $(PRJ)$/source$/mkinc$/fullcpp.mk


# --- Files --------------------------------------------------------

OBJFILES= \
    $(OBJ)$/hfi_constgroup.obj  \
    $(OBJ)$/hfi_doc.obj         \
    $(OBJ)$/hfi_enum.obj        \
    $(OBJ)$/hfi_globalindex.obj \
    $(OBJ)$/hfi_hierarchy.obj   \
    $(OBJ)$/hfi_interface.obj   \
    $(OBJ)$/hfi_method.obj      \
    $(OBJ)$/hfi_module.obj      \
    $(OBJ)$/hfi_navibar.obj     \
    $(OBJ)$/hfi_property.obj    \
    $(OBJ)$/hfi_service.obj     \
    $(OBJ)$/hfi_singleton.obj   \
    $(OBJ)$/hfi_siservice.obj   \
    $(OBJ)$/hfi_struct.obj      \
    $(OBJ)$/hfi_tag.obj         \
    $(OBJ)$/hfi_typedef.obj     \
    $(OBJ)$/hfi_typetext.obj    \
    $(OBJ)$/hfi_xrefpage.obj    \
    $(OBJ)$/hi_ary.obj        	\
    $(OBJ)$/hi_display.obj      \
    $(OBJ)$/hi_env.obj          \
    $(OBJ)$/hi_factory.obj      \
    $(OBJ)$/hi_linkhelper.obj   \
    $(OBJ)$/hi_main.obj


# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk



