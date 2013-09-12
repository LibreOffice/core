# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

gb_Rdb__get_install_target = $(gb_INSTROOT)/$(gb_PROGRAMDIRNAME)/services/$(1).rdb

define gb_Rdb__command
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $@) && \
	(\
		echo '<list>' && \
		$(foreach component,$(COMPONENTS),echo "<filename>$(call gb_ComponentTarget_get_target,$(component))</filename>" &&) \
		echo '</list>' \
	) > $(1).input && \
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet -o $(1) $(SOLARENV)/bin/packcomponents.xslt $(1).input && \
	rm $(1).input)
endef

$(call gb_Rdb_get_target,%) :| $(call gb_ExternalExecutable_get_dependencies,xsltproc)
	$(call gb_Output_announce,$*,$(true),RDB,1)
	$(call gb_Rdb__command,$@,$*,$?,$^)

.PHONY : $(call gb_Rdb_get_clean_target,%)
$(call gb_Rdb_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),RDB,1)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_Rdb__get_install_target,$*) $(call gb_Rdb_get_target,$*))

define gb_Rdb__Rdb_impl
$(call gb_Rdb_get_target,$(1)) : COMPONENTS :=
$$(eval $$(call gb_Module_register_target,$(2),$(call gb_Rdb_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),Rdb,$(2))

endef

define gb_Rdb_Rdb
$(call gb_Rdb__Rdb_impl,$(1),$(call gb_Rdb_get_target,$(1)))

endef

# FIXME this needs some layer-like thing for the special case in URE
define gb_Rdb_Rdb_install
$(call gb_Rdb__Rdb_impl,$(1),$(if $(2),$(gb_INSTROOT)/$(2),$(call gb_Rdb__get_install_target,$(1))))
$(call gb_Helper_install_final, \
	$(if $(2),$(gb_INSTROOT)/$(2),$(call gb_Rdb__get_install_target,$(1))), \
	$(call gb_Rdb_get_target,$(1)))

endef

define gb_Rdb_install
$(error gb_Rdb_install removed use gb_Rdb_Rdb_install instead)

endef

define gb_Rdb_add_component
$(call gb_Rdb_get_target,$(1)) : $(call gb_ComponentTarget_get_target,$(2))
$(call gb_Rdb_get_target,$(1)) : COMPONENTS += $(2)

endef

define gb_Rdb_add_components
$(foreach component,$(2),$(call gb_Rdb_add_component,$(1),$(component)))

endef

# vim: set noet sw=4 ts=4:
