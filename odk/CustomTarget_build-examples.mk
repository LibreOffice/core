# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

my_example_dirs = \
    CLI/CSharp/Spreadsheet \
    CLI/VB.NET/WriterDemo \
    DevelopersGuide/FirstSteps/FirstUnoContact/cxx \
    DevelopersGuide/FirstSteps/FirstLoadComponent/cxx \
    DevelopersGuide/FirstSteps/HelloTextTableShape/cxx \
    DevelopersGuide/BasicAndDialogs/ToolkitControls \
    DevelopersGuide/Components/Addons/ProtocolHandlerAddon_cpp \
    DevelopersGuide/Components/CppComponent \
    DevelopersGuide/Database/DriverSkeleton \
    DevelopersGuide/Extensions/DialogWithHelp \
    DevelopersGuide/OfficeDev/FilterDevelopment/FlatXmlFilter_cpp \
    DevelopersGuide/ProfUNO/CppBinding \
    DevelopersGuide/ProfUNO/SimpleBootstrap_cpp \
    OLE/activex \
    cpp/Draw \
    cpp/Convertor \
    cpp/DocumentLoader \
    cpp/complextoolbarcontrols \
    cpp/counter \
    cpp/remoteclient \
    python/toolpanel \
    python/minimal-extension \


#    cpp/custompanel \


$(eval $(call odk_build-examples_test,odk/build-examples,$(my_example_dirs)))

# vim: set noet sw=4 ts=4:
