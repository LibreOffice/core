# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,odk/build-examples))

my_example_dirs = \
    CLI/CSharp/Spreadsheet \
    CLI/VB.NET/WriterDemo \
    DevelopersGuide/BasicAndDialogs/ToolkitControls \
    DevelopersGuide/Components/Addons/ProtocolHandlerAddon_cpp \
    DevelopersGuide/Components/CppComponent \
    DevelopersGuide/Database/DriverSkeleton \
    DevelopersGuide/Extensions/DialogWithHelp \
    DevelopersGuide/OfficeDev/FilterDevelopment/FlatXmlFilter_cpp \
    DevelopersGuide/ProfUNO/CppBinding \
    DevelopersGuide/ProfUNO/SimpleBootstrap_cpp \
    OLE/activex \
    cpp/DocumentLoader \
    cpp/complextoolbarcontrols \
    cpp/counter \
    cpp/remoteclient \
    python/toolpanel \


#    cpp/custompanel \


ifeq ($(ENABLE_JAVA),TRUE)
my_example_dirs += \
    DevelopersGuide/BasicAndDialogs/CreatingDialogs \
    DevelopersGuide/Charts \
    DevelopersGuide/Components/Addons/JobsAddon \
    DevelopersGuide/Components/Addons/ProtocolHandlerAddon_java \
    DevelopersGuide/Components/JavaComponent \
    DevelopersGuide/Components/SimpleLicense \
    DevelopersGuide/Components/Thumbs \
    DevelopersGuide/Components/dialogcomponent \
    DevelopersGuide/Config \
    DevelopersGuide/Database \
    DevelopersGuide/Drawing \
    DevelopersGuide/FirstSteps \
    DevelopersGuide/Forms \
    DevelopersGuide/GUI \
    DevelopersGuide/OfficeBean \
    DevelopersGuide/OfficeDev \
    DevelopersGuide/OfficeDev/Clipboard \
    DevelopersGuide/OfficeDev/DesktopEnvironment \
    DevelopersGuide/OfficeDev/DisableCommands \
    DevelopersGuide/OfficeDev/FilterDevelopment/AsciiFilter \
    DevelopersGuide/OfficeDev/FilterDevelopment/FlatXmlFilterDetection \
    DevelopersGuide/OfficeDev/FilterDevelopment/FlatXmlFilter_java \
    DevelopersGuide/OfficeDev/Linguistic \
    DevelopersGuide/OfficeDev/PathSettings \
    DevelopersGuide/OfficeDev/PathSubstitution \
    DevelopersGuide/OfficeDev/TerminationTest \
    DevelopersGuide/ProfUNO/InterprocessConn \
    DevelopersGuide/ProfUNO/Lifetime \
    DevelopersGuide/ProfUNO/SimpleBootstrap_java \
    DevelopersGuide/ScriptingFramework/SayHello \
    DevelopersGuide/ScriptingFramework/ScriptSelector \
    DevelopersGuide/Spreadsheet \
    DevelopersGuide/Text \
    DevelopersGuide/UCB \
    java/DocumentHandling \
    java/Drawing \
    java/Inspector \
    java/MinimalComponent \
    java/PropertySet \
    java/Spreadsheet \
    java/Text \
    java/ToDo \

endif

#    java/ConverterServlet \
#    java/EmbedDocument/Container1 \
#    java/EmbedDocument/EmbeddedObject \
#    java/NotesAccess \
#    java/Storage \


.PHONY: $(call gb_CustomTarget_get_target,odk/build-examples)

$(call gb_CustomTarget_get_target,odk/build-examples): \
        $(call gb_CustomTarget_get_workdir,odk/build-examples)/setsdkenv
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),CHK,1)
	(. $< \
		&& unset LD_LIBRARY_PATH \
        && export \
            UserInstallation=$(call gb_Helper_make_url,$(call gb_CustomTarget_get_workdir,odk/build-examples)/user) \
        $(foreach my_dir,$(my_example_dirs), \
            && (cd $(INSTDIR)/$(SDKDIRNAME)/examples/$(my_dir) \
                && printf 'yes\n' | make))) \
            >$(call gb_CustomTarget_get_workdir,odk/build-examples)/log 2>&1 \
        || (RET=$$? \
            && cat $(call gb_CustomTarget_get_workdir,odk/build-examples)/log \
            && exit $$RET)

$(call gb_CustomTarget_get_workdir,odk/build-examples)/setsdkenv: \
        $(SRCDIR)/odk/config/setsdkenv_unix.sh.in \
		$(BUILDDIR)/config_$(gb_Side).mk | \
        $(call gb_CustomTarget_get_workdir,odk/build-examples)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),SED,1)
	sed -e 's!@OO_SDK_NAME@!sdk!' \
        -e 's!@OO_SDK_HOME@!$(INSTDIR)/$(SDKDIRNAME)!' \
        -e 's!@OFFICE_HOME@!$(INSTROOTBASE)!' -e 's!@OO_SDK_MAKE_HOME@!!' \
        -e 's!@OO_SDK_ZIP_HOME@!!' -e 's!@OO_SDK_CAT_HOME@!!' \
        -e 's!@OO_SDK_SED_HOME@!!' -e 's!@OO_SDK_CPP_HOME@!!' \
        -e 's!@OO_SDK_JAVA_HOME@!$(JAVA_HOME)!' \
        -e 's!@OO_SDK_OUTPUT_DIR@!$(call gb_CustomTarget_get_workdir,odk/build-examples)/out!' \
        -e 's!@SDK_AUTO_DEPLOYMENT@!YES!' $< > $@

# vim: set noet sw=4 ts=4:
