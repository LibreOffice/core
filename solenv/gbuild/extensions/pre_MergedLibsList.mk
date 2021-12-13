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
	fwk \
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
	lnth \
	localebe1 \
	msfilter \
	mtfrenderer \
	numbertext \
	odfflatxml \
	offacc \
	$(if $(ENABLE_OPENGL_CANVAS),oglcanvas) \
	$(if $(filter OPENCL,$(BUILD_TYPE)),opencl) \
	package2 \
	passwordcontainer \
	sax \
	sb \
	simplecanvas \
	sfx \
	sofficeapp \
	sot \
	spell \
	$(if $(DISABLE_GUI),,spl) \
	srtrs1 \
	$(call gb_Helper_optional,SCRIPTING,stringresource) \
	svgio \
	svl \
	svt \
	svx \
	svxcore \
	syssh \
	textfd \
	tk \
	tl \
	ucb1 \
	ucbhelper \
	ucpexpand1 \
	ucpext \
	ucphier1 \
	ucpimage \
	ucpfile1 \
	ucptdoc1 \
	unordf \
	unoxml \
	updatefeed \
	utl \
	uui \
	vbaevents \
	vbahelper \
	vcl \
	vclcanvas \
	xmlfa \
	xmlfd \
	xmlscript \
	xo \
	xof \
	xsltdlg \
	xsltfilter \
	xstor \


# allow module-deps.pl to color based on this.
ifneq ($(ENABLE_PRINT_DEPS),)

$(info MergeLibContents: $(MERGE_LIBRARY_LIST))

endif

ifneq ($(MERGELIBS),)

gb_MERGEDLIBS := $(MERGE_LIBRARY_LIST)

endif

# vim: set noet sw=4 ts=4:
