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
    $$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,etonyek,\
	-DBOOST_ALL_NO_LIB \
	-DDLL_EXPORT \
	-DLIBETONYEK_BUILD \
	-DNDEBUG \
))

$(eval $(call gb_Library_add_generated_exception_objects,etonyek,\
	EtonyekDocument \
	IWORKChainedTokenizer \
	IWORKCollector \
	IWORKDictionary \
	IWORKDocumentInterface \
	IWORKMemoryStream \
	IWORKOutputElements \
	IWORKParser \
	IWORKPath \
	IWORKPresentationRedirector \
	IWORKProperties \
	IWORKPropertyMap \
	IWORKShape \
	IWORKSpreadsheetRedirector \
	IWORKStyle \
	IWORKStyleStack \
	IWORKStylesheet \
	IWORKTable \
	IWORKText \
	IWORKTextRedirector \
	IWORKToken \
	IWORKTokenizer \
	IWORKTokenizerBase \
	IWORKTransformation \
	IWORKTypes \
	IWORKXMLContext \
	IWORKXMLContextBase \
	IWORKXMLParserState \
	IWORKZlibStream \
	IWORKZoneManager \
	KEY1Parser \
	KEY1Token \
	KEY2Parser \
	KEY2ParserState \
	KEY2Token \
	KEYCollector \
	KEYDictionary \
	KEYTypes \
	NUM1Parser \
	NUM1ParserState \
	NUM1Token \
	NUMCollector \
	NUMDictionary \
	PAG1Parser \
	PAG1ParserState \
	PAG1Token \
	PAGCollector \
	PAGDictionary \
	contexts/IWORKColorElement \
	contexts/IWORKGeometryElement \
	contexts/IWORKLayoutElement \
	contexts/IWORKMediaElement \
	contexts/IWORKPElement \
	contexts/IWORKPositionElement \
	contexts/IWORKPropertyMapElement \
	contexts/IWORKRefContext \
	contexts/IWORKSizeElement \
	contexts/IWORKStyleContext \
	contexts/IWORKStyleRefContext \
	contexts/IWORKStylesContext \
	contexts/IWORKTabularInfoElement \
	contexts/IWORKTextBodyElement \
	contexts/IWORKTextStorageElement \
	contexts/KEY2StyleContext \
	contexts/KEY2StyleRefContext \
	libetonyek_utils \
	libetonyek_xml \
))

# vim: set noet sw=4 ts=4:
