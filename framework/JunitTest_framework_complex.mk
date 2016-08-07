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



$(eval $(call gb_JunitTest_JunitTest,framework_complex,SRCDIR))

$(eval $(call gb_JunitTest_set_defs,framework_complex,\
    $$(DEFS) \
    -Dorg.openoffice.test.arg.tdoc=$(SRCDIR)/framework/qa/complex/broken_document/test_documents \
))

$(eval $(call gb_JunitTest_add_jars,framework_complex,\
    $(OUTDIR)/bin/OOoRunner.jar \
    $(OUTDIR)/bin/ridl.jar \
    $(OUTDIR)/bin/test.jar \
    $(OUTDIR)/bin/unoil.jar \
    $(OUTDIR)/bin/jurt.jar \
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
