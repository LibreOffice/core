# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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

$(eval $(call gb_Library_Library,xof))

$(eval $(call gb_Library_set_componentfile,xof,xmloff/source/transform/xof))

# WASM_CHART change
ifneq ($(ENABLE_WASM_STRIP_CHART),TRUE)
$(eval $(call gb_Library_set_componentfile,xof,xmloff/source/transform/xof.extended))
endif

$(eval $(call gb_Library_set_include,xof,\
    -I$(SRCDIR)/xmloff/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_use_custom_headers,xof,\
	xmloff/generated \
))

$(eval $(call gb_Library_set_precompiled_header,xof,xmloff/inc/pch/precompiled_xof))

$(eval $(call gb_Library_use_external,xof,boost_headers))

$(eval $(call gb_Library_use_sdk_api,xof))

$(eval $(call gb_Library_use_libraries,xof,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    salhelper \
    tl \
	sax \
    xo \
))

# WASM_CHART change
ifneq ($(ENABLE_WASM_STRIP_CHART),TRUE)
$(eval $(call gb_Library_add_exception_objects,xof,\
    xmloff/source/transform/ChartOASISTContext \
    xmloff/source/transform/ChartOOoTContext \
    xmloff/source/transform/ChartPlotAreaOASISTContext \
    xmloff/source/transform/ChartPlotAreaOOoTContext \
))
endif

$(eval $(call gb_Library_add_exception_objects,xof,\
    xmloff/source/transform/OOo2Oasis \
    xmloff/source/transform/Oasis2OOo \
    xmloff/source/transform/ControlOASISTContext \
    xmloff/source/transform/ControlOOoTContext \
    xmloff/source/transform/CreateElemTContext \
    xmloff/source/transform/DeepTContext \
    xmloff/source/transform/DlgOASISTContext \
    xmloff/source/transform/DocumentTContext \
    xmloff/source/transform/EventMap \
    xmloff/source/transform/EventOASISTContext \
    xmloff/source/transform/EventOOoTContext \
    xmloff/source/transform/FlatTContext \
    xmloff/source/transform/FormPropOASISTContext \
    xmloff/source/transform/FormPropOOoTContext \
    xmloff/source/transform/FrameOASISTContext \
    xmloff/source/transform/FrameOOoTContext \
    xmloff/source/transform/IgnoreTContext \
    xmloff/source/transform/MergeElemTContext \
    xmloff/source/transform/MetaTContext \
    xmloff/source/transform/MutableAttrList \
    xmloff/source/transform/NotesTContext \
    xmloff/source/transform/PersAttrListTContext \
    xmloff/source/transform/PersMixedContentTContext \
    xmloff/source/transform/ProcAddAttrTContext \
    xmloff/source/transform/ProcAttrTContext \
    xmloff/source/transform/PropertyActionsOASIS \
    xmloff/source/transform/PropertyActionsOOo \
    xmloff/source/transform/RenameElemTContext \
    xmloff/source/transform/StyleOASISTContext \
    xmloff/source/transform/StyleOOoTContext \
    xmloff/source/transform/TransformerActions \
    xmloff/source/transform/TransformerBase \
    xmloff/source/transform/TransformerContext \
    xmloff/source/transform/TransformerTokenMap \
))

# vim: set noet sw=4 ts=4:
