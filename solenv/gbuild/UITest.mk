# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# UITest class

gb_UITest_UNITTESTFAILED ?= $(GBUILDDIR)/uitest-failed-default.sh

ifeq ($(SYSTEM_PYTHON),)
gb_UITest_EXECUTABLE := $(gb_Python_INSTALLED_EXECUTABLE)
gb_UITest_DEPS ?= $(call gb_Package_get_target,python3)
else
gb_UITest_EXECUTABLE := $(PYTHON_FOR_BUILD)
gb_UITest_DEPS :=
endif

# UITests are much more likely to generate core files for the soffice than for
# the python executable, but solenv/bin/gdb-core-bt.sh is often unable to
# determine the executable that generated a core file, so make it fall back to
# the soffice executable rather than to gb_UITest_EXECUTABLE:
gb_UITest_EXECUTABLE_GDB := $(call gb_Executable_get_target,soffice_bin)

ifneq ($(strip $(UITESTTRACE)),)
gb_UITest_GDBTRACE := --gdb
gb_UITest__interactive := $(true)
endif

ifneq ($(gb_UITest_DEBUGRUN),)
gb_UITest_SOFFICEARG:=connect:pipe,name=$(USER)
gb_UITest__interactive := $(true)
else
gb_UITest_SOFFICEARG:=path:$(INSTROOT)/$(LIBO_BIN_FOLDER)/soffice
endif

gb_UITest_COMMAND = $(ICECREAM_RUN) $(gb_CppunitTest_coredumpctl_run) $(gb_CppunitTest_RR) $(gb_UITest_EXECUTABLE) $(SRCDIR)/uitest/test_main.py

gb_TEST_ENV_VARS += LIBO_LANG=C

.PHONY : $(call gb_UITest_get_clean_target,%)
$(call gb_UITest_get_clean_target,%) :
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $@ $@.log)

ifneq ($(DISABLE_PYTHON),TRUE)

# qadevOOo/qa/registrymodifications.xcu is copied to user profile directory to ensure en_US locale;
# this might be overwritten later when gb_UITest_use_config is set
.PHONY : $(call gb_UITest_get_target,%)
$(call gb_UITest_get_target,%) :| $(gb_UITest_DEPS)
ifneq ($(gb_SUPPRESS_TESTS),)
	@true
else
	$(call gb_Output_announce,$*,$(true),UIT,2)
	$(call gb_Trace_StartRange,$*,UIT)
	$(call gb_Helper_abbreviate_dirs,\
		rm -rf $(dir $(call gb_UITest_get_target,$*)) && \
		mkdir -p $(dir $(call gb_UITest_get_target,$*))/user/user && \
		cp -T $(if $(gb_UITest_use_config),$(gb_UITest_use_config),$(SRCDIR)/qadevOOo/qa/registrymodifications.xcu) $(dir $(call gb_UITest_get_target,$*))/user/user/registrymodifications.xcu && \
		$(if $(gb_UITest__interactive),, \
		    rm -fr $@.core && mkdir -p $(dir $(call gb_UITest_get_target,$*))user/ && mkdir $@.core && cd $@.core && ) \
		$(call gb_CppunitTest_coredumpctl_setup,$@) \
		($(gb_UITest_PRECOMMAND) \
		$(if $(G_SLICE),G_SLICE=$(G_SLICE)) \
		$(if $(GLIBCXX_FORCE_NEW),GLIBCXX_FORCE_NEW=$(GLIBCXX_FORCE_NEW)) \
		$(DEFS) \
		$(if $(filter WNT,$(OS)),SAL_LOG_FILE="$(dir $(call gb_UITest_get_target,$*))/soffice.out.log") \
		TEST_LIB=$(call gb_Library_get_target,test) \
		URE_BOOTSTRAP=vnd.sun.star.pathname:$(call gb_Helper_get_rcfile,$(INSTROOT)/$(LIBO_ETC_FOLDER)/fundamental) \
		PYTHONPATH="$(PYPATH)" \
		TestUserDir="$(call gb_Helper_make_url,$(dir $(call gb_UITest_get_target,$*)))" \
		PYTHONDONTWRITEBYTECODE=0 \
		$(if $(ENABLE_WERROR),PYTHONWARNINGS=error) \
		$(if $(filter WNT,$(OS)),TZ=) \
		$(gb_TEST_ENV_VARS) \
		$(gb_UITest_COMMAND) \
		--soffice="$(gb_UITest_SOFFICEARG)" \
		$(if $(ONEPROCESS),--oneprocess) \
		--userdir=$(call gb_Helper_make_url,$(dir $(call gb_UITest_get_target,$*))user) \
		--dir=$(strip $(MODULES)) \
		$(gb_UITest_GDBTRACE) \
		$(if $(gb_UITest__interactive),, \
		    > $@.log 2>&1 \
		    || ($(if $(value gb_CppunitTest_postprocess), \
				    RET=$$?; \
				    $(call gb_CppunitTest_postprocess,$(gb_UITest_EXECUTABLE_GDB),$@.core,$$RET) >> $@.log 2>&1;) \
                $(if $(filter WNT,$(OS)), \
                    printf '%s: <<<\n' $(dir $(call gb_UITest_get_target,$*))/soffice.out.log; \
                    cat $(dir $(call gb_UITest_get_target,$*))/soffice.out.log; \
                    printf ' >>>\n\n';) \
			    cat $@.log; $(gb_UITest_UNITTESTFAILED) UI $*))))
	$(call gb_Trace_EndRange,$*,UIT)
endif

# always use udkapi and URE services
define gb_UITest_UITest
$(call gb_UITest_get_target,$(1)) : PYPATH := $(SRCDIR)/uitest$$(gb_CLASSPATHSEP)$(SRCDIR)/unotest/source/python$$(gb_CLASSPATHSEP)$(INSTROOT)/$(LIBO_LIB_PYUNO_FOLDER)$(if $(filter-out $(LIBO_LIB_PYUNO_FOLDER),$(LIBO_LIB_FOLDER)),$(gb_CLASSPATHSEP)$(INSTROOT)/$(LIBO_LIB_FOLDER))
$(call gb_UITest_get_target,$(1)) : MODULES :=

$(eval $(call gb_TestHelpers_use_more_fonts,$(call gb_UITest_get_target,$(1))))
$(eval $(call gb_Module_register_target,$(call gb_UITest_get_target,$(1)),$(call gb_UITest_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),UITest)
$(call gb_UITest_get_target,$(1)) : ONEPROCESS := $(true)

endef

define gb_UITest_set_defs
$(call gb_UITest_get_target,$(1)) : DEFS := $(2)

endef

# put the directory on the PYTHONPATH because the "unittest" loader
# mysteriously fails to load modules given as absolute path unless the $PWD is
# a prefix of the absolute path, which it is not when we go into a certain
# dir to get a core dump there
#
# gb_UITest_add_modules directory module(s)
define gb_UITest_add_modules
$(call gb_UITest_get_target,$(1)) : PYPATH := $$(PYPATH)$$(gb_CLASSPATHSEP)$(strip $(2))/$(strip $(3))
$(call gb_UITest_get_target,$(1)) : MODULES += $(strip $(2))/$(strip $(3))

endef

define gb_UITest_use_customtarget
$(call gb_UITest_get_target,$(1)) : $(gb_CustomTarget_workdir)/$(2)

endef

define gb_UITest_use_configuration
$(call gb_UITest_get_target,$(1)) : gb_UITest_use_config := $(2)
endef

define gb_UITest_avoid_oneprocess
$(call gb_UITest_get_target,$(1)) : ONEPROCESS := $(false)
endef


else # DISABLE_PYTHON

.PHONY : $(call gb_UITest_get_target,$(1))
$(call gb_UITest_get_target,%) :
ifeq ($(gb_SUPPRESS_TESTS),)
	$(call gb_Output_announce,$* (skipped - no UITest),$(true),PYT,2)
endif
	@true

define gb_UITest_UITest
$(eval $(call gb_Module_register_target,$(call gb_UITest_get_target,$(1)),$(call gb_UITest_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),UITest)

endef

gb_UITest_set_defs :=
gb_UITest_add_modules :=
gb_UITest_use_customtarget :=

endif # DISABLE_PYTHON
# vim: set noet sw=4:
