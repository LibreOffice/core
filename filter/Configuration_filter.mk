#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

# this deviates from Configuration.mk in rather bizarre ways

# most of the rules here use some weird merge program, and this is sort of
# semi-integrated with the stuff from Configuration.mk; not exactly pretty...

filter_MERGE_TARGET := $(call gb_ExternalExecutable_get_dependencies,python) \
	$(SRCDIR)/filter/source/config/tools/merge/pyAltFCFGMerge
filter_MERGE := $(call gb_ExternalExecutable_get_command,python) \
	$(SRCDIR)/filter/source/config/tools/merge/pyAltFCFGMerge \
	$(if $(VERBOSE)$(verbose),-verbose)

### filter configuration rules: generic stuff #######################

### types

filter_XcuFilterTypesTarget_get_target = $(WORKDIR)/XcuFilterTypesTarget/$(1)
filter_XcuFilterTypesTarget_get_clean_target = \
 $(WORKDIR)/Clean/XcuFilterTypesTarget/$(1)

$(call filter_XcuFilterTypesTarget_get_target,%) : $(filter_MERGE_TARGET)
	$(call gb_Output_announce,$*,$(true),FIT,1)
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@) && \
		RESPONSEFILE=`$(gb_MKTEMP)` && \
		echo "items=$(basename $(notdir $(filter %.xcu,$^)))" \
			| sed "s/ /$(COMMA)/g" > $${RESPONSEFILE} && \
		$(filter_MERGE) tempdir=$(TMPDIR) \
			share_subdir_name=$(LIBO_SHARE_FOLDER) \
		 	fragmentsdir=$(dir $(firstword $(filter %.xcu,$^))).. \
			outdir=$(dir $@) pkg=$@ xmlpackage=Types tcfg=$${RESPONSEFILE} && \
		rm -f $${RESPONSEFILE})

# delivering is handled by the rule for gb_XcuModuleTarget_get_outdir_target

$(call filter_XcuFilterTypesTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),FIT,1)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call filter_XcuFilterTypesTarget_get_target,$*) \
			  $(call gb_XcuModuleTarget_get_outdir_target,$*))

# $(call filter_Configuration__add_module,configuration,module,prefix,xcufiles,target,cleantarget)
define filter_Configuration__add_module
$(call gb_Configuration_get_target,$(1)) : \
	$(call gb_XcuModuleTarget_get_outdir_target,$(2))
$(call gb_Configuration_get_clean_target,$(1)) : $(6)
$(if $(4),,$(error filter_Configuration__add_module: no input files))
$(5) : \
	$(addprefix $(SRCDIR)/$(3)/,$(addsuffix .xcu,$(4)))
$(call gb_XcuModuleTarget_get_outdir_target,$(2)) : $(5)
$(call gb_XcuModuleTarget_get_outdir_target,$(2)) |: $(dir $(call gb_XcuModuleTarget_get_outdir_target,$(2))).dir
$(call gb_Deliver_add_deliverable,\
	$(call gb_XcuModuleTarget_get_outdir_target,$(2)),$(5),$(2))
endef

# $(call filter_Configuration_add_types,configuration,typesfile,prefix,xcufiles)
define filter_Configuration_add_types
$(eval $(call filter_Configuration__add_module,$(1),$(2),$(3),$(4),\
 $(call filter_XcuFilterTypesTarget_get_target,$(2)),\
 $(call filter_XcuFilterTypesTarget_get_clean_target,$(2))))
endef

### filters

filter_XcuFilterFiltersTarget_get_target = \
 $(WORKDIR)/XcuFilterFiltersTarget/$(1)
filter_XcuFilterFiltersTarget_get_clean_target = \
 $(WORKDIR)/Clean/XcuFilterFiltersTarget/$(1)

$(call filter_XcuFilterFiltersTarget_get_target,%) : $(filter_MERGE_TARGET)
	$(call gb_Output_announce,$*,$(true),FIF,1)
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@) && \
		RESPONSEFILE=`$(gb_MKTEMP)` && \
		echo "items=$(basename $(notdir $(filter %.xcu,$^)))" \
			| sed "s/ /$(COMMA)/g" > $${RESPONSEFILE} && \
		$(filter_MERGE) tempdir=$(TMPDIR) \
			share_subdir_name=$(LIBO_SHARE_FOLDER) \
			fragmentsdir=$(dir $(firstword $(filter %.xcu,$^))).. \
			outdir=$(dir $@) pkg=$@ xmlpackage=Filter fcfg=$${RESPONSEFILE} && \
		rm -f $${RESPONSEFILE})

# delivering is handled by the rule for gb_XcuModuleTarget_get_outdir_target

$(call filter_XcuFilterFiltersTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),FIF,1)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call filter_XcuFilterFiltersTarget_get_target,$*) \
			  $(call gb_XcuModuleTarget_get_outdir_target,$*))

# $(call filter_Configuration_add_filters,configuration,typesfile,prefix,xcufiles)
define filter_Configuration_add_filters
$(eval $(call filter_Configuration__add_module,$(1),$(2),$(3),$(4),\
 $(call filter_XcuFilterFiltersTarget_get_target,$(2)),\
 $(call filter_XcuFilterFiltersTarget_get_clean_target,$(2))))
endef

### others (frameloaders, contenthandlers)

filter_XcuFilterOthersTarget_get_target = $(WORKDIR)/XcuFilterOthersTarget/$(1)
filter_XcuFilterOthersTarget_get_clean_target = \
 $(WORKDIR)/Clean/XcuFilterOthersTarget/$(1)

$(call filter_XcuFilterOthersTarget_get_target,%) : $(filter_MERGE_TARGET)
	$(call gb_Output_announce,$*,$(true),FIO,1)
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@) && \
		RESPONSEFILE=`$(gb_MKTEMP)` && \
		RESPONSEFILE2=`$(gb_MKTEMP)` && \
		echo "items=$(strip $(foreach xcu,$(filter %.xcu,$^),$(if $(filter frameloaders,$(notdir $(patsubst %/,%,$(dir $(xcu))))),$(basename $(notdir $(xcu),)))))" \
			| sed "s/ /$(COMMA)/g" > $${RESPONSEFILE} && \
		echo "items=$(strip $(foreach xcu,$(filter %.xcu,$^),$(if $(filter contenthandlers,$(notdir $(patsubst %/,%,$(dir $(xcu))))),$(basename $(notdir $(xcu),)))))" \
			| sed "s/ /$(COMMA)/g" > $${RESPONSEFILE2} && \
		$(filter_MERGE) tempdir=$(TMPDIR) \
			share_subdir_name=$(LIBO_SHARE_FOLDER) \
			fragmentsdir=$(dir $(firstword $(filter %.xcu,$^))).. \
			outdir=$(dir $@) pkg=$@ xmlpackage=Misc \
			lcfg=$${RESPONSEFILE} ccfg=$${RESPONSEFILE2} && \
		rm -f $${RESPONSEFILE} $${RESPONSEFILE2})

$(call filter_XcuFilterOthersTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),FIO,1)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call filter_XcuFilterOthersTarget_get_target,$*) \
			  $(call gb_XcuModuleTarget_get_outdir_target,$*))

# delivering is handled by the rule for gb_XcuModuleTarget_get_outdir_target

# $(call filter_Configuration_add_others,configuration,typesfile,prefix,xcufiles)
define filter_Configuration_add_others
$(eval $(call filter_Configuration__add_module,$(1),$(2),$(3),$(4),\
 $(call filter_XcuFilterOthersTarget_get_target,$(2)),\
 $(call filter_XcuFilterOthersTarget_get_clean_target,$(2))))
endef

### internal filters

filter_XcuFilterInternalTarget_get_target = \
 $(WORKDIR)/XcuFilterInternalTarget/$(1)
filter_XcuFilterInternalTarget_get_clean_target = \
 $(WORKDIR)/Clean/XcuFilterInternalTarget/$(1)

$(call filter_XcuFilterInternalTarget_get_target,%) : $(filter_MERGE_TARGET)
	$(call gb_Output_announce,$*,$(true),FII,1)
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@) && \
		RESPONSEFILE=`$(gb_MKTEMP)` && \
		echo "items=$(basename $(notdir $(filter %.xcu,$^)))" \
			| sed "s/ /$(COMMA)/g" > $${RESPONSEFILE} && \
		$(filter_MERGE) tempdir=$(TMPDIR) \
			share_subdir_name=$(LIBO_SHARE_FOLDER) \
			fragmentsdir=$(dir $(firstword $(filter %.xcu,$^))).. \
			outdir=$(dir $@) pkg=$@ xmlpackage=GraphicFilter \
			fcfg=$${RESPONSEFILE} subdir_filters=internalgraphicfilters && \
		rm -f $${RESPONSEFILE})

# delivering is handled by the rule for gb_XcuModuleTarget_get_outdir_target

$(call filter_XcuFilterInternalTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),FII,1)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call filter_XcuFilterInternalTarget_get_target,$*) \
			  $(call gb_XcuModuleTarget_get_outdir_target,$*))

# $(call filter_Configuration_add_internal_filters,configuration,typesfile,prefix,xcufiles)
define filter_Configuration_add_internal_filters
$(eval $(call filter_Configuration__add_module,$(1),$(2),$(3),$(4),\
 $(call filter_XcuFilterInternalTarget_get_target,$(2)),\
 $(call filter_XcuFilterInternalTarget_get_clean_target,$(2))))
endef


### filter configuration rules: l10n stuff: #########################

# Configuration fcfg_langpack
#  => $(lang)/org/openoffice/TypeDetection/Filter.xcu
#     xslt=> filter_ui.xcu
#         merge=> *.xcu $(ALL_UI_FILTERS) [if WITH_LANG]
#                 cfgex=> source/%.xcu
#         merge=> source/*.xcu [if !WITH_LANG]

filter_XSLT_langfilter := \
	$(SRCDIR)/filter/source/config/fragments/langfilter.xsl
filter_XcuFilterUiTarget = $(WORKDIR)/XcuFilterUiTarget/filter_ui.xcu
filter_XcuFilterUiCleanTarget = $(WORKDIR)/Clean/XcuFilterUiTarget/filter_ui.xcu
filter_XCU_filter := org/openoffice/TypeDetection/Filter.xcu
filter_XcuResTarget_get_target = \
 $(call gb_XcuResTarget_get_target,fcfg_langpack/$(1)/$(filter_XCU_filter))

$(filter_XcuFilterUiTarget) : $(filter_MERGE_TARGET)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),FIU,1)
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@) && \
		RESPONSEFILE=`$(gb_MKTEMP)` && \
		echo "items=$(basename $(notdir $(filter %.xcu,$^)))" \
			| sed "s/ /$(COMMA)/g" > $${RESPONSEFILE} && \
		$(filter_MERGE) tempdir=$(TMPDIR) \
			share_subdir_name=$(LIBO_SHARE_FOLDER) \
			fragmentsdir=$(dir $(firstword $(filter %.xcu,$^))).. \
			pkg=$@ xmlpackage=Filter fcfg=$${RESPONSEFILE} languagepack=true \
		&& rm -f $${RESPONSEFILE})

$(filter_XcuFilterUiCleanTarget) :
	$(call gb_Output_announce,$(filter_XcuFilterUiTarget),$(false),FIU,1)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(filter_XcuFilterUiTarget))

$(call gb_Configuration_get_clean_target,fcfg_langpack) : \
	$(filter_XcuFilterUiCleanTarget)

# this is _not_ a pattern rule:
# there is already a pattern rule for gb_XcuResTarget_get_target,
# so generate non-pattern rules which have higher priority even in GNUmake 3.81
define filter_XcuResTarget__rule
$$(call filter_XcuResTarget_get_target,$(1)) : \
		$(filter_XSLT_langfilter) $(filter_XcuFilterUiTarget) \
		| $(call gb_ExternalExecutable_get_dependencies,xsltproc)
	$$(call gb_Output_announce,$(1),$(true),XCU,1)
	$$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $$(dir $$@) && \
		$(call gb_ExternalExecutable_get_command,xsltproc) --nonet --stringparam lang $(1) \
			$(filter_XSLT_langfilter) \
			$(filter_XcuFilterUiTarget) > $$@)
endef

$(foreach lang,$(gb_Configuration_LANGS),$(eval \
	$(call filter_XcuResTarget__rule,$(lang))))

define filter_Configuration_Configuration
$(call gb_Configuration_Configuration,$(1))

$(call gb_Configuration_get_target,$(1)) : \
	$(foreach lang,$(gb_Configuration_LANGS),$(call filter_XcuResTarget_get_target,$(lang)))
$(call gb_Configuration_get_clean_target,$(1)) : \
	$(foreach lang,$(gb_Configuration_LANGS),$(call filter_XcuResTarget_get_clean_target,$(lang)))

endef


# $(call filter_Configuration_add_ui_filter,configuration,prefix,xcufile)
define filter_Configuration_add_ui_filter
ifeq ($(WITH_LANG),)
$(filter_XcuFilterUiTarget) : \
	$(call gb_Configuration__get_source,$(1),$(2)/$(3))
else
$(call gb_XcuMergeTarget_XcuMergeTarget,$(2)/$(3),$(1),$(2),$(3))
$(filter_XcuFilterUiTarget) : \
	$(call gb_XcuMergeTarget_get_target,$(2)/$(3))
endif
$(call gb_Configuration_get_clean_target,$(1)) : \
	$(call gb_XcuMergeTarget_get_clean_target,$(2)/$(3))
endef

# $(call filter_Configuration_add_ui_filters,configuration,prefix,xcufile)
define filter_Configuration_add_ui_filters
$(foreach xcu,$(3),$(eval \
	$(call filter_Configuration_add_ui_filter,$(1),$(2),$(xcu).xcu)))
endef


### the filter configuration ########################################

$(eval $(call filter_Configuration_Configuration,fcfg_langpack))

# fcfg_base
$(call filter_Configuration_add_types,fcfg_langpack,fcfg_base_types.xcu,filter/source/config/fragments/types,\
	writer_web_HTML_help \
	oxt_OpenOffice_Extension \
	wav_Wave_Audio_File \
	component_Bibliography \
	component_DB \
)

$(call filter_Configuration_add_filters,fcfg_langpack,fcfg_base_filters.xcu,filter/source/config/fragments/filters,\
	writer_web_HTML_help \
)

$(call filter_Configuration_add_others,fcfg_langpack,fcfg_base_others.xcu,filter/source/config/fragments,\
	frameloaders/com_sun_star_frame_Bibliography \
	frameloaders/com_sun_star_sdb_ContentLoader \
	contenthandlers/com_sun_star_comp_framework_SoundHandler \
	contenthandlers/com_sun_star_comp_framework_oxt_handler \
)

# fcfg_database
$(call filter_Configuration_add_types,fcfg_langpack,fcfg_database_types.xcu,filter/source/config/fragments/types,\
	StarBase \
)

$(call filter_Configuration_add_filters,fcfg_langpack,fcfg_database_filters.xcu,filter/source/config/fragments/filters,\
	StarOffice_XML__Base_ \
)

$(call filter_Configuration_add_others,fcfg_langpack,fcfg_database_others.xcu,filter/source/config/fragments,\
	frameloaders/org_openoffice_comp_dbflt_DBContentLoader2 \
)

$(call filter_Configuration_add_ui_filters,fcfg_langpack,filter/source/config/fragments/filters,\
	StarOffice_XML__Base__ui \
)

# fcfg_writer
$(call filter_Configuration_add_types,fcfg_langpack,fcfg_writer_types.xcu,filter/source/config/fragments/types,\
	generic_HTML \
	calc_MS_Excel_40 \
	calc_MS_Excel_5095 \
	calc_MS_Excel_95 \
	writer_MS_WinWord_5 \
	writer_MS_WinWord_60 \
	writer_MS_Word_95 \
	writer_MS_Word_95_Vorlage \
	writer_MS_Word_97 \
	writer_MS_Word_97_Vorlage \
	writer_Rich_Text_Format \
	writer_StarOffice_XML_Writer \
	writer_WordPerfect_Document \
	writer_MS_Works_Document \
	writer_ClarisWorks \
	writer_DocMaker \
	writer_eDoc_Document \
	writer_FullWrite_Professional \
	writer_HanMac_Word_K \
	writer_LightWayText \
	writer_Mac_Word \
	writer_Mac_Works \
	writer_MacWrite \
	writer_MacWritePro \
	writer_Mariner_Write \
	writer_MindWrite \
	writer_Nisus_Writer \
	writer_TeachText \
	writer_TexEdit \
	writer_WriteNow \
	writer_WriterPlus \
	writer_ZWrite \
	writer_T602_Document \
	writer_LotusWordPro_Document \
	generic_Text \
	writer_MIZI_Hwp_97 \
	writer_StarOffice_XML_Writer_Template \
	pdf_Portable_Document_Format \
	writer8_template \
	writer8 \
	writer_MS_Word_2003_XML \
	writer_MS_Word_2007_XML \
	writer_MS_Word_2007_XML_Template \
	writer_OOXML \
	writer_OOXML_Template \
	writer_layout_dump_xml \
)

$(call filter_Configuration_add_filters,fcfg_langpack,fcfg_writer_filters.xcu,filter/source/config/fragments/filters,\
	HTML__StarWriter_ \
	MS_WinWord_5 \
	MS_WinWord_6_0 \
	MS_Word_95 \
	MS_Word_95_Vorlage \
	MS_Word_97 \
	MS_Word_97_Vorlage \
	Rich_Text_Format \
	StarOffice_XML__Writer_ \
	WordPerfect \
	MS_Works \
	ClarisWorks \
	DocMaker \
	eDoc_Document \
	FullWrite_Professional \
	HanMac_Word_K \
	LightWayText \
	Mac_Word \
	Mac_Works \
	MacWrite \
	MacWritePro \
	Mariner_Write \
	MindWrite \
	Nisus_Writer \
	TeachText \
	TexEdit \
	WriteNow \
	WriterPlus \
	ZWrite \
	T602Document \
	LotusWordPro \
	Text \
	Text__encoded_ \
	writer_MIZI_Hwp_97 \
	writer_StarOffice_XML_Writer_Template \
	writer_pdf_Export\
	writer8\
	writer8_template \
	MS_Word_2003_XML \
	MS_Word_2007_XML \
	MS_Word_2007_XML_Template \
	OOXML_Text \
	OOXML_Text_Template \
	writer_layout_dump \
)

$(call filter_Configuration_add_ui_filters,fcfg_langpack,filter/source/config/fragments/filters,\
	HTML__StarWriter__ui \
	MS_Word_95_Vorlage_ui \
	MS_Word_97_Vorlage_ui \
	StarOffice_XML__Writer__ui \
	Text_ui \
	Text__encoded__ui \
	writer_StarOffice_XML_Writer_Template_ui \
	writer8_ui \
	writer8_template_ui \
	MS_Word_2003_XML_ui \
	MS_Word_2007_XML_ui \
	MS_Word_2007_XML_Template_ui \
	OOXML_Text_ui \
	OOXML_Text_Template_ui \
)

# fcfg_web
$(call filter_Configuration_add_types,fcfg_langpack,fcfg_web_types.xcu,filter/source/config/fragments/types,\
	generic_HTML \
	generic_Text \
	writer_web_HTML_help \
	writer_StarOffice_XML_Writer \
	writer_web_StarOffice_XML_Writer_Web_Template \
	pdf_Portable_Document_Format \
	writerweb8_writer_template \
)

$(call filter_Configuration_add_filters,fcfg_langpack,fcfg_web_filters.xcu,filter/source/config/fragments/filters,\
	HTML \
	Text__StarWriter_Web_ \
	Text__encoded___StarWriter_Web_ \
	writer_web_HTML_help \
	writer_web_StarOffice_XML_Writer \
	writer_web_StarOffice_XML_Writer_Web_Template \
	writer_web_pdf_Export\
	writerweb8_writer_template\
	writerweb8_writer \
)

$(call filter_Configuration_add_ui_filters,fcfg_langpack,filter/source/config/fragments/filters,\
	HTML_ui \
	Text__StarWriter_Web__ui \
	Text__encoded___StarWriter_Web__ui \
	writer_web_StarOffice_XML_Writer_ui \
	writer_web_StarOffice_XML_Writer_Web_Template_ui \
	writerweb8_writer_template_ui \
	writerweb8_writer_ui \
)

# fcfg_global
$(call filter_Configuration_add_types,fcfg_langpack,fcfg_global_types.xcu,filter/source/config/fragments/types,\
	generic_Text \
	writer_StarOffice_XML_Writer \
	writer_globaldocument_StarOffice_XML_Writer_GlobalDocument \
	pdf_Portable_Document_Format \
	writerglobal8 \
)

$(call filter_Configuration_add_filters,fcfg_langpack,fcfg_global_filters.xcu,filter/source/config/fragments/filters,\
	Text__encoded___StarWriter_GlobalDocument_ \
	writer_globaldocument_StarOffice_XML_Writer \
	writer_globaldocument_StarOffice_XML_Writer_GlobalDocument \
	writer_globaldocument_pdf_Export \
	writerglobal8 \
	writerglobal8_writer \
	writerglobal8_HTML \
)

$(call filter_Configuration_add_ui_filters,fcfg_langpack,filter/source/config/fragments/filters,\
	Text__encoded___StarWriter_GlobalDocument__ui \
	writer_globaldocument_StarOffice_XML_Writer_ui \
	writer_globaldocument_StarOffice_XML_Writer_GlobalDocument_ui \
	writerglobal8_ui \
	writerglobal8_writer_ui \
)

# fcfg_calc
$(call filter_Configuration_add_types,fcfg_langpack,fcfg_calc_types.xcu,filter/source/config/fragments/types,\
	calc_DIF \
	generic_HTML \
	generic_Text \
	calc_Lotus \
	calc_QPro \
	calc_MS_Excel_40 \
	calc_MS_Excel_40_VorlageTemplate \
	calc_MS_Excel_5095 \
	calc_MS_Excel_5095_VorlageTemplate \
	calc_MS_Excel_95 \
	calc_MS_Excel_95_VorlageTemplate \
	calc_MS_Excel_97 \
	calc_MS_Excel_97_VorlageTemplate \
	writer_Rich_Text_Format \
	calc_SYLK \
	calc_StarOffice_XML_Calc \
	calc_StarOffice_XML_Calc_Template \
	pdf_Portable_Document_Format \
	calc_dBase\
	calc8 \
	calc8_template \
	calc_MS_Excel_2003_XML \
	MS_Excel_2007_XML \
	MS_Excel_2007_XML_Template \
	MS_Excel_2007_Binary \
	calc_OOXML \
	calc_OOXML_Template \
)

$(call filter_Configuration_add_filters,fcfg_langpack,fcfg_calc_filters.xcu,filter/source/config/fragments/filters,\
	DIF \
	HTML__StarCalc_ \
	Lotus \
	QPro \
	MS_Excel_4_0 \
	MS_Excel_4_0_Vorlage_Template \
	MS_Excel_5_0_95 \
	MS_Excel_5_0_95_Vorlage_Template \
	MS_Excel_95 \
	MS_Excel_95_Vorlage_Template \
	MS_Excel_97 \
	MS_Excel_97_Vorlage_Template \
	Rich_Text_Format__StarCalc_ \
	SYLK \
	StarOffice_XML__Calc_ \
	Text___txt___csv__StarCalc_ \
	calc_HTML_WebQuery \
	calc_StarOffice_XML_Calc_Template \
	calc_pdf_Export \
	dBase \
	calc8 \
	calc8_template \
	MS_Excel_2003_XML \
	calc_MS_Excel_2007_XML \
	calc_MS_Excel_2007_XML_Template \
	calc_MS_Excel_2007_Binary \
	calc_OOXML \
	calc_OOXML_Template \
)

