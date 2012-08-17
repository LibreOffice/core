# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2011 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_Jar_Jar,nlpsolver))

$(eval $(call gb_Jar_use_jars,nlpsolver,\
	ridl \
	unoil \
	jurt \
	juh \
	java_uno \
	EvolutionarySolver \
))

$(eval $(call gb_Jar_set_packageroot,nlpsolver,com))

$(eval $(call gb_Jar_set_manifest,nlpsolver,$(SRCDIR)/nlpsolver/src/com/sun/star/comp/Calc/NLPSolver/Manifest.mf))

$(eval $(call gb_Jar_add_sourcefiles,nlpsolver,\
    nlpsolver/src/com/sun/star/comp/Calc/NLPSolver/Registration \
    nlpsolver/src/com/sun/star/comp/Calc/NLPSolver/PropertyInfo \
    nlpsolver/src/com/sun/star/comp/Calc/NLPSolver/dialogs/controls/BaseControl \
    nlpsolver/src/com/sun/star/comp/Calc/NLPSolver/dialogs/controls/ProgressBar \
    nlpsolver/src/com/sun/star/comp/Calc/NLPSolver/dialogs/controls/Button \
    nlpsolver/src/com/sun/star/comp/Calc/NLPSolver/dialogs/controls/TextAlign \
    nlpsolver/src/com/sun/star/comp/Calc/NLPSolver/dialogs/controls/LabeledControl \
    nlpsolver/src/com/sun/star/comp/Calc/NLPSolver/dialogs/controls/Label \
    nlpsolver/src/com/sun/star/comp/Calc/NLPSolver/dialogs/ModalState \
    nlpsolver/src/com/sun/star/comp/Calc/NLPSolver/dialogs/DummyEvolutionarySolverStatusDialog \
    nlpsolver/src/com/sun/star/comp/Calc/NLPSolver/dialogs/IEvolutionarySolverStatusDialog \
    nlpsolver/src/com/sun/star/comp/Calc/NLPSolver/dialogs/EvolutionarySolverStatusUno \
    nlpsolver/src/com/sun/star/comp/Calc/NLPSolver/dialogs/BaseDialog \
    nlpsolver/src/com/sun/star/comp/Calc/NLPSolver/DEPSSolverImpl \
    nlpsolver/src/com/sun/star/comp/Calc/NLPSolver/SCOSolverImpl \
    nlpsolver/src/com/sun/star/comp/Calc/NLPSolver/BaseEvolutionarySolver \
    nlpsolver/src/com/sun/star/comp/Calc/NLPSolver/ResourceManager \
    nlpsolver/src/com/sun/star/comp/Calc/NLPSolver/BaseNLPSolver \
))

# vim: set noet sw=4 ts=4:
