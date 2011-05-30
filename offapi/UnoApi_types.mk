#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.	If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_UnoApiTarget_UnoApiTarget,types))

$(eval $(call gb_UnoApiTarget_add_merge_rdbfiles,types,\
    udkapi \
    offapi \
))

$(eval $(call gb_UnoApiTarget_add_reference_rdbfile,types,\
    offapi/type_reference/types \
))

# ugly hack for now!
statistic : $(gb_Helper_MISC)/api_statistic

$(gb_Helper_MISC)/api_statistic: $(call gb_UnoApiTarget_get_target,types)
	$(PERL) util/checknewapi.pl $(call gb_UnoApiTarget_get_target,types) $(UNOAPI_REFERENCE_types) "$(RSCREVISION)" "$(gb_UnoApiTarget_REGVIEWTARGET)" > $@

# vim: set noet sw=4 ts=4:
