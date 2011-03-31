# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
#       Bjoern Michaelsen, Canonical Ltd. <bjoern.michaelsen@canonical.com>
# Portions created by the Initial Developer are Copyright (C) 2010 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Library_Library,lomenubar))

#$(eval $(call gb_Library_set_componentfile,lomenubar,MODULE/COMPONENT_FILE))

# FIXME pkg-config call belong in ./configure obviously

$(eval $(call gb_Library_set_include,lomenubar,\
	$$(INCLUDE) \
	-I$(OUTDIR)/inc/offuh \
	$(shell pkg-config --cflags-only-I dbusmenu-gtk-0.4) \
))

$(eval $(call gb_Library_set_ldflags,lomenubar,\
	$$(LDFLAGS) \
	$(shell pkg-config --libs dbusmenu-gtk-0.4) \
	$(GTK_LIBS) \
))

$(eval $(call gb_Library_add_linked_libs,lomenubar,\
	sal \
	cppu \
	cppuhelper \
	X11 \
))

$(eval $(call gb_Library_add_exception_objects,lomenubar,\
	framework/source/lomenubar/MenuItemInfo \
	framework/source/lomenubar/AwtKeyToDbusmenuString \
	framework/source/lomenubar/DesktopJob \
	framework/source/lomenubar/FrameJob \
	framework/source/lomenubar/FrameHelper \
	framework/source/lomenubar/MenuItemStatusListener \
	framework/source/lomenubar/exports \
))

# vim: set noet sw=4:
