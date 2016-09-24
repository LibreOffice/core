# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

wintools_WORKDIR := $(call gb_CustomTarget_get_workdir,setup_native/wintools)
wintools_SRCDIR := $(SRCDIR)/setup_native/source/win32/wintools

$(eval $(call gb_CustomTarget_CustomTarget,setup_native/wintools))

$(eval $(call gb_CustomTarget_register_targets,setup_native/wintools,\
	makecab.exe \
	msidb.exe \
	msiinfo.exe \
	msimsp.exe \
	msitran.exe \
))

$(wintools_WORKDIR)/makecab.exe: $(wintools_SRCDIR)/makecab/makecab.c $(wintools_SRCDIR)/makecab/parseddf.c
	$(WINEGCC) -o $@ $< -m32 -mconsole -lmsi

$(wintools_WORKDIR)/msidb.exe: $(wintools_SRCDIR)/msidb/msidb.c
	$(WINEGCC) -o $@ $< -m32 -municode -lmsi

$(wintools_WORKDIR)/msiinfo.exe: $(wintools_SRCDIR)/msiinfo/msiinfo.c
	$(WINEGCC) -o $@ $< -m32 -mconsole -municode -lmsi

$(wintools_WORKDIR)/msimsp.exe: $(wintools_SRCDIR)/msimsp/msimsp.c
	$(WINEGCC) -o $@ $< -m32 -mconsole -lmsi

$(wintools_WORKDIR)/msitran.exe: $(wintools_SRCDIR)/msitran/msitran.c
	$(WINEGCC) -o $@ $< -m32 -mconsole -lmsi

# vim: set noet sw=4 ts=4:
