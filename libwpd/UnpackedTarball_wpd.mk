# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,wpd))

$(eval $(call gb_UnpackedTarball_set_tarball,wpd,$(WPD_TARBALL)))

$(eval $(call gb_UnpackedTarball_add_patches,wpd,\
	libwpd/libwpd-0.9.4.patch \
	libwpd/libwpd-0.9.4-warning.patch \
	libwpd/libwpd-0.9.4-msvc-warning.patch \
))
# libwpd/libwpd-0.9.4-warning.patch: see upstream
#  <http://libwpd.git.sourceforge.net/git/gitweb.cgi?p=libwpd/libwpd;a=commit;
#  h=ca59d3ce82d1695e381255912ce1b60e826b2ca6> "-Werror,-Wunused-private-field"
# libwpd/libwpd-0.9.4-msvc-warning.patch: see upstream
#  <http://libwpd.git.sourceforge.net/git/gitweb.cgi?p=libwpd/libwpd;a=commit;
#  h=fb3827aeb5de64ed84eb1140bae9e32e50ca5133> "MSVC warning C4310: cast
#  truncates constant value"

# vim: set noet sw=4 ts=4:
