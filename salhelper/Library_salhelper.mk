# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,salhelper))

$(eval $(call gb_Library_add_defs,salhelper,\
	-DSALHELPER_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_libraries,salhelper,\
	sal \
	$(gb_UWINAPI) \
))

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

$(eval $(call gb_Library_set_soversion_script,salhelper,$(SRCDIR)/salhelper/source/gcc3.map))

# vim: set noet sw=4 ts=4:
