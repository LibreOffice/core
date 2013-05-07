# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

.PHONY : $(call gb_Gallery_get_clean_target,%)
$(call gb_Gallery_get_clean_target,%) :
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_Gallery_get_target,$*) $(call gb_Gallery_get_target,$*).log)

.PHONY : $(call gb_Gallery_get_target,%)
$(call gb_Gallery_get_target,%) :| $(call gb_Gallery_get_target,$(1))/$(2).thm
	@echo "foo"


define gb_Gallery_Gallery

$(call gb_Gallery_get_target,$(1)) : CONFIGURATION_LAYERS :=
$(call gb_Gallery_get_target,$(1)) : URE := $(false)
$(call gb_Gallery_get_target,$(1)) : UNO_SERVICES :=
$(call gb_Gallery_get_target,$(1)) : UNO_TYPES :=
$(call gb_Gallery_get_target,$(1)) : IMAGE_FILES :=

$(call gb_Gallery_get_target,$(1)) :| $(dir $(call gb_Gallery_get_target,$(1))).dir

$(dir $(call gb_Gallery_get_target,$(1))).dir :
	mkdir -p $(dir $(call gb_Gallery_get_target,$(1)))
	touch $(dir $(call gb_Gallery_get_target,$(1))).dir

$(eval $(call gb_Module_register_target,$(call gb_Gallery_get_target,$(1)),$(call gb_Gallery_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),Gallery)

$(eval $(call gb_Gallery_use_components,$(1), \
    comphelper/util/comphelp \
    configmgr/source/configmgr \
    fileaccess/source/fileacc \
    i18npool/util/i18npool \
    package/source/xstor/xstor \
    package/util/package2 \
    sfx2/util/sfx \
    svx/util/svx \
    svx/util/svxcore \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    unoxml/source/service/unoxml \
))

# setup URE
$(eval $(call gb_Gallery__use_configuration,$(1),xcsxcu,$(gb_Configuration_registry)))
$(eval $(call gb_Gallery__use_configuration,$(1),module,$(gb_Configuration_registry)/spool))
$(eval $(call gb_Gallery__use_configuration,$(1)/,xcsxcu,$(OUTDIR)/unittest/registry))
$(eval $(call gb_Gallery__use_api,$(1),udkapi))
$(eval $(call gb_Gallery__use_api,$(1),offapi))

$(call gb_Gallery_get_target,$(1)) : \
		$(call gb_Gallery_get_target,$(1))/$(2).thm

# main gallery build rule
$(call gb_Gallery_get_target,$(1))/$(2).thm : \
			$(call gb_Executable_get_runtime_dependencies,gengal)
	$(call gb_Output_announce,building gallery: $*,$(true),MOD,1)
	$(call gb_Helper_abbreviate_dirs, \
			$(call gb_Executable_get_command,gengal) \
				$(call gb_Gallery_make_args) \
				--name "$(3)" \
				--path $(call gb_Gallery_get_target,$(1))) \
		    	$(foreach item,$(IMAGE_FILES),$(item)) && \
	mv -f $(call gb_Gallery_get_target,$(1))/001.thm $(call gb_Gallery_get_target,$(1))/$(2).thm

endef

# horrendous cut/paste from CppunitTest

define gb_Gallery_use_component
$(call gb_Gallery_get_target,$(1)) : \
    $(call gb_ComponentTarget_get_outdir_target,$(2))
$(call gb_Gallery_get_target,$(1)) : \
    UNO_SERVICES += $(call gb_ComponentTarget_get_outdir_target,$(2))

endef

define gb_Gallery__use_api
$(call gb_Gallery_get_target,$(1)) : $(call gb_UnoApi_get_target,$(2))
$(call gb_Gallery_get_target,$(1)) : UNO_TYPES += $(call gb_UnoApi_get_target,$(2))

endef

define gb_Gallery__use_configuration
$(call gb_Gallery_get_target,$(1)) : CONFIGURATION_LAYERS += $(2):$(call gb_Helper_make_url,$(3))

endef

define gb_Gallery_use_components
$(foreach component,$(call gb_CppunitTest__filter_not_built_components,$(2)),$(call gb_Gallery_use_component,$(1),$(component)))

endef

define gb_Gallery_make_args
	--build-tree \
    $(if $(strip $(UNO_SERVICES)),\
	"-env:UNO_SERVICES=$(foreach item,$(UNO_SERVICES),$(call gb_Helper_make_url,$(item)))") \
    $(if $(strip $(CONFIGURATION_LAYERS)),\
	    "-env:CONFIGURATION_LAYERS=$(strip $(CONFIGURATION_LAYERS))") \
    $(if $(strip $(UNO_TYPES)),\
	    "-env:UNO_TYPES=$(foreach item,$(UNO_TYPES),$(call gb_Helper_make_url,$(item)))") \
    $(foreach dir,URE_INTERNAL_LIB_DIR LO_LIB_DIR,\
	    -env:$(dir)=$(call gb_Helper_make_url,$(gb_Helper_OUTDIRLIBDIR)))
endef

define gb_Gallery_add_file
$(call gb_Gallery_get_target,$(1)) : IMAGE_FILES += $(call gb_Helper_make_url,$(2))
endef

define gb_Gallery_add_files
$(foreach fname,$(2),$(call gb_Gallery__add_file,$(1),$(fname)))
endef

# vim: set noet sw=4 ts=4:
