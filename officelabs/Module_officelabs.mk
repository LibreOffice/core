# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-

$(eval $(call gb_Module_Module,officelabs))

$(eval $(call gb_Module_add_targets,officelabs,\
    Library_officelabs \
    UIConfig_officelabs \
))

# CEF WebView support (conditional on --with-cef)
ifeq ($(ENABLE_CEF),TRUE)
$(eval $(call gb_Module_add_targets,officelabs,\
    Executable_officelabs_cef_subprocess \
    Package_cef \
))
endif

# vim: set noet sw=4 ts=4:
