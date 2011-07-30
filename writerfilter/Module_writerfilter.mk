# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
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
#       David Tardon, Red Hat Inc. <dtardon@redhat.com>
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

$(eval $(call gb_Module_Module,writerfilter))

$(eval $(call gb_Module_add_targets,writerfilter,\
    Library_doctok \
    Library_ooxml \
    Library_resourcemodel \
    Library_rtftok \
    Library_writerfilter \
    Library_writerfilter_uno \
    Package_writerfilter_generated \
))

$(eval $(call gb_Module_add_check_targets,writerfilter,\
    CppunitTest_writerfilter_doctok \
))

$(eval $(call gb_Module_add_subsequentcheck_targets,writerfilter,\
    JunitTest_writerfilter_complex \
))

# vim: set noet sw=4 ts=4:
