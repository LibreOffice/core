# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http:#www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2011 Tor Lillqvist <tml@iki.fi> (initial developer)
# Copyright (C) 2011 SUSE Linux http://suse.com (initial developer's employer)
#
# Zip parsing code lifted from Mozilla's other-licenses/android/APKOpen.cpp,
# by Michael Wu <mwu@mozilla.com>.
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

PRJ = ..
PRJNAME = sal
TARGET = lo-bootstrap

# Too many warnings from android_native_app_glue.[ch]
EXTERNAL_WARNINGS_NOT_ERRORS = TRUE

.INCLUDE :  settings.mk

.IF "$(OS)" != "ANDROID"
ALL:
# do nothing
.ENDIF

SHL1TARGET = $(TARGET)

SHL1OBJS = \
    $(SLO)$/lo-bootstrap.obj

# We don't want to link liblo-bootstrap.so against
# libgnustl_shared.so. The Android dynamic linker won't find it
# anyway. One very point of liblo-bootstrap is its wrapper for
# dlopen() that searches also in the app's lib folder for needed
# shared libraries. So just re-define STDSHLCUIMT.

STDSHLCUIMT := -llog -landroid

# Also don't pointless

.INCLUDE :  target.mk
