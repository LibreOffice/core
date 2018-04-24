$(call gb_CustomTarget_get_target,$(testname)): \
	$(call gb_CustomTarget_get_workdir,$(testname))/setsdkenv
ifneq ($(gb_SUPPRESS_TESTS),)
	@true
else
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),CHK,1)
	rm -fr $(call gb_CustomTarget_get_workdir,$(testname))/{out,user}
ifeq (MACOSX,$(OS))
	$(eval ODK_BUILD_SHELL := $(shell $(gb_MKTEMP)))
	cp /bin/sh "$(ODK_BUILD_SHELL)"
	chmod 0700 "$(ODK_BUILD_SHELL)"
endif
	(saved_library_path=$${$(gb_Helper_LIBRARY_PATH_VAR)} && . $< \
	$(if $(filter MACOSX,$(OS)),, \
	    && $(gb_Helper_LIBRARY_PATH_VAR)=$$saved_library_path) \
	&& export \
	    UserInstallation=$(call gb_Helper_make_url,$(call gb_CustomTarget_get_workdir,$(testname))/user) \
	$(foreach my_dir,$(my_example_dirs), \
	    && (cd $(INSTDIR)/$(SDKDIRNAME)/examples/$(my_dir) \
		&& printf 'yes\n' | LC_ALL=C make \
		    $(if $(filter MACOSX,$(OS)), SHELL=$(ODK_BUILD_SHELL), )))) \
	    >$(call gb_CustomTarget_get_workdir,$(testname))/log 2>&1 \
	|| (RET=$$? \
	    $(if $(filter MACOSX,$(OS)), && rm -f $(ODK_BUILD_SHELL) , ) \
	    && cat $(call gb_CustomTarget_get_workdir,$(testname))/log \
	    && exit $$RET)
ifeq (MACOSX,$(OS))
	-rm -f $(ODK_BUILD_SHELL)
endif
endif

$(call gb_CustomTarget_get_workdir,$(testname))/setsdkenv: \
	$(SRCDIR)/odk/config/setsdkenv_unix.sh.in \
	$(BUILDDIR)/config_$(gb_Side).mk | \
	$(call gb_CustomTarget_get_workdir,$(testname))/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),SED,1)
	sed -e 's!@OO_SDK_NAME@!sdk!' \
	-e 's!@OO_SDK_HOME@!$(INSTDIR)/$(SDKDIRNAME)!' \
	-e 's!@OFFICE_HOME@!$(INSTROOTBASE)!' -e 's!@OO_SDK_MAKE_HOME@!!' \
	-e 's!@OO_SDK_ZIP_HOME@!!' -e 's!@OO_SDK_CAT_HOME@!!' \
	-e 's!@OO_SDK_SED_HOME@!!' -e 's!@OO_SDK_CPP_HOME@!!' \
	-e 's!@OO_SDK_JAVA_HOME@!$(JAVA_HOME)!' \
	-e 's!@OO_SDK_OUTPUT_DIR@!$(call gb_CustomTarget_get_workdir,$(testname))/out!' \
	-e 's!@SDK_AUTO_DEPLOYMENT@!YES!' $< > $@
