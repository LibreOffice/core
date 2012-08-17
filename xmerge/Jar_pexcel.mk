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

$(eval $(call gb_Jar_Jar,pexcel))

$(eval $(call gb_Jar_add_packagefile,pexcel,META-INF/converter.xml,$(SRCDIR)/xmerge/source/pexcel/converter.xml))

$(eval $(call gb_Jar_set_packageroot,pexcel,org))

$(eval $(call gb_Jar_use_jars,pexcel,\
	xmerge \
))

$(eval $(call gb_Jar_add_sourcefiles,pexcel,\
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/ConverterCapabilitiesImpl \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/PluginFactoryImpl \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/PocketExcelConstants \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/PocketExcelDecoder \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/PocketExcelEncoder \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/BeginningOfFile \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/BIFFRecord \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/BlankCell \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/BoolErrCell \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/BoundSheet \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/CellValue \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/CodePage \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/ColInfo \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/DefColWidth \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/DefinedName \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/DefRowHeight \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/Eof \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/ExtendedFormat \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/FloatNumber \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/FontDescription \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/Formula \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/formula/FormulaCompiler \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/formula/FormulaHelper \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/formula/FormulaParser \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/formula/FormulaParsingException \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/formula/FunctionLookup \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/formula/OperandLookup \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/formula/OperatorLookup \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/formula/ParseToken \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/formula/PrecedenceTable \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/formula/SymbolLookup \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/formula/Token \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/formula/TokenConstants \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/formula/TokenDecoder \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/formula/TokenEncoder \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/formula/TokenFactory \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/formula/UnsupportedFunctionException \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/LabelCell \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/NumberFormat \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/Pane \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/Row \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/Selection \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/StringValue \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/UnsupportedFormulaException \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/Window1 \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/Window2 \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/Workbook \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/records/Worksheet \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/SxcDocumentDeserializerImpl \
    xmerge/source/pexcel/java/org/openoffice/xmerge/converter/xml/sxc/pexcel/SxcDocumentSerializerImpl \
))
