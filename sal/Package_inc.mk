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
# Copyright (C) 2011 Matúš Kukan <matus.kukan@gmail.com> (initial developer)
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

$(eval $(call gb_Package_Package,sal_inc,$(SRCDIR)/sal/inc))

$(eval $(call gb_Package_add_file,sal_inc,inc/protectorfactory.hxx,cppunittester/protectorfactory.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/detail/file.h,osl/detail/file.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/detail/android-bootstrap.h,osl/detail/android-bootstrap.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/detail/ios-bootstrap.h,osl/detail/ios-bootstrap.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/detail/component-mapping.h,osl/detail/component-mapping.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/sal/log-areas.dox,sal/log-areas.dox))
$(eval $(call gb_Package_add_file,sal_inc,inc/systools/win32/comptr.hxx,systools/win32/comptr.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/systools/win32/comtools.hxx,systools/win32/comtools.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/systools/win32/qswin32.h,systools/win32/qswin32.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/systools/win32/snprintf.h,systools/win32/snprintf.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/systools/win32/uwinapi.h,systools/win32/uwinapi.h))

# vim: set noet sw=4 ts=4:
