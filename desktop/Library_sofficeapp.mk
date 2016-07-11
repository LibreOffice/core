# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,sofficeapp))

$(eval $(call gb_Library_set_include,sofficeapp,\
    $$(INCLUDE) \
    -I$(SRCDIR)/desktop/inc \
    -I$(SRCDIR)/desktop/source/inc \
    -I$(SRCDIR)/desktop/source/deployment/inc \
))

$(eval $(call gb_Library_add_libs,sofficeapp,\
    $(if $(filter LINUX %BSD SOLARIS, $(OS)), \
        $(DLOPEN_LIBS) \
        -lpthread \
    ) \
))

$(eval $(call gb_Library_use_externals,sofficeapp, \
	$(if $(filter OPENCL,$(BUILD_TYPE)),clew) \
    boost_headers \
    dbus \
))

ifeq ($(ENABLE_BREAKPAD),TRUE)
$(eval $(call gb_Library_use_external,sofficeapp,breakpad))
endif

$(eval $(call gb_Library_use_custom_headers,sofficeapp,\
	officecfg/registry \
))

$(eval $(call gb_Library_use_sdk_api,sofficeapp))

$(eval $(call gb_Library_add_defs,sofficeapp,\
    -DDESKTOP_DLLIMPLEMENTATION \
    $(if $(filter WNT,$(OS)),-DENABLE_QUICKSTART_APPLET) \
    $(if $(filter MACOSX,$(OS)),-DENABLE_QUICKSTART_APPLET) \
    $(if $(filter TRUE,$(ENABLE_SYSTRAY_GTK)),-DENABLE_QUICKSTART_APPLET) \
))

$(eval $(call gb_Library_set_precompiled_header,sofficeapp,$(SRCDIR)/desktop/inc/pch/precompiled_sofficeapp))

$(eval $(call gb_Library_use_libraries,sofficeapp,\
    comphelper \
    cppu \
    cppuhelper \
    $(if $(filter TRUE,$(ENABLE_BREAKPAD)), \
        crashreport \
    ) \
    deploymentmisc \
    editeng \
    i18nlangtag \
    $(if $(filter OPENCL,$(BUILD_TYPE)),opencl) \
    sal \
    salhelper \
    sb \
    sfx \
    svl \
    svxcore \
    svt \
    tk \
    tl \
    ucbhelper \
    utl \
    vcl \
	$(gb_UWINAPI) \
))

ifeq ($(OS),MACOSX)

$(eval $(call gb_Library_add_cxxflags,sofficeapp,\
    $(gb_OBJCXXFLAGS) \
))

$(eval $(call gb_Library_use_system_darwin_frameworks,sofficeapp,\
    Foundation \
))

endif

ifeq ($(OS),IOS)
$(eval $(call gb_Library_add_cflags,sofficeapp,\
    $(gb_OBJCFLAGS) \
))
endif

$(eval $(call gb_Library_add_exception_objects,sofficeapp,\
    desktop/source/app/app \
    desktop/source/app/appinit \
    desktop/source/app/check_ext_deps \
    desktop/source/app/cmdlineargs \
    desktop/source/app/cmdlinehelp \
    desktop/source/app/desktopcontext \
    desktop/source/app/desktopresid \
    desktop/source/app/dispatchwatcher \
    desktop/source/app/langselect \
    desktop/source/app/lockfile2 \
    desktop/source/app/officeipcthread \
    desktop/source/app/opencl \
    desktop/source/app/sofficemain \
    desktop/source/app/userinstall \
    desktop/source/migration/migration \
))

ifeq ($(ENABLE_HEADLESS),TRUE)
$(eval $(call gb_Library_add_libs,sofficeapp,\
	-lm $(DLOPEN_LIBS) \
	-lpthread \
))
else
ifeq ($(OS), $(filter LINUX %BSD SOLARIS, $(OS)))
$(eval $(call gb_Library_use_static_libraries,sofficeapp,\
    glxtest \
))

$(eval $(call gb_Library_add_libs,sofficeapp,\
	-lm $(DLOPEN_LIBS) \
	-lpthread \
    -lX11 \
))
endif
endif

# LibreOfficeKit bits
ifneq ($(filter $(OS),ANDROID IOS MACOSX WNT),)
$(eval $(call gb_Library_add_exception_objects,sofficeapp,\
	desktop/source/lib/init \
	desktop/source/lib/lokinteractionhandler \
	desktop/source/lib/lokclipboard \
	$(if $(filter $(OS),ANDROID), \
		desktop/source/lib/lokandroid) \
))
else
ifeq ($(USING_X11),TRUE)
$(eval $(call gb_Library_add_exception_objects,sofficeapp,\
	desktop/source/lib/init \
	desktop/source/lib/lokinteractionhandler \
	desktop/source/lib/lokclipboard \
))
endif
ifeq ($(ENABLE_HEADLESS),TRUE)
$(eval $(call gb_Library_add_exception_objects,sofficeapp,\
    desktop/source/lib/init \
    desktop/source/lib/lokinteractionhandler \
    desktop/source/lib/lokclipboard \
))
endif
endif

ifeq ($(ENABLE_TELEPATHY),TRUE)
$(eval $(call gb_Library_use_libraries,sofficeapp,tubes))
endif

# vim: set ts=4 sw=4 et:
