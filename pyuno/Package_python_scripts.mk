# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,pyuno_python,$(SRCDIR)/pyuno/source))

ifeq ($(OS),WNT)
pyuno_PYTHON_SCRIPT_DIR=bin
else
pyuno_PYTHON_SCRIPT_DIR=lib
endif

$(eval $(call gb_Package_add_file,pyuno_python,$(pyuno_PYTHON_SCRIPT_DIR)/pyuno/unohelper.py,module/unohelper.py))
$(eval $(call gb_Package_add_file,pyuno_python,$(pyuno_PYTHON_SCRIPT_DIR)/pyuno/uno.py,module/uno.py))
$(eval $(call gb_Package_add_file,pyuno_python,$(pyuno_PYTHON_SCRIPT_DIR)/pyuno/pythonloader.py,loader/pythonloader.py))

# vim: set noet sw=4 ts=4:
