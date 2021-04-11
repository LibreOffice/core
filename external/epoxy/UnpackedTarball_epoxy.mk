# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,epoxy))

$(eval $(call gb_UnpackedTarball_set_tarball,epoxy,$(EPOXY_TARBALL)))

$(call gb_UnpackedTarball_get_target,epoxy) :| $(call gb_ExternalExecutable_get_dependencies,python)

epoxy_PYTHON := $(call gb_ExternalExecutable_get_command,python)

# previous versions of epoxy bundled the output, but now it has to be generated
$(eval $(call gb_UnpackedTarball_set_pre_action,epoxy,\
	$(epoxy_PYTHON) ./src/gen_dispatch.py --srcdir src --includedir include/epoxy registry/gl.xml && \
	$(epoxy_PYTHON) ./src/gen_dispatch.py --srcdir src --includedir include/epoxy registry/glx.xml && \
	$(epoxy_PYTHON) ./src/gen_dispatch.py --srcdir src --includedir include/epoxy registry/egl.xml && \
	$(epoxy_PYTHON) ./src/gen_dispatch.py --srcdir src --includedir include/epoxy registry/wgl.xml \
))

$(eval $(call gb_UnpackedTarball_set_patchlevel,epoxy,0))

$(eval $(call gb_UnpackedTarball_add_patches,epoxy, \
    external/epoxy/epoxy.visibility.patch \
    external/epoxy/epoxy.windows.api.patch \
    external/epoxy/epoxy.noegl.by.default.patch \
    external/epoxy/clang-cl.patch \
    external/epoxy/epoxy.pyresource-leak.patch \
))

# vim: set noet sw=4 ts=4:
