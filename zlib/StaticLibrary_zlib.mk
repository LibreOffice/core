# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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
# Copyright (C) 2012 Red Hat, Inc., Caolán McNamara <caolanm@redhat.com>
#  (initial developer)
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

$(eval $(call gb_StaticLibrary_StaticLibrary,z))

$(eval $(call gb_StaticLibrary_use_package,z,zlib_source))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,zlib))

$(eval $(call gb_StaticLibrary_set_include,z,\
	$$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_defs,z,\
    $(LFS_CFLAGS) \
))

# zlib does not depend on sal nor needs uwinapi here
$(eval $(call gb_StaticLibrary_use_libraries,z,\
    $(filter-out uwinapi,$(gb_STDLIBS)) \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_StaticLibrary_use_libraries,z,\
    m \
    pthread \
))
endif

$(eval $(call gb_StaticLibrary_add_generated_cobjects,z,\
	CustomTarget/zlib/source/adler32 \
	CustomTarget/zlib/source/compress \
	CustomTarget/zlib/source/deflate \
	CustomTarget/zlib/source/crc32 \
	CustomTarget/zlib/source/inffast \
	CustomTarget/zlib/source/inflate \
	CustomTarget/zlib/source/inftrees \
	CustomTarget/zlib/source/trees \
	CustomTarget/zlib/source/zutil \
	,$(if $(filter TRUE,$(EXTERNAL_WARNINGS_NOT_ERRORS)),\
		$(filter-out $(gb_CFLAGS_WERROR),$(CFLAGS)),\
		$(CFLAGS)) \
	 $(gb_COMPILEROPTFLAGS) \
))

# vim: set noet sw=4 ts=4:
