# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#

$(eval $(call gb_Library_Library,npsoplugin))

$(eval $(call gb_Library_use_externals,npsoplugin,\
    boost_headers \
    npapi_headers \
))

$(eval $(call gb_Library_use_static_libraries,npsoplugin,\
	npsoenv \
	nputils \
))

ifneq ($(OS),WNT)

$(eval $(call gb_Library_add_libs,npsoplugin,\
	-ldl \
))

$(eval $(call gb_Library_use_external,npsoplugin,gtk))

$(eval $(call gb_Library_add_defs,npsoplugin,\
	-DMOZ_X11 \
))

ifeq ($(HAVE_NON_CONST_NPP_GETMIMEDESCRIPTION),TRUE)
$(eval $(call gb_Library_add_defs,npsoplugin,\
	-DHAVE_NON_CONST_NPP_GETMIMEDESCRIPTION=1 \
))
endif

ifeq ($(filter-out LINUX FREEBSD NETBSD OPENBSD DRAGONFLY,$(OS)),)
$(eval $(call gb_Library_add_defs,npsoplugin,\
	-DNP_LINUX \
))
endif

else ifeq ($(OS),WNT)

$(eval $(call gb_Library_use_static_libraries,npsoplugin,\
	ooopathutils \
))

$(eval $(call gb_Library_use_system_win32_libs,npsoplugin,\
	advapi32 \
	comdlg32 \
	gdi32 \
	kernel32 \
	ole32 \
	oleaut32 \
	shell32 \
	user32 \
	uuid \
	winspool \
	ws2_32 \
))

$(eval $(call gb_Library_add_defs,npsoplugin,\
	-DENGLISH \
))

ifeq ($(COM),MSC)
$(eval $(call gb_Library_add_ldflags,npsoplugin,\
	/EXPORT:NPP_GetMIMEDescription \
	/EXPORT:NPP_Initialize \
	/EXPORT:NPP_Shutdown \
	/EXPORT:NPP_New \
	/EXPORT:NPP_Destroy \
	/EXPORT:NPP_SetWindow \
	/EXPORT:NPP_NewStream \
	/EXPORT:NPP_WriteReady \
	/EXPORT:NPP_Write \
	/EXPORT:NPP_DestroyStream \
	/EXPORT:NPP_StreamAsFile \
	/EXPORT:NPP_URLNotify \
	/EXPORT:NPP_Print \
	/EXPORT:NP_GetEntryPoints \
	/EXPORT:NP_Initialize \
	/EXPORT:NP_Shutdown \
	/EXPORT:NP_GetMIMEDescription \
))
endif

$(eval $(call gb_Library_set_nativeres,npsoplugin,npsoplugin/npsoplugin))

endif

$(eval $(call gb_Library_add_exception_objects,npsoplugin,\
	extensions/source/nsplugin/source/npshell \
))

# vim:set shiftwidth=4 softtabstop=4 noexpandtab:
