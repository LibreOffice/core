# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

.PHONY: all

include config_host_callcatcher.mk

export CC:=callcatcher $(CC)
export CXX:=callcatcher $(CXX)
ifeq ($(AR),)
export AR:=callarchive ar
else
export AR:=callarchive $(AR)
endif
export dbglevel:=2

include $(SOLARENV)/gbuild/gbuild.mk

findunusedcode:
	$(GNUMAKE) -j $(PARALLELISM) $(GMAKE_OPTIONS) -f Makefile.build
	ooinstall -l $(DEVINSTALLDIR)/opt
	$(GNUMAKE) -j $(PARALLELISM) $(GMAKE_OPTIONS) -f Makefile.build subsequentcheck
	callanalyse $(WORKDIR)/LinkTarget/*/* > unusedcode.all

# vim: set noet sw=4 ts=4:
