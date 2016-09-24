# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,pyuno))

ifneq ($(DISABLE_PYTHON),TRUE)

$(eval $(call gb_Module_add_targets,pyuno,\
    CustomTarget_pyuno_pythonloader_ini \
    Library_pyuno \
    Library_pythonloader \
    Package_python_scripts \
    Package_pyuno_pythonloader_ini \
    Rdb_pyuno \
))

ifneq ($(OS),WNT)
$(eval $(call gb_Module_add_targets,pyuno,\
    Library_pyuno_wrapper \
))
endif

#
# Windows: only --enable-python=internal possible
# mingw: both cases possible: internal && system
# that why it makes sense to handle the next 3 targets
# with SYSTEM_PYTHON=TRUE and SYSTEM_PYTHON=

# python-core: pyuno/python.exe on Windows
ifeq ($(OS),WNT)
$(eval $(call gb_Module_add_targets,pyuno,\
    Executable_python \
))
endif

ifeq ($(SYSTEM_PYTHON),)

# python-core: python.sh on Unix
ifneq ($(OS),WNT)
$(eval $(call gb_Module_add_targets,pyuno,\
    CustomTarget_python_shell \
    Package_python_shell \
))
endif

else # SYSTEM_PYTHON

# these two targets have to be executed only with system-python on mingw
# FIXME remove this
ifeq ($(OS)$(COM),WNTGCC)
$(eval $(call gb_Module_add_targets,pyuno,\
    GeneratedPackage_python-core \
))
endif

endif # SYSTEM_PYTHON

ifneq ($(OS),MACOSX)
$(eval $(call gb_Module_add_check_targets,pyuno, \
    PythonTest_pyuno_pytests_ssl \
))
endif

ifneq (,$(filter PythonTest_pytests,$(MAKECMDGOALS)))
$(eval $(call gb_Module_add_targets,pyuno, \
    PythonTest_pytests \
    PythonTest_pyuno_pytests_testcollections \
    PythonTest_pyuno_pytests_insertremovecells \
))
endif

endif # DISABLE_PYTHON

# vim:set noet sw=4 ts=4:
