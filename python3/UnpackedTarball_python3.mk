# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,python3))

#FIXME configure
PYTHON3_TARBALL := b3b2524f72409d919a4137826a870a8f-Python-3.3.0.tar.bz2

$(eval $(call gb_UnpackedTarball_set_tarball,python3,$(PYTHON3_TARBALL)))

$(eval $(call gb_UnpackedTarball_fix_end_of_line,python3,\
	PCbuild/pcbuild.sln \
	PC/VS9.0/pcbuild.sln \
	PC/VS9.0/make_versioninfo.vcproj \
	PC/VS9.0/x64.vsprops \
	PC/VS9.0/_ssl.vcproj \
))

$(eval $(call gb_UnpackedTarball_add_patches,python3,\
	python3/i100492-freebsd.patch.2 \
	python3/python-3.3.0-i42553.patch.2 \
	python3/python-3.3.0-aix.patch.1 \
	python3/python-3.3.0-darwin.patch.1 \
	python3/python-3.3.0-msvc2012.patch.1 \
	python3/python-3.3.0-msvc-disable.patch.1 \
	python3/python-3.3.0-msvc-x64.patch.1 \
	python3/python-3.3.0-ssl.patch.1 \
))

# vim: set noet sw=4 ts=4:
