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
	ios/switch-off.svg \
	ios/switch-off-disabled.svg \
	ios/switch-off-pressed.svg \
	ios/switch-on.svg \
	ios/switch-on-pressed.svg \
	ios/switch-on-disabled.svg \
	ios/tick-off.svg \
	ios/tick-off-disabled.svg \
	ios/tick-off-pressed.svg \
	ios/tick-on.svg \
	ios/tick-on-pressed.svg \
	ios/tick-on-disabled.svg \
	ios/spinbox-left.svg \
	ios/spinbox-left-pressed.svg \
	ios/spinbox-left-disabled.svg \
	ios/spinbox-right.svg \
	ios/spinbox-right-pressed.svg \
	ios/spinbox-right-disabled.svg \
	ios/common-rect.svg \
	ios/common-rect-disabled.svg \
	ios/common-rect-focus.svg \
	ios/common-rect-focus-slim.svg \
	ios/pushbutton-default.svg \
	ios/pushbutton-rollover.svg \
	ios/pushbutton-disabled.svg \
	ios/tabitem-first.svg \
	ios/tabitem-middle.svg \
	ios/tabitem-last.svg \
	ios/tabitem-first-selected.svg \
	ios/tabitem-middle-selected.svg \
	ios/tabitem-last-selected.svg \
	ios/scrollbar-horizontal.svg \
	ios/scrollbar-vertical.svg \
	ios/combobox.svg \
	ios/combobox-disabled.svg \
	ios/combobox-button.svg \
	ios/combobox-button-disabled.svg \
	ios/arrow-up.svg \
	ios/arrow-down.svg \
	ios/slider-button.svg \
	ios/slider-button-disabled.svg \
))

# vim: set noet sw=4 ts=4:
