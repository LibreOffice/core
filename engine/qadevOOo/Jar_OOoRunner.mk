#
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#

$(eval $(call gb_Jar_Jar,OOoRunner))

$(eval $(call gb_Jar_add_manifest_classpath,OOoRunner,\
	libreoffice.jar \
))

$(eval $(call gb_Jar_set_manifest,OOoRunner,$(SRCDIR)/qadevOOo/runner/manifest))

$(eval $(call gb_Jar_use_jars,OOoRunner,\
	libreoffice \
	java_uno \
))

$(eval $(call gb_Jar_set_packageroot,OOoRunner,\
	base \
	complexlib \
	graphical \
	helper \
	lib \
	org \
	share \
	util \
))

$(eval $(call gb_Jar_add_sourcefiles,OOoRunner,\
    qadevOOo/runner/base/TestBase \
    qadevOOo/runner/complexlib/Assurance \
    qadevOOo/runner/complexlib/ComplexTestCase \
    qadevOOo/runner/complexlib/MethodThread \
    qadevOOo/runner/graphical/FileHelper \
    qadevOOo/runner/helper/AppProvider \
    qadevOOo/runner/helper/ClParser \
    qadevOOo/runner/helper/ConfigHelper \
    qadevOOo/runner/helper/FileTools \
    qadevOOo/runner/helper/OfficeProvider \
    qadevOOo/runner/helper/OfficeWatcher \
    qadevOOo/runner/helper/OSHelper \
    qadevOOo/runner/helper/ProcessHandler \
    qadevOOo/runner/helper/StreamSimulator \
    qadevOOo/runner/helper/StringHelper \
    qadevOOo/runner/helper/UnoProvider \
    qadevOOo/runner/helper/URLHelper \
    qadevOOo/runner/lib/ExceptionStatus \
    qadevOOo/runner/lib/RunState \
    qadevOOo/runner/lib/SimpleStatus \
    qadevOOo/runner/lib/Status \
    qadevOOo/runner/lib/StatusException \
    qadevOOo/runner/lib/TestParameters \
    qadevOOo/runner/org/openoffice/Runner \
	qadevOOo/runner/share/DescEntry \
	qadevOOo/runner/share/ComplexTest \
    qadevOOo/runner/share/LogWriter \
    qadevOOo/runner/share/Watcher \
    qadevOOo/runner/util/utils \
    qadevOOo/runner/util/dbg \
    qadevOOo/runner/util/AccessibilityTools \
    qadevOOo/runner/util/BookmarkDsc \
    qadevOOo/runner/util/DesktopTools \
    qadevOOo/runner/util/DrawTools \
    qadevOOo/runner/util/DynamicClassLoader \
    qadevOOo/runner/util/FootnoteDsc \
    qadevOOo/runner/util/FormTools \
    qadevOOo/runner/util/FrameDsc \
    qadevOOo/runner/util/InstDescr \
    qadevOOo/runner/util/InstCreator \
    qadevOOo/runner/util/PropertyName \
    qadevOOo/runner/util/ShapeDsc \
    qadevOOo/runner/util/SOfficeFactory \
    qadevOOo/runner/util/TableDsc \
    qadevOOo/runner/util/TextSectionDsc \
	qadevOOo/runner/util/SysUtils \
	qadevOOo/runner/util/UITools \
    qadevOOo/runner/util/ValueChanger \
    qadevOOo/runner/util/ValueComparer \
	qadevOOo/runner/util/WaitUnreachable \
	qadevOOo/runner/util/WriterTools \
	qadevOOo/runner/util/XInstCreator \
))
