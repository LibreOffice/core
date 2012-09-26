# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2011 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_Library_Library,xof))

$(eval $(call gb_Library_set_componentfile,xof,xmloff/source/transform/xof))

$(eval $(call gb_Library_set_include,xof,\
    -I$(SRCDIR)/xmloff/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_use_sdk_api,xof))

$(eval $(call gb_Library_use_libraries,xof,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    salhelper \
	sax \
    xo \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_standard_system_libs,xof))

$(eval $(call gb_Library_add_exception_objects,xof,\
    xmloff/source/transform/ChartOASISTContext \
    xmloff/source/transform/ChartOOoTContext \
    xmloff/source/transform/ChartPlotAreaOASISTContext \
    xmloff/source/transform/ChartPlotAreaOOoTContext \
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
    xmloff/source/transform/OOo2Oasis \
    xmloff/source/transform/Oasis2OOo \
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
    xmloff/source/transform/XMLFilterRegistration \
))

# vim: set noet sw=4 ts=4:
