
gb_MAKEFILEDIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))

ifneq ($(strip $(gb_PARTIALBUILD)),)

SRCDIR:=$(realpath $(gb_MAKEFILEDIR)/..)
gb_SourceEnvAndRecurse_reconfigure=true

else

SRCDIR:=$(realpath $(gb_MAKEFILEDIR))
gb_SourceEnvAndRecurse_reconfigure=$(MAKE) -f $(firstword $(MAKEFILE_LIST)) $(SRCDIR)/Env.Host.sh

endif

source-env-and-recurse:
	$(gb_SourceEnvAndRecurse_reconfigure) && \
	if test -f $(SRCDIR)/config.mk ; then . $(SRCDIR)/config.mk; fi && \
	if test -f $(SRCDIR)/Env.Host.sh; then . $(SRCDIR)/Env.Host.sh; fi && \
	if test -z "$${SOLARENV}"; then echo "no configuration found and could not create one" && exit 1; fi && \
	$(MAKE) -f $(firstword $(MAKEFILE_LIST)) $(MAKECMDGOALS)

ifneq ($(strip $(MAKECMDGOALS)),)
ifneq ($(MAKECMDGOALS),$(SRCDIR)/Env.Host.sh)
$(eval $(MAKECMDGOALS) : source-env-and-recurse)
endif
endif
