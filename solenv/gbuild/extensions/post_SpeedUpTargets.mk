# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

ifeq ($(CROSS_COMPILING),YES)
gb_Module_add_targets_for_build :=
gb_Module_SKIPTARGETS := check slowcheck subsequentcheck
endif

ifeq ($(gb_Side),build)
gb_Module_SKIPTARGETS := check slowcheck subsequentcheck
endif

ifeq ($(MAKECMDGOALS),build)
gb_Module_SKIPTARGETS := check slowcheck subsequentcheck
endif

ifneq ($(strip $(MAKECMDGOALS)),)
# speed up depending on the target
gb_SpeedUpTargets_LEVEL_4 := debugrun help translations
gb_SpeedUpTargets_LEVEL_3 := showmodules $(gb_SpeedUpTargets_LEVEL_4)
gb_SpeedUpTargets_LEVEL_2 := $(gb_SpeedUpTargets_LEVEL_3) install-package-%
gb_SpeedUpTargets_LEVEL_1 := clean showdeliverables $(gb_SpeedUpTargets_LEVEL_2)

ifeq (T,$(if $(filter-out $(gb_SpeedUpTargets_LEVEL_1),$(MAKECMDGOALS)),,T))
gb_FULLDEPS :=

ifeq (T,$(if $(filter-out $(gb_SpeedUpTargets_LEVEL_2),$(MAKECMDGOALS)),,T))
gb_Module_SKIPTARGETS += check slowcheck subsequentcheck

ifeq (T,$(if $(filter-out $(gb_SpeedUpTargets_LEVEL_3),$(MAKECMDGOALS)),,T))
gb_Module_SKIPTARGETS += build

ifeq (T,$(if $(filter-out $(gb_SpeedUpTargets_LEVEL_4),$(MAKECMDGOALS)),,T))
gb_Module_SKIPTARGETS += module

endif
endif
endif
endif

endif


ifneq (,$(filter build,$(gb_Module_SKIPTARGETS)))
gb_Module_add_target =
endif

ifneq (,$(filter check,$(gb_Module_SKIPTARGETS)))
gb_Module_add_check_target =
endif

ifneq (,$(filter slowcheck,$(gb_Module_SKIPTARGETS)))
gb_Module_add_slowcheck_target =
endif

ifneq (,$(filter subsequentcheck,$(gb_Module_SKIPTARGETS)))
gb_Module_add_subsequentcheck_target =
endif

ifneq (,$(filter module,$(gb_Module_SKIPTARGETS)))
gb_Module_add_moduledir =
endif

# vim:set shiftwidth=4 softtabstop=4 noexpandtab:
