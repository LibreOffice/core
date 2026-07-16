# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Design templates the AI assistant applies to generated presentations. They are
# installed unconditionally (independent of WITH_TEMPLATES) into a dedicated
# share subdirectory rather than share/template/common/presnt, because with an
# integrator the kit mounts the per-config presets templates over that directory
# and would hide them. The .otp files are produced by CustomTarget_templates from
# the source directories under extras/source/templates/cool-ai.

$(eval $(call gb_Package_Package,extras_aitemplates,$(gb_CustomTarget_workdir)/extras/source/templates/cool-ai))

$(eval $(call gb_Package_add_files,extras_aitemplates,$(LIBO_SHARE_FOLDER)/cool-ai-templates,\
	Cobalt.otp \
	Onyx.otp \
	Slate.otp \
	Ivory.otp \
	Paper.otp \
	Mint.otp \
))

# vim: set noet sw=4 ts=4:
