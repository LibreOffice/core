# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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
	$(if $(filter WNT,$(OS)),avmediawin) \
	$(call gb_Helper_optional,SCRIPTING, \
		basctl \
		basprov \
	) \
	basegfx \
	canvasfactory \
	canvastools \
	comphelper \
	configmgr \
	cppcanvas \
	$(call gb_Helper_optional,BREAKPAD,crashreport) \
	ctl \
	dbtools \
	deployment \
	deploymentmisc \
	$(if $(filter-out MACOSX WNT,$(OS)),desktopbe1) \
	$(if $(USING_X11),desktop_detector) \
	$(if $(filter WNT,$(OS)),directx9canvas) \
	drawinglayer \
	editeng \
	emfio \
	$(if $(filter WNT,$(OS)),emser) \
	evtatt \
	filterconfig \
	for \
	forui \
	fps_office \
	frm \
	fsstorage \
	fwe \
	fwi \
	fwk \
	fwl \
	$(if $(filter WNT,$(OS)),gdipluscanvas) \
	guesslang \
	$(call gb_Helper_optional,DESKTOP,helplinker) \
	hyphen \
	i18nsearch \
	i18npool \
	i18nutil \
	io \
	$(if $(ENABLE_JAVA),javaloader) \
	$(if $(ENABLE_JAVA),javavm) \
	lng \
	localebe1 \
	mcnttype \
	msfilter \
	mtfrenderer \
	$(if $(filter OPENCL,$(BUILD_TYPE)),opencl) \
	package2 \
	sax \
	sb \
	simplecanvas \
	sfx \
	sofficeapp \
	sot \
	$(if $(DISABLE_GUI),,spl) \
	$(call gb_Helper_optional,SCRIPTING,stringresource) \
	svl \
	svt \
	svx \
	svxcore \
	tk \
	tl \
	ucb1 \
	ucbhelper \
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
