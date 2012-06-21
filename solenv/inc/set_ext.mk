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

#defaults
TARFILE_ROOTDIR*=$(TARFILE_NAME)
UNTAR_FLAG_FILE*=so_unpacked_$(TARGET)
ADD_FILES_FLAG_FILE*=so_addfiles_$(TARGET)
PATCH_FLAG_FILE*=so_patched_$(TARGET)
CONFIGURE_FLAG_FILE*=so_configured_$(TARGET)
BUILD_FLAG_FILE*=so_built_$(TARGET)
INSTALL_FLAG_FILE*=so_installed_$(TARGET)
PREDELIVER_FLAG_FILE*=so_predeliver_$(TARGET)

