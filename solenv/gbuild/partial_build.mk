ifeq ($(gb_Side),)
gb_Side:=host
endif

ifeq (,$(BUILDDIR))
BUILDDIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))../..
endif

ifeq ($(SOLARENV),)
include $(BUILDDIR)/config_$(gb_Side).mk
endif

gb_PARTIAL_BUILD := T
include $(SOLARENV)/gbuild/gbuild.mk

$(eval $(call gb_Module_make_global_targets,$(wildcard $(module_directory)/Module*.mk)))
