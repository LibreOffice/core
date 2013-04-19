#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

gb_CountersOutdated_COUNTER_ALL:=
gb_CountersOutdated_COUNTER_TYPES:=

.PHONY: countoutdated
countoutdated:
	$(info total outdated files: $(words $(gb_CountersOutdated_COUNTER_ALL)))
	$(info types of outdated files: $(gb_CountersOutdated_TYPES))
	$(foreach type,$(gb_CountersOutdated_TYPES),$(info $(type): $(words $(gb_CountersOutdated_COUNTER_$(type)))))
	@true

ifneq ($(filter countoutdated,$(MAKECMDGOALS)),)

$(WORKDIR)/%:
	$(eval gb_CountersOutdated_COUNTER_ALL+= x)
	$(eval gb_CountersOutdated__TYPE=$(firstword $(subst /, ,$*)))
	$(if $(filter undefined,$(origin gb_CountersOutdated_COUNTER_$(gb_CountersOutdated__TYPE))),$(eval gb_CountersOutdated_COUNTER_$(gb_CountersOutdated__TYPE):=) $(eval gb_CountersOutdated_TYPES+=$(gb_CountersOutdated__TYPE)))
	$(eval gb_CountersOutdated_COUNTER_$(gb_CountersOutdated__TYPE)+= x)
	@true
	
$(OUTDIR)/%:
	@true

endif
# vim: set noet ts=4 sw=4:
