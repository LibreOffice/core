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

gb_Gallery__UNO_COMPONENTS := \
	comphelper/util/comphelp \
	configmgr/source/configmgr \
	drawinglayer/drawinglayer \
	fileaccess/source/fileacc \
	framework/util/fwk \
	i18npool/util/i18npool \
	package/source/xstor/xstor \
	package/util/package2 \
	sax/source/expatwrap/expwrap \
	sfx2/util/sfx \
	svgio/svgio \
	svx/util/svx \
	svx/util/svxcore \
	ucb/source/core/ucb1 \
	ucb/source/ucp/file/ucpfile1 \
	unoxml/source/service/unoxml

gb_Gallery__UNO_TYPES := \
	offapi \
	udkapi

gb_Gallery_TRANSLATE := $(SOLARENV)/bin/desktop-translate.pl

# TODO: this should be in RepositoryExternal.mk, but it would lead to
# duplication. Fix.
gb_Gallery_EXTRA_DEPENCENCIES := \
	$(foreach component,$(gb_Gallery__UNO_COMPONENTS),$(call gb_ComponentTarget_get_target_for_build,$(component))) \
	$(foreach api,$(gb_Gallery__UNO_TYPES),$(call gb_UnoApi_get_target,$(api)))

gb_Gallery_INSTDIR := share/gallery

# BRAND_BASE_DIR is for resource files
define gb_Gallery__make_env_args
"-env:BRAND_BASE_DIR=$(call gb_Helper_make_url,$(OUTDIR_FOR_BUILD)/unittest/install)" \
"-env:CONFIGURATION_LAYERS=xcsxcu:$(call gb_Helper_make_url,$(gb_Configuration_registry)) \
	module:$(call gb_Helper_make_url,$(gb_Configuration_registry)/spool)" \
"-env:UNO_SERVICES=$(call gb_Helper_make_url,$(call gb_Rdb_get_outdir_target_for_build,ure/services)) \
	$(foreach item,$(gb_Gallery__UNO_COMPONENTS),\
		$(call gb_Helper_make_url,$(call gb_ComponentTarget_get_target_for_build,$(item))))" \
"-env:UNO_TYPES=$(foreach item,$(gb_Gallery__UNO_TYPES),\
	$(call gb_Helper_make_url,$(call gb_UnoApi_get_target,$(item))))" \
$(foreach dir,URE_INTERNAL_LIB_DIR LO_LIB_DIR,\
	-env:$(dir)=$(call gb_Helper_make_url,$(gb_Helper_OUTDIR_FOR_BUILDLIBDIR)))
endef

define gb_Gallery__command
$(call gb_Output_announce,$(2),$(true),GAL,1)
$(call gb_Helper_abbreviate_dirs,\
	rm -f $(call gb_Gallery_get_workdir,$(2))/* && \
	$(call gb_Helper_print_on_error,\
		$(if $(filter-out MACOSX WNT,$(OS_FOR_BUILD)),$(if $(ENABLE_HEADLESS),, \
			SAL_USE_VCLPLUGIN=svp \
		)) \
		$(call gb_Executable_get_command,gengal,$(ICECREAM_RUN)) \
			$(call gb_Gallery__make_env_args) \
			--build-tree \
			--destdir $(GALLERY_BASEDIR) \
			--name "$(GALLERY_NAME)" \
			--path $(call gb_Gallery_get_workdir,$(2)) \
			$(GALLERY_FILES),\
		$@.log \
	) && \
	touch $@ \
)
endef

define gb_Gallery__command_str
$(call gb_Output_announce,$(2),$(true),STR,1)
cp -f $(GALLERY_STRFILE) $@ && \
$(PERL) $(gb_Gallery_TRANSLATE) \
		--ext "str" --key "name" \
		-d $(GALLERY_WORKDIR) \
		$(GALLERY_ULFFILE)
endef

gb_Gallery__get_final_target = $(WORKDIR)/Gallery/$(1).final

$(dir $(call gb_Gallery_get_target,$(1))).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_Gallery_get_target,$(1)))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_Gallery_get_target,%) : \
		$(call gb_Executable_get_runtime_dependencies,gengal) \
		$(gb_Gallery_EXTRA_DEPENCENCIES)
	$(call gb_Gallery__command,$@,$*)

$(call gb_Gallery__get_final_target,%) :
	touch $@

# difficult to determine source dep for this one...
$(call gb_Gallery_get_workdir,%).ulf : \
		$(call gb_Executable_get_runtime_dependencies,ulfex)
	$(call gb_CustomTarget_ulfex__command,$@,$(GALLERY_ULFFILE),\
		$(foreach lang,$(gb_TRANS_LANGS),\
			$(gb_POLOCATION)/$(lang)/extras/source/gallery/share.po))

$(call gb_Gallery_get_workdir,%).str : $(gb_Gallery_TRANSLATE)
	$(call gb_Gallery__command_str,$@,$*)

.PHONY : $(call gb_Gallery_get_clean_target,%)
$(call gb_Gallery_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),GAL,1)
	$(call gb_Helper_abbreviate_dirs,\
		rm -rf \
			$(call gb_Gallery__get_final_target,$*) \
			$(call gb_Gallery_get_target,$*) \
			$(call gb_Gallery_get_target,$*).log \
			$(call gb_Gallery_get_workdir,$*) \
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
$(call gb_Package_set_outdir,$(call gb_Gallery_get_files_packagename,$(1)),$(INSTDIR))
$(call gb_Gallery__get_final_target,$(1)) : $(call gb_Package_get_target,$(call gb_Gallery_get_files_packagename,$(1)))
$(call gb_Gallery_get_clean_target,$(1)) : $(call gb_Package_get_clean_target,$(call gb_Gallery_get_files_packagename,$(1)))

endef


gb_Gallery_basedir = $(patsubst %/,%,$(dir $(SRCDIR)/$(1)))

# TODO: we process the same ulf file for every gallery. That does not
# make sense.
#
# gb_Gallery__Gallery_impl gallery package basedir name
define gb_Gallery__Gallery_impl
$(call gb_Package_Package_internal,$(2),$(call gb_Gallery_get_workdir,$(1)))
$(call gb_Package_set_outdir,$(2),$(INSTDIR))
$(call gb_Package_add_file,$(2),$(gb_Gallery_INSTDIR)/$(1).sdg,$(1).sdg)
$(call gb_Package_add_file,$(2),$(gb_Gallery_INSTDIR)/$(1).sdv,$(1).sdv)
$(call gb_Package_add_file,$(2),$(gb_Gallery_INSTDIR)/$(1).thm,$(1).thm)
$(call gb_Package_add_file,$(2),$(gb_Gallery_INSTDIR)/$(1).str,$(1).str)

# strip URL, without / to help the internal gallery system
$(call gb_Gallery_get_target,$(1)) : GALLERY_BASEDIR := $(call gb_Helper_make_url,$(call gb_Gallery_basedir,$(3)))
$(call gb_Gallery_get_target,$(1)) : GALLERY_FILES :=
$(call gb_Gallery_get_target,$(1)) : GALLERY_NAME := $(1)
$(call gb_Gallery_get_workdir,$(1))/$(1).str : GALLERY_STRFILE := $(SRCDIR)/$(3)/$(1).str
$(call gb_Gallery_get_workdir,$(1))/$(1).str : GALLERY_ULFFILE := $(call gb_Gallery_get_workdir,$(1))/$(1).ulf
$(call gb_Gallery_get_workdir,$(1))/$(1).str : GALLERY_WORKDIR := $(call gb_Gallery_get_workdir,$(1))
$(call gb_Gallery_get_workdir,$(1))/$(1).ulf : GALLERY_BASEDIR := $(3)
$(call gb_Gallery_get_workdir,$(1))/$(1).ulf : GALLERY_ULFFILE := $(call gb_Gallery_basedir,$(3))/share/gallery_names.ulf

$(call gb_Gallery_get_workdir,$(1))/$(1).ulf : \
	$(call gb_Gallery_basedir,$(3))/share/gallery_names.ulf \
	$(call gb_Gallery_get_target,$(1)) # that rule pre-cleans our output directory

$(call gb_Gallery_get_workdir,$(1))/$(1).str : $(call gb_Gallery_get_workdir,$(1))/$(1).ulf

$(call gb_Gallery_get_workdir,$(1))/$(1).sdg \
$(call gb_Gallery_get_workdir,$(1))/$(1).sdv \
$(call gb_Gallery_get_workdir,$(1))/$(1).thm : $(call gb_Gallery_get_target,$(1))
$(call gb_Gallery__get_final_target,$(1)) : $(call gb_Package_get_target,$(2))

$(call gb_Gallery_get_clean_target,$(1)) : $(call gb_Package_get_clean_target,$(2))
$(call gb_Gallery_get_target,$(1)) :| $(dir $(call gb_Gallery_get_target,$(1))).dir \
	$(call gb_Gallery_get_workdir,$(1))/.dir

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
