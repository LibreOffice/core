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

# RES is the name of the resource; its file name will be RES-LOCALE.res
# MODULE is the name of the code module the makefile is located in

$(eval $(call gb_AllLangResTarget_AllLangResTarget,RES))

# declaration of images folder (e.g. icon-themes/(name of theme)/MODULE/res)
$(eval $(call gb_AllLangResTarget_set_reslocation,RES,MODULE))

# declaration of a resource intermediate file (srs)
$(eval $(call gb_AllLangResTarget_add_srs,RES, RES/res))
$(eval $(call gb_SrsTarget_SrsTarget,RES/res))

# add any additional include paths here
$(eval $(call gb_SrsTarget_set_include,RES/res,\
	$$(INCLUDE) \
))

# add src files here (complete path relative to repository root)
$(eval $(call gb_SrsTarget_add_files,RES/res,\
))

# vim: set noet sw=4:
