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
	PCbuild/libffi.props \
	PCbuild/pcbuild.sln \
))

ifneq ($(MSYSTEM),)
# use binary flag so patch from git-bash won't choke on mixed line-endings in patches
$(eval $(call gb_UnpackedTarball_set_patchflags,python3,--binary))
endif

$(eval $(call gb_UnpackedTarball_add_patches,python3,\
	external/python3/i100492-freebsd.patch.1 \
	external/python3/python-3.3.0-darwin.patch.1 \
	external/python3/python-3.8-msvc-libffi.patch.1 \
	external/python3/python-3.7.6-msvc-ssl.patch.1 \
	external/python3/python-3.5.4-msvc-disable.patch.1 \
	external/python3/ubsan.patch.0 \
	external/python3/py_deprecated_warning.patch.0 \
	external/python3/python-3.5.tweak.strip.soabi.patch \
	external/python3/darwin.patch.0 \
	external/python3/macos-11.patch.0 \
	external/python3/tsan.patch.0 \
	external/python3/init-sys-streams-cant-initialize-stdin.patch.0 \
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

ifneq ($(SYSTEM_ZLIB),TRUE)
$(eval $(call gb_UnpackedTarball_add_patches,python3, \
    external/python3/internal-zlib.patch.0 \
))
endif

# vim: set noet sw=4 ts=4:
