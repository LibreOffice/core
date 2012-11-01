# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
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
# 	Peter Foley <pefoley2@verizon.net>
# Portions created by the Initial Developer are Copyright (C) 2011 the
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
#

$(eval $(call gb_Extension_Extension,nlpsolver,nlpsolver/src/com/sun/star/comp/Calc/NLPSolver))

$(eval $(call gb_Extension_add_file,nlpsolver,components.rdb,$(SRCDIR)/nlpsolver/src/com/sun/star/comp/Calc/NLPSolver/components.rdb))
$(eval $(call gb_Extension_add_file,nlpsolver,nlpsolver.jar,$(OUTDIR)/bin/nlpsolver.jar))
$(eval $(call gb_Extension_add_file,nlpsolver,EvolutionarySolver.jar,$(OUTDIR)/bin/EvolutionarySolver.jar))

$(eval $(call gb_Extension_add_file,nlpsolver,locale/NLPSolverCommon_en_US.default,$(SRCDIR)/nlpsolver/src/locale/NLPSolverCommon_en_US.default))
$(eval $(call gb_Extension_add_file,nlpsolver,locale/NLPSolverStatusDialog_en_US.default,$(SRCDIR)/nlpsolver/src/locale/NLPSolverStatusDialog_en_US.default))

$(eval $(call gb_Extension_localize_properties,nlpsolver,locale/NLPSolverCommon_en_US.properties,$(SRCDIR)/nlpsolver/src/locale/NLPSolverCommon_en_US.properties))
$(eval $(call gb_Extension_localize_properties,nlpsolver,locale/NLPSolverStatusDialog_en_US.properties,$(SRCDIR)/nlpsolver/src/locale/NLPSolverStatusDialog_en_US.properties))

$(eval $(call gb_Extension_add_helpfiles,nlpsolver,$(SRCDIR)/nlpsolver/help/en, \
    com.sun.star.comp.Calc.NLPSolver/Options.xhp \
	com.sun.star.comp.Calc.NLPSolver/Usage.xhp \
))

$(eval $(call gb_Extension_add_helptreefile,nlpsolver,$(SRCDIR)/nlpsolver/help/en,help.tree,com.sun.star.comp.Calc.NLPSolver/help.tree,com.sun.star.comp.Calc.NLPSolver))


# vim: set noet sw=4 ts=4:
