
.PHONY : bootstrap

gb_PARTIALBUILD := T

ifeq ($(SOLARENV),)
ifeq ($(gb_Side),)
gb_Side:=host
endif
include $(module_directory)/../config_$(gb_Side).mk
endif

$(WORKDIR)/bootstrap:
	@cd $(SRCDIR) && ./bootstrap
	@mkdir -p $(dir $@) && touch $@

bootstrap: $(WORKDIR)/bootstrap

gb_SourceEnvAndRecurse_STAGE=gbuild
include $(SOLARENV)/gbuild/gbuild.mk

$(eval $(call gb_Module_make_global_targets,$(shell ls $(module_directory)/Module*.mk)))
