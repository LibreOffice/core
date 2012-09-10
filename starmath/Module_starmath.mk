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
# The Initial Developer of the Original Code is
# Norbert Thiebaud <nthiebaud@gmail.com> (C) 2010, All Rights Reserved.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Module_Module,starmath))

$(eval $(call gb_Module_add_targets,starmath,\
    AllLangResTarget_sm \
    Library_sm \
    Library_smd \
    Package_uiconfig \
    UI_smath \
))

#$(eval $(call gb_Module_add_check_targets,starmath,\
#    CppunitTest_starmath_qa_cppunit \
#))

$(eval $(call gb_Module_add_subsequentcheck_targets,starmath,\
    JunitTest_starmath_unoapi \
))

# vim: set noet sw=4 ts=4:
