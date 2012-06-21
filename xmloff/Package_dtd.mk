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

$(eval $(call gb_Package_Package,xmloff_dtd,$(SRCDIR)/xmloff/dtd))
$(eval $(call gb_Package_add_file,xmloff_dtd,bin/Blocklist.dtd,Blocklist.dtd))
$(eval $(call gb_Package_add_file,xmloff_dtd,bin/chart.mod,chart.mod))
$(eval $(call gb_Package_add_file,xmloff_dtd,bin/datastyl.mod,datastyl.mod))
$(eval $(call gb_Package_add_file,xmloff_dtd,bin/defs.mod,defs.mod))
$(eval $(call gb_Package_add_file,xmloff_dtd,bin/drawing.mod,drawing.mod))
$(eval $(call gb_Package_add_file,xmloff_dtd,bin/dtypes.mod,dtypes.mod))
$(eval $(call gb_Package_add_file,xmloff_dtd,bin/form.mod,form.mod))
$(eval $(call gb_Package_add_file,xmloff_dtd,bin/meta.mod,meta.mod))
$(eval $(call gb_Package_add_file,xmloff_dtd,bin/nmspace.mod,nmspace.mod))
$(eval $(call gb_Package_add_file,xmloff_dtd,bin/office.dtd,office.dtd))
$(eval $(call gb_Package_add_file,xmloff_dtd,bin/office.mod,office.mod))
$(eval $(call gb_Package_add_file,xmloff_dtd,bin/openoffice-2.0-schema.rng,openoffice-2.0-schema.rng))
$(eval $(call gb_Package_add_file,xmloff_dtd,bin/script.mod,script.mod))
$(eval $(call gb_Package_add_file,xmloff_dtd,bin/settings.mod,settings.mod))
$(eval $(call gb_Package_add_file,xmloff_dtd,bin/style.mod,style.mod))
$(eval $(call gb_Package_add_file,xmloff_dtd,bin/table.mod,table.mod))
$(eval $(call gb_Package_add_file,xmloff_dtd,bin/text.mod,text.mod))

# vim: set noet sw=4 ts=4:
