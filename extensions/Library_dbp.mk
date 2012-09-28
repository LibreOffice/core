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
# The Initial Developer of the Original Code is
# 	Peter Foley <pefoley2@verizon.net>
# Portions created by the Initial Developer are Copyright (C) 2011 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.
#

$(eval $(call gb_Library_Library,dbp))

$(eval $(call gb_Library_set_componentfile,dbp,extensions/source/dbpilots/dbp))

$(eval $(call gb_Library_set_include,dbp,\
	$$(INCLUDE) \
	-I$(SRCDIR)/extensions/inc \
	-I$(SRCDIR)/extensions/source/inc \
))

$(eval $(call gb_Library_add_defs,dbp,\
	-DCOMPMOD_NAMESPACE=dbp \
))

$(eval $(call gb_Library_use_sdk_api,dbp))

$(eval $(call gb_Library_add_exception_objects,dbp,\
	extensions/source/dbpilots/commonpagesdbp \
	extensions/source/dbpilots/controlwizard \
	extensions/source/dbpilots/dbpservices \
	extensions/source/dbpilots/dbptools \
	extensions/source/dbpilots/gridwizard \
	extensions/source/dbpilots/groupboxwiz \
	extensions/source/dbpilots/listcombowizard \
	extensions/source/dbpilots/moduledbp \
	extensions/source/dbpilots/optiongrouplayouter \
	extensions/source/dbpilots/wizardservices \
))

$(eval $(call gb_Library_use_libraries,dbp,\
	svt \
	vcl \
	tl \
	svl \
	sfx \
	dbtools \
	utl \
	comphelper \
	cppuhelper \
	cppu \
	sal \
	$(gb_UWINAPI) \
))

# vim:set shiftwidth=4 softtabstop=4 expandtab:
