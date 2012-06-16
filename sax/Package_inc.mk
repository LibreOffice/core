# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,sax_inc,$(SRCDIR)/sax/inc))

$(eval $(call gb_Package_add_file,sax_inc,inc/sax/fastattribs.hxx,sax/fastattribs.hxx))
$(eval $(call gb_Package_add_file,sax_inc,inc/sax/fshelper.hxx,sax/fshelper.hxx))
$(eval $(call gb_Package_add_file,sax_inc,inc/sax/saxdllapi.h,sax/saxdllapi.h))
$(eval $(call gb_Package_add_file,sax_inc,inc/sax/tools/converter.hxx,sax/tools/converter.hxx))
$(eval $(call gb_Package_add_file,sax_inc,inc/sax/tools/documenthandleradapter.hxx,sax/tools/documenthandleradapter.hxx))

# vim: set noet sw=4 ts=4:
