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

# MODULE is the name of the module the makefile is located in

# TYPE is an arbitrary name that should denote what is inside this package
# typical names are "inc", "uiconfig", "xml" etc.
# "MODULE_TYPE" is the package name
# the file name of the corresponding makefile should be Package_TYPE.mk
# SOURCE is the name of the root folder of the package that shall be delivered
$(eval $(call gb_Package_Package,MODULE_TYPE,$(SRCDIR)/MODULE/SOURCE))

# add files to the package
# SOURCE_RELPATH/file.ext is the name of the source file (relative to the root give above)
# DESTINATION_RELPATH/file.ext is the destination file (relative to $(OUTDIR))
$(eval $(call gb_Package_add_file,MODULE_TYPE,DESTINATION_RELPATH/file.ext,SOURCE_RELPATH/file.ext))

# vim: set noet sw=4:
