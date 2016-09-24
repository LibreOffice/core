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

$(eval $(call gb_Module_Module,vbahelper))

ifneq ($(filter SCRIPTING,$(BUILD_TYPE)),)

# the targets to be inserted are their file names without .mk extension
$(eval $(call gb_Module_add_targets,vbahelper,\
    Library_vbahelper \
    Library_msforms \
))

# add any runtime tests (unit tests) here
# remove if no tests
#$(eval $(call gb_Module_add_check_targets,vbahelper,\
#))

# add any subsequent checks (e.g. complex tests) here
#$(eval $(call gb_Module_add_subsequentcheck_targets,vbahelper,\
#))

endif

$(eval $(call gb_Module_add_targets,vbahelper,\
))

# vim: set noet sw=4 ts=4:
