# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,crashrep))

$(eval $(call gb_Executable_add_defs,crashrep,\
	-D_INPATH=\"$(INPATH)\" \
))

ifeq ($(OS),WNT)

$(eval $(call gb_Executable_add_exception_objects,crashrep,\
	crashrep/source/win32/soreport \
	crashrep/source/win32/base64 \
))

$(eval $(call gb_Executable_use_libraries,crashrep,\
	sal \
))

$(eval $(call gb_Executable_use_system_win32_libs,crashrep,\
	gdi32 \
	comctl32 \
	comdlg32 \
	advapi32 \
	ws2_32 \
	shell32 \
	dbghelp \
	psapi \
))

$(eval $(call gb_Executable_set_targettype_gui,crashrep,YES))

$(eval $(call gb_Executable_add_nativeres,crashrep,crashrep))

else

$(eval $(call gb_Executable_add_exception_objects,crashrep,\
	crashrep/source/unx/main \
))

ifeq ($(filter FREEBSD NETBSD DRAGONFLY MACOSX,$(OS)),)
$(eval $(call gb_Executable_add_libs,crashrep,\
   -ldl \
   -lnsl \
))
endif

ifeq ($(OS),SOLARIS)
$(eval $(call gb_Executable_add_libs,crashrep,\
   -lsocket \
))
endif

endif



# vim: set noet sw=4 ts=4:
