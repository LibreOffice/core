# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,vcl_theme_definitions,$(SRCDIR)/vcl/uiconfig/theme_definitions))

$(eval $(call gb_Package_add_files,vcl_theme_definitions,$(LIBO_SHARE_FOLDER)/theme_definitions,\
	definition.xml \
	switch-off.svgx \
	switch-off-disabled.svgx \
	switch-off-pressed.svgx \
	switch-on.svgx \
	switch-on-pressed.svgx \
	switch-on-disabled.svgx \
	tick-off.svgx \
	tick-off-disabled.svgx \
	tick-off-pressed.svgx \
	tick-on.svgx \
	tick-on-pressed.svgx \
	tick-on-disabled.svgx \
	spinbox-left.svgx \
	spinbox-left-pressed.svgx \
	spinbox-left-rollover.svgx \
	spinbox-left-disabled.svgx \
	spinbox-right.svgx \
	spinbox-right-pressed.svgx \
	spinbox-right-rollover.svgx \
	spinbox-right-disabled.svgx \
	spinbox-entire.svgx \
	common-rect.svgx \
	common-rect-disabled.svgx \
	pushbutton-default.svgx \
	pushbutton-rollover.svgx \
	tabitem-first.svgx \
	tabitem-middle.svgx \
	tabitem-last.svgx \
	tabitem-first-selected.svgx \
	tabitem-middle-selected.svgx \
	tabitem-last-selected.svgx \
	scrollbar-horizontal.svgx \
	scrollbar-vertical.svgx \
	combobox.svgx \
	combobox-disabled.svgx \
))

# vim: set noet sw=4 ts=4:
