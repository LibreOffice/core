# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

define gb_CondBuildLockfile
$(if $(and $(filter-out ANDROID MACOSX iOS WNT,$(OS))),$(1),$(2))
endef

define gb_CondSalTextEncodingLibrary
$(if $(filter ANDROID iOS,$(OS))$(filter FUZZERS,$(BUILD_TYPE)),$(2),$(1))
endef

# vim: set noet sw=4 ts=4:
