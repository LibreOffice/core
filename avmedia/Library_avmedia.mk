# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,avmedia))

$(eval $(call gb_Library_set_componentfile,avmedia,avmedia/util/avmedia))

$(eval $(call gb_Library_set_include,avmedia,\
	$$(INCLUDE) \
	-I$(SRCDIR)/avmedia/inc \
	-I$(SRCDIR)/avmedia/source/inc \
))

$(eval $(call gb_Library_use_sdk_api,avmedia,))

$(eval $(call gb_Library_add_defs,avmedia,\
	-DAVMEDIA_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_external,avmedia,boost_headers))

$(eval $(call gb_Library_use_libraries,avmedia,\
	comphelper \
	ucbhelper \
	cppu \
	cppuhelper \
	sal \
	i18nlangtag \
	sfx \
	svl \
	svt \
	tl \
	utl \
	vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,avmedia,\
	avmedia/source/framework/mediacontrol \
	avmedia/source/framework/mediaitem \
	avmedia/source/framework/mediamisc \
	avmedia/source/framework/mediaplayer \
	avmedia/source/framework/mediatoolbox \
	avmedia/source/framework/modeltools \
	avmedia/source/framework/soundhandler \
	avmedia/source/viewer/mediaevent_impl \
	avmedia/source/viewer/mediawindow \
	avmedia/source/viewer/mediawindow_impl \
))

# vim: set noet sw=4 ts=4:
