# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#   Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
#   The contents of this file are subject to the Mozilla Public License Version
#   1.1 (the "License"); you may not use this file except in compliance with
#   the License or as specified alternatively below. You may obtain a copy of
#   the License at http://www.mozilla.org/MPL/
#
#   Software distributed under the License is distributed on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
#   for the specific language governing rights and limitations under the
#   License.
#
#   Major Contributor(s):
#   [ Copyright (C) 2011 Bjoern Michaelsen <bjoern.michaelsen@canonical.com> (initial developer) ]
#
#   All Rights Reserved.
#
#   For minor contributions see the git repository.
#
#   Alternatively, the contents of this file may be used under the terms of
#   either the GNU General Public License Version 3 or later (the "GPLv3+"), or
#   the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
#   in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
#   instead of those above.

ifeq ($(CROSS_COMPILING),YES)
gb_Module_add_targets_for_build :=
endif

ifeq ($(gb_Side),build)
gb_Module_SKIPTARGETS := check slowcheck subsequentcheck
endif

ifneq ($(strip $(MAKECMDGOALS)),)
# speed up depending on the target
gb_SpeedUpTargets_LEVEL_4 := debugrun help translations
gb_SpeedUpTargets_LEVEL_3 := showmodules $(gb_SpeedUpTargets_LEVEL_4)
gb_SpeedUpTargets_LEVEL_2 := build $(gb_SpeedUpTargets_LEVEL_3)
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
