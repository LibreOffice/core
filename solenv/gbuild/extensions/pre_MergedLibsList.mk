# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# we link all object files from these libraries into one, merged library
MERGE_LIBRARY_LIST := \
	avmedia \
	$(if $(filter $(OS),ANDROID),,basebmp) \
	basegfx \
	canvastools \
	configmgr \
	cppcanvas \
	$(call gb_Helper_optional,DBCONNECTIVITY,dbtools) \
	deployment \
	deploymentmisc \
	$(if $(filter-out MACOSX WNT,$(OS)),desktopbe1) \
	$(if $(USING_X11),desktop_detector) \
	drawinglayer \
	editeng \
	filterconfig \
	fsstorage \
	fwe \
	fwi \
	fwk \
	$(call gb_Helper_optional,DESKTOP,helplinker) \
	i18npool \
	i18nutil \
	lng \
	localebe1 \
	mcnttype \
	msfilter \
	package2 \
	sax \
	sb \
	sfx \
	sofficeapp \
	sot \
	$(if $(ENABLE_HEADLESS),,spl) \
	svl \
	svt \
	svx \
	svxcore \
	tk \
	tl \
	$(if $(filter TRUE,$(ENABLE_TELEPATHY)),tubes) \
	ucb1 \
	ucpexpand1 \
	ucpfile1 \
	unoxml \
	utl \
	uui \
	vcl \
	xmlscript \
	xo \
	xstor \


# allow module-deps.pl to color based on this.
ifneq ($(ENABLE_PRINT_DEPS),)

$(info MergeLibContents: $(MERGE_LIBRARY_LIST))

endif

ifneq ($(MERGELIBS),)

gb_MERGEDLIBS := $(MERGE_LIBRARY_LIST)

endif

# vim: set noet sw=4 ts=4:
