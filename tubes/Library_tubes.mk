# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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
# Copyright (C) 2012 Red Hat, Inc., Eike Rathke <erack@redhat.com>
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

$(eval $(call gb_Library_Library,tubes))

$(eval $(call gb_Library_set_include,tubes,\
	-I$(SRCDIR)/tubes/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_use_sdk_api,tubes))

$(eval $(call gb_Library_add_defs,tubes,\
	-DTUBES_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_libraries,tubes,\
	$(gb_STDLIBS) \
	comphelper \
	cppu \
	sal \
	svt \
	svxcore \
	tl \
	utl \
	vcl \
))

$(eval $(call gb_Library_use_externals,tubes,\
	telepathy \
))

$(eval $(call gb_Library_add_exception_objects,tubes,\
	tubes/source/collaboration \
	tubes/source/conference \
	tubes/source/contact-list \
	tubes/source/contacts \
	tubes/source/manager \
))

$(eval $(call gb_Library_add_cobjects,tubes,\
	tubes/source/file-transfer-helper \
))

# vim:set shiftwidth=4 tabstop=4 noexpandtab: */
