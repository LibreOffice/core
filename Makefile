# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# Copyright 2012 LibreOffice contributors.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# must not be empty so we can have a target for it
ifeq ($(MAKECMDGOALS),)
MAKECMDGOALS:=all
endif

SHELL=/usr/bin/env bash
SRCDIR:=$(patsubst %/,%,$(dir $(realpath $(firstword $(MAKEFILE_LIST)))))

.PHONY : $(filter-out $(SRCDIR)/config_host.mk,$(MAKECMDGOALS))

# recursively invoke Makefile.top, which includes config_host.mk
$(filter-out help,$(firstword $(MAKECMDGOALS))) : $(SRCDIR)/config_host.mk
	$(MAKE) -r -f $(SRCDIR)/Makefile.top $(MAKECMDGOALS)

# run configure in an environment not polluted by config_host.mk
$(SRCDIR)/config_host.mk : \
		$(SRCDIR)/config_host.mk.in \
		$(SRCDIR)/ooo.lst.in \
		$(SRCDIR)/configure.ac \
		$(SRCDIR)/autogen.lastrun
	./autogen.sh

# dummy rule in case autogen.lastrun does not exist
$(SRCDIR)/autogen.lastrun:
	@true

help:
	@cat $(SRCDIR)/solenv/gbuild/gbuild.help.txt
	@true

# vim: set noet sw=4 ts=4:
