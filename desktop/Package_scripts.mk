# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,desktop_scripts,$(SRCDIR)/desktop/scripts))

ifneq ($(OS),WNT)

$(eval $(call gb_Package_add_file,desktop_scripts,bin/gdbtrace,gdbtrace))
$(eval $(call gb_Package_add_file,desktop_scripts,bin/sbase,sbase.sh))
$(eval $(call gb_Package_add_file,desktop_scripts,bin/scalc,scalc.sh))
$(eval $(call gb_Package_add_file,desktop_scripts,bin/sdraw,sdraw.sh))
$(eval $(call gb_Package_add_file,desktop_scripts,bin/simpress,simpress.sh))
$(eval $(call gb_Package_add_file,desktop_scripts,bin/smath,smath.sh))
$(eval $(call gb_Package_add_file,desktop_scripts,bin/swriter,swriter.sh))
$(eval $(call gb_Package_add_file,desktop_scripts,bin/unoinfo,unoinfo.sh))
$(eval $(call gb_Package_add_file,desktop_scripts,bin/unopkg,unopkg.sh))

endif

# vim: set ts=4 sw=4 et:
