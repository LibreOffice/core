# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

gb_UnoApi_DOCDIR := $(gb_Package_SDKDIRNAME)/docs/common/ref
# NOTE: this is the output dir used in odk/CustomTarget_autodoc.mk
gb_UnoApi_SRCDOCDIR := $(call gb_CustomTarget_get_workdir,odk/docs/common/ref)
gb_UnoApi_ENABLE_INSTALL := $(and $(filter host,$(gb_Side)),$(filter ODK,$(BUILD_TYPE)))

# NOTE: This is needed temporarily to force rebuild with API files from
# $(WORKDIR), thus fixing generated deps. Otherwise, a change of an .idl
# file would not rebuild a .cxx if it still depended on the $(OUTDIR)
# version of the header.
define gb_UnoApi__make_outdir_headers_rule
$(OUTDIR)/inc/$(1)/%.hdl $(OUTDIR)/inc/$(1)/%.hpp :
	@true
endef

.PHONY : $(call gb_UnoApi_get_clean_target,%)
$(call gb_UnoApi_get_clean_target,%) :
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_UnoApi_get_target,$*))

define gb_UnoApi_UnoApi
$(call gb_UnoApiTarget_UnoApiTarget,$(1))
$(call gb_UnoApiHeadersTarget_UnoApiHeadersTarget,$(1))
ifneq ($(gb_UnoApi_ENABLE_INSTALL),)
$(call gb_Package_Package_internal,$(1)_idl,$(SRCDIR))
$(call gb_Package_set_outdir,$(1)_idl,$(INSTDIR))
$(call gb_Package_Package_internal,$(1)_doc,$(gb_UnoApi_SRCDOCDIR))
$(call gb_Package_set_outdir,$(1)_doc,$(INSTDIR))
endif

$(call gb_UnoApiTarget_set_root,$(1),UCR)

$(call gb_UnoApi_get_target,$(1)) :| $(dir $(call gb_UnoApi_get_target,$(1))).dir
$(call gb_UnoApi_get_target,$(1)) : $(call gb_UnoApiTarget_get_target,$(1))
$(call gb_UnoApi_get_target,$(1)) : $(call gb_UnoApiHeadersTarget_get_target,$(1))
$(call gb_UnoApi_get_clean_target,$(1)) : $(call gb_UnoApiTarget_get_clean_target,$(1))
$(call gb_UnoApi_get_clean_target,$(1)) : $(call gb_UnoApiHeadersTarget_get_clean_target,$(1))

$(call gb_UnoApi__make_outdir_headers_rule,$(1))

$(call gb_Deliver_add_deliverable,$(call gb_UnoApi_get_target,$(1)),$(call gb_UnoApiTarget_get_target,$(1)),$(1))

$$(eval $$(call gb_Module_register_target,$(call gb_UnoApi_get_target,$(1)),$(call gb_UnoApi_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),UnoApi)

endef

ifneq ($(gb_UnoApi_ENABLE_INSTALL),)

# Create a package of IDL files for putting into SDK.
#
# gb_UnoApi_package_idlfiles api
define gb_UnoApi_package_idlfiles
$(call gb_UnoApi_get_target,$(1)) : $(call gb_Package_get_target,$(1)_idl)
$(call gb_UnoApi_get_clean_target,$(1)) : $(call gb_Package_get_clean_target,$(1)_idl)

endef

define gb_UnoApi__add_idlfile
$(call gb_Package_add_file,$(1)_idl,$(patsubst $(1)/%,$(gb_Package_SDKDIRNAME)/idl/%,$(2)),$(2))

endef

else # !gb_UnoApi_ENABLE_INSTALL

gb_UnoApi_package_idlfiles :=
gb_UnoApi__add_idlfile :=

endif

define gb_UnoApi__add_headerfile_impl
$(call gb_UnoApiHeadersTarget_add_headerfile,$(1),$(2),$(3))

endef

# Note: the expression in 3rd arg of call of gb_UnoApi__add_headerfile_impl
# removes the leftmost dir in the subpath .-)
define gb_UnoApi__add_headerfile
$(call gb_UnoApi__add_headerfile_impl,$(1),$(2),$(subst $() $(),/,$(wordlist 2,$(words $(subst /, ,$(2))),$(subst /, ,$(2)))))

endef

ifneq ($(gb_UnoApi_ENABLE_INSTALL),)

# gb_UnoApi__add_docfile_impl api htmlfile
define gb_UnoApi__add_docfile_impl
$(if $(filter-out com ooo org,$(firstword $(subst /, ,$(2)))),\
	$(call gb_Output_error,gb_UnoApi__add_docfile_impl: unknown namespace: $(firstword $(subst /, ,$(2)))) \
)
$(call gb_Package_add_file,$(1)_doc,$(gb_UnoApi_DOCDIR)/$(2),$(2))

endef

# gb_UnoApi__add_docfile_direct api htmlfile
define gb_UnoApi__add_docfile_direct
$(call gb_UnoApi__add_docfile_impl,$(1),$(patsubst $(1)/%,%,$(2)))

endef

# gb_UnoApi__add_docfile_for_idl api idlfile has-xref?
define gb_UnoApi__add_docfile_for_idl
$(if $(filter-out modules %-modules,$(basename $(notdir $(2)))),\
	$(call gb_UnoApi__add_docfile_direct,$(1),$(2).html) \
	$(if $(3),$(call gb_UnoApi__add_docfile_direct,$(1),$(2)-xref.html)) \
)

endef

# gb_UnoApi__add_docfile api idlfile has-xref?
define gb_UnoApi__add_docfile
$(call gb_UnoApi__add_docfile_for_idl,$(1),$(2),$(3))

endef

# gb_UnoApi__add_docfiles api dir idlfiles has-xref?
define gb_UnoApi__add_docfiles
$(foreach idlfile,$(3),$(call gb_UnoApi__add_docfile_for_idl,$(1),$(2)/$(idlfile),$(3)))

endef

else # !gb_UnoApi_ENABLE_INSTALL

gb_UnoApi__add_docfile :=
gb_UnoApi__add_docfiles :=

endif

define gb_UnoApi__add_idlfile_noheader
$(call gb_UnoApi__add_idlfile,$(1),$(2).idl)

endef

define gb_UnoApi__add_idlfile_nohdl
$(call gb_UnoApi__add_idlfile_noheader,$(1),$(2))
$(call gb_UnoApi__add_headerfile,$(1),$(2).hpp)

endef

define gb_UnoApi__add_idlfile_full
$(call gb_UnoApi__add_idlfile_nohdl,$(1),$(2))
$(call gb_UnoApi__add_headerfile,$(1),$(2).hdl)

endef

# for interfaces, exceptions, structs, enums, constant groups
define gb_UnoApi_add_idlfile
$(call gb_UnoApiTarget_add_idlfile,$(1),$(2),$(3))
$(call gb_UnoApi__add_idlfile_full,$(1),$(2)/$(3))
$(call gb_UnoApi__add_docfile,$(1),$(2)/$(3),$(true))

endef

define gb_UnoApi_add_idlfiles
$(call gb_UnoApiTarget_add_idlfiles,$(1),$(2),$(3))
$(foreach idl,$(3),$(call gb_UnoApi__add_idlfile_full,$(1),$(2)/$(idl)))
$(call gb_UnoApi__add_docfiles,$(1),$(2),$(3),$(true))

endef

# for new-style services
define gb_UnoApi_add_idlfile_nohdl
$(call gb_UnoApiTarget_add_idlfile,$(1),$(2),$(3))
$(call gb_UnoApi__add_idlfile_nohdl,$(1),$(2)/$(3))
$(call gb_UnoApi__add_docfile,$(1),$(2)/$(3))

endef

define gb_UnoApi_add_idlfiles_nohdl
$(call gb_UnoApiTarget_add_idlfiles,$(1),$(2),$(3))
$(foreach idl,$(3),$(call gb_UnoApi__add_idlfile_nohdl,$(1),$(2)/$(idl)))
$(call gb_UnoApi__add_docfiles,$(1),$(2),$(3))

endef

# for old-style services and modules
define gb_UnoApi_add_idlfile_noheader
$(call gb_UnoApiTarget_add_idlfile,$(1),$(2),$(3))
$(call gb_UnoApi__add_idlfile_noheader,$(1),$(2)/$(3))
$(call gb_UnoApi__add_docfile,$(1),$(2)/$(3))

endef

define gb_UnoApi_add_idlfiles_noheader
$(call gb_UnoApiTarget_add_idlfiles,$(1),$(2),$(3))
$(foreach idl,$(3),$(call gb_UnoApi__add_idlfile_noheader,$(1),$(2)/$(idl)))
$(call gb_UnoApi__add_docfiles,$(1),$(2),$(3))

endef

define gb_UnoApi__use_api
$(call gb_UnoApiTarget_use_api,$(1),$(2))
$(call gb_UnoApiHeadersTarget_use_api,$(1),$(2))
$(call gb_UnoApi_get_target,$(1)) :| $(call gb_UnoApi_get_target,$(2))

endef

define gb_UnoApi_use_api
$(foreach rdb,$(2),$(call gb_UnoApi__use_api,$(1),$(rdb)))

endef

define gb_UnoApi_add_reference_rdbfile
$$(call gb_Output_error,gb_UnoApi_add_reference_rdbfile: use gb_UnoApi_set_reference_rdbfile instead.)

endef

define gb_UnoApi_set_reference_rdbfile
$(call gb_UnoApiTarget_set_reference_rdbfile,$(1),$(2))

endef

define gb_UnoApi_set_include
$(call gb_UnoApiTarget_set_include,$(1),$(2))

endef

# vim: set noet sw=4 ts=4:
