# vim: set noet sw=4 ts=4:
# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,static/wasm_fs_image))

# WASM assumes the image has the same layout then instdir, so everything you want
# to include must be "installed" somewhere in it, as there is no mechanism to easily
# change paths or rename them (you could hack soffice.data.js.metadata).
# We make them relative paths as input for the file_packager.py!
gb_wasm_image_filelist := \
    $(call gb_UnoApi_get_target,offapi) \
    $(call gb_UnoApi_get_target,udkapi) \
    $(INSTROOT)/$(LIBO_ETC_FOLDER)/$(call gb_Helper_get_rcfile,fundamental) \
    $(INSTROOT)/$(LIBO_ETC_FOLDER)/$(call gb_Helper_get_rcfile,louno) \
    $(INSTROOT)/$(LIBO_ETC_FOLDER)/services/services.rdb \
    $(INSTROOT)/$(LIBO_URE_ETC_FOLDER)/$(call gb_Helper_get_rcfile,uno) \
    $(INSTROOT)/$(LIBO_URE_MISC_FOLDER)/services.rdb \

wasm_fs_image_WORKDIR := $(call gb_CustomTarget_get_workdir,static/wasm_fs_image)

# we just need data.js.link at link time, which is equal to soffice.data.js
$(call gb_CustomTarget_get_target,static/wasm_fs_image): \
    $(wasm_fs_image_WORKDIR)/soffice.data \
    $(wasm_fs_image_WORKDIR)/soffice.data.js.link \
    $(wasm_fs_image_WORKDIR)/soffice.data.js.metadata \

$(wasm_fs_image_WORKDIR)/soffice.data $(wasm_fs_image_WORKDIR)/soffice.data.js : $(wasm_fs_image_WORKDIR)/soffice.data.js.metadata

$(wasm_fs_image_WORKDIR)/soffice.data.js.link: $(wasm_fs_image_WORKDIR)/soffice.data.js
	cp $^ $^.tmp
	$(call gb_Helper_replace_if_different_and_touch,$^.tmp,$@)

$(wasm_fs_image_WORKDIR)/soffice.data.filelist: $(gb_wasm_image_filelist) | $(wasm_fs_image_WORKDIR)/.dir
	TEMPFILE=$(call var2file,$(shell $(gb_MKTEMP)),1,$(subst $(BUILDDIR)/,,$^)) && \
	    mv $$TEMPFILE $@.tmp
	$(call gb_Helper_replace_if_different_and_touch,$@.tmp,$@)

# Unfortunatly the file packager just allows a cmdline file list, but all paths are
# relative to $(BUILDDIR), so we won't run out of cmdline space that fast...
$(wasm_fs_image_WORKDIR)/soffice.data.js.metadata: $(wasm_fs_image_WORKDIR)/soffice.data.filelist \
	$(call gb_Output_announce,$(subst $(BUILDDIR)/,,$(wasm_fs_image_WORKDIR)/soffice.data),$(true),GEN,2)
	$(EMSDK_FILE_PACKAGER) $(wasm_fs_image_WORKDIR)/soffice.data --preload $(shell cat $^) --js-output=$(wasm_fs_image_WORKDIR)/soffice.data.js --separate-metadata \
	    || rm -f $(wasm_fs_image_WORKDIR)/soffice.data.js $(wasm_fs_image_WORKDIR)/soffice.data $(wasm_fs_image_WORKDIR)/soffice.data.js.metadata

# vim: set noet sw=4:
