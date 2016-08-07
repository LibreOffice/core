###############################################################
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
###############################################################



# unotools is the name of the module the makefile is located in

# declare a library
# utl is the name of the library as it is found in Repository.mk
$(eval $(call gb_Library_Library,utl))

# declare packages that will be delivered before compilation of utl
# learn more about TYPE in the Package.mk template
$(eval $(call gb_Library_add_package_headers,utl,unotools_inc))

# for platforms supporting PCH: declare the location of the pch file
# this is the name of the cxx file (without extension)
$(eval $(call gb_Library_add_precompiled_header,utl,$(SRCDIR)/unotools/inc/pch/precompiled_unotools))

# in case UNO services are exported: declare location of component file
$(eval $(call gb_Library_set_componentfile,utl,unotools/util/utl))

$(eval $(call gb_Library_add_api,utl,\
	udkapi \
	offapi \
))

# add any additional include paths for this library here
$(eval $(call gb_Library_set_include,utl,\
	$$(INCLUDE) \
	-I$(SRCDIR)/unotools/inc \
	-I$(SRCDIR)/unotools/inc/pch \
))

# add any additional definitions to be set for compilation here
# (e.g. -DLIB_DLLIMPLEMENTATION)
$(eval $(call gb_Library_add_defs,utl,\
	-DUNOTOOLS_DLLIMPLEMENTATION \
))

# add libraries to be linked to utl; again these names need to be given as
# specified in Repository.mk
$(eval $(call gb_Library_add_linked_libs,utl,\
	sal \
	salhelper \
	stl \
	comphelper \
	ucbhelper \
	cppu \
	cppuhelper \
	i18nisolang1 \
	tl \
	vos3 \
    $(gb_STDLIBS) \
))

# add all source files that shall be compiled with exceptions enabled
# the name is relative to $(SRCROOT) and must not contain an extension
$(eval $(call gb_Library_add_exception_objects,utl,\
	unotools/source/streaming/streamhelper \
	unotools/source/streaming/streamwrap \
	unotools/source/accessibility/accessiblestatesethelper \
	unotools/source/accessibility/accessiblerelationsethelper \
	unotools/source/property/propertysethelper \
	unotools/source/property/propertysetinfo \
	unotools/source/misc/sharedunocomponent \
	unotools/source/misc/componentresmodule \
	unotools/source/misc/eventlisteneradapter \
	unotools/source/misc/fontcvt \
	unotools/source/misc/syslocale \
	unotools/source/misc/fontdefs \
	unotools/source/misc/datetime \
	unotools/source/misc/atom \
	unotools/source/misc/desktopterminationobserver \
	unotools/source/processfactory/processfactory \
	unotools/source/ucbhelper/localfilehelper \
	unotools/source/ucbhelper/ucbstreamhelper \
	unotools/source/ucbhelper/xtempfile \
	unotools/source/ucbhelper/ucblockbytes \
	unotools/source/ucbhelper/tempfile \
	unotools/source/ucbhelper/ucbhelper \
	unotools/source/ucbhelper/progresshandlerwrap \
	unotools/source/config/historyoptions \
	unotools/source/config/optionsdlg \
	unotools/source/config/bootstrap \
	unotools/source/config/configpathes \
	unotools/source/config/javaoptions \
	unotools/source/config/lingucfg \
	unotools/source/config/fontoptions \
	unotools/source/config/useroptions \
	unotools/source/config/internaloptions \
	unotools/source/config/viewoptions \
	unotools/source/config/workingsetoptions \
	unotools/source/config/fltrcfg \
	unotools/source/config/localisationoptions \
	unotools/source/config/itemholder1 \
	unotools/source/config/cmdoptions \
	unotools/source/config/cacheoptions \
	unotools/source/config/docinfohelper \
	unotools/source/config/inetoptions \
	unotools/source/config/misccfg \
	unotools/source/config/options \
	unotools/source/config/xmlaccelcfg \
	unotools/source/config/fontcfg \
	unotools/source/config/startoptions \
	unotools/source/config/extendedsecurityoptions \
	unotools/source/config/printwarningoptions \
	unotools/source/config/configmgr \
	unotools/source/config/eventcfg \
	unotools/source/config/configitem \
	unotools/source/config/defaultoptions \
	unotools/source/config/searchopt \
	unotools/source/config/confignode \
	unotools/source/config/sourceviewconfig \
	unotools/source/config/compatibility \
	unotools/source/config/pathoptions \
	unotools/source/config/dynamicmenuoptions \
	unotools/source/config/moduleoptions \
	unotools/source/config/undoopt \
	unotools/source/config/configvaluecontainer \
	unotools/source/config/syslocaleoptions \
	unotools/source/config/accelcfg \
	unotools/source/config/saveopt \
	unotools/source/config/securityoptions \
	unotools/source/i18n/charclass \
	unotools/source/i18n/textsearch \
	unotools/source/i18n/readwritemutexguard \
	unotools/source/i18n/intlwrapper \
	unotools/source/i18n/transliterationwrapper \
	unotools/source/i18n/calendarwrapper \
	unotools/source/i18n/localedatawrapper \
	unotools/source/i18n/collatorwrapper \
	unotools/source/i18n/numberformatcodewrapper \
	unotools/source/i18n/nativenumberwrapper \
))

# vim: set noet sw=4 ts=4:
