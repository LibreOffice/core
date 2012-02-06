# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This build the modules needed on the build side for a cross-compilation
#

SHELL=/usr/bin/env bash
include ../config_build.mk

#
# Bootstap
#
$(WORKDIR)/bootstrap:
	@cd $(SRC_ROOT) && ./bootstrap
	@mkdir -p $(dir $@) && touch $@

bootstrap: $(WORKDIR)/bootstrap

.PHONY : build

build:
	$(SOLARENV)/bin/build.pl -P$(BUILD_NCPUS) --all -- -P$(GMAKE_PARALLELISM)

