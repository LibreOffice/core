# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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
#       Bjoern Michaelsen, Canonical Ltd. <bjoern.michaelsen@canonical.com>
# Portions created by the Initial Developer are Copyright (C) 2010 the
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

$(eval $(call gb_Module_Module,dbaccess))

ifneq (,$(filter DBCONNECTIVITY,$(BUILD_TYPE)))

$(eval $(call gb_Module_add_targets,dbaccess,\
    AllLangResTarget_dba \
    AllLangResTarget_dbmm \
    AllLangResTarget_dbu \
    AllLangResTarget_sdbt \
    $(if $(filter WNT,$(OS)),Executable_odbcconfig) \
    Library_dba \
    Library_dbaxml \
    Library_dbmm \
    Library_dbu \
    Library_sdbt \
    Package_inc \
    UIConfig_dbaccess \
	UIConfig_dbapp \
	UIConfig_dbbrowser \
	UIConfig_dbquery \
	UIConfig_dbrelation \
	UIConfig_dbtable \
	UIConfig_dbtdata \
))

$(eval $(call gb_Module_add_check_targets,dbaccess,\
))

# disable test because it still fails in some situations
#    CppunitTest_dbaccess_macros_test \
#


$(eval $(call gb_Module_add_subsequentcheck_targets,dbaccess,\
	JunitTest_dbaccess_complex \
    JunitTest_dbaccess_unoapi \
))

endif

# vim: set noet sw=4 ts=4:
