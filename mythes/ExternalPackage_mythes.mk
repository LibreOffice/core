# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,mythes,mythes))

ifneq ($(COM),MSC)

$(eval $(call gb_ExternalPackage_use_external_project,mythes,mythes))

$(eval $(call gb_ExternalPackage_add_file,mythes,lib/libmythes.a,.libs/libmythes-1.2.a))

endif

$(eval $(call gb_ExternalPackage_add_files,mythes,bin,\
	th_gen_idx.pl \
))
$(eval $(call gb_ExternalPackage_add_unpacked_files,mythes,inc,\
    mythes.hxx \
))

# vim: set noet sw=4 ts=4:
