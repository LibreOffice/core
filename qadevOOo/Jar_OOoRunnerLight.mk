#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#

#force debug information for OOoRunnerLight
gb_JavaClassSet_JAVACDEBUG := -g

$(eval $(call gb_Jar_Jar,OOoRunnerLight))

$(eval $(call gb_Jar_add_manifest_classpath,OOoRunnerLight,\
	ridl.jar \
	unoil.jar \
))

$(eval $(call gb_Jar_set_manifest,OOoRunnerLight,$(SRCDIR)/qadevOOo/runner/manifest))

$(eval $(call gb_Jar_use_jars,OOoRunnerLight,\
	ridl \
	unoil \
	jurt \
	juh \
	java_uno \
))

$(eval $(call gb_Jar_set_packageroot,OOoRunnerLight,\
	base \
	complexlib \
	convwatch \
	graphical \
	helper \
	lib \
	org \
	share \
	stats \
	util \
))

$(eval $(call gb_Jar_add_sourcefiles,OOoRunnerLight,\
    qadevOOo/runner/base/java_complex \
    qadevOOo/runner/base/java_fat \
    qadevOOo/runner/base/java_fat_service \
    qadevOOo/runner/base/TestBase \
    qadevOOo/runner/complexlib/Assurance \
    qadevOOo/runner/complexlib/ComplexTestCase \
    qadevOOo/runner/complexlib/MethodThread \
    qadevOOo/runner/complexlib/ShowTargets \
    qadevOOo/runner/convwatch/DB \
    qadevOOo/runner/convwatch/DBHelper \
    qadevOOo/runner/convwatch/GlobalLogWriter \
    qadevOOo/runner/graphical/FileHelper \
    qadevOOo/runner/helper/APIDescGetter \
    qadevOOo/runner/helper/AppProvider \
    qadevOOo/runner/helper/BuildEnvTools \
    qadevOOo/runner/helper/CfgParser \
    qadevOOo/runner/helper/ClParser \
    qadevOOo/runner/helper/ComplexDescGetter \
    qadevOOo/runner/helper/ConfigHelper \
    qadevOOo/runner/helper/ConfigurationRead \
    qadevOOo/runner/helper/ContextMenuInterceptor \
    qadevOOo/runner/helper/CwsDataExchangeImpl \
    qadevOOo/runner/helper/FileTools \
    qadevOOo/runner/helper/LoggingThread \
    qadevOOo/runner/helper/OfficeProvider \
    qadevOOo/runner/helper/OfficeWatcher \
    qadevOOo/runner/helper/OSHelper \
    qadevOOo/runner/helper/ParameterNotFoundException \
    qadevOOo/runner/helper/ProcessHandler \
    qadevOOo/runner/helper/PropertyHandlerFactroy \
    qadevOOo/runner/helper/PropertyHandlerImpl \
    qadevOOo/runner/helper/StreamSimulator \
    qadevOOo/runner/helper/StringHelper \
    qadevOOo/runner/helper/UnoProvider \
    qadevOOo/runner/helper/URLHelper \
    qadevOOo/runner/helper/WindowListener \
    qadevOOo/runner/lib/DynamicClassLoader \
    qadevOOo/runner/lib/ExceptionStatus \
    qadevOOo/runner/lib/MultiMethodTest \
    qadevOOo/runner/lib/MultiPropertyTest \
    qadevOOo/runner/lib/SimpleStatus \
    qadevOOo/runner/lib/Status \
    qadevOOo/runner/lib/StatusException \
    qadevOOo/runner/lib/TestCase \
    qadevOOo/runner/lib/TestEnvironment \
    qadevOOo/runner/lib/TestParameters \
    qadevOOo/runner/lib/TestResult \
    qadevOOo/runner/org/openoffice/Runner \
    qadevOOo/runner/org/openoffice/RunnerService \
    qadevOOo/runner/share/ComplexTest \
    qadevOOo/runner/share/DescEntry \
    qadevOOo/runner/share/DescGetter \
    qadevOOo/runner/share/LogWriter \
    qadevOOo/runner/share/Watcher \
    qadevOOo/runner/stats/InternalLogWriter \
    qadevOOo/runner/stats/OutProducerFactory \
    qadevOOo/runner/stats/SimpleLogWriter \
    qadevOOo/runner/stats/SimpleOutProducer \
    qadevOOo/runner/stats/Summarizer \
    qadevOOo/runner/util/AccessibilityTools \
    qadevOOo/runner/util/BookmarkDsc \
    qadevOOo/runner/util/CalcTools \
    qadevOOo/runner/util/db/DatabaseDocument \
    qadevOOo/runner/util/db/DataSource \
    qadevOOo/runner/util/db/DataSourceDescriptor \
    qadevOOo/runner/util/dbg \
    qadevOOo/runner/util/DBTools \
    qadevOOo/runner/util/DefaultDsc \
    qadevOOo/runner/util/DesktopTools \
    qadevOOo/runner/util/DrawTools \
    qadevOOo/runner/util/DynamicClassLoader \
    qadevOOo/runner/util/FootnoteDsc \
    qadevOOo/runner/util/FormTools \
    qadevOOo/runner/util/FrameDsc \
    qadevOOo/runner/util/InstCreator \
    qadevOOo/runner/util/InstDescr \
    qadevOOo/runner/util/ParagraphDsc \
    qadevOOo/runner/util/PropertyName \
    qadevOOo/runner/util/RegistryTools \
    qadevOOo/runner/util/ShapeDsc \
    qadevOOo/runner/util/SOfficeFactory \
    qadevOOo/runner/util/SysUtils \
    qadevOOo/runner/util/TableDsc \
    qadevOOo/runner/util/TextSectionDsc \
    qadevOOo/runner/util/UITools \
    qadevOOo/runner/util/utils \
    qadevOOo/runner/util/ValueChanger \
    qadevOOo/runner/util/ValueComparer \
    qadevOOo/runner/util/WaitUnreachable \
    qadevOOo/runner/util/WriterTools \
    qadevOOo/runner/util/XInstCreator \
    qadevOOo/runner/util/XLayerHandlerImpl \
    qadevOOo/runner/util/XLayerImpl \
    qadevOOo/runner/util/XMLTools \
    qadevOOo/runner/util/XSchemaHandlerImpl \
))
