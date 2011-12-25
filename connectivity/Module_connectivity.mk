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
# Copyright (C) 2011 Peter Foley <pefoley2@verizon.net> (initial developer)
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

$(eval $(call gb_Module_Module,connectivity))

$(eval $(call gb_Module_add_targets,connectivity,\
	AllLangResTarget_connectivity \
	Package_inc \
	Package_xml \
	Library_dbtools \
	Library_sdbc2 \
	Library_dbpool2 \
	Library_calc \
	Library_odbcbase \
	Library_odbc \
	Library_mysql \
	Library_file \
	Library_flat \
	Library_dbase \
))

ifneq ($(SOLAR_JAVA),)
$(eval $(call gb_Module_add_targets,connectivity,\
	Jar_ConnectivityTools \
	Jar_sdbc_hsqldb \
	Library_jdbc \
))
endif

ifeq ($(GUI),UNX)

ifeq ($(ENABLE_KAB),TRUE)
$(eval $(call gb_Module_add_targets,connectivity,\
	Library_kab1 \
	Library_kabdrv1 \
))
endif

ifeq ($(OS),MACOSX)
$(eval $(call gb_Module_add_targets,connectivity,\
	Library_macab1 \
	Library_macabdrv1 \
))
endif

endif

ifeq ($(GUI),WNT)
$(eval $(call gb_Module_add_targets,connectivity,\
	Library_ado \
))
endif

ifeq ($(ENABLE_EVOAB2),TRUE)
$(eval $(call gb_Module_add_targets,connectivity,\
	Library_evoab2 \
))
endif

# vim: set noet sw=4 ts=4:
