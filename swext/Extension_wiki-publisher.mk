# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#

$(eval $(call gb_Extension_Extension,wiki-publisher,swext/mediawiki/src))

$(eval $(call gb_Extension_use_unpacked,wiki-publisher,xsltml))

$(eval $(call gb_Extension_use_default_description,wiki-publisher,swext/mediawiki/src/description-en-US.txt))
$(eval $(call gb_Extension_use_default_license,wiki-publisher))

$(eval $(call gb_Extension_add_file,wiki-publisher,Addons.xcu,$(call gb_XcuFile_for_extension,swext/mediawiki/src/registry/data/org/openoffice/Office/Addons.xcu)))
$(eval $(call gb_Extension_add_file,wiki-publisher,Filter.xcu,$(SRCDIR)/swext/mediawiki/src/registry/data/org/openoffice/TypeDetection/Filter.xcu))
$(eval $(call gb_Extension_add_file,wiki-publisher,OptionsDialog.xcu,$(call gb_XcuFile_for_extension,swext/mediawiki/src/registry/data/org/openoffice/Office/OptionsDialog.xcu)))
$(eval $(call gb_Extension_add_file,wiki-publisher,Paths.xcu,$(SRCDIR)/swext/mediawiki/src/registry/data/org/openoffice/Office/Paths.xcu))
$(eval $(call gb_Extension_add_file,wiki-publisher,ProtocolHandler.xcu,$(SRCDIR)/swext/mediawiki/src/registry/data/org/openoffice/Office/ProtocolHandler.xcu))
$(eval $(call gb_Extension_add_file,wiki-publisher,Types.xcu,$(SRCDIR)/swext/mediawiki/src/registry/data/org/openoffice/TypeDetection/Types.xcu))
$(eval $(call gb_Extension_add_file,wiki-publisher,WikiExtension.xcs,$(SRCDIR)/swext/mediawiki/src/registry/schema/org/openoffice/Office/Custom/WikiExtension.xcs))
$(eval $(call gb_Extension_add_file,wiki-publisher,WikiExtension.xcu,$(call gb_XcuFile_for_extension,swext/mediawiki/src/registry/data/org/openoffice/Office/Custom/WikiExtension.xcu)))
$(eval $(call gb_Extension_add_file,wiki-publisher,mediawiki.jar,$(call gb_Jar_get_target,mediawiki)))
$(eval $(call gb_Extension_add_file,wiki-publisher,components.rdb,$(SRCDIR)/swext/mediawiki/src/components.rdb))
$(eval $(call gb_Extension_add_file,wiki-publisher,WikiEditor/EditSetting.xdl,$(SRCDIR)/swext/mediawiki/dialogs/EditSetting.xdl))
$(eval $(call gb_Extension_add_file,wiki-publisher,WikiEditor/Settings.xdl,$(SRCDIR)/swext/mediawiki/dialogs/Settings.xdl))
$(eval $(call gb_Extension_add_file,wiki-publisher,WikiEditor/Module1.xba,$(SRCDIR)/swext/mediawiki/dialogs/Module1.xba))
$(eval $(call gb_Extension_add_file,wiki-publisher,WikiEditor/script.xlb,$(SRCDIR)/swext/mediawiki/dialogs/script.xlb))
$(eval $(call gb_Extension_add_file,wiki-publisher,WikiEditor/dialog.xlb,$(SRCDIR)/swext/mediawiki/dialogs/dialog.xlb))
$(eval $(call gb_Extension_add_file,wiki-publisher,WikiEditor/SendToMediaWiki.xdl,$(SRCDIR)/swext/mediawiki/dialogs/SendToMediaWiki.xdl))
$(eval $(call gb_Extension_add_file,wiki-publisher,filter/math/cmarkup.xsl,$(gb_UnpackedTarball_workdir)/xsltml/cmarkup.xsl))
$(eval $(call gb_Extension_add_file,wiki-publisher,filter/math/entities.xsl,$(gb_UnpackedTarball_workdir)/xsltml/entities.xsl))
$(eval $(call gb_Extension_add_file,wiki-publisher,filter/math/glayout.xsl,$(gb_UnpackedTarball_workdir)/xsltml/glayout.xsl))
$(eval $(call gb_Extension_add_file,wiki-publisher,filter/math/mmltex.xsl,$(gb_UnpackedTarball_workdir)/xsltml/mmltex.xsl))
$(eval $(call gb_Extension_add_file,wiki-publisher,filter/math/scripts.xsl,$(gb_UnpackedTarball_workdir)/xsltml/scripts.xsl))
$(eval $(call gb_Extension_add_file,wiki-publisher,filter/math/tables.xsl,$(gb_UnpackedTarball_workdir)/xsltml/tables.xsl))
$(eval $(call gb_Extension_add_file,wiki-publisher,filter/math/tokens.xsl,$(gb_UnpackedTarball_workdir)/xsltml/tokens.xsl))
$(eval $(call gb_Extension_add_file,wiki-publisher,filter/odt2mediawiki.xsl,$(SRCDIR)/swext/mediawiki/src/filter/odt2mediawiki.xsl))
$(eval $(call gb_Extension_add_file,wiki-publisher,templates/MediaWiki/mediawiki.ott,$(gb_CustomTarget_workdir)/mediawiki/template/mediawiki.ott))
$(eval $(call gb_Extension_add_helpfile,wiki-publisher,$(SRCDIR)/swext/mediawiki/help,com.sun.wiki-publisher/wikisend.xhp,wikisend.xhp))
$(eval $(call gb_Extension_add_helpfile,wiki-publisher,$(SRCDIR)/swext/mediawiki/help,com.sun.wiki-publisher/wikiformats.xhp,wikiformats.xhp))
$(eval $(call gb_Extension_add_helpfile,wiki-publisher,$(SRCDIR)/swext/mediawiki/help,com.sun.wiki-publisher/wikiaccount.xhp,wikiaccount.xhp))
$(eval $(call gb_Extension_add_helpfile,wiki-publisher,$(SRCDIR)/swext/mediawiki/help,com.sun.wiki-publisher/wiki.xhp,wiki.xhp))
$(eval $(call gb_Extension_add_helpfile,wiki-publisher,$(SRCDIR)/swext/mediawiki/help,com.sun.wiki-publisher/wikisettings.xhp,wikisettings.xhp))

$(eval $(call gb_Extension_add_helptreefile,wiki-publisher,$(SRCDIR)/swext/mediawiki/help,help.tree,help.tree,com.sun.wiki-publisher))

# Build mediawiki.ott
define run_zip_template_recipe =
$(call gb_Output_announce,$(subst $(gb_CustomTarget_workdir)/mediawiki/template/,,$@),$(true),ZIP,2)
$(call gb_Trace_StartRange,$(subst $(gb_CustomTarget_workdir)/mediawiki/template/,,$@),ZIP)
cd $(dir $<) && \
$(call gb_Helper_wsl_path,\
$(WSL) zip -q0X --filesync --must-match $@ mimetype && \
$(WSL) zip -qrX --must-match $@ $(subst $(dir $<),,$^) -x mimetype) \
$(call gb_Helper_make_zip_deterministic,$@)
$(call gb_Trace_EndRange,$(subst $(gb_CustomTarget_workdir)/mediawiki/template/,,$@),ZIP)
endef

.SECONDEXPANSION:
# secondexpansion since the patterns not just cover a filename portion, but also include a
# directory portion with different number of elements
# copy regular files (mimetype, *.jpg, *.png, *.rdf, *.svg, *.svm, …)
$(gb_CustomTarget_workdir)/mediawiki/template/% : $(SRCDIR)/swext/mediawiki/src/filter/% \
        | $$(dir $(gb_CustomTarget_workdir)/mediawiki/template/$$*).dir
	$(call gb_Output_announce,templates/$*,$(true),CPY,1)
	$(call gb_Trace_StartRange,templates/$*,CPY)
	cp $< $@
	$(call gb_Trace_EndRange,templates/$*,CPY)

# test and copy xml files
$(gb_CustomTarget_workdir)/mediawiki/template/%.xml : $(SRCDIR)/swext/mediawiki/src/filter/%.xml \
        | $(call gb_ExternalExecutable_get_dependencies,xsltproc) \
          $$(dir $(gb_CustomTarget_workdir)/mediawiki/template/$$*.xml).dir
	$(call gb_Output_announce,templates/$*.xml,$(true),XSL,1)
	$(call gb_Trace_StartRange,templates/$*.xml,XSL)
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet -o $@ $(SRCDIR)/extras/util/compact.xsl $<
	$(call gb_Trace_EndRange,templates/$*.xml,XSL)


$(gb_CustomTarget_workdir)/mediawiki/template/%.ott : \
        $$(addprefix $(gb_CustomTarget_workdir)/mediawiki/template/$$*/,\
            mimetype META-INF/manifest.xml content.xml meta.xml styles.xml \
            settings.xml manifest.rdf Thumbnails/thumbnail.png )
	$(run_zip_template_recipe)



# vim: set noet sw=4 ts=4:
