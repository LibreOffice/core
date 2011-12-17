# -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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

ifeq ($(SYSTEM_MOZILLA_HEADERS),YES)
$(eval $(call gb_Library_set_include,npsoplugin,\
	$$(INCLUDE) \
	$(MOZILLA_HEADERS_CFLAGS)
))
else
$(eval $(call gb_Library_set_include,npsoplugin,\
	-I$(OUTDIR)/inc/npsdk \
	$$(INCLUDE) \
))
endif

ifeq ($(GUI),UNX)

$(eval $(call gb_Library_add_ldflags,npsoplugin,\
	$(OUTDIR)/lib/npunix.o \
))

$(eval $(call gb_Library_add_libs,npsoplugin,\
	-ldl \
))

$(eval $(call gb_Library_use_external,npsoplugin,gtk))

$(eval $(call gb_Library_add_defs,npsoplugin,\
	-DMOZ_X11 \
))

ifeq ($(filter-out LINUX FREEBSD NETBSD OPENBSD DRAGONFLY,$(OS)),)
$(eval $(call gb_Library_add_defs,npsoplugin,\
	-DNP_LINUX \
))
endif

endif # GUI=UNX

ifeq ($(GUI),WNT)

$(eval $(call gb_Library_add_ldflags,npsoplugin,\
	$(OUTDIR)/lib/pathutils-obj.obj \
	$(OUTDIR)/lib/npwin.obj \
))

$(eval $(call gb_Library_add_linked_libs,npsoplugin,\
	ws2_32 \
	shell32 \
	ole32 \
	kernel32 \
	user32 \
	gdi32 \
	winspool \
	comdlg32 \
	advapi32 \
	oleaut32 \
	uuid \
))

$(eval $(call gb_Library_add_defs,npsoplugin,\
	-DENGLISH \
))

$(eval $(call gb_Library_add_nativeres,npsoplugin,npsoplugin_res))

endif # GUI=WNT

$(eval $(call gb_Library_add_exception_objects,npsoplugin,\
	extensions/source/nsplugin/source/so_env \
	extensions/source/nsplugin/source/npshell \
))

# vim:set shiftwidth=4 softtabstop=4 expandtab:
