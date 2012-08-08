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

define gb_Rdb__command
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $@) && \
	(\
		echo '<list>' && \
		$(foreach component,$(COMPONENTS),echo "<filename>$(call gb_ComponentTarget_get_outdir_target,$(component))</filename>" &&) \
		echo '</list>' \
	) > $(1).input && \
	$(gb_XSLTPROC) --nonet -o $(1) $(SOLARENV)/bin/packcomponents.xslt $(1).input && \
	rm $(1).input)
endef

$(call gb_Rdb_get_target,%) :
	$(call gb_Output_announce,$*,$(true),RDB,1)
	$(call gb_Rdb__command,$@,$*,$?,$^)

.PHONY : $(call gb_Rdb_get_clean_target,%)
$(call gb_Rdb_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),RDB,1)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_Rdb_get_outdir_target,$*) $(call gb_Rdb_get_target,$*))

# DO NOT DEFINE: it overwrites the definition from Package.mk for xml/
# and doesn't work on 3.81
#$(dir $(call gb_Rdb_get_outdir_target,))%/.dir :
#	mkdir -p $(dir $@)

$(call gb_Rdb_get_outdir_target,%) :
	$(call gb_Deliver_deliver,$<,$@)

define gb_Rdb_Rdb
$(call gb_Rdb_get_target,$(1)) : COMPONENTS :=
$(call gb_Rdb_get_clean_target,$(1)) : COMPONENTS :=
$(call gb_Rdb_get_outdir_target,$(1)) : $(call gb_Rdb_get_target,$(1)) \
	| $(dir $(call gb_Rdb_get_outdir_target,$(1))).dir
$(call gb_Deliver_add_deliverable,$(call gb_ResTarget_get_outdir_target,$(1)),$(call gb_Rdb_get_target,$(1)),$(1))

$$(eval $$(call gb_Module_register_target,$(call gb_Rdb_get_outdir_target,$(1)),$(call gb_Rdb_get_clean_target,$(1))))
endef

define gb_Rdb_add_component
$(call gb_Rdb_get_target,$(1)) : $(call gb_ComponentTarget_get_outdir_target,$(2))
$(call gb_Rdb_get_target,$(1)) : COMPONENTS += $(2)
$(call gb_Rdb_get_clean_target,$(1)) : COMPONENTS += $(2)

endef

define gb_Rdb_add_components
$(foreach component,$(2),$(call gb_Rdb_add_component,$(1),$(component)))

endef

# vim: set noet sw=4 ts=4:
