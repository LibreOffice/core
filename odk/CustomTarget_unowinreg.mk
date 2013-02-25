# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,odk/unowinreg))

$(eval $(call gb_CustomTarget_register_target,odk/unowinreg,unowinreg.dll))

ifeq ($(BUILD_UNOWINREG),YES)
ifeq ($(COM),MSC)
$(call gb_CustomTarget_get_workdir,odk/unowinreg)/unowinreg.dll: $(call gb_Library_get_target,unowinreg)
	cp $< $@
else # COM=GCC
$(call gb_CustomTarget_get_workdir,odk/unowinreg)/unowinreg.dll : \
		$(SRCDIR)/odk/source/unowinreg/win/unowinreg.cxx
	$(MINGWCXX) -Wall -D_JNI_IMPLEMENTATION_ \
		-I$(JAVA_HOME)/include \
		$(if $(filter LINUX,$(OS)),-I$(JAVA_HOME)/include/linux,\
		$(if $(filter FREEBSD,$(OS)),-I$(JAVA_HOME)/include/freebsd \
		-I$(JAVA_HOME)/include/bsd -I$(JAVA_HOME)/include/linux,\
		$(if $(filter NETBSD,$(OS)),-I$(JAVA_HOME)/include/netbsd))) \
		-shared -o $@ $< \
		-Wl,--kill-at -lkernel32 -ladvapi32
	$(MINGWSTRIP) $@
endif
else # BUILD_UNOWINREG=NO
$(call gb_CustomTarget_get_workdir,odk/unowinreg)/unowinreg.dll: $(TARFILE_LOCATION)/$(UNOWINREG_DLL)
	cp $< $@
endif

# vim: set noet sw=4 ts=4:
