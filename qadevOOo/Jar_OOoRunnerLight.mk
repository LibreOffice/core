#
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
# 	Peter Foley <pefoley2@verizon.net>
# Portions created by the Initial Developer are Copyright (C) 2011 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.
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
	$(OUTDIR)/bin/ridl.jar \
	$(OUTDIR)/bin/unoil.jar \
	$(OUTDIR)/bin/jurt.jar \
	$(OUTDIR)/bin/juh.jar \
	$(OUTDIR)/bin/java_uno.jar \
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
    qadevOOo/runner/base/java_cmp \
    qadevOOo/runner/base/java_complex \
    qadevOOo/runner/base/java_fat \
    qadevOOo/runner/base/java_fat_service \
    qadevOOo/runner/base/TestBase \
    qadevOOo/runner/complexlib/Assurance \
    qadevOOo/runner/complexlib/ComplexTestCase \
    qadevOOo/runner/complexlib/MethodThread \
    qadevOOo/runner/complexlib/ShowTargets \
    qadevOOo/runner/convwatch/Basename \
    qadevOOo/runner/convwatch/BorderRemover \
    qadevOOo/runner/convwatch/BuildID \
    qadevOOo/runner/convwatch/ConvWatch \
    qadevOOo/runner/convwatch/ConvWatchCancelException \
    qadevOOo/runner/convwatch/ConvWatchException \
    qadevOOo/runner/convwatch/ConvWatchStarter \
    qadevOOo/runner/convwatch/CrashLoopTest \
    qadevOOo/runner/convwatch/DateHelper \
    qadevOOo/runner/convwatch/DB \
    qadevOOo/runner/convwatch/DBHelper \
    qadevOOo/runner/convwatch/DirectoryHelper \
    qadevOOo/runner/convwatch/DocumentConverter \
    qadevOOo/runner/convwatch/EnhancedComplexTestCase \
    qadevOOo/runner/convwatch/FileHelper \
    qadevOOo/runner/convwatch/FilenameHelper \
    qadevOOo/runner/convwatch/GfxCompare \
    qadevOOo/runner/convwatch/GlobalLogWriter \
    qadevOOo/runner/convwatch/GraphicalDifferenceCheck \
    qadevOOo/runner/convwatch/GraphicalTestArguments \
    qadevOOo/runner/convwatch/HTMLOutputter \
    qadevOOo/runner/convwatch/ImageHelper \
    qadevOOo/runner/convwatch/IniFile \
    qadevOOo/runner/convwatch/INIOutputter \
    qadevOOo/runner/convwatch/LISTOutputter \
    qadevOOo/runner/convwatch/MSOfficePrint \
    qadevOOo/runner/convwatch/NameHelper \
    qadevOOo/runner/convwatch/OfficePrint \
    qadevOOo/runner/convwatch/OSHelper \
    qadevOOo/runner/convwatch/PerformanceContainer \
    qadevOOo/runner/convwatch/PixelCounter \
    qadevOOo/runner/convwatch/PRNCompare \
    qadevOOo/runner/convwatch/PropertyName \
    qadevOOo/runner/convwatch/ReferenceBuilder \
    qadevOOo/runner/convwatch/ReportDesignerTest \
    qadevOOo/runner/convwatch/SimpleFileSemaphore \
    qadevOOo/runner/convwatch/StatusHelper \
    qadevOOo/runner/convwatch/TimeHelper \
    qadevOOo/runner/convwatch/TriState \
    qadevOOo/runner/convwatch/ValueNotFoundException \
    qadevOOo/runner/graphical/BuildID \
    qadevOOo/runner/graphical/DateHelper \
    qadevOOo/runner/graphical/DirectoryHelper \
    qadevOOo/runner/graphical/EnhancedComplexTestCase \
    qadevOOo/runner/graphical/FileHelper \
    qadevOOo/runner/graphical/GlobalLogWriter \
    qadevOOo/runner/graphical/GraphicalComparator \
    qadevOOo/runner/graphical/HTMLResult \
    qadevOOo/runner/graphical/IDocument \
    qadevOOo/runner/graphical/ImageHelper \
    qadevOOo/runner/graphical/IniFile \
    qadevOOo/runner/graphical/IOffice \
    qadevOOo/runner/graphical/JPEGComparator \
    qadevOOo/runner/graphical/JPEGCreator \
    qadevOOo/runner/graphical/JPEGEvaluator \
    qadevOOo/runner/graphical/MSOfficePostscriptCreator \
    qadevOOo/runner/graphical/Office \
    qadevOOo/runner/graphical/OfficeException \
    qadevOOo/runner/graphical/OpenOfficeDatabaseReportExtractor \
    qadevOOo/runner/graphical/OpenOfficePostscriptCreator \
    qadevOOo/runner/graphical/ParameterHelper \
    qadevOOo/runner/graphical/PerformanceContainer \
    qadevOOo/runner/graphical/PixelCounter \
    qadevOOo/runner/graphical/PostscriptCreator \
    qadevOOo/runner/graphical/PropertyName \
    qadevOOo/runner/graphical/TimeHelper \
    qadevOOo/runner/graphical/Tolerance \
    qadevOOo/runner/graphical/WrongEnvironmentException \
    qadevOOo/runner/graphical/WrongSuffixException \
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
    qadevOOo/runner/helper/InetTools \
    qadevOOo/runner/helper/LoggingThread \
    qadevOOo/runner/helper/ObjectInspectorModelImpl \
    qadevOOo/runner/helper/OfficeProvider \
    qadevOOo/runner/helper/OfficeWatcher \
    qadevOOo/runner/helper/OSHelper \
    qadevOOo/runner/helper/ParameterNotFoundException \
    qadevOOo/runner/helper/ProcessHandler \
    qadevOOo/runner/helper/PropertyHandlerFactroy \
    qadevOOo/runner/helper/PropertyHandlerImpl \
    qadevOOo/runner/helper/PropertyHelper \
    qadevOOo/runner/helper/SimpleMailSender \
    qadevOOo/runner/helper/StreamSimulator \
    qadevOOo/runner/helper/StringHelper \
    qadevOOo/runner/helper/UnoProvider \
    qadevOOo/runner/helper/URLHelper \
    qadevOOo/runner/helper/WindowListener \
    qadevOOo/runner/lib/DynamicClassLoader \
    qadevOOo/runner/lib/ExceptionStatus \
    qadevOOo/runner/lib/MultiMethodTest \
    qadevOOo/runner/lib/MultiPropertyTest \
    qadevOOo/runner/lib/Parameters \
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
    qadevOOo/runner/share/CwsDataExchange \
    qadevOOo/runner/share/DescEntry \
    qadevOOo/runner/share/DescGetter \
    qadevOOo/runner/share/LogWriter \
    qadevOOo/runner/share/Watcher \
    qadevOOo/runner/stats/ComplexDataBaseOutProducer \
    qadevOOo/runner/stats/DataBaseOutProducer \
    qadevOOo/runner/stats/FatDataBaseOutProducer \
    qadevOOo/runner/stats/FileLogWriter \
    qadevOOo/runner/stats/InternalLogWriter \
    qadevOOo/runner/stats/OutProducerFactory \
    qadevOOo/runner/stats/SimpleFileOutProducer \
    qadevOOo/runner/stats/SimpleLogWriter \
    qadevOOo/runner/stats/SimpleOutProducer \
    qadevOOo/runner/stats/SQLExecution \
    qadevOOo/runner/stats/Summarizer \
    qadevOOo/runner/util/AccessibilityTools \
    qadevOOo/runner/util/BasicMacroTools \
    qadevOOo/runner/util/BookmarkDsc \
    qadevOOo/runner/util/CalcTools \
    qadevOOo/runner/util/compare/DocComparator \
    qadevOOo/runner/util/compare/DocComparatorFactory \
    qadevOOo/runner/util/compare/GraphicalComparator \
    qadevOOo/runner/util/compare/PDFComparator \
    qadevOOo/runner/util/compare/XMLComparator \
    qadevOOo/runner/util/ControlDsc \
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
    qadevOOo/runner/util/ReferenceMarkDsc \
    qadevOOo/runner/util/RegistryTools \
    qadevOOo/runner/util/ShapeDsc \
    qadevOOo/runner/util/SOfficeFactory \
    qadevOOo/runner/util/StyleFamilyDsc \
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
