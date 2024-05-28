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

$(eval $(call gb_JunitTest_JunitTest,framework_complex))

$(eval $(call gb_JunitTest_set_defs,framework_complex,\
    $$(DEFS) \
    -Dorg.openoffice.test.arg.tdoc=$(SRCDIR)/framework/qa/complex/broken_document/test_documents \
))

$(eval $(call gb_JunitTest_use_unoapi_jars,framework_complex))

$(eval $(call gb_JunitTest_add_sourcefiles,framework_complex,\
    framework/qa/complex/disposing/GetServiceWhileDisposingOffice \
    framework/qa/complex/path_substitution/PathSubstitutionTest \
    framework/qa/complex/loadAllDocuments/InteractionHandler \
    framework/qa/complex/loadAllDocuments/CheckXComponentLoader \
    framework/qa/complex/loadAllDocuments/StatusIndicator \
    framework/qa/complex/broken_document/TestDocument \
    framework/qa/complex/broken_document/LoadDocument \
    framework/qa/complex/framework/autosave/AutoSave \
    framework/qa/complex/framework/autosave/Protocol \
    framework/qa/complex/framework/autosave/ConfigHelper \
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
    framework/qa/complex/dispatches/checkdispatchapi \
    framework/qa/complex/dispatches/Interceptor \
    framework/qa/complex/ModuleManager/CheckXModuleManager \
))

$(eval $(call gb_JunitTest_add_classes,framework_complex,\
    complex.dispatches.checkdispatchapi \
))
# these were disabled in the old build system too, please check
# carefully before reenabling
#	complex.ModuleManager.CheckXModuleManager \
    complex.accelerators.AcceleratorsConfigurationTest \
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
