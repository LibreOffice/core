# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_GeneratedPackage_GeneratedPackage,python-core,$(MINGW_SYSROOT)/lib))

# NOTE: The original code excluded *.pyc, *.py~, *.orig and *_failed . I
# assume it was just a size optimization. If their presence actually
# causes a problem, an extension of GeneratedPackage is is left as an
# excercise for the poor misguided person who wants to cross-compile
# with mingw and system python and actually use the results for
# something.
$(eval $(call gb_GeneratedPackage_add_dir,python-core,$(INSTDIR)/program/python-core-$(PYTHON_VERSION)/lib,python$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)))

# vim: set noet sw=4 ts=4:
