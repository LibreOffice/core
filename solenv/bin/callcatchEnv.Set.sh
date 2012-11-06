#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
CC="callcatcher ${CC:-gcc}"
CXX="callcatcher ${CXX:-g++}"
AR="callarchive ${AR:-ar}"
#old-school ones, can go post-gbuildification is complete
LINK="callcatcher $CXX"
LIBMGR="callarchive ${LIBMGR:-ar}"
export CC CXX AR LINK LIBMGR
export dbglevel=2
