# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,desktop_scripts_install,$(SRCDIR)/desktop/scripts))

$(eval $(call gb_Package_set_outdir,desktop_scripts_install,$(gb_INSTROOT)))

ifneq ($(OS),WNT)

$(eval $(call gb_Package_add_file,desktop_scripts_install,$(gb_PROGRAMDIRNAME)/gdbtrace,gdbtrace))
$(eval $(call gb_Package_add_file,desktop_scripts_install,$(gb_PROGRAMDIRNAME)/sbase,sbase.sh))
$(eval $(call gb_Package_add_file,desktop_scripts_install,$(gb_PROGRAMDIRNAME)/scalc,scalc.sh))
$(eval $(call gb_Package_add_file,desktop_scripts_install,$(gb_PROGRAMDIRNAME)/sdraw,sdraw.sh))
$(eval $(call gb_Package_add_file,desktop_scripts_install,$(gb_PROGRAMDIRNAME)/simpress,simpress.sh))
$(eval $(call gb_Package_add_file,desktop_scripts_install,$(gb_PROGRAMDIRNAME)/smath,smath.sh))
$(eval $(call gb_Package_add_file,desktop_scripts_install,$(gb_PROGRAMDIRNAME)/swriter,swriter.sh))
$(eval $(call gb_Package_add_file,desktop_scripts_install,$(gb_PROGRAMDIRNAME)/unoinfo,unoinfo.sh))
$(eval $(call gb_Package_add_file,desktop_scripts_install,$(gb_PROGRAMDIRNAME)/unopkg,unopkg.sh))

endif

# vim: set ts=4 sw=4 noet:
