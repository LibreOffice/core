# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
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
# Major Contributor(s):
# Copyright (C) 2012 Matúš Kukan <matus.kukan@gmail.com> (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

ifeq ($(MERGELIBS),TRUE)

# we link all object files from these libraries into one, merged library
gb_MERGEDLIBS := \
	avmedia \
	canvastools \
	configmgr \
	cppcanvas \
	deploymentmisc \
	desktopbe1 \
	drawinglayer \
	editeng \
	filterconfig \
	fsstorage \
	fwe \
	fwi \
	fwk \
	i18npool \
	i18nutil \
	lng \
	localebe1 \
	package2 \
	sax \
	sb \
	sfx \
	sofficeapp \
	sot \
	spl \
	svl \
	svt \
	svx \
	svxcore \
	tk \
	ucb1 \
	ucpfile1 \
	utl \
	uui \
	vcl \
	xmlscript \
	xo \

gb_MERGEDLIBS += $(if $(filter TRUE,$(ENABLE_TELEPATHY)),tubes)

ifeq ($(OS),ANDROID)
gb_MERGEDLIBS += \
	analysis \
	ctl \
	date \
	embobj \
	evtatt \
	fastsax \
	fileacc \
	for \
	forui \
	fwl \
	fwm \
	hwp \
	hyphen \
	lnth \
	pricing \
	spell \
	xstor \

endif

endif

# vim: set noet sw=4 ts=4:
