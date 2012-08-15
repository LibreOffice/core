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

$(eval $(call gb_Jar_Jar,xmerge))

$(eval $(call gb_Jar_set_manifest,xmerge,$(SRCDIR)/xmerge/source/xmerge/xmerge.mf))

$(eval $(call gb_Jar_use_jars,xmerge,\
	$(OUTDIR)/bin/unoil.jar \
	$(OUTDIR)/bin/ridl.jar \
	$(OUTDIR)/bin/jurt.jar \
	$(OUTDIR)/bin/juh.jar \
))

$(eval $(call gb_Jar_set_packageroot,xmerge,org))

$(eval $(call gb_Jar_add_sourcefiles,xmerge,\
    xmerge/source/xmerge/java/org/openoffice/xmerge/Convert \
    xmerge/source/xmerge/java/org/openoffice/xmerge/ConvertData \
    xmerge/source/xmerge/java/org/openoffice/xmerge/ConverterCapabilities \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/dom/DOMDocument \
    xmerge/source/xmerge/java/org/openoffice/xmerge/ConverterFactory \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/palm/PalmDB \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/palm/PalmDocument \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/palm/PdbDecoder \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/palm/PdbEncoder \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/palm/PdbHeader \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/palm/PdbUtil \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/palm/Record \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/xml/EmbeddedBinaryObject \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/xml/EmbeddedObject \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/xml/EmbeddedXMLObject \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/xml/OfficeConstants \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/xml/OfficeDocument \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/xml/OfficeDocumentException \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/xml/OfficeZip \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/xml/ParaStyle \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/xml/Style \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/xml/StyleCatalog \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/xml/sxc/BookSettings \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/xml/sxc/CellStyle \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/xml/sxc/ColumnRowInfo \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/xml/sxc/ColumnStyle \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/xml/sxc/DocumentMergerImpl \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/xml/sxc/Format \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/xml/sxc/NameDefinition \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/xml/sxc/RowStyle \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/xml/sxc/SheetSettings \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/xml/sxc/SpreadsheetDecoder \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/xml/sxc/SpreadsheetEncoder \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/xml/sxc/SxcConstants \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/xml/sxc/SxcDocument \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/xml/sxc/SxcDocumentDeserializer \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/xml/sxc/SxcDocumentSerializer \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/xml/sxc/SxcPluginFactory \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/xml/sxw/SxwDocument \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/xml/sxw/SxwPluginFactory \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/xml/TextStyle \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/xml/xslt/ConverterCapabilitiesImpl \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/xml/xslt/DocumentDeserializerImpl \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/xml/xslt/DocumentMergerImpl \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/xml/xslt/DocumentSerializerImpl \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/xml/xslt/GenericOfficeDocument \
    xmerge/source/xmerge/java/org/openoffice/xmerge/converter/xml/xslt/PluginFactoryImpl \
    xmerge/source/xmerge/java/org/openoffice/xmerge/ConvertException \
    xmerge/source/xmerge/java/org/openoffice/xmerge/Document \
    xmerge/source/xmerge/java/org/openoffice/xmerge/DocumentDeserializer \
    xmerge/source/xmerge/java/org/openoffice/xmerge/DocumentDeserializer2 \
    xmerge/source/xmerge/java/org/openoffice/xmerge/DocumentDeserializerFactory \
    xmerge/source/xmerge/java/org/openoffice/xmerge/DocumentMerger \
    xmerge/source/xmerge/java/org/openoffice/xmerge/DocumentMergerFactory \
    xmerge/source/xmerge/java/org/openoffice/xmerge/DocumentSerializer \
    xmerge/source/xmerge/java/org/openoffice/xmerge/DocumentSerializer2 \
    xmerge/source/xmerge/java/org/openoffice/xmerge/DocumentSerializerFactory \
    xmerge/source/xmerge/java/org/openoffice/xmerge/MergeException \
    xmerge/source/xmerge/java/org/openoffice/xmerge/merger/DiffAlgorithm \
    xmerge/source/xmerge/java/org/openoffice/xmerge/merger/diff/CellNodeIterator \
    xmerge/source/xmerge/java/org/openoffice/xmerge/merger/diff/CharacterParser \
    xmerge/source/xmerge/java/org/openoffice/xmerge/merger/diff/CharArrayLCSAlgorithm \
    xmerge/source/xmerge/java/org/openoffice/xmerge/merger/Difference \
    xmerge/source/xmerge/java/org/openoffice/xmerge/merger/diff/IteratorLCSAlgorithm \
    xmerge/source/xmerge/java/org/openoffice/xmerge/merger/diff/IteratorRowCompare \
    xmerge/source/xmerge/java/org/openoffice/xmerge/merger/diff/NodeIterator \
    xmerge/source/xmerge/java/org/openoffice/xmerge/merger/diff/ObjectArrayIterator \
    xmerge/source/xmerge/java/org/openoffice/xmerge/merger/diff/ParaNodeIterator \
    xmerge/source/xmerge/java/org/openoffice/xmerge/merger/diff/RowIterator \
    xmerge/source/xmerge/java/org/openoffice/xmerge/merger/diff/TextNodeEntry \
    xmerge/source/xmerge/java/org/openoffice/xmerge/merger/diff/TextNodeIterator \
    xmerge/source/xmerge/java/org/openoffice/xmerge/merger/Iterator \
    xmerge/source/xmerge/java/org/openoffice/xmerge/merger/MergeAlgorithm \
    xmerge/source/xmerge/java/org/openoffice/xmerge/merger/merge/CharacterBaseParagraphMerge \
    xmerge/source/xmerge/java/org/openoffice/xmerge/merger/merge/DocumentMerge \
    xmerge/source/xmerge/java/org/openoffice/xmerge/merger/merge/PositionBaseRowMerge \
    xmerge/source/xmerge/java/org/openoffice/xmerge/merger/merge/SheetMerge \
    xmerge/source/xmerge/java/org/openoffice/xmerge/merger/merge/SheetUtil \
    xmerge/source/xmerge/java/org/openoffice/xmerge/merger/NodeMergeAlgorithm \
    xmerge/source/xmerge/java/org/openoffice/xmerge/PluginFactory \
    xmerge/source/xmerge/java/org/openoffice/xmerge/test/ConverterInfoList \
    xmerge/source/xmerge/java/org/openoffice/xmerge/test/Driver \
    xmerge/source/xmerge/java/org/openoffice/xmerge/util/ActiveSyncDriver \
    xmerge/source/xmerge/java/org/openoffice/xmerge/util/ColourConverter \
    xmerge/source/xmerge/java/org/openoffice/xmerge/util/Debug \
    xmerge/source/xmerge/java/org/openoffice/xmerge/util/EndianConverter \
    xmerge/source/xmerge/java/org/openoffice/xmerge/util/IntArrayList \
    xmerge/source/xmerge/java/org/openoffice/xmerge/util/OfficeUtil \
    xmerge/source/xmerge/java/org/openoffice/xmerge/util/registry/ConverterInfo \
    xmerge/source/xmerge/java/org/openoffice/xmerge/util/registry/ConverterInfoMgr \
    xmerge/source/xmerge/java/org/openoffice/xmerge/util/registry/ConverterInfoReader \
    xmerge/source/xmerge/java/org/openoffice/xmerge/util/registry/RegistryException \
    xmerge/source/xmerge/java/org/openoffice/xmerge/util/Resources \
    xmerge/source/xmerge/java/org/openoffice/xmerge/util/TwipsConverter \
    xmerge/source/xmerge/java/org/openoffice/xmerge/util/XmlUtil \
    xmerge/source/xmerge/java/org/openoffice/xmerge/Version \
))