$(call filter_Configuration_add_ui_filters,fcfg_langpack,filter/source/config/fragments/filters,\
	HTML__StarCalc__ui \
	MS_Excel_4_0_Vorlage_Template_ui \
	MS_Excel_5_0_95_Vorlage_Template_ui \
	MS_Excel_95_Vorlage_Template_ui \
	MS_Excel_97_Vorlage_Template_ui \
	StarOffice_XML__Calc__ui \
	Text___txt___csv__StarCalc__ui \
	calc_HTML_WebQuery_ui \
	calc_StarOffice_XML_Calc_Template_ui \
	calc8_ui \
	calc8_template_ui \
	MS_Excel_2003_XML_ui \
	calc_MS_Excel_2007_XML_ui \
	calc_MS_Excel_2007_XML_Template_ui \
	calc_MS_Excel_2007_Binary_ui \
	calc_OOXML_ui \
	calc_OOXML_Template_ui \
)

# fcfg_draw
$(call filter_Configuration_add_types,fcfg_langpack,fcfg_draw_types.xcu,filter/source/config/fragments/types,\
	draw_StarOffice_XML_Draw \
	draw_StarOffice_XML_Draw_Template \
	pdf_Portable_Document_Format \
	draw8 \
	draw8_template \
	draw_WordPerfect_Graphics \
	draw_Visio_Document \
    draw_Publisher_Document \
	draw_CorelDraw_Document \
	draw_Corel_Presentation_Exchange \
)

$(call filter_Configuration_add_filters,fcfg_langpack,fcfg_draw_filters.xcu,filter/source/config/fragments/filters,\
	StarOffice_XML__Draw_ \
	draw_StarOffice_XML_Draw_Template \
	draw_pdf_Export \
	draw8 \
	draw8_template \
	WordPerfectGraphics \
	VisioDocument \
    PublisherDocument \
	CorelDrawDocument \
	CorelPresentationExchange \
)

$(call filter_Configuration_add_ui_filters,fcfg_langpack,filter/source/config/fragments/filters,\
	StarOffice_XML__Draw__ui \
	draw_StarOffice_XML_Draw_Template_ui \
	draw8_ui \
	draw8_template_ui \
)

# fcfg_impress
$(call filter_Configuration_add_types,fcfg_langpack,fcfg_impress_types.xcu,filter/source/config/fragments/types,\
	draw_StarOffice_XML_Draw \
	impress_MS_PowerPoint_97 \
	impress_MS_PowerPoint_97_AutoPlay \
	impress_MS_PowerPoint_97_Vorlage \
	impress_StarOffice_XML_Impress \
	impress_StarOffice_XML_Impress_Template \
	pdf_Portable_Document_Format \
	pwp_PlaceWare \
	impress8 \
	impress8_template \
	draw8 \
	MS_PowerPoint_2007_XML \
	MS_PowerPoint_2007_XML_AutoPlay \
	MS_PowerPoint_2007_XML_Template \
	impress_OOXML_Presentation \
	impress_OOXML_Presentation_Template \
	impress_OOXML_Presentation_AutoPlay \
)

$(call filter_Configuration_add_filters,fcfg_langpack,fcfg_impress_filters.xcu,filter/source/config/fragments/filters,\
	MS_PowerPoint_97 \
	MS_PowerPoint_97_AutoPlay \
	MS_PowerPoint_97_Vorlage \
	impress_StarOffice_XML_Draw \
	StarOffice_XML__Impress_ \
	impress_StarOffice_XML_Impress_Template \
	impress_pdf_Export \
	placeware_Export \
	impress8 \
	impress8_template \
	impress8_draw \
	impress_MS_PowerPoint_2007_XML \
	impress_MS_PowerPoint_2007_XML_AutoPlay \
	impress_MS_PowerPoint_2007_XML_Template \
	impress_OOXML \
	impress_OOXML_Template \
	impress_OOXML_AutoPlay \
)

$(call filter_Configuration_add_ui_filters,fcfg_langpack,filter/source/config/fragments/filters,\
	MS_PowerPoint_97_Vorlage_ui \
	impress_StarOffice_XML_Draw_ui \
	StarOffice_XML__Impress__ui \
	impress_StarOffice_XML_Impress_Template_ui \
	impress8_ui \
	impress8_template_ui \
	impress8_draw_ui \
	impress_MS_PowerPoint_2007_XML_ui \
	impress_MS_PowerPoint_2007_XML_Template_ui \
	impress_OOXML_ui \
	impress_OOXML_Template_ui \
)

# fcfg_chart
$(call filter_Configuration_add_types,fcfg_langpack,fcfg_chart_types.xcu,filter/source/config/fragments/types,\
	chart_StarOffice_XML_Chart \
	chart8 \
)

$(call filter_Configuration_add_filters,fcfg_langpack,fcfg_chart_filters.xcu,filter/source/config/fragments/filters,\
	StarOffice_XML__Chart_ \
	chart8 \
)

$(call filter_Configuration_add_others,fcfg_langpack,fcfg_chart_others.xcu,filter/source/config/fragments,\
	frameloaders/com_sun_star_comp_chart2_ChartFrameLoader \
)

$(call filter_Configuration_add_ui_filters,fcfg_langpack,filter/source/config/fragments/filters,\
	StarOffice_XML__Chart__ui \
	chart8_ui \
)

# fcfg_math
$(call filter_Configuration_add_types,fcfg_langpack,fcfg_math_types.xcu,filter/source/config/fragments/types,\
	math_MathML_XML_Math \
	math_MathType_3x \
	math_StarOffice_XML_Math \
	pdf_Portable_Document_Format \
	math8 \
)

$(call filter_Configuration_add_filters,fcfg_langpack,fcfg_math_filters.xcu,filter/source/config/fragments/filters,\
	MathML_XML__Math_ \
	MathType_3_x \
	StarOffice_XML__Math_ \
	math_pdf_Export \
	math8 \
)

$(call filter_Configuration_add_ui_filters,fcfg_langpack,filter/source/config/fragments/filters,\
	StarOffice_XML__Math__ui \
	math8_ui \
)

# fcfg_drawgraphics
$(call filter_Configuration_add_types,fcfg_langpack,fcfg_drawgraphics_types.xcu,filter/source/config/fragments/types,\
	bmp_MS_Windows \
	dxf_AutoCAD_Interchange \
	emf_MS_Windows_Metafile \
	eps_Encapsulated_PostScript \
	gif_Graphics_Interchange \
	graphic_HTML \
	graphic_SWF \
	jpg_JPEG \
	met_OS2_Metafile \
	pbm_Portable_Bitmap \
	pcd_Photo_CD_Base \
	pcd_Photo_CD_Base16 \
	pcd_Photo_CD_Base4 \
	pct_Mac_Pict \
	pcx_Zsoft_Paintbrush \
	pgm_Portable_Graymap \
	png_Portable_Network_Graphic \
	ppm_Portable_Pixelmap \
	psd_Adobe_Photoshop \
	ras_Sun_Rasterfile \
	sgf_StarOffice_Writer_SGF \
	sgv_StarDraw_20 \
	svg_Scalable_Vector_Graphics \
	svm_StarView_Metafile \
	tga_Truevision_TARGA \
	tif_Tag_Image_File \
	wmf_MS_Windows_Metafile \
	xbm_X_Consortium \
	xpm_XPM \
)

$(call filter_Configuration_add_filters,fcfg_langpack,fcfg_drawgraphics_filters.xcu,filter/source/config/fragments/filters,\
	BMP___MS_Windows \
	DXF___AutoCAD_Interchange \
	EMF___MS_Windows_Metafile \
	EPS___Encapsulated_PostScript \
	GIF___Graphics_Interchange \
	JPG___JPEG \
	MET___OS_2_Metafile \
	PBM___Portable_Bitmap \
	PCT___Mac_Pict \
	PCX___Zsoft_Paintbrush \
	PGM___Portable_Graymap \
	PNG___Portable_Network_Graphic \
	PPM___Portable_Pixelmap \
	PSD___Adobe_Photoshop \
	RAS___Sun_Rasterfile \
	SGF___StarOffice_Writer_SGF \
	SGV___StarDraw_2_0 \
	SVG___Scalable_Vector_Graphics \
	SVM___StarView_Metafile \
	TGA___Truevision_TARGA \
	TIF___Tag_Image_File \
	WMF___MS_Windows_Metafile \
	XBM___X_Consortium \
	XPM \
	draw_PCD_Photo_CD_Base \
	draw_PCD_Photo_CD_Base16 \
	draw_PCD_Photo_CD_Base4 \
	draw_bmp_Export \
	draw_emf_Export \
	draw_eps_Export \
	draw_flash_Export \
	draw_gif_Export \
	draw_html_Export \
	draw_jpg_Export \
	draw_met_Export \
	draw_pbm_Export \
	draw_pct_Export \
	draw_pgm_Export \
	draw_png_Export \
	draw_ppm_Export \
	draw_ras_Export \
	draw_svg_Export \
	draw_svm_Export \
	draw_tif_Export \
	draw_wmf_Export \
	draw_xpm_Export \
)

$(call filter_Configuration_add_ui_filters,fcfg_langpack,filter/source/config/fragments/filters,\
	draw_html_Export_ui \
)

# fcfg_impressgraphics
$(call filter_Configuration_add_types,fcfg_langpack,fcfg_impressgraphics_types.xcu,filter/source/config/fragments/types,\
	bmp_MS_Windows \
	emf_MS_Windows_Metafile \
	eps_Encapsulated_PostScript \
	gif_Graphics_Interchange \
	graphic_HTML \
	graphic_SWF \
	impress_CGM_Computer_Graphics_Metafile \
	jpg_JPEG \
	met_OS2_Metafile \
	pbm_Portable_Bitmap \
	pct_Mac_Pict \
	pgm_Portable_Graymap \
	png_Portable_Network_Graphic \
	ppm_Portable_Pixelmap \
	ras_Sun_Rasterfile \
	svg_Scalable_Vector_Graphics \
	svm_StarView_Metafile \
	tif_Tag_Image_File \
	wmf_MS_Windows_Metafile \
	xpm_XPM \
)

$(call filter_Configuration_add_filters,fcfg_langpack,fcfg_impressgraphics_filters.xcu,filter/source/config/fragments/filters,\
	CGM___Computer_Graphics_Metafile \
	impress_bmp_Export \
	impress_emf_Export \
	impress_eps_Export \
	impress_flash_Export \
	impress_gif_Export \
	impress_html_Export \
	impress_jpg_Export \
	impress_met_Export \
	impress_pbm_Export \
	impress_pct_Export \
	impress_pgm_Export \
	impress_png_Export \
	impress_ppm_Export \
	impress_ras_Export \
	impress_svg_Export \
	impress_svm_Export \
	impress_tif_Export \
	impress_wmf_Export \
	impress_xpm_Export \
)

$(call filter_Configuration_add_ui_filters,fcfg_langpack,filter/source/config/fragments/filters,\
	impress_html_Export_ui \
)

# fcfg_writergraphics
$(call filter_Configuration_add_types,fcfg_langpack,fcfg_writergraphics_types.xcu,filter/source/config/fragments/types,\
	jpg_JPEG \
	png_Portable_Network_Graphic \
)

$(call filter_Configuration_add_filters,fcfg_langpack,fcfg_writergraphics_filters.xcu,filter/source/config/fragments/filters,\
	writer_jpg_Export \
	writer_png_Export \
)

# fcfg_internalgraphics
$(call filter_Configuration_add_types,fcfg_langpack,fcfg_internalgraphics_types.xcu,filter/source/config/fragments/types,\
	bmp_MS_Windows \
	dxf_AutoCAD_Interchange \
	emf_MS_Windows_Metafile \
	eps_Encapsulated_PostScript \
	gif_Graphics_Interchange \
	jpg_JPEG \
	met_OS2_Metafile \
	pbm_Portable_Bitmap \
	pcd_Photo_CD_Base \
	pcd_Photo_CD_Base16 \
	pcd_Photo_CD_Base4 \
	pct_Mac_Pict \
	pcx_Zsoft_Paintbrush \
	pgm_Portable_Graymap \
	png_Portable_Network_Graphic \
	ppm_Portable_Pixelmap \
	psd_Adobe_Photoshop \
	ras_Sun_Rasterfile \
	sgf_StarOffice_Writer_SGF \
	sgv_StarDraw_20 \
	svg_Scalable_Vector_Graphics \
	svm_StarView_Metafile \
	tga_Truevision_TARGA \
	tif_Tag_Image_File \
	wmf_MS_Windows_Metafile \
	xbm_X_Consortium \
	xpm_XPM \
)

$(call filter_Configuration_add_internal_filters,fcfg_langpack,fcfg_internalgraphics_filters.xcu,filter/source/config/fragments/internalgraphicfilters,\
	bmp_Export \
	bmp_Import \
	dxf_Import \
	emf_Export \
	emf_Import \
	eps_Export \
	eps_Import \
	gif_Export \
	gif_Import \
	jpg_Export \
	jpg_Import \
	met_Export \
	met_Import \
	pbm_Export \
	pbm_Import \
	pcd_Import_Base \
	pcd_Import_Base4 \
	pcd_Import_Base16 \
	pct_Export \
	pct_Import \
	pcx_Import \
	pgm_Export \
	pgm_Import \
	png_Export \
	png_Import \
	ppm_Export \
	ppm_Import \
	psd_Import \
	ras_Export \
	ras_Import \
	sgf_Import \
	sgv_Import \
	svg_Export \
	svg_Import \
	svm_Export \
	svm_Import \
	tga_Import \
	tif_Export \
	tif_Import \
	wmf_Export \
	wmf_Import \
	xbm_Import \
	xpm_Export \
	xpm_Import \
)

# fcfg_xslt
$(call filter_Configuration_add_types,fcfg_langpack,fcfg_xslt_types.xcu,filter/source/config/fragments/types,\
	calc_ODS_FlatXML \
	draw_ODG_FlatXML \
	impress_ODP_FlatXML \
	writer_DocBook_File \
	writer_ODT_FlatXML \
	XHTML_File \
	Unified_Office_Format_text \
	Unified_Office_Format_spreadsheet \
	Unified_Office_Format_presentation \
)

$(call filter_Configuration_add_filters,fcfg_langpack,fcfg_xslt_filters.xcu,filter/source/config/fragments/filters,\
	DocBook_File \
	ODG_FlatXML \
	ODP_FlatXML \
	ODS_FlatXML \
	ODT_FlatXML \
	XHTML_Calc_File \
	XHTML_Draw_File \
	XHTML_Impress_File \
	XHTML_Writer_File \
	UOF_text \
	UOF_spreadsheet \
	UOF_presentation \
)

# vim: set noet sw=4 ts=4:
