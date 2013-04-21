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
# Major Contributor(s):
# Copyright (C) 2012 David Ostrovsky <d.ostrovsky@gmx.de> (initial developer)
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

$(eval $(call gb_Module_Module,pyuno))

ifneq ($(DISABLE_PYTHON),TRUE)

$(eval $(call gb_Module_add_targets,pyuno,\
    Library_pyuno \
    Library_pythonloader \
    Package_pyunorc \
    Package_python_scripts \
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
# with SYSTEM_PYTHON=YES and SYSTEM_PYTHON=NO

# zipcore: pyuno/python.exe on Windows
ifeq ($(OS),WNT)
$(eval $(call gb_Module_add_targets,pyuno,\
    Executable_python_wrapper \
))
endif

ifneq ($(SYSTEM_PYTHON),YES)

# zipcore: python.sh on Unix
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
    CustomTarget_zipcore \
    Package_zipcore \
))
endif

endif # SYSTEM_PYTHON

endif # DISABLE_PYTHON

# vim:set noet sw=4 ts=4:
