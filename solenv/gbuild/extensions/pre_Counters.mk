# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
#       Bjoern Michaelsen, Canonical Ltd. <bjoern.michaelsen@canonical.com>
# Portions created by the Initial Developer are Copyright (C) 2010 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

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
