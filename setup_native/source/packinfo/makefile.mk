# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

PRJ = ../..
PRJNAME = setup_native
TARGET = packinfo

.INCLUDE: settings.mk

ALLTAR: $(OUT)/inc/spellchecker_selection.hxx

.INCLUDE: target.mk

$(OUT)/inc/spellchecker_selection.hxx .ERRREMOVE : spellchecker_selection.pl \
        spellchecker_selection.txt
    $(PERL) -w spellchecker_selection.pl <spellchecker_selection.txt >$@

# vim: set noet sw=4 ts=4:
