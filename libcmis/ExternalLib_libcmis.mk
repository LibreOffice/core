
# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2011 SUSE <cbosdonnat@suse.com>
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

$(eval $(call gb_ExternalLib_ExternalLib,libcmis,autotools))

$(eval $(call gb_ExternalLib_set_src_package,libcmis,d821b3cdeba34db1d084b9bd709b3a52-libcmis-0.1.0.tar.gz))
$(eval $(call gb_ExternalLib_add_conf_arg,libcmis,--disable-client))
$(eval $(call gb_ExternalLib_add_patch,libcmis,libcmis/libcmis-0.1.0-autotools.patch))

ifneq ($(filter $(OS),MACOSX ANDROID),)

$(eval $(call gb_ExternalLib_add_conf_arg,libcmis,'CPPUNIT_LIBS=""'))
$(eval $(call gb_ExternalLib_add_conf_arg,libcmis,'CPPUNIT_CFLAGS=""'))
$(eval $(call gb_ExternalLib_add_conf_arg,libcmis,--with-boost=$(OUTDIR)/inc))

endif

# vim: set noet sw=4 ts=4:
