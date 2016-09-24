# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,avmediavlc))

$(eval $(call gb_Library_set_componentfile,avmediavlc,avmedia/source/vlc/avmediavlc))

$(eval $(call gb_Library_set_include,avmediavlc,\
    $$(INCLUDE) \
    -I$(SRCDIR)/avmedia/source/inc \
))

$(eval $(call gb_Library_use_custom_headers,avmediavlc,\
	officecfg/registry \
))

$(eval $(call gb_Library_use_sdk_api,avmediavlc))

$(eval $(call gb_Library_use_externals,avmediavlc,\
	boost_headers \
	boost_system \
))

$(eval $(call gb_Library_use_libraries,avmediavlc,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    tl \
    vcl \
    utl \
    salhelper \
    $(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,avmediavlc,\
    avmedia/source/vlc/vlcmanager \
    avmedia/source/vlc/vlcplayer \
    avmedia/source/vlc/vlcuno \
    avmedia/source/vlc/vlcwindow \
    avmedia/source/vlc/vlcframegrabber \
    avmedia/source/vlc/wrapper/Instance \
    avmedia/source/vlc/wrapper/Media \
    avmedia/source/vlc/wrapper/Player \
    avmedia/source/vlc/wrapper/EventManager \
    avmedia/source/vlc/wrapper/EventHandler \
    avmedia/source/vlc/wrapper/Common \
))

# vim: set noet sw=4 ts=4:
