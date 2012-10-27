# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This build the modules needed on the build side for a cross-compilation
#

SHELL=/usr/bin/env bash
include ../config_build.mk

.PHONY : build

build:
	$(SOLARENV)/bin/build.pl -P$(PARALLELISM) --all -- -P$(PARALLELISM)

