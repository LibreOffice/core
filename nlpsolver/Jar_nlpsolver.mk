# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

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
