# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Dummy .mk to have a single "make PythonTest_pytests" goal to run all the
# pyuno/PythonTest_pyuno_pytests_*.mk tests (which are not run by default).
#
# To add a new test pyuno/PythonTest_pyuno_pytests_NEW.mk, add
#
#   $(call gb_PythonTest_get_target,pyuno_pytests_NEW) \
#
# to the below list and
#
#   PythonTest_pyuno_pytests_NEW \
#
# to the list in the "ifneq (,$(filter PythonTest_pytests,$(MAKECMDGOALS)))"
# section of pyuno/Module_pyuno.mk.

$(eval $(call gb_PythonTest_PythonTest,pytests))

$(call gb_PythonTest_get_target,pytests) : \
    $(call gb_PythonTest_get_target,pyuno_pytests_insertremovecells) \

# vim: set noet sw=4 ts=4:
