# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangResTarget_AllLangResTarget,sd))

$(eval $(call gb_AllLangResTarget_set_reslocation,sd,sd))

$(eval $(call gb_AllLangResTarget_add_srs,sd,\
    sd/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,sd/res))

$(eval $(call gb_SrsTarget_set_include,sd/res,\
    $$(INCLUDE) \
    -I$(SRCDIR)/sd/inc \
    -I$(SRCDIR)/sd/source/ui/inc \
    -I$(SRCDIR)/sd/source/ui/slidesorter/inc \
))

$(eval $(call gb_SrsTarget_add_files,sd/res,\
    sd/source/core/glob.src \
    sd/source/ui/accessibility/accessibility.src \
    sd/source/ui/animations/CustomAnimation.src \
    sd/source/ui/annotations/annotations.src \
    sd/source/ui/app/popup.src \
    sd/source/ui/app/res_bmp.src \
    sd/source/ui/app/sdstring.src \
    sd/source/ui/app/strings.src \
    sd/source/ui/app/toolbox.src \
    sd/source/ui/dlg/navigatr.src \
    sd/source/ui/slideshow/slideshow.src \
    sd/source/ui/view/DocumentRenderer.src \
))

$(eval $(call gb_SrsTarget_add_nonlocalizable_files,sd/res,\
    sd/source/ui/slidesorter/view/SlsResource.src \
))

# vim: set noet sw=4 ts=4:
