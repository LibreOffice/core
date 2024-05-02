# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,setup_native/spell))

$(eval $(call gb_CustomTarget_register_target,setup_native/spell,spellchecker_selection.hxx))

$(gb_CustomTarget_workdir)/setup_native/spell/spellchecker_selection.hxx: $(SRCDIR)/setup_native/source/packinfo/spellchecker_selection.txt $(SRCDIR)/setup_native/source/packinfo/spellchecker_selection.pl
	$(PERL) -w $(SRCDIR)/setup_native/source/packinfo/spellchecker_selection.pl  <$< >$@

# vim: set noet sw=4 ts=4:
