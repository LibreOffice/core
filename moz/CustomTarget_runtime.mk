# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

MOZ_DEL_LIST := freebl3.dll \
	nspr4.dll \
	nss3.dll \
	plc4.dll \
	plds4.dll \
	smime3.dll \
	softokn3.dll \
	ssl3.dll

$(eval $(call gb_CustomTarget_CustomTarget,moz/runtime))

$(eval $(call gb_CustomTarget_register_target,moz/runtime,mozruntime.zip))

$(call gb_CustomTarget_get_workdir,moz/runtime)/mozruntime.zip: $(TARFILE_LOCATION)/$(MOZ_ZIP_RUNTIME)
	cp $< $@
	zip -qd $@ $(MOZ_DEL_LIST)

# vim: set noet sw=4 ts=4:
