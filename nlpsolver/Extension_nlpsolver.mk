# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#

$(eval $(call gb_Extension_Extension,nlpsolver,nlpsolver/src/com/sun/star/comp/Calc/NLPSolver))

$(eval $(call gb_Extension_use_default_description,nlpsolver))
$(eval $(call gb_Extension_use_default_license,nlpsolver))

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
