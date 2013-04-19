# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,jurt_jnilib,$(call gb_CustomTarget_get_workdir,jurt/util)))

$(eval $(call gb_Package_add_file,jurt_jnilib,lib/$(basename $(call gb_Library_get_filename,jpipe)).jnilib,libjpipe.jnilib))

# vim:set noet sw=4 ts=4:
