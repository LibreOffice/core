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
    liblangtag \
	libxml2 \
	mdds_headers \
	revenge \
	zlib \
))

$(eval $(call gb_Library_set_warnings_disabled,etonyek))

$(eval $(call gb_Library_set_precompiled_header,etonyek,external/libetonyek/inc/pch/precompiled_etonyek))

$(eval $(call gb_Library_set_include,etonyek,\
	-I$(call gb_UnpackedTarball_get_dir,libetonyek)/inc \
	-I$(call gb_UnpackedTarball_get_dir,libetonyek)/src/lib \
	-I$(call gb_UnpackedTarball_get_dir,libetonyek)/src/lib/contexts \
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
	UnpackedTarball/libetonyek/src/lib/IWAField \
	UnpackedTarball/libetonyek/src/lib/IWAMessage \
	UnpackedTarball/libetonyek/src/lib/IWAObjectIndex \
	UnpackedTarball/libetonyek/src/lib/IWAParser \
	UnpackedTarball/libetonyek/src/lib/IWAReader \
	UnpackedTarball/libetonyek/src/lib/IWASnappyStream \
	UnpackedTarball/libetonyek/src/lib/IWAText \
	UnpackedTarball/libetonyek/src/lib/IWORKChainedTokenizer \
	UnpackedTarball/libetonyek/src/lib/IWORKChart \
	UnpackedTarball/libetonyek/src/lib/IWORKCollector \
	UnpackedTarball/libetonyek/src/lib/IWORKDictionary \
	UnpackedTarball/libetonyek/src/lib/IWORKDiscardContext \
	UnpackedTarball/libetonyek/src/lib/IWORKDocumentInterface \
	UnpackedTarball/libetonyek/src/lib/IWORKFormula \
	UnpackedTarball/libetonyek/src/lib/IWORKLanguageManager \
	UnpackedTarball/libetonyek/src/lib/IWORKMemoryStream \
	UnpackedTarball/libetonyek/src/lib/IWORKOutputElements \
	UnpackedTarball/libetonyek/src/lib/IWORKOutputManager \
	UnpackedTarball/libetonyek/src/lib/IWORKParser \
	UnpackedTarball/libetonyek/src/lib/IWORKPath \
	UnpackedTarball/libetonyek/src/lib/IWORKPresentationRedirector \
	UnpackedTarball/libetonyek/src/lib/IWORKProperties \
	UnpackedTarball/libetonyek/src/lib/IWORKPropertyHandler \
	UnpackedTarball/libetonyek/src/lib/IWORKPropertyMap \
	UnpackedTarball/libetonyek/src/lib/IWORKRecorder \
	UnpackedTarball/libetonyek/src/lib/IWORKShape \
	UnpackedTarball/libetonyek/src/lib/IWORKSpreadsheetRedirector \
	UnpackedTarball/libetonyek/src/lib/IWORKStyle \
	UnpackedTarball/libetonyek/src/lib/IWORKStyleStack \
	UnpackedTarball/libetonyek/src/lib/IWORKStylesheet \
	UnpackedTarball/libetonyek/src/lib/IWORKSubDirStream \
	UnpackedTarball/libetonyek/src/lib/IWORKTable \
	UnpackedTarball/libetonyek/src/lib/IWORKTableRecorder \
	UnpackedTarball/libetonyek/src/lib/IWORKText \
	UnpackedTarball/libetonyek/src/lib/IWORKTextRecorder \
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
	UnpackedTarball/libetonyek/src/lib/KEY1Dictionary \
	UnpackedTarball/libetonyek/src/lib/KEY1Parser \
	UnpackedTarball/libetonyek/src/lib/KEY1ParserState \
	UnpackedTarball/libetonyek/src/lib/KEY1Token \
	UnpackedTarball/libetonyek/src/lib/KEY2Dictionary \
	UnpackedTarball/libetonyek/src/lib/KEY2Parser \
	UnpackedTarball/libetonyek/src/lib/KEY2ParserState \
	UnpackedTarball/libetonyek/src/lib/KEY2Token \
	UnpackedTarball/libetonyek/src/lib/KEY6Parser \
	UnpackedTarball/libetonyek/src/lib/KEYCollector \
	UnpackedTarball/libetonyek/src/lib/KEYProperties \
	UnpackedTarball/libetonyek/src/lib/KEYTypes \
	UnpackedTarball/libetonyek/src/lib/NUM1Dictionary \
	UnpackedTarball/libetonyek/src/lib/NUM1Parser \
	UnpackedTarball/libetonyek/src/lib/NUM1ParserState \
	UnpackedTarball/libetonyek/src/lib/NUM1Token \
	UnpackedTarball/libetonyek/src/lib/NUM3Parser \
	UnpackedTarball/libetonyek/src/lib/NUMCollector \
	UnpackedTarball/libetonyek/src/lib/PAG1Dictionary \
	UnpackedTarball/libetonyek/src/lib/PAG1Parser \
	UnpackedTarball/libetonyek/src/lib/PAG1ParserState \
	UnpackedTarball/libetonyek/src/lib/PAG1Token \
	UnpackedTarball/libetonyek/src/lib/PAG5Parser \
	UnpackedTarball/libetonyek/src/lib/PAGCollector \
	UnpackedTarball/libetonyek/src/lib/PAGProperties \
	UnpackedTarball/libetonyek/src/lib/PAGTypes \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKBezierElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKBinaryElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKBrContext \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKCalcEngineContext \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKCellCommentDrawableInfoElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKChartInfoElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKColorElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKCoreImageFilterDescriptorElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKDataElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKFieldElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKFillElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKFilteredImageElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKFormatElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKFormulaElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKGeometryElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKGroupElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKHeaderFooterContext \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKImageContext \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKImageElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKLayoutElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKLineElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKLineEndElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKLinkElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKListLabelGeometriesProperty \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKListLabelGeometryElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKListLabelIndentsProperty \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKListLabelTypeinfoElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKListLabelTypesProperty \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKListTextIndentsProperty \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKMediaElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKMetadataElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKNumberConverter \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKPElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKPathElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKPositionElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKPropertyContextBase \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKPropertyMapElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKRefContext \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKShapeContext \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKSizeElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKSpanElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKStringElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKStrokeContext \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKStyleContext \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKStyleRefContext \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKStylesContext \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKStylesheetBase \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKTabElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKTableInfoElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKTabsElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKTabularInfoElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKTabularModelElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKTextBodyElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKTextElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKTextLabelElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKTextStorageElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKUnfilteredElement \
	UnpackedTarball/libetonyek/src/lib/contexts/IWORKWrapElement \
	UnpackedTarball/libetonyek/src/lib/contexts/KEY1ContentElement \
	UnpackedTarball/libetonyek/src/lib/contexts/KEY1DivElement \
	UnpackedTarball/libetonyek/src/lib/contexts/KEY1FillElement \
	UnpackedTarball/libetonyek/src/lib/contexts/KEY1SpanElement \
	UnpackedTarball/libetonyek/src/lib/contexts/KEY1StringConverter \
	UnpackedTarball/libetonyek/src/lib/contexts/KEY1StylesContext \
	UnpackedTarball/libetonyek/src/lib/contexts/KEY1TableElement \
	UnpackedTarball/libetonyek/src/lib/contexts/KEY2StyleContext \
	UnpackedTarball/libetonyek/src/lib/contexts/PAG1AnnotationContext \
	UnpackedTarball/libetonyek/src/lib/contexts/PAG1AnnotationElement \
	UnpackedTarball/libetonyek/src/lib/contexts/PAG1FootnotesElement \
	UnpackedTarball/libetonyek/src/lib/contexts/PAG1ShapeContext \
	UnpackedTarball/libetonyek/src/lib/contexts/PAG1StyleContext \
	UnpackedTarball/libetonyek/src/lib/contexts/PAG1TextStorageElement \
	UnpackedTarball/libetonyek/src/lib/libetonyek_utils \
	UnpackedTarball/libetonyek/src/lib/libetonyek_xml \
))

# vim: set noet sw=4 ts=4:
