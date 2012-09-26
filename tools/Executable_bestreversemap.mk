# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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
# The Initial Developer of the Original Code is
# Norbert Thiebaud <nthiebaud@gmail.com> (C) 2010, All Rights Reserved.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Executable_Executable,bestreversemap))

$(eval $(call gb_Executable_set_include,bestreversemap,\
	$$(INCLUDE) \
))

$(eval $(call gb_Executable_use_libraries,bestreversemap,\
	sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Executable_add_standard_system_libs,bestreversemap))

$(eval $(call gb_Executable_add_exception_objects,bestreversemap,\
	tools/source/reversemap/bestreversemap \
))

# vim: set noet sw=4 ts=4:
