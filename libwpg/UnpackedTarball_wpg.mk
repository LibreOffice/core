# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,wpg))

$(eval $(call gb_UnpackedTarball_set_tarball,wpg,$(WPG_TARBALL)))

$(eval $(call gb_UnpackedTarball_add_patches,wpg,\
    libwpg/libwpg-0.2.1-warning.patch \
))
# libwpg/libwpg-0.2.1-warning.patch: see upstream
#  <http://libwpg.git.sourceforge.net/git/gitweb.cgi?p=libwpg/libwpg;a=commit;
#  h=5cf190eb365df00a1446738b3ab0d3e55291def3> "Fix clang build" and
#  <http://libwpg.git.sourceforge.net/git/gitweb.cgi?p=libwpg/libwpg;a=commit;
#  h=f98c1f4308b75ccbb2b98ed8b4011b4f84e841d3> "-Werror,-Wunused-private-field"

# vim: set noet sw=4 ts=4:
