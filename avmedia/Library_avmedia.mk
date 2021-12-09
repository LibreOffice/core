# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,avmedia))

$(eval $(call gb_Library_set_include,avmedia,\
	$$(INCLUDE) \
	-I$(SRCDIR)/avmedia/inc \
	-I$(SRCDIR)/avmedia/source/inc \
))

$(eval $(call gb_Library_use_libraries,avmedia,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    svl \
    ucbhelper \
    vcl \
))

$(eval $(call gb_Library_use_sdk_api,avmedia,))

$(eval $(call gb_Library_use_externals,avmedia,\
	boost_headers \
))

$(eval $(call gb_Library_add_defs,avmedia,\
    -DAVMEDIA_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_exception_objects,avmedia,\
    avmedia/source/framework/mediaitem \
))

ifneq (,$(filter AVMEDIA,$(BUILD_TYPE)))

ifeq ($(DISABLE_GUI),)
$(eval $(call gb_Library_use_externals,avmedia,\
    epoxy \
))

$(eval $(call gb_Library_set_componentfile,avmedia,avmedia/util/avmedia,services))

$(eval $(call gb_Library_use_libraries,avmedia,\
	i18nlangtag \
	sfx \
	svt \
	tl \
	utl \
))

$(eval $(call gb_Library_add_exception_objects,avmedia,\
	avmedia/source/framework/mediacontrol \
	avmedia/source/framework/MediaControlBase \
	avmedia/source/framework/mediaplayer \
	avmedia/source/framework/mediatoolbox \
	avmedia/source/framework/soundhandler \
	avmedia/source/viewer/mediaevent_impl \
	avmedia/source/viewer/mediawindow \
	avmedia/source/viewer/mediawindow_impl \
))

endif # !DISABLE_GUI
endif # AVMEDIA

# vim: set noet sw=4 ts=4:
