# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,python3))

$(eval $(call gb_UnpackedTarball_set_tarball,python3,$(PYTHON_TARBALL),,python3))

$(eval $(call gb_UnpackedTarball_fix_end_of_line,python3,\
	PCbuild/pcbuild.sln \
))

$(eval $(call gb_UnpackedTarball_add_patches,python3,\
	external/python3/i100492-freebsd.patch.1 \
	external/python3/python-3.3.3-aix.patch.1 \
	external/python3/python-3.3.0-darwin.patch.1 \
	external/python3/python-3.3.0-msvc-disable.patch.1 \
	external/python3/python-3.3.0-ssl.patch.1 \
	external/python3/python-3.3.3-py17797.patch.1 \
	external/python3/python-3.3.0-i42553.patch.2 \
	external/python3/python-3.3.0-pythreadstate.patch.1 \
	external/python3/python-3.3.0-clang.patch.1 \
	external/python3/python-3.3.5-pyexpat-symbols.patch.1 \
	external/python3/python-3.3.5-vs2013.patch.1 \
	external/python3/python-lsan.patch.0 \
	external/python3/ubsan.patch.0 \
	external/python3/python-3.5.tweak.strip.soabi.patch \
	external/python3/python-3.5.0-tcltk.disable.patch \
))

ifneq ($(filter DRAGONFLY FREEBSD LINUX NETBSD OPENBSD SOLARIS,$(OS)),)
$(eval $(call gb_UnpackedTarball_add_patches,python3,\
	external/python3/python-3.3.3-elf-rpath.patch.1 \
))
endif

ifneq ($(ENABLE_RUNTIME_OPTIMIZATIONS),TRUE)
$(eval $(call gb_UnpackedTarball_add_patches,python3,\
	external/python3/python-3.3.3-disable-obmalloc.patch.0 \
))
endif

ifeq ($(OS)-$(COM),WNT-MSC)
ifneq ($(filter 120,$(VCVER)),)
$(eval $(call gb_UnpackedTarball_add_patches,python3,\
	external/python3/python-vc2013.patch.1 \
))
endif
endif

# vim: set noet sw=4 ts=4:
