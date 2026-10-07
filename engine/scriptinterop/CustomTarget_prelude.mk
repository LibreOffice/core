# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# Copyright the Collabora Office contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,scriptinterop/prelude))

$(call gb_CustomTarget_get_target,scriptinterop/prelude) : \
    $(gb_CustomTarget_workdir)/scriptinterop/prelude/prelude.hxx

$(gb_CustomTarget_workdir)/scriptinterop/prelude/prelude.hxx : \
        $(call gb_ExternalExecutable_get_dependencies,python) \
        $(SRCDIR)/scriptinterop/js/prelude.js $(SRCDIR)/scriptinterop/js/genprelude.py \
        | $(gb_CustomTarget_workdir)/scriptinterop/prelude/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,PY ,1)
	$(call gb_Helper_abbreviate_dirs, \
        $(call gb_ExternalExecutable_get_command,python) $(SRCDIR)/scriptinterop/js/genprelude.py \
            $(SRCDIR)/scriptinterop/js/prelude.js $@)

# vim: set noet sw=4 ts=4:
