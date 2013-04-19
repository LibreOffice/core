# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,brand/intro,$(WORKDIR)/Zip/brand_intro))

$(eval $(call gb_Zip_add_file,brand/intro,shell/about.svg,\
	$(if $(ABOUT_BACKGROUND_SVG),\
	    $(ABOUT_BACKGROUND_SVG),\
		$(SRCDIR)/icon-themes/galaxy/brand/shell/about.svg)\
))

$(eval $(call gb_Zip_add_file,brand/intro,flat_logo.svg,\
	$(if $(FLAT_LOGO_SVG),\
	    $(FLAT_LOGO_SVG),\
		$(SRCDIR)/icon-themes/galaxy/brand/flat_logo.svg)\
))

$(eval $(call gb_Zip_add_file,brand/intro,intro.png,\
	$(if $(INTRO_BITMAP),\
	    $(INTRO_BITMAP),\
		$(SRCDIR)/icon-themes/galaxy/brand/intro.png)\
))

$(eval $(call gb_Zip_add_file,brand/intro,shell/backing_left.png,\
	$(if $(STARTCENTER_LEFT_BITMAP),\
	    $(STARTCENTER_LEFT_BITMAP),\
		$(SRCDIR)/icon-themes/galaxy/brand/shell/backing_left.png)\
))

$(eval $(call gb_Zip_add_file,brand/intro,shell/backing_right.png,\
	$(if $(STARTCENTER_RIGHT_BITMAP),\
	    $(STARTCENTER_RIGHT_BITMAP),\
		$(SRCDIR)/icon-themes/galaxy/brand/shell/backing_right.png)\
))

$(eval $(call gb_Zip_add_file,brand/intro,shell/backing_space.png,\
	$(if $(STARTCENTER_SPACE_BITMAP),\
	    $(STARTCENTER_SPACE_BITMAP),\
		$(SRCDIR)/icon-themes/galaxy/brand/shell/backing_space.png)\
))

$(eval $(call gb_Zip_add_file,brand/intro,shell/backing_rtl_left.png,\
	$(if $(STARTCENTER_RTL_LEFT_BITMAP),\
	    $(STARTCENTER_RTL_LEFT_BITMAP),\
		$(SRCDIR)/icon-themes/galaxy/brand/shell/backing_rtl_left.png)\
))

$(eval $(call gb_Zip_add_file,brand/intro,shell/backing_rtl_right.png,\
	$(if $(STARTCENTER_RTL_RIGHT_BITMAP),\
	    $(STARTCENTER_RTL_RIGHT_BITMAP),\
		$(SRCDIR)/icon-themes/galaxy/brand/shell/backing_rtl_right.png)\
))

# vim: set ts=4 sw=4 et:
