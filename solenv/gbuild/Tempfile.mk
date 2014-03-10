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

ifneq ($(HAVE_GNUMAKE_FILE_FUNC),)
define var2file
$(file >$(1),$(3))$(1)
endef
else
# Write string to temporary file by chopping into pieces that
# fit the commandline
# parameters: filename, maxitems (for one write), string
# returns: filename
define var2file
$(strip $(1)
$(eval gb_var2file_helpervar := $$(shell printf "%s" "" > $(1) ))\
$(foreach item,$(3),$(eval gb_var2file_curblock += $(item)
	ifeq ($$(words $$(gb_var2file_curblock)),$(2)) 
		gb_var2file_helpervar := $$(shell printf "%s" "$$(gb_var2file_curblock)" >> $(1) )
		gb_var2file_curblock :=
	endif
	))\
	$(eval gb_var2file_helpervar := $(shell echo " "$(gb_var2file_curblock) >> $(1) )
		gb_var2file_curblock :=
	))
endef
endif

# vim: set noet sw=4 ts=4:
