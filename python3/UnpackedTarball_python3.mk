# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,python3))

$(eval $(call gb_UnpackedTarball_set_tarball,python3,$(PYTHON_TARBALL)))

$(eval $(call gb_UnpackedTarball_fix_end_of_line,python3,\
	PCbuild/pcbuild.sln \
))

$(eval $(call gb_UnpackedTarball_add_patches,python3,\
	python3/python-3.3.0-i42553.patch.2 \
	python3/python-3.3.0-msvc2012.patch.1 \
	python3/python-3.3.0-msvc-disable.patch.1 \
	python3/python-3.3.0-msvc-x64.patch.1 \
	python3/python-3.3.0-ssl.patch.1 \
	python3/python-3.3.0-gcc-4.8.patch.1 \
	python3/python-3.3.0-pythreadstate.patch.1 \
	python3/python-3.3.3-py17797.patch.1 \
	python3/python-3.3.3-msvc2012-winxp.patch.1 \
	python3/python-msvc-disable-sse2.patch.1 \
	python3/ubsan.patch.0 \
	python3/python-3.3.7-py30657.patch.1 \
))

ifneq ($(filter DRAGONFLY FREEBSD LINUX NETBSD OPENBSD SOLARIS,$(OS)),)
$(eval $(call gb_UnpackedTarball_add_patches,python3,\
	python3/python-3.3.3-elf-rpath.patch.1 \
))
endif

# vim: set noet sw=4 ts=4:
