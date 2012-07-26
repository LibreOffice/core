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

PRJ=..$/..

PRJNAME=more_fonts
TARGET=ttf_liberation

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------
# Note: Current release does not include Liberation Sans Narrow font because
# that couldn't be relicensed to OFL, so we take the Narrow version from
# the old release
TARFILE_NAME=liberation-fonts-ttf-2.00.0
TARFILE_MD5=cfbf1ac6f61bf6cf45342a0cc9381be5
TARFILE_ROOTDIR=liberation-fonts-ttf-2.00.0

PATCH_FILES=

CONFIGURE_DIR=
CONFIGURE_ACTION=

BUILD_DIR=
BUILD_ACTION=
BUILD_FLAGS=

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk

.IF "$(WITH_FONTS)"!="NO"
.INCLUDE : tg_ext.mk
.ENDIF
