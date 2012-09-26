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
# Copyright (C) 2011 Matúš Kukan <matus.kukan@gmail.com>
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

$(eval $(call gb_Library_Library,salhelper))

$(eval $(call gb_Library_use_package,salhelper,salhelper_inc))

$(eval $(call gb_Library_add_defs,salhelper,\
	-DSALHELPER_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_libraries,salhelper,\
	sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_standard_system_libs,salhelper))

$(eval $(call gb_Library_add_exception_objects,salhelper,\
    salhelper/source/condition \
    salhelper/source/dynload \
    salhelper/source/simplereferenceobject \
    salhelper/source/thread \
    salhelper/source/timer \
))

# At least on Linux, for backwards compatibility the salhelper library needs a
# specific soname and symbol versioning.  The symbols in the gcc3.map should be
# those not already filtered out due to hidden visibility, so combining the
# visibility feature with the version map file feature works well.
# Solaris would need something like this, too.  Its backwards compatibility is
# broken for now:

$(eval $(call gb_Library_set_soversion_script,salhelper,3,$(SRCDIR)/salhelper/source/gcc3.map))

# vim: set noet sw=4 ts=4:
