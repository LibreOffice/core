# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# class Gallery

# Handles creation of image galleries.

gb_Gallery_TRANSLATE := $(SRCDIR)/solenv/bin/desktop-translate.py
gb_Gallery_INSTDIR := $(LIBO_SHARE_FOLDER)/gallery

define gb_Gallery__command
$(call gb_Helper_abbreviate_dirs,\
	rm -f $(gb_Gallery_workdir)/$(2)/* && \
	RESPONSEFILE=$(call gb_var2file,$(shell $(call gb_MKTEMP)),$(GALLERY_FILES)) && \
	$(call gb_Helper_print_on_error,\
		$(call gb_Executable_get_command,gengal,$(ICECREAM_RUN)) \
			--build-tree \
			--destdir $(GALLERY_BASEDIR) \
			--name "$(GALLERY_NAME)" \
			--path $(gb_Gallery_workdir)/$(2) \
			--filenames $(call gb_Helper_make_url,$$RESPONSEFILE) \
			-env:UserInstallation=$(call gb_Helper_make_url,$(gb_Gallery_workdir)/$(2)/user),\
		$@.log \
	) && \
	rm $$RESPONSEFILE && \
	touch $@ \
)
endef

define gb_Gallery__command_str
cp -f $(GALLERY_STRFILE) $@ && \
$(call gb_ExternalExecutable_get_command,python) \
		$(gb_Gallery_TRANSLATE) \
		--ext "str" \
		-d $(GALLERY_WORKDIR) \
		$(GALLERY_ULFFILE)
endef

gb_Gallery__get_final_target = $(WORKDIR)/Gallery/$(1).final

$(dir $(call gb_Gallery_get_target,$(1))).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_Gallery_get_target,$(1)))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_Gallery_get_target,%) : $(call gb_Executable_get_target,gengal) \
        | $(call gb_Executable_get_runtime_dependencies,gengal)
	$(call gb_Output_announce,$*,$(true),GAL,1)
	$(call gb_Trace_StartRange,$*,GAL)
	$(call gb_Gallery__command,$@,$*)
	$(call gb_Trace_EndRange,$*,GAL)

$(call gb_Gallery__get_final_target,%) :
	touch $@

# difficult to determine source dep for this one...
$(gb_Gallery_workdir)/%.ulf : \
		$(call gb_Executable_get_runtime_dependencies,ulfex)
	$(call gb_CustomTarget_ulfex__command,$@,$(GALLERY_ULFFILE),\
		$(foreach lang,$(gb_TRANS_LANGS),\
			$(gb_POLOCATION)/$(lang)/extras/source/gallery/share.po))

$(gb_Gallery_workdir)/%.str : $(gb_Gallery_TRANSLATE) \
		$(call gb_ExternalExecutable_get_dependencies,python)
	$(call gb_Output_announce,$*,$(true),STR,1)
	$(call gb_Trace_StartRange,$*,STR)
	$(call gb_Gallery__command_str,$@,$*)
	$(call gb_Trace_EndRange,$*,STR)

.PHONY : $(call gb_Gallery_get_clean_target,%)
$(call gb_Gallery_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),GAL,1)
	$(call gb_Helper_abbreviate_dirs,\
		rm -rf \
			$(call gb_Gallery__get_final_target,$*) \
			$(call gb_Gallery_get_target,$*) \
			$(call gb_Gallery_get_target,$*).log \
			$(gb_Gallery_workdir)/$* \
	)

# the theme package
gb_Gallery_get_packagename = Gallery/$(1)
# the files package
gb_Gallery_get_files_packagename = Gallery/Files/$(1)

# Create a gallery.
#
# basedir less one directory will be stripped from paths of the files when they are
# inserted into the gallery.
#
# gb_Gallery_Gallery gallery basedir name
define gb_Gallery_Gallery
$(call gb_Gallery__Gallery_impl,$(1),$(call gb_Gallery_get_packagename,$(1)),$(2),$(3))

# setup the files package - we install all of these too
$(call gb_Package_Package_internal,$(call gb_Gallery_get_files_packagename,$(1)),$(SRCDIR)/$(2))
$(call gb_Gallery__get_final_target,$(1)) : $(call gb_Package_get_target,$(call gb_Gallery_get_files_packagename,$(1)))
$(call gb_Gallery_get_clean_target,$(1)) : $(call gb_Package_get_clean_target,$(call gb_Gallery_get_files_packagename,$(1)))

endef


gb_Gallery_basedir = $(patsubst %/,%,$(dir $(SRCDIR)/$(1)))

# TODO: we process the same ulf file for every gallery. That does not
# make sense.
#
# gb_Gallery__Gallery_impl gallery package basedir name
define gb_Gallery__Gallery_impl
$(call gb_Package_Package_internal,$(2),$(gb_Gallery_workdir)/$(1))
$(call gb_Package_add_file,$(2),$(gb_Gallery_INSTDIR)/$(1).sdg,$(1).sdg)
$(call gb_Package_add_file,$(2),$(gb_Gallery_INSTDIR)/$(1).sdv,$(1).sdv)
$(call gb_Package_add_file,$(2),$(gb_Gallery_INSTDIR)/$(1).thm,$(1).thm)
$(call gb_Package_add_file,$(2),$(gb_Gallery_INSTDIR)/$(1).str,$(1).str)

# strip URL, without / to help the internal gallery system
$(call gb_Gallery_get_target,$(1)) : GALLERY_BASEDIR := $(call gb_Helper_make_url,$(call gb_Gallery_basedir,$(3)))
$(call gb_Gallery_get_target,$(1)) : GALLERY_FILES :=
$(call gb_Gallery_get_target,$(1)) : GALLERY_NAME := $(1)
$(gb_Gallery_workdir)/$(1)/$(1).str : GALLERY_STRFILE := $(SRCDIR)/$(3)/$(1).str
$(gb_Gallery_workdir)/$(1)/$(1).str : GALLERY_ULFFILE := $(gb_Gallery_workdir)/$(1)/$(1).ulf
$(gb_Gallery_workdir)/$(1)/$(1).str : GALLERY_WORKDIR := $(gb_Gallery_workdir)/$(1)
$(gb_Gallery_workdir)/$(1)/$(1).ulf : GALLERY_BASEDIR := $(3)
$(gb_Gallery_workdir)/$(1)/$(1).ulf : GALLERY_ULFFILE := $(call gb_Gallery_basedir,$(3))/share/gallery_names.ulf

$(gb_Gallery_workdir)/$(1)/$(1).ulf : \
	$(call gb_Gallery_basedir,$(3))/share/gallery_names.ulf \
	$(call gb_Gallery_get_target,$(1)) # that rule pre-cleans our output directory

$(gb_Gallery_workdir)/$(1)/$(1).str : $(gb_Gallery_workdir)/$(1)/$(1).ulf

# order-only, the Gallery-Target also makes those files
$(addprefix $(gb_Gallery_workdir)/$(1)/$(1),.sdg .sdv .thm): | $(call gb_Gallery_get_target,$(1))
$(call gb_Gallery__get_final_target,$(1)) : $(call gb_Package_get_target,$(2))

$(call gb_Gallery_get_clean_target,$(1)) : $(call gb_Package_get_clean_target,$(2))
$(call gb_Gallery_get_target,$(1)) :| $(dir $(call gb_Gallery_get_target,$(1))).dir \
	$(gb_Gallery_workdir)/$(1)/.dir

$$(eval $$(call gb_Module_register_target,$(call gb_Gallery__get_final_target,$(1)),$(call gb_Gallery_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),Gallery,$(call gb_Gallery__get_final_target,$(1)))

endef

# Add a file to the gallery.
#
# The file is given by path relative to $(SRCDIR).
#
# gb_Gallery_add_file gallery file
define gb_Gallery_add_file
$(call gb_Gallery_get_target,$(1)) : $(SRCDIR)/$(3)
$(call gb_Gallery_get_target,$(1)) : GALLERY_FILES += $(call gb_Helper_make_url,$(SRCDIR)/$(3))
$(call gb_Package_add_file,$(call gb_Gallery_get_files_packagename,$(1)),$(2)/$(notdir $(3)),$(notdir $(3)))

endef

# Add several files to the gallery at once.
#
# The files are given by path relative to $(SRCDIR).
#
# gb_Gallery_add_files gallery file(s)
define gb_Gallery_add_files
$(foreach fname,$(3),$(call gb_Gallery_add_file,$(1),$(2),$(fname)))

endef

# vim: set noet sw=4 ts=4:
