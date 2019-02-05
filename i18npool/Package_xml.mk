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



$(eval $(call gb_Package_Package,i18npool_xml,$(SRCDIR)/i18npool/xml))

$(eval $(call gb_Package_add_file,i18npool_xml,xml/BreakIterator.xml,BreakIterator.xml))
$(eval $(call gb_Package_add_file,i18npool_xml,xml/Calendar.xml,Calendar.xml))
$(eval $(call gb_Package_add_file,i18npool_xml,xml/ChapterCollator.xml,ChapterCollator.xml))
$(eval $(call gb_Package_add_file,i18npool_xml,xml/CharacterClassification.xml,CharacterClassification.xml))
$(eval $(call gb_Package_add_file,i18npool_xml,xml/Collator.xml,Collator.xml))
$(eval $(call gb_Package_add_file,i18npool_xml,xml/DefaultNumberingProvider.xml,DefaultNumberingProvider.xml))
$(eval $(call gb_Package_add_file,i18npool_xml,xml/IndexEntrySupplier.xml,IndexEntrySupplier.xml))
$(eval $(call gb_Package_add_file,i18npool_xml,xml/LocaleData.xml,LocaleData.xml))
$(eval $(call gb_Package_add_file,i18npool_xml,xml/NumberFormatCodeMapper.xml,NumberFormatCodeMapper.xml))
$(eval $(call gb_Package_add_file,i18npool_xml,xml/OrdinalSuffix.xml,OrdinalSuffix.xml))
$(eval $(call gb_Package_add_file,i18npool_xml,xml/TextSearch.xml,TextSearch.xml))
$(eval $(call gb_Package_add_file,i18npool_xml,xml/Transliteration.xml,Transliteration.xml))

