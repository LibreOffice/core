# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

osxicons_LIST := $(shell $(PERL) -w $(SRCDIR)/sysui/desktop/macosx/list_icons.pl < $(SRCDIR)/sysui/desktop/macosx/Info.plist)

$(eval $(call gb_Zip_Zip,osxicons,$(SRCDIR)/sysui/desktop))

$(eval $(call gb_Zip_add_files,osxicons,\
	$(addprefix icons/,$(osxicons_LIST)) \
	macosx/Info.plist \
	macosx/extract_icons_names.pl \
))

# vim: set ts=4 sw=4 et:
