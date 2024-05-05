# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,external/onlineupdate/generated))

onlineupdate_INC := $(gb_CustomTarget_workdir)/external/onlineupdate/generated

$(onlineupdate_INC)/primaryCert.h $(onlineupdate_INC)/secondaryCert.h : \
		$(call gb_ExternalExecutable_get_dependencies,python) \
        | $(call gb_UnpackedTarball_get_target,onlineupdate)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,,1)
	mkdir -p $(dir $@)
	PYPATH=$${PYPATH:+$$PYPATH$(gb_CLASSPATHSEP)}'$(gb_UnpackedTarball_workdir)/onlineupdate/onlineupdate/source/update/updater' $(call gb_ExternalExecutable_get_command,python) $(SRCDIR)/external/onlineupdate/generate.py $@ '$(ONLINEUPDATE_MAR_CERTIFICATEDER)'

$(call gb_CustomTarget_get_target,external/onlineupdate/generated) : \
	$(onlineupdate_INC)/primaryCert.h \
	$(onlineupdate_INC)/secondaryCert.h \

# vim: set noet sw=4 ts=4:
