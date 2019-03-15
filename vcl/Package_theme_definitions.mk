# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,vcl_theme_definitions,$(SRCDIR)/vcl/uiconfig/theme_definitions))

$(eval $(call gb_Package_add_files_with_dir,vcl_theme_definitions,$(LIBO_SHARE_FOLDER)/theme_definitions,\
	ios/definition.xml \
	ios/switch-off.svgx \
	ios/switch-off-disabled.svgx \
	ios/switch-off-pressed.svgx \
	ios/switch-on.svgx \
	ios/switch-on-pressed.svgx \
	ios/switch-on-disabled.svgx \
	ios/tick-off.svgx \
	ios/tick-off-disabled.svgx \
	ios/tick-off-pressed.svgx \
	ios/tick-on.svgx \
	ios/tick-on-pressed.svgx \
	ios/tick-on-disabled.svgx \
	ios/spinbox-left.svgx \
	ios/spinbox-left-pressed.svgx \
	ios/spinbox-left-rollover.svgx \
	ios/spinbox-left-disabled.svgx \
	ios/spinbox-right.svgx \
	ios/spinbox-right-pressed.svgx \
	ios/spinbox-right-rollover.svgx \
	ios/spinbox-right-disabled.svgx \
	ios/common-rect.svgx \
	ios/common-rect-disabled.svgx \
	ios/common-rect-focus.svgx \
	ios/common-rect-focus-slim.svgx \
	ios/pushbutton-default.svgx \
	ios/pushbutton-rollover.svgx \
	ios/pushbutton-disabled.svgx \
	ios/tabitem-first.svgx \
	ios/tabitem-middle.svgx \
	ios/tabitem-last.svgx \
	ios/tabitem-first-selected.svgx \
	ios/tabitem-middle-selected.svgx \
	ios/tabitem-last-selected.svgx \
	ios/scrollbar-horizontal.svgx \
	ios/scrollbar-vertical.svgx \
	ios/combobox.svgx \
	ios/combobox-disabled.svgx \
	ios/combobox-button.svgx \
	ios/combobox-button-disabled.svgx \
	ios/arrow-up.svgx \
	ios/arrow-down.svgx \
	ios/slider-button.svgx \
))

# vim: set noet sw=4 ts=4:
