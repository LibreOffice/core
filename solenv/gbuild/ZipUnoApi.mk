# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# ZipUnoApi class
# this is a bit of a hack, hopefully needed only temporarily because
# scp2 can easily deal with zip files but not so easily with 100s of
# individual files; ideally the Package itself should be sufficient.

define gb_ZipUnoApi_ZipUnoApi
$(call gb_UnoApi_UnoApi,$(1),$(2))
$(call gb_Zip_Zip_internal,$(1),$(WORKDIR)/Zip/$(1))
$(call gb_UnoApi_get_target,$(1)) : $(call gb_Zip_get_outdir_bin_target,$(1))
$(call gb_Zip_get_outdir_bin_target,$(1)) : $(call gb_Zip_get_target,$(1))
	cp $$< $$@
$(call gb_Zip_get_outdir_bin_target,$(1)) :| $(dir $(call gb_Zip_get_outdir_bin_target,$(1))).dir
$(call gb_UnoApi_get_clean_target,$(1)) : $(call gb_Zip_get_clean_target,$(1))
$(call gb_UnoApiTarget_get_headers_target,$(1)) : $(call gb_Zip_get_target,$(1))
$(call gb_Deliver_add_deliverable,$(call gb_Zip_get_outdir_bin_target,$(1)),$(call gb_Zip_get_target,$(1)),$(1))

endef

# hard-code "idl" path in zip so we don't need another parameter
define gb_ZipUnoApi__add_idlfiles
$(foreach file,$(3),$(call gb_Zip_add_file,$(1),$(patsubst $(1)/%,idl/%,$(2))/$(file).idl,$(SRCDIR)/$(2)/$(file).idl))

endef

# sadly source files in udkapi/ but ODK path is idl/ so need to
# copy files
# $(1) package
# $(2) dir in source
# $(3) file list
define gb_ZipUnoApi_add_idlfiles
$(call gb_UnoApi_add_idlfiles,$(1),$(2),$(3))
$(call gb_ZipUnoApi__add_idlfiles,$(1),$(2),$(3))

endef

define gb_ZipUnoApi_add_idlfiles_nohdl
$(call gb_UnoApi_add_idlfiles_nohdl,$(1),$(2),$(3))
$(call gb_ZipUnoApi__add_idlfiles,$(1),$(2),$(3))

endef

define gb_ZipUnoApi_add_idlfiles_noheader
$(call gb_UnoApi_add_idlfiles_noheader,$(1),$(2),$(3))
$(call gb_ZipUnoApi__add_idlfiles,$(1),$(2),$(3))

endef

# vim: set noet sw=4 ts=4:
