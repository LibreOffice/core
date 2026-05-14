# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,odffilter))

$(eval $(call gb_Library_set_include,odffilter,\
	-I$(SRCDIR)/shell/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,odffilter,\
	-DISOLATION_AWARE_ENABLED \
	-DWIN32_LEAN_AND_MEAN \
	-UNOMINMAX \
))

$(eval $(call gb_Library_use_externals,odffilter,\
	expat \
	zlib \
))

$(eval $(call gb_Library_use_system_win32_libs,odffilter,\
	advapi32 \
	comctl32 \
	kernel32 \
	msvcprt \
	oldnames \
	ole32 \
	shell32 \
	uuid \
))

ifeq ($(ENABLE_DBGUTIL),TRUE)
$(eval $(call gb_Library_use_system_win32_libs,odffilter,\
	msvcrt \
))
endif

$(eval $(call gb_Library_use_static_libraries,odffilter,\
	shell_xmlparser \
	shlxthandler_common \
))

$(eval $(call gb_Library_add_ldflags,odffilter,\
	/DEF:$(SRCDIR)/shell/source/win32/shlxthandler/odffilter/odffilter.def \
	/NODEFAULTLIB:libcmt.lib \
))

$(eval $(call gb_Library_add_exception_objects,odffilter,\
    shell/source/win32/shlxthandler/odffilter/odffilter \
    shell/source/win32/shlxthandler/odffilter/propspec \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
