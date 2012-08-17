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

$(eval $(call gb_JunitTest_JunitTest,framework_complex))

$(eval $(call gb_JunitTest_set_defs,framework_complex,\
    $$(DEFS) \
    -Dorg.openoffice.test.arg.tdoc=$(SRCDIR)/framework/qa/complex/broken_document/test_documents \
))

$(eval $(call gb_JunitTest_use_jars,framework_complex,\
    OOoRunner \
    ridl \
    test \
    unoil \
    jurt \
))

$(eval $(call gb_JunitTest_add_sourcefiles,framework_complex,\
    framework/qa/complex/disposing/GetServiceWhileDisposingOffice \
    framework/qa/complex/path_substitution/PathSubstitutionTest \
    framework/qa/complex/loadAllDocuments/InteractionHandler \
    framework/qa/complex/loadAllDocuments/StreamSimulator \
    framework/qa/complex/loadAllDocuments/TestDocument \
    framework/qa/complex/loadAllDocuments/CheckXComponentLoader \
    framework/qa/complex/loadAllDocuments/StatusIndicator \
    framework/qa/complex/broken_document/TestDocument \
    framework/qa/complex/broken_document/LoadDocument \
    framework/qa/complex/XUserInputInterception/EventTest \
    framework/qa/complex/framework/autosave/AutoSave \
    framework/qa/complex/framework/autosave/Protocol \
    framework/qa/complex/framework/autosave/ConfigHelper \
    framework/qa/complex/framework/recovery/TimeoutThread \
    framework/qa/complex/framework/recovery/KlickButtonThread \
    framework/qa/complex/framework/recovery/RecoveryTools \
    framework/qa/complex/framework/recovery/RecoveryTest \
    framework/qa/complex/framework/recovery/CrashThread \
    framework/qa/complex/accelerators/AcceleratorsConfigurationTest \
    framework/qa/complex/accelerators/KeyMapping \
    framework/qa/complex/contextMenuInterceptor/CheckContextMenuInterceptor \
    framework/qa/complex/contextMenuInterceptor/ContextMenuInterceptor \
    framework/qa/complex/path_settings/PathSettingsTest \
    framework/qa/complex/desktop/DesktopTerminate \
    framework/qa/complex/imageManager/_XComponent \
    framework/qa/complex/imageManager/CheckImageManager \
    framework/qa/complex/imageManager/_XTypeProvider \
    framework/qa/complex/imageManager/_XInitialization \
    framework/qa/complex/imageManager/_XImageManager \
    framework/qa/complex/imageManager/_XUIConfigurationPersistence \
    framework/qa/complex/imageManager/_XUIConfiguration \
    framework/qa/complex/api_internal/CheckAPI \
    framework/qa/complex/dispatches/Interceptor \
    framework/qa/complex/ModuleManager/CheckXModuleManager \
))

# does not build
#	framework/qa/complex/dispatches/checkdispatchapi \

$(eval $(call gb_JunitTest_add_classes,framework_complex,\
))
# these were disabled in the old build system too, please check
# carefully before reenabling
#	complex.ModuleManager.CheckXModuleManager \
    complex.XUserInputInterception.EventTest \
    complex.accelerators.AcceleratorsConfigurationTest \
    complex.dispatches.checkdispatchapi \
    complex.api_internal.CheckAPI \
    complex.broken_document.LoadDocument \
    complex.desktop.DesktopTerminate \
    complex.disposing.GetServiceWhileDisposingOffice \
    complex.framework.autosave.AutoSave \
    complex.framework.recovery.RecoveryTest \
    complex.imageManager.CheckImageManager \
    complex.loadAllDocuments.CheckXComponentLoader \
    complex.path_settings.PathSettingsTest \
    complex.path_substitution.PathSubstitutionTest \

# vim: set noet sw=4 ts=4:
