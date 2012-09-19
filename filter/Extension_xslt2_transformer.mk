# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Extension_Extension,xslt2-transformer,filter/source/xsltfilter))

$(eval $(call gb_Extension_add_files,xslt2-transformer,,\
    $(call gb_Jar_get_outdir_target,saxon9) \
    $(call gb_Jar_get_outdir_target,XSLTFilter) \
	$(SRCDIR)/filter/source/xsltfilter/components.rdb \
))

# vim: set noet sw=4 ts=4:
