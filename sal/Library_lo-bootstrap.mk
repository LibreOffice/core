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
# Major Contributor(s):
# Copyright (C) 2011 Matúš Kukan <matus.kukan@gmail.com>
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Library_Library,lo-bootstrap))

# We explicitly *don't* want gb_STDLIBS to be linked here
$(eval $(call gb_Library_add_libs,lo-bootstrap,\
	-lz \
	-llog \
	-landroid \
	-lstlport_static \
))

$(eval $(call gb_Library_add_cobjects,lo-bootstrap,\
	sal/android/lo-bootstrap \
))

$(eval $(call gb_Library_add_cxxobjects,lo-bootstrap,\
	sal/android/faulty.lib/linker/CustomElf \
	sal/android/faulty.lib/linker/ElfLoader \
	sal/android/faulty.lib/linker/Mappable \
	sal/android/faulty.lib/linker/SeekableZStream \
	sal/android/faulty.lib/linker/Zip \
))

$(eval $(call gb_Library_set_include,lo-bootstrap,\
	-I$(ANDROID_NDK_HOME)/sources/cxx-stl/stlport/stlport \
	$$(INCLUDE) \
	-I$(SRCDIR)/sal/android/faulty.lib \
	-I$(SRCDIR)/sal/inc \
	-DHAVE_DLADDR \
))

$(eval $(call gb_Library_set_ldflags,lo-bootstrap,\
	-L$(ANDROID_NDK_HOME)/sources/cxx-stl/stlport/libs/$(ANDROID_APP_ABI) \
	$$(LDFLAGS) \
))

# vim: set noet sw=4 ts=4:
