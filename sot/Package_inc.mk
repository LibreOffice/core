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

$(eval $(call gb_Package_Package,sot_inc,$(SRCDIR)/sot/inc))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/exchange.hxx,sot/exchange.hxx))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/factory.hxx,sot/factory.hxx))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/filelist.hxx,sot/filelist.hxx))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/formats.hxx,sot/formats.hxx))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/object.hxx,sot/object.hxx))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/sotdata.hxx,sot/sotdata.hxx))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/sotdllapi.h,sot/sotdllapi.h))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/stg.hxx,sot/stg.hxx))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/storage.hxx,sot/storage.hxx))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/storinfo.hxx,sot/storinfo.hxx))

# vim: set noet sw=4 ts=4:
