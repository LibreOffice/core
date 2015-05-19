# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,etonyek))

$(eval $(call gb_Library_use_unpacked,etonyek,libetonyek))

$(eval $(call gb_Library_use_externals,etonyek,\
    boost_headers \
    glm_headers \
	libxml2 \
	revenge \
	zlib \
))

$(eval $(call gb_Library_set_warnings_not_errors,etonyek))

$(eval $(call gb_Library_set_include,etonyek,\
	-I$(call gb_UnpackedTarball_get_dir,libetonyek)/inc \
	-I$(call gb_UnpackedTarball_get_dir,libetonyek)/src/lib \
	-I$(call gb_UnpackedTarball_get_dir,libetonyek)/src/lib/contexts \
	-I$(SRCDIR)/external/libetonyek/generated \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,etonyek,\
	-DBOOST_ALL_NO_LIB \
	-DDLL_EXPORT \
	-DLIBETONYEK_BUILD \
	-DNDEBUG \
))

$(eval $(call gb_Library_set_generated_cxx_suffix,etonyek,cpp))

$(eval $(call gb_Library_add_generated_exception_objects,etonyek,\
	UnpackedTarball/libetonyek/src/lib/EtonyekDocument \
	UnpackedTarball/libetonyek/src/lib/IWORKChainedTokenizer \
	UnpackedTarball/libetonyek/src/lib/IWORKCollector \
	UnpackedTarball/libetonyek/src/lib/IWORKDictionary \
	UnpackedTarball/libetonyek/src/lib/IWORKDocumentInterface \
	UnpackedTarball/libetonyek/src/lib/IWORKMemoryStream \
	UnpackedTarball/libetonyek/src/lib/IWORKOutputElements \
	UnpackedTarball/libetonyek/src/lib/IWORKParser \
	UnpackedTarball/libetonyek/src/lib/IWORKPath \
	UnpackedTarball/libetonyek/src/lib/IWORKPresentationRedirector \
	UnpackedTarball/libetonyek/src/lib/IWORKProperties \
	UnpackedTarball/libetonyek/src/lib/IWORKPropertyMap \
	UnpackedTarball/libetonyek/src/lib/IWORKShape \
	UnpackedTarball/libetonyek/src/lib/IWORKSpreadsheetRedirector \
	UnpackedTarball/libetonyek/src/lib/IWORKStyle \
	UnpackedTarball/libetonyek/src/lib/IWORKStyleStack \
	UnpackedTarball/libetonyek/src/lib/IWORKStylesheet \
	UnpackedTarball/libetonyek/src/lib/IWORKTable \
	UnpackedTarball/libetonyek/src/lib/IWORKText \
	UnpackedTarball/libetonyek/src/lib/IWORKTextRedirector \
	UnpackedTarball/libetonyek/src/lib/IWORKToken \
	UnpackedTarball/libetonyek/src/lib/IWORKTokenizer \
	UnpackedTarball/libetonyek/src/lib/IWORKTokenizerBase \
	UnpackedTarball/libetonyek/src/lib/IWORKTransformation \
	UnpackedTarball/libetonyek/src/lib/IWORKTypes \
	UnpackedTarball/libetonyek/src/lib/IWORKXMLContext \
	UnpackedTarball/libetonyek/src/lib/IWORKXMLContextBase \
	UnpackedTarball/libetonyek/src/lib/IWORKXMLParserState \
	UnpackedTarball/libetonyek/src/lib/IWORKZlibStream \
	UnpackedTarball/libetonyek/src/lib/IWORKZoneManager \
	UnpackedTarball/libetonyek/src/lib/KEY1Parser \
	UnpackedTarball/libetonyek/src/lib/KEY1Token \
	UnpackedTarball/libetonyek/src/lib/KEY2Parser \
	UnpackedTarball/libetonyek/src/lib/KEY2ParserState \
	UnpackedTarball/libetonyek/src/lib/KEY2Token \
	UnpackedTarball/libetonyek/src/lib/KEYCollector \
	UnpackedTarball/libetonyek/src/lib/KEYDictionary \
	UnpackedTarball/libetonyek/src/lib/KEYTypes \
	UnpackedTarball/libetonyek/src/lib/NUM1Parser \
	UnpackedTarball/libetonyek/src/lib/NUM1ParserState \
	UnpackedTarball/libetonyek/src/lib/NUM1Token \
	UnpackedTarball/libetonyek/src/lib/NUMCollector \
	UnpackedTarball/libetonyek/src/lib/NUMDictionary \
	UnpackedTarball/libetonyek/src/lib/PAG1Parser \
	UnpackedTarball/libetonyek/src/lib/PAG1ParserState \
	UnpackedTarball/libetonyek/src/lib/PAG1Token \
	UnpackedTarball/libetonyek/src/lib/PAGCollector \
	UnpackedTarball/libetonyek/src/lib/PAGDictionary \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKColorElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKGeometryElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKLayoutElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKMediaElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKPElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKPositionElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKPropertyMapElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKRefContext \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKSizeElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKStyleContext \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKStyleRefContext \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKStylesContext \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKTabularInfoElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKTextBodyElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKTextStorageElement \
	UnpackedTarball/libetonyek/src/lib/contexts/KEY2StyleContext \
	UnpackedTarball/libetonyek/src/lib/contexts/KEY2StyleRefContext \
	UnpackedTarball/libetonyek/src/lib/libetonyek_utils \
	UnpackedTarball/libetonyek/src/lib/libetonyek_xml \
))

# vim: set noet sw=4 ts=4:
