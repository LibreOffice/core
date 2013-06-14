# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,libatomic_ops,libatomic_ops))

$(eval $(call gb_ExternalPackage_use_external_project,libatomic_ops,libatomic_ops))

ifeq ($(OS)$(COM),WNTMSC)
$(eval $(call gb_ExternalPackage_add_file,libatomic_ops,lib/atomic_ops-7.2d.lib,src/lib/.libs/libatomic_ops-7.2d.lib))
else
$(eval $(call gb_ExternalPackage_add_file,libatomic_ops,lib/libatomic_ops-7.2d.a,src/lib/.libs/libatomic_ops-7.2d.a))
endif

# vim: set noet sw=4 ts=4:
