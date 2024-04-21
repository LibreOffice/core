# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

cli_ure_source_MAKEFILE := $(lastword $(MAKEFILE_LIST))

include $(SRCDIR)/cli_ure/version/version.txt

$(eval $(call gb_CustomTarget_CustomTarget,cli_ure/source))

$(call gb_CustomTarget_get_target,cli_ure/source) : \
	$(call gb_CustomTarget_get_workdir,cli_ure/source)/basetypes/assembly.cs \
	$(call gb_CustomTarget_get_workdir,cli_ure/source)/native/assembly.cxx \
	$(call gb_CustomTarget_get_workdir,cli_ure/source)/ure/assembly.cs

$(call gb_CustomTarget_get_workdir,cli_ure/source)/basetypes/assembly.cs : \
        $(SRCDIR)/cli_ure/source/basetypes/assembly.cs \
        $(SRCDIR)/cli_ure/version/version.txt \
        $(cli_ure_source_MAKEFILE) \
        | $(call gb_CustomTarget_get_workdir,cli_ure/source)/basetypes/.dir
	sed -e "s/@CLI_BASETYPES_NEW_VERSION@/$(CLI_BASETYPES_NEW_VERSION)/g" $< > $@

$(call gb_CustomTarget_get_workdir,cli_ure/source)/native/assembly.cxx : \
        $(SRCDIR)/cli_ure/source/native/assembly.cxx \
        $(SRCDIR)/cli_ure/version/version.txt \
        $(cli_ure_source_MAKEFILE) \
        | $(call gb_CustomTarget_get_workdir,cli_ure/source)/native/.dir
	sed -e "s/@CLI_CPPUHELPER_NEW_VERSION@/$(CLI_CPPUHELPER_NEW_VERSION)/g" $< > $@

$(call gb_CustomTarget_get_workdir,cli_ure/source)/ure/assembly.cs : \
        $(SRCDIR)/cli_ure/source/ure/assembly.cs \
        $(SRCDIR)/cli_ure/version/version.txt \
        $(cli_ure_source_MAKEFILE) \
        | $(call gb_CustomTarget_get_workdir,cli_ure/source)/ure/.dir
	sed -e "s/@CLI_URE_NEW_VERSION@/$(CLI_URE_NEW_VERSION)/g" $< > $@

# vim: set noet sw=4 ts=4:
