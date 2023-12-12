# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,onlineupdate/generated))

onlineupdate_INC := $(call gb_CustomTarget_get_workdir,onlineupdate/generated)/onlineupdate

# For debug purposes, ONLINEUPDATE_MAR_CERTIFICATEDER can be empty, but gen_cert_header.py always
# expects an existing certificate-der=... pathname, so as a hack fall back to the generated
# update.ini itself as the pathanme of the certfiicate DER file (though that will cause
# gen_cert_header.py to generate nonsense data, of course):
$(call gb_CustomTarget_get_workdir,onlineupdate/generated)/update.ini: | \
        $(call gb_CustomTarget_get_workdir,onlineupdate/generated)/.dir
	printf '[Updater]\ncertificate-der=%s\n' '$(or $(ONLINEUPDATE_MAR_CERTIFICATEDER),$@)' > $@

$(onlineupdate_INC)/primaryCert.h : \
		$(SRCDIR)/onlineupdate/source/update/updater/gen_cert_header.py \
		$(call gb_ExternalExecutable_get_dependencies,python) \
		$(call gb_CustomTarget_get_workdir,onlineupdate/generated)/update.ini
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,,1)
	mkdir -p $(dir $@)
	$(call gb_ExternalExecutable_get_command,python) $(SRCDIR)/onlineupdate/source/update/updater/gen_cert_header.py "primaryCertData" $(call gb_CustomTarget_get_workdir,onlineupdate/generated)/update.ini > $(onlineupdate_INC)/primaryCert.h

$(onlineupdate_INC)/secondaryCert.h : \
		$(SRCDIR)/onlineupdate/source/update/updater/gen_cert_header.py \
		$(call gb_ExternalExecutable_get_dependencies,python) \
		$(call gb_CustomTarget_get_workdir,onlineupdate/generated)/update.ini
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,,1)
	mkdir -p $(dir $@)
	$(call gb_ExternalExecutable_get_command,python) $(SRCDIR)/onlineupdate/source/update/updater/gen_cert_header.py "secondaryCertData" $(call gb_CustomTarget_get_workdir,onlineupdate/generated)/update.ini > $(onlineupdate_INC)/secondaryCert.h

$(call gb_CustomTarget_get_target,onlineupdate/generated) : \
	$(onlineupdate_INC)/primaryCert.h \
	$(onlineupdate_INC)/secondaryCert.h \

# vim: set noet sw=4 ts=4:
