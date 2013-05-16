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
