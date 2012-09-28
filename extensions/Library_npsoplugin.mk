# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
# 	Peter Foley <pefoley2@verizon.net>
# Portions created by the Initial Developer are Copyright (C) 2011 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.
#

$(eval $(call gb_Library_Library,npsoplugin))

$(eval $(call gb_Library_use_external,npsoplugin,mozilla_headers))

$(eval $(call gb_Library_use_static_libraries,npsoplugin,\
	npsoenv \
	nputils \
))

ifeq ($(GUI),UNX)

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

endif # GUI=UNX

ifeq ($(OS),WNT)

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
	/EXPORT:NPP_Shutdown \
	/EXPORT:NP_GetEntryPoints \
	/EXPORT:NP_Initialize \
	/EXPORT:NP_Shutdown \
	/EXPORT:NP_GetMIMEDescription \
))
endif

# Trick to get rid of the default.res to avoid duplicate VERSION
# resource: Set NATIVERES for npsoplugin to be *only* npsoplugin_res

$(eval $(call gb_LinkTarget_get_target,npsoplugin) : $(call gb_WinResTarget_get_target,npsoplugin_res))
$(eval $(call gb_LinkTarget_get_target,npsoplugin) : NATIVERES := $(call gb_WinResTarget_get_target,npsoplugin_res))

endif # GUI=WNT

$(eval $(call gb_Library_add_exception_objects,npsoplugin,\
	extensions/source/nsplugin/source/npshell \
))

# vim:set shiftwidth=4 softtabstop=4 noexpandtab:
