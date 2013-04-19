# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# cppunittester is built in this module; cannot use delivered version
# this should be kept in sync with the definition in solenv/inc/settings.mk
.IF "$(CROSS_COMPILING)"=="YES"
CPPUNITTESTER=\#
.ELSE
CPPUNITTESTER = $(AUGMENT_LIBRARY_PATH_LOCAL) $(GDBCPPUNITTRACE) $(VALGRINDTOOL) $(BIN)/cppunittester
.ENDIF

.INCLUDE : _cppunit.mk
