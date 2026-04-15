# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-

module_directory=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))

ifeq ($(origin SRC_ROOT),undefined)
SRC_ROOT=$(module_directory)..
endif

include $(SRC_ROOT)/solenv/gbuild/partial_build.mk

# vim: set noet sw=4 ts=4:
