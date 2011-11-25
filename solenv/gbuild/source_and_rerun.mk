
gb_MAKEFILEDIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))

source-env-and-recurse:
	@if test -f $(gb_MAKEFILEDIR)/config.mk; then . $(gb_MAKEFILEDIR)/config.mk; else if test -f $(gb_MAKEFILEDIR)/../config.mk ; then . $(gb_MAKEFILEDIR)/../config.mk; fi; fi && \
	if test -f $(gb_MAKEFILEDIR)/Env.Host.sh; then . $(gb_MAKEFILEDIR)/Env.Host.sh; else if test -f $(gb_MAKEFILEDIR)/../Env.Host.sh; then . $(gb_MAKEFILEDIR)/../Env.Host.sh; fi ; fi && \
	if test -z "$${SOLARENV}"; then echo "No environment set!" 2>&1; exit 1; fi && \
	$(MAKE) -f $(firstword $(MAKEFILE_LIST)) $(MAKECMDGOALS)

ifneq ($(strip $(MAKECMDGOALS)),)
$(eval $(MAKECMDGOALS) : source-env-and-recurse)
endif

