#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

ifneq ($(filter countoutdated,$(MAKECMDGOALS)),)

gb_SrsTarget_add_template=
gb_SrsTarget_add_templates=
gb_Output_announce=

ifneq ($(strip $(filter-out countoutdated,$(MAKECMDGOALS))),)
countoutdated: $(filter-out countoutdated,$(MAKECMDGOALS))
else
countoutdated: $(.DEFAULT_GOAL)
endif

endif
# vim: set noet ts=4 sw=4:
