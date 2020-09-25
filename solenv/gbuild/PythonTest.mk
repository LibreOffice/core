# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# PythonTest class

# (gb_PythonTest_GDBTRACE et al are defined alongside gb_CppunitTest_GDBTRACE in CppunitTest.mk)

gb_PythonTest_UNITTESTFAILED ?= $(GBUILDDIR)/platform/unittest-failed-default.sh

ifeq ($(SYSTEM_PYTHON),)
gb_PythonTest_EXECUTABLE := $(gb_Python_INSTALLED_EXECUTABLE)
gb_PythonTest_EXECUTABLE_GDB := $(gb_Python_INSTALLED_EXECUTABLE_GDB)
gb_PythonTest_DEPS ?= $(call gb_Package_get_target,python3) $(call gb_Package_get_target,python_shell)
else
gb_PythonTest_EXECUTABLE := $(PYTHON_FOR_BUILD)
gb_PythonTest_EXECUTABLE_GDB := $(PYTHON_FOR_BUILD)
gb_PythonTest_DEPS :=
endif

gb_PythonTest_COMMAND := $(gb_PythonTest_EXECUTABLE) -m org.libreoffice.unittest

.PHONY : $(call gb_PythonTest_get_clean_target,%)
$(call gb_PythonTest_get_clean_target,%) :
	$(call gb_Helper_abbreviate_dirs,\
		rm -fr $(WORKDIR)/PythonTest/$*)

ifneq ($(DISABLE_PYTHON),TRUE)

.PHONY : $(call gb_PythonTest_get_target,%)
$(call gb_PythonTest_get_target,%) :\
        $(call gb_Library_get_target,pyuno) \
        $(if $(filter-out WNT,$(OS)),$(call gb_Library_get_target,pyuno_wrapper)) \
        | $(gb_PythonTest_DEPS)
ifneq ($(gb_SUPPRESS_TESTS),)
	@true
else
	$(call gb_Output_announce,$*,$(true),PYT,2)
	$(call gb_Trace_StartRange,$*,PYT)
	$(call gb_Helper_abbreviate_dirs,\
		rm -rf $(dir $(call gb_PythonTest_get_target,$*)) && \
		mkdir -p $(dir $(call gb_PythonTest_get_target,$*))user/user/autotext && \
		$(if $(gb_CppunitTest__interactive),, \
			$(if $(value gb_CppunitTest_postprocess), \
				rm -fr $@.core && mkdir $@.core && cd $@.core &&)) \
		{ \
		$(if $(gb_PythonTest_PREGDBTRACE),$(gb_PythonTest_PREGDBTRACE) &&) \
		$(if $(filter gdb,$(gb_PythonTest_GDBTRACE)),,$(gb_PythonTest_PRECOMMAND)) \
		$(if $(G_SLICE),G_SLICE=$(G_SLICE)) \
		$(if $(GLIBCXX_FORCE_NEW),GLIBCXX_FORCE_NEW=$(GLIBCXX_FORCE_NEW)) \
		$(DEFS) \
		TEST_LIB=$(call gb_Library_get_target,test) \
		URE_BOOTSTRAP=vnd.sun.star.pathname:$(call gb_Helper_get_rcfile,$(INSTROOT)/$(LIBO_ETC_FOLDER)/fundamental) \
		PYTHONPATH="$(PYPATH)" \
		UserInstallation=$(call gb_Helper_make_url,$(dir $(call gb_PythonTest_get_target,$*))user) \
		TestUserDir="$(call gb_Helper_make_url,$(dir $(call gb_PythonTest_get_target,$*)))" \
		PYTHONDONTWRITEBYTECODE=1 \
		$(gb_TEST_ENV_VARS) \
		$(ICECREAM_RUN) $(gb_PythonTest_GDBTRACE) $(gb_CppunitTest_VALGRINDTOOL) $(gb_CppunitTest_RR) \
			$(gb_PythonTest_COMMAND) \
			$(if $(PYTHON_TEST_NAME),$(PYTHON_TEST_NAME),$(MODULES)) \
		$(if $(gb_PythonTest_POSTGDBTRACE), \
			; RET=$$? && $(gb_PythonTest_POSTGDBTRACE) && (exit $$RET)) \
		; } \
		$(if $(gb_CppunitTest__interactive),, \
			> $@.log 2>&1 \
			|| ($(if $(value gb_CppunitTest_postprocess), \
					RET=$$?; \
					$(call gb_CppunitTest_postprocess,$(gb_PythonTest_EXECUTABLE_GDB),$@.core,$$RET) >> $@.log 2>&1;) \
				cat $@.log; $(gb_PythonTest_UNITTESTFAILED) Python $*)))
	$(call gb_Trace_EndRange,$*,PYT)
endif

# always use udkapi and URE services
define gb_PythonTest_PythonTest
$(call gb_PythonTest_get_target,$(1)) : PYPATH := $(SRCDIR)/unotest/source/python$$(gb_CLASSPATHSEP)$(INSTROOT)/$(LIBO_LIB_PYUNO_FOLDER)$(if $(filter-out $(LIBO_LIB_PYUNO_FOLDER),$(LIBO_LIB_FOLDER)),$(gb_CLASSPATHSEP)$(INSTROOT)/$(LIBO_LIB_FOLDER))
$(call gb_PythonTest_get_target,$(1)) : MODULES :=

$(eval $(call gb_Module_register_target,$(call gb_PythonTest_get_target,$(1)),$(call gb_PythonTest_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),PythonTest)

endef

define gb_PythonTest_set_defs
$(call gb_PythonTest_get_target,$(1)) : DEFS := $(2)

endef

# put the directory on the PYTHONPATH because the "unittest" loader
# mysteriously fails to load modules given as absolute path unless the $PWD is
# a prefix of the absolute path, which it is not when we go into a certain
# dir to get a core dump there
#
# gb_PythonTest_add_modules directory module(s)
define gb_PythonTest_add_modules
$(call gb_PythonTest_get_target,$(1)) : PYPATH := $$(PYPATH)$$(gb_CLASSPATHSEP)$(2)
$(call gb_PythonTest_get_target,$(1)) : MODULES += $(3)

endef

define gb_PythonTest_use_customtarget
$(call gb_PythonTest_get_target,$(1)) : $(call gb_CustomTarget_get_workdir,$(2))

endef


else # DISABLE_PYTHON

.PHONY : $(call gb_PythonTest_get_target,$(1))
$(call gb_PythonTest_get_target,%) :
ifeq ($(gb_SUPPRESS_TESTS),)
	$(call gb_Output_announce,$* (skipped - no PythonTest),$(true),PYT,2)
endif
	@true

define gb_PythonTest_PythonTest
$(eval $(call gb_Module_register_target,$(call gb_PythonTest_get_target,$(1)),$(call gb_PythonTest_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),PythonTest)

endef

gb_PythonTest_set_defs :=
gb_PythonTest_add_modules :=
gb_PythonTest_use_customtarget :=

endif # DISABLE_PYTHON
# vim: set noet sw=4:
