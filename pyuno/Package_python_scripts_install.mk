# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,pyuno_python_scripts_install,$(SRCDIR)/pyuno/source))

$(eval $(call gb_Package_set_outdir,pyuno_python_scripts_install,$(INSTDIR)))

$(eval $(call gb_Package_add_files,pyuno_python_scripts_install,$(LIBO_LIB_PYUNO_FOLDER),\
	loader/pythonloader.py \
	module/uno.py \
	module/unohelper.py \
))

# vim: set noet sw=4 ts=4:
