# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# because the installation system is so incredibly inflexible...
$(eval $(call gb_Package_Package,cpputools_bin,$(gb_Executable_BINDIR)))

$(eval $(call gb_Package_add_file,cpputools_bin,bin/uno.bin,uno))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
