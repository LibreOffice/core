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
	-llog \
	-landroid \
	-lgnustl_static \
))

$(eval $(call gb_Library_add_cobjects,lo-bootstrap,\
	sal/android/lo-bootstrap \
	sal/android/bionic/linker/dlfcn \
	sal/android/bionic/linker/linker_format \
	sal/android/bionic/linker/linker_phdr \
	sal/android/bionic/linker/rt \
))

$(eval $(call gb_Library_add_cxxobjects,lo-bootstrap,\
	sal/android/bionic/linker/linker \
))

# gb_PRODUCT being empty means --enable-dbgutil, and in that case turn on the
# debugging output possibility in the imported Bionic linker code
$(eval $(call gb_Library_set_include,lo-bootstrap,\
	$$(INCLUDE) \
	-I$(SRCDIR)/sal/inc \
	-DLINKER_DEBUG=$(if $(gb_PRODUCT),0,1) \
	$(if $(filter ARM,$(CPUNAME)),   -DANDROID_ARM_LINKER) \
	$(if $(filter INTEL,$(CPUNAME)), -DANDROID_X86_LINKER) \
	$(if $(filter GODSON,$(CPUNAME)),-DANDROID_MIPS_LINKER) \
))

# vim: set noet sw=4 ts=4:
