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
         SOFFICE_BIN=$(DEVINSTALLDIR)/opt/program/soffice \
         URE_BOOTSTRAP=file://$(DEVINSTALLDIR)/opt/program/fundamentalrc \
		    $(gb_PythonTest_COMMAND) \
            $(CLASSES) > $@.log 2>&1 || \
		(cat $@.log \
		&& false)))
	$(CLEAN_CMD)

define gb_PythonTest_PythonTest
$(call gb_PythonTest_get_target,$(1)) : T_CP :=
$(call gb_PythonTest_get_target,$(1)) : CLASSES :=

$(eval $(call gb_Module_register_target,$(call gb_PythonTest_get_target,$(1)),$(call gb_PythonTest_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),PythonTest)

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

gb_PythonTest_add_classes :=
gb_PythonTest_add_class :=
gb_JunitTest_use_customtarget :=

endif # DISABLE_PYTHON
# vim: set noet sw=4:
