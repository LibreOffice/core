# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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
# Major Contributor(s):
# Copyright (C) 2010 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

gb_TypesRdb_REGMERGETARGET := $(call gb_Executable_get_target_for_build,regmerge)
gb_TypesRdb_REGMERGECOMMAND := $(gb_Helper_set_ld_path) SOLARBINDIR=$(OUTDIR_FOR_BUILD)/bin $(gb_TypesRdb_REGMERGETARGET)

define gb_TypesRdb__command_impl
RESPONSEFILE=$(call var2file,$(shell $(gb_MKTEMP)),500,$(call gb_Helper_convert_native,$(1) $(2) $(3))) && \
$(gb_TypesRdb_REGMERGECOMMAND) @$${RESPONSEFILE} && \
rm -f $${RESPONSEFILE}
endef

define gb_TypesRdb__command
$(call gb_Output_announce,$*,$(true),RDB,3)
mkdir -p $(dir $(1)) && \
$(if $(TYPESRDB_FILES),$(call gb_TypesRdb__command_impl,$(1),UCR,$(TYPESRDB_FILES)),true) && \
$(if $(TYPESRDB_MERGE),$(call gb_TypesRdb__command_impl,$(1),/,$(TYPESRDB_MERGE)),true)
endef

$(call gb_TypesRdb_get_outdir_target,%) :
	$(call gb_Deliver_deliver,$(call gb_TypesRdb_get_target,$*),$@)

$(call gb_TypesRdb_get_target,%) :
	$(call gb_TypesRdb__command,$@,$*,$<,$?)

.PHONY : $(call gb_TypesRdb_get_clean_target,%)
$(call gb_TypesRdb_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),RDB,3)
	$(call gb_Helper_convert_native,\
		rm -f $(call gb_TypesRdb_get_target,$*) $(call gb_TypesRdb_get_outdir_target,$*))

define gb_TypesRdb_TypesRdb
$(call gb_TypesRdb_get_outdir_target,$(1)) : $(call gb_TypesRdb_get_target,$(1))
$(call gb_TypesRdb_get_target,$(1)) : TYPESRDB_FILES :=
$(call gb_TypesRdb_get_target,$(1)) : TYPESRDB_MERGE :=

$(call gb_Deliver_add_deliverable,$(call gb_TypesRdb_get_outdir_target,$(1)),$(call gb_TypesRdb_get_target,$(1)),$(1))

$$(eval $$(call gb_Module_register_target,$(call gb_TypesRdb_get_outdir_target,$(1)),$(call gb_TypesRdb_get_clean_target,$(1))))

endef

define gb_TypesRdb_add_urdfile
$(call gb_TypesRdb_get_target,$(1)) : TYPESRDB_FILES += $(2)
$(call gb_TypesRdb_get_target,$(1)) : $(2)

endef

define gb_TypesRdb_add_urdfiles
$(foreach urd,$(2),$(call gb_TypesRdb_add_urdfile,$(1),$(urd)))

endef

define gb_TypesRdb_add_rdbfile
$(call gb_TypesRdb_get_target,$(1)) : TYPESRDB_MERGE += $(call gb_TypesRdb_get_outdir_target,$(2))
$(call gb_TypesRdb_get_target,$(1)) : $(call gb_TypesRdb_get_outdir_target,$(2))

endef

define gb_TypesRdb_add_rdbfiles
$(foreach rdb,$(2),$(call gb_TypesRdb_add_rdbfile,$(1),$(rdb)))

endef

# vim: set noet sw=4 ts=4:
