# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangResTarget_AllLangResTarget,avmedia))

$(eval $(call gb_AllLangResTarget_add_srs,avmedia,\
	avmedia/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,avmedia/res))

$(eval $(call gb_SrsTarget_set_include,avmedia/res,\
	$$(INCLUDE) \
	-I$(SRCDIR)/avmedia/inc \
	-I$(SRCDIR)/avmedia/source/framework \
	-I$(SRCDIR)/avmedia/source/viewer \
))

$(eval $(call gb_SrsTarget_add_files,avmedia/res,\
	avmedia/source/framework/mediacontrol.src \
	avmedia/source/viewer/mediawindow.src \
))

# vim: set noet sw=4 ts=4:
