
dummy:
	@if test -f ./config.mk; then . ./config.mk; else if test -f ../config.mk ; then . ../config.mk; fi; fi && \
	if test -f ./Env.Host.sh; then . ./Env.Host.sh; else if test -f ../Env.Host.sh; then . ../Env.Host.sh; fi ; fi && \
	if test -z "$${SOLARENV}"; then echo "No environment set!" 2>&1; exit 1; fi && \
	$(MAKE) $(MAKECMDGOALS)

ifneq ($(strip $(MAKECMDGOALS)),)
$(eval $(MAKECMDGOALS) : dummy)
endif

