# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

my_example_dirs_java = \
    DevelopersGuide/BasicAndDialogs/CreatingDialogs \
    DevelopersGuide/Charts/java \
    DevelopersGuide/Components/Addons/JobsAddon \
    DevelopersGuide/Components/Addons/ProtocolHandlerAddon_java \
    DevelopersGuide/Components/JavaComponent \
    DevelopersGuide/Components/SimpleLicense \
    DevelopersGuide/Components/Thumbs \
    DevelopersGuide/Components/dialogcomponent \
    DevelopersGuide/Config \
    DevelopersGuide/Database \
    DevelopersGuide/Drawing \
    DevelopersGuide/FirstSteps/FirstUnoContact/java \
    DevelopersGuide/FirstSteps/FirstLoadComponent/java \
    DevelopersGuide/FirstSteps/HelloTextTableShape/java \
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
    DevelopersGuide/OfficeDev/PathSubstitution/java \
    DevelopersGuide/OfficeDev/TerminationTest/java \
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

#    java/ConverterServlet \
#    java/EmbedDocument/Container1 \
#    java/EmbedDocument/EmbeddedObject \
#    java/NotesAccess \
#    java/Storage \

$(eval $(call odk_build-examples_test,odk/build-examples_java,$(my_example_dirs_java)))

# vim: set noet sw=4 ts=4:
