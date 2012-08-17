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

$(eval $(call gb_Jar_Jar,EvolutionarySolver))

$(eval $(call gb_Jar_set_packageroot,EvolutionarySolver,net/adaptivebox))

$(eval $(call gb_Jar_set_manifest,EvolutionarySolver,$(SRCDIR)/nlpsolver/ThirdParty/EvolutionarySolver/src/net/adaptivebox/Manifest.mf))

$(eval $(call gb_Jar_add_sourcefiles,EvolutionarySolver,\
    nlpsolver/ThirdParty/EvolutionarySolver/src/net/adaptivebox/goodness/SRComparator \
    nlpsolver/ThirdParty/EvolutionarySolver/src/net/adaptivebox/goodness/ACRComparator \
    nlpsolver/ThirdParty/EvolutionarySolver/src/net/adaptivebox/goodness/BCHComparator \
    nlpsolver/ThirdParty/EvolutionarySolver/src/net/adaptivebox/goodness/IGoodnessCompareEngine \
    nlpsolver/ThirdParty/EvolutionarySolver/src/net/adaptivebox/goodness/PenaltyComparator \
    nlpsolver/ThirdParty/EvolutionarySolver/src/net/adaptivebox/global/CompareValue \
    nlpsolver/ThirdParty/EvolutionarySolver/src/net/adaptivebox/global/GlobalString \
    nlpsolver/ThirdParty/EvolutionarySolver/src/net/adaptivebox/global/BasicArray \
    nlpsolver/ThirdParty/EvolutionarySolver/src/net/adaptivebox/global/GlobalCompare \
    nlpsolver/ThirdParty/EvolutionarySolver/src/net/adaptivebox/global/BasicBound \
    nlpsolver/ThirdParty/EvolutionarySolver/src/net/adaptivebox/global/OutputMethods \
    nlpsolver/ThirdParty/EvolutionarySolver/src/net/adaptivebox/global/RandomGenerator \
    nlpsolver/ThirdParty/EvolutionarySolver/src/net/adaptivebox/global/GlobalFile \
    nlpsolver/ThirdParty/EvolutionarySolver/src/net/adaptivebox/global/IUpdateCycleEngine \
    nlpsolver/ThirdParty/EvolutionarySolver/src/net/adaptivebox/global/BasicTag \
    nlpsolver/ThirdParty/EvolutionarySolver/src/net/adaptivebox/encode/EvalStruct \
    nlpsolver/ThirdParty/EvolutionarySolver/src/net/adaptivebox/encode/EvalElement \
    nlpsolver/ThirdParty/EvolutionarySolver/src/net/adaptivebox/encode/IEncodeEngine \
    nlpsolver/ThirdParty/EvolutionarySolver/src/net/adaptivebox/space/ILocationEngine \
    nlpsolver/ThirdParty/EvolutionarySolver/src/net/adaptivebox/space/BasicPoint \
    nlpsolver/ThirdParty/EvolutionarySolver/src/net/adaptivebox/space/DesignSpace \
    nlpsolver/ThirdParty/EvolutionarySolver/src/net/adaptivebox/space/DesignDim \
    nlpsolver/ThirdParty/EvolutionarySolver/src/net/adaptivebox/problem/UnconstrainedProblemEncoder \
    nlpsolver/ThirdParty/EvolutionarySolver/src/net/adaptivebox/problem/ProblemEncoder \
    nlpsolver/ThirdParty/EvolutionarySolver/src/net/adaptivebox/knowledge/Library \
    nlpsolver/ThirdParty/EvolutionarySolver/src/net/adaptivebox/knowledge/ILibEngine \
    nlpsolver/ThirdParty/EvolutionarySolver/src/net/adaptivebox/knowledge/SearchPoint \
    nlpsolver/ThirdParty/EvolutionarySolver/src/net/adaptivebox/deps/DEPSAgent \
    nlpsolver/ThirdParty/EvolutionarySolver/src/net/adaptivebox/deps/behavior/PSGTBehavior \
    nlpsolver/ThirdParty/EvolutionarySolver/src/net/adaptivebox/deps/behavior/AbsGTBehavior \
    nlpsolver/ThirdParty/EvolutionarySolver/src/net/adaptivebox/deps/behavior/DEGTBehavior \
    nlpsolver/ThirdParty/EvolutionarySolver/src/net/adaptivebox/sco/SCAgent \
))

# vim: set noet sw=4 ts=4:
