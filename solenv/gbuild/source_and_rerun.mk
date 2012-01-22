
gb_MAKEFILEDIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))

define gb_SourceEnvAndRecurse_recurse
$(MAKE) -f $(firstword $(MAKEFILE_LIST)) -j $${GMAKE_PARALLELISM} $(1) gb_SourceEnvAndRecurse_STAGE=$(2)
endef

ifneq ($(strip $(gb_PARTIALBUILD)),)

SRCDIR:=$(realpath $(gb_MAKEFILEDIR)/..)
gb_SourceEnvAndRecurse_reconfigure=true
gb_SourceEnvAndRecurse_buildpl=true

else

SRCDIR:=$(realpath $(gb_MAKEFILEDIR))
gb_SourceEnvAndRecurse_reconfigure=$(call gb_SourceEnvAndRecurse_recurse,$(SRCDIR)/config_host.mk,reconfigure)
gb_SourceEnvAndRecurse_buildpl=$(call gb_SourceEnvAndRecurse_recurse,$(MAKECMDGOALS),buildpl)

endif

source-env-and-recurse:
	@$(gb_SourceEnvAndRecurse_reconfigure) && \
	if test -f $(SRCDIR)/config_host.mk ; then . $(SRCDIR)/config_host.mk; fi && \
	if test -z "$${SOLARENV}"; then echo "no configuration found and could not create one" && exit 1; fi && \
	$(gb_SourceEnvAndRecurse_buildpl) && \
	$(call gb_SourceEnvAndRecurse_recurse,$(MAKECMDGOALS),gbuild) \


ifneq ($(strip $(MAKECMDGOALS)),)
ifneq ($(MAKECMDGOALS),$(SRCDIR)/config_host.mk)
$(eval $(MAKECMDGOALS) : source-env-and-recurse)
endif
endif
