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

$(eval $(call gb_Module_Module,ucb))

$(eval $(call gb_Module_add_targets,ucb,\
	Library_cached1 \
	Library_srtrs1 \
	Library_ucb1 \
	Library_ucpcmis1 \
	Library_ucpexpand1 \
	Library_ucpext \
	Library_ucpfile1 \
	Library_ucpftp1 \
	Library_ucphier1 \
	Library_ucppkg1 \
	Library_ucptdoc1 \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Module_add_targets,ucb,\
	Library_ucpodma1 \
	Package_odma_inc \
	StaticLibrary_odma_lib \
))
endif

ifneq ($(DISABLE_NEON),TRUE)
$(eval $(call gb_Module_add_targets,ucb,\
	Library_ucpdav1 \
))
endif

ifeq ($(ENABLE_GIO),TRUE)
$(eval $(call gb_Module_add_targets,ucb,\
	Library_ucpgio1 \
))
endif

ifeq ($(ENABLE_GNOMEVFS),TRUE)
$(eval $(call gb_Module_add_targets,ucb,\
	Library_ucpgvfs1 \
))
endif

$(eval $(call gb_Module_add_subsequentcheck_targets,ucb,\
	JunitTest_ucb_complex \
	JunitTest_ucb_unoapi \
))

# vim: set noet sw=4 ts=4:
