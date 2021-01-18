# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,shell_senddoc,$(SRCDIR)/shell/source/unix/misc))

ifeq ($(OS), MACOSX)
# only mach-o binaries allowed in bin folder (signing scripts would require extended attributes)
# so install it into Resources folder and use a symlink instead
# see https://developer.apple.com/library/archive/technotes/tn2206/_index.html
$(eval $(call gb_Package_add_file,shell_senddoc,$(LIBO_SHARE_FOLDER)/senddoc,senddoc.sh))
$(eval $(call gb_Package_add_symbolic_link,shell_senddoc,$(LIBO_BIN_FOLDER)/senddoc,../$(LIBO_SHARE_FOLDER)/senddoc))
else
$(eval $(call gb_Package_add_file,shell_senddoc,$(LIBO_BIN_FOLDER)/senddoc,senddoc.sh))
endif

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
