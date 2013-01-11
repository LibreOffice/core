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

$(eval $(call gb_UnoApiMerge_UnoApiMerge,types))

$(eval $(call gb_UnoApiMerge_use_api,types,\
    udkapi \
    offapi \
))

$(eval $(call gb_UnoApiMerge_set_reference_rdbfile,types,\
    offapi/type_reference/types \
))

# ugly hack for now!
statistic : $(gb_Helper_MISC)/api_statistic

$(gb_Helper_MISC)/api_statistic: $(call gb_UnoApiMerge_get_target,types)
	$(PERL) util/checknewapi.pl $(call gb_UnoApiMerge_get_target,types) $(UNOAPI_REFERENCE_types) "$(LIBO_VERSION_MAJOR).$(LIBO_VERSION_MINOR).$(LIBO_VERSION_MICRO).$(LIBO_VERSION_PATCH)" "$(gb_UnoApiMerge_REGVIEWTARGET)" > $@

# vim: set noet sw=4 ts=4:
