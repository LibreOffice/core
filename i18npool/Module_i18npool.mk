#**************************************************************
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
#**************************************************************



$(eval $(call gb_Module_Module,i18npool))

$(eval $(call gb_Module_add_targets,i18npool,\
	Executable_gencoll_rule \
	Executable_genconv_dict \
	Executable_gendict \
	Executable_genindex_data \
	Executable_saxparser \
	Library_collator_data \
	Library_dict_ja \
	Library_dict_zh \
	Library_i18nisolang1 \
	Library_i18npaper \
	Library_i18npool \
	Library_i18nsearch \
	Library_index_data \
	Library_localedata_en \
	Library_localedata_es \
	Library_localedata_euro \
	Library_localedata_others \
	Library_textconv_dict \
	Package_inc \
	Package_version \
	Package_xml \
))


# What's left to do:
#
# DONE inp  i18npool\inc                               nmake   -   all inp_inc NULL
# DONE inp  i18npool\source\registerservices           nmake   -   all inp_rserv inp_inc NULL
# DONE  i18npool\source\breakiterator              nmake   -   all inp_brkit inp_inc NULL
# DONE inp  i18npool\source\characterclassification    nmake   -   all inp_chclass inp_inc NULL
# DONE inp  i18npool\source\breakiterator\data         nmake   -   all inp_dict inp_brkit inp_inc NULL
# DONE inp  i18npool\source\transliteration            nmake   -   all inp_translit inp_inc NULL
# DONE inp  i18npool\source\isolang                    nmake   -   all inp_isolang inp_inc NULL
# DONE inp  i18npool\source\localedata                 nmake   -   all inp_localedata inp_isolang inp_inc NULL
# DONE  i18npool\source\localedata\data            nmake   -   all inp_locdata_data inp_localedata inp_inc NULL
# DONE inp  i18npool\source\paper                      nmake   -   all inp_paper inp_isolang inp_inc NULL
# DONE inp  i18npool\source\calendar                   nmake   -   all inp_cal inp_inc NULL
# DONE inp  i18npool\source\numberformatcode           nmake   -   all inp_numformat inp_inc NULL
# DONE inp  i18npool\source\defaultnumberingprovider   nmake   -   all inp_dnum inp_inc NULL
# DONE inp  i18npool\source\nativenumber               nmake   -   all inp_natnum inp_inc NULL
# DONE inp  i18npool\source\indexentry                 nmake   -   all inp_index inp_inc NULL
# TEST  i18npool\source\indexentry\data            nmake   -   all inp_index_data inp_index inp_inc NULL
# DONE  i18npool\source\collator                   nmake   -   all inp_collator inp_inc NULL
# DONE  i18npool\source\collator\data              nmake   -   all inp_collator_data inp_collator inp_inc NULL
# DONE inp  i18npool\source\inputchecker               nmake   -   all inp_inputchecker inp_inc NULL
# DONE inp  i18npool\source\textconversion             nmake   -   all inp_textconversion inp_inc NULL
# DONE inp  i18npool\source\textconversion\data        nmake   -   all inp_textconv_dict inp_textconversion inp_inc NULL
# DONE inp  i18npool\source\search                     nmake   -   all inp_search inp_inc NULL
# DONE inp  i18npool\source\ordinalsuffix              nmake   -   all inp_ordinalsuffix NULL
# inp  i18npool\util                              nmake   -   all inp_util inp_brkit inp_dict inp_chclass inp_translit inp_cal inp_dnum inp_natnum inp_localedata inp_locdata_data inp_numformat inp_rserv inp_index inp_index_data 
#                                                                          inp_collator inp_collator_data inp_inputchecker inp_textconversion inp_textconv_dict inp_search inp_isolang inp_paper inp_ordinalsuffix NULL



# vim: set noet sw=4 ts=4:
