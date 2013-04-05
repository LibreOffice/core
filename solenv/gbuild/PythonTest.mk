# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# PythonTest class

# TODO: FixMe problem with internal python:
# Fatal Python error: Py_Initialize: Unable to get the locale encoding
#gb_Python_EXE := $(call gb_Executable_get_command,python)
gb_Python_EXE := python3

gb_PythonTest_COMMAND := $(gb_Python_EXE) -m unittest

.PHONY : $(call gb_PythonTest_get_clean_target,%)
$(call gb_PythonTest_get_clean_target,%) :
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $@ $@.log)

ifneq ($(DISABLE_PYTHON),TRUE)

.PHONY : $(call gb_PythonTest_get_target,%)
$(call gb_PythonTest_get_target,%) :
	$(call gb_Output_announce,$*,$(true),PYT,2)
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $(call gb_PythonTest_get_target,$*)) && \
		(PYTHONPATH=$(SRCDIR)/unotest/source/python:$(DEVINSTALLDIR)/opt/program \
		UserInstallation="$(call gb_Helper_make_url,$(OUTDIR)/unittest)" \
		BRAND_BASE_DIR="$(call gb_Helper_make_url,$(OUTDIR)/unittest/install)" \
		CONFIGURATION_LAYERS="$(strip $(CONFIGURATION_LAYERS))" \
		UNO_TYPES="$(foreach item,$(UNO_TYPES),$(call gb_Helper_make_url,$(item)))" \
		UNO_SERVICES="$(foreach item,$(UNO_SERVICES),$(call gb_Helper_make_url,$(item)))" \
		$(foreach dir,URE_INTERNAL_LIB_DIR LO_LIB_DIR,\
			$(dir)="$(call gb_Helper_make_url,$(gb_CppunitTest_LIBDIR))") \
		$(gb_CppunitTest_GDBTRACE) $(gb_CppunitTest_VALGRINDTOOL) $(gb_PythonTest_COMMAND) \
			$(CLASSES) \
		$(if $(gb_CppunitTest__interactive),, \
			> $@.log 2>&1 \
			|| (cat $@.log && $(UNIT_FAILED_MSG) \
				$(if $(value gb_CppunitTest_postprocess), \
					&& $(call gb_CppunitTest_postprocess,$(gb_CppunitTest_CPPTESTCOMMAND),$@.core)) \
				&& false))))

# always use udkapi and URE services
define gb_PythonTest_PythonTest
$(call gb_PythonTest_get_target,$(1)) : T_CP :=
$(call gb_PythonTest_get_target,$(1)) : CLASSES :=
$(call gb_PythonTest_get_target,$(1)) : CONFIGURATION_LAYERS :=
$(call gb_PythonTest_get_target,$(1)) : UNO_TYPES :=
$(call gb_PythonTest_get_target,$(1)) : UNO_SERVICES :=

$(call gb_PythonTest_use_api,$(1),udkapi)
$(call gb_PythonTest_use_rdb,$(1),ure/services)

$(eval $(call gb_Module_register_target,$(call gb_PythonTest_get_target,$(1)),$(call gb_PythonTest_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),PythonTest)

endef

define gb_PythonTest_use_configuration
$(call gb_PythonTest_get_target,$(1)) : \
	$(call gb_Configuration_get_target,registry) \
	$(call gb_Configuration_get_target,fcfg_langpack) \
	$(call gb_Package_get_target,test_unittest)
$(call gb_PythonTest_get_target,$(1)) : CONFIGURATION_LAYERS += \
	xcsxcu:$(call gb_Helper_make_url,$(gb_Configuration_registry)) \
	module:$(call gb_Helper_make_url,$(gb_Configuration_registry)/spool) \
	xcsxcu:$(call gb_Helper_make_url,$(OUTDIR)/unittest/registry)

endef

define gb_PythonTest__use_api
$(call gb_PythonTest_get_target,$(1)) : $(call gb_UnoApi_get_target,$(2))
$(call gb_PythonTest_get_target,$(1)) : \
	UNO_TYPES += $(call gb_UnoApi_get_target,$(2))

endef

define gb_PythonTest_use_api
$(foreach api,$(2),$(call gb_PythonTest__use_api,$(1),$(api)))
endef

define gb_PythonTest_use_rdb
$(call gb_PythonTest_get_target,$(1)) : $(call gb_Rdb_get_outdir_target,$(2))
$(call gb_PythonTest_get_target,$(1)) : \
	UNO_SERVICES += $(call gb_Rdb_get_outdir_target,$(2))

endef

define gb_PythonTest_use_component
$(call gb_PythonTest_get_target,$(1)) : \
    $(call gb_ComponentTarget_get_outdir_target,$(2))
$(call gb_PythonTest_get_target,$(1)) : \
    UNO_SERVICES += $(call gb_ComponentTarget_get_outdir_target,$(2))

endef

define gb_PythonTest_use_components
$(foreach component,$(call gb_CppunitTest__filter_not_built_components,$(2)),$(call gb_PythonTest_use_component,$(1),$(component)))

endef

define gb_PythonTest_add_classes
$(call gb_PythonTest_get_target,$(1)) : CLASSES += $(2)

endef

define gb_PythonTest_add_class
$(call gb_PythonTest_add_classes,$(1),$(2))

endef

define gb_PythonTest_use_customtarget
$(call gb_PythonTest_get_target,$(1)) : $(call gb_CustomTarget_get_workdir,$(2))

endef


else # DISABLE_PYTHON

.PHONY : $(call gb_PythonTest_get_target,$(1))
$(call gb_PythonTest_get_target,%) :
	$(call gb_Output_announce,$* (skipped - no PythonTest),$(true),PYT,2)
	@true

define gb_PythonTest_PythonTest
$(eval $(call gb_Module_register_target,$(call gb_PythonTest_get_target,$(1)),$(call gb_PythonTest_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),PythonTest)

endef

gb_PythonTest_use_configuration :=
gb_PythonTest_use_api :=
gb_PythonTest_use_rdb :=
gb_PythonTest_use_components :=
gb_PythonTest_add_classes :=
gb_PythonTest_add_class :=
gb_PythonTest_use_customtarget :=

endif # DISABLE_PYTHON
# vim: set noet sw=4:
