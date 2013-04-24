# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,codemaker))

$(eval $(call gb_StaticLibrary_use_external,codemaker,boost_headers))

$(eval $(call gb_StaticLibrary_add_exception_objects,codemaker,\
	codemaker/source/codemaker/exceptiontree \
	codemaker/source/codemaker/global \
	codemaker/source/codemaker/options \
	codemaker/source/codemaker/typemanager \
	codemaker/source/codemaker/unotype \
	codemaker/source/codemaker/codemaker \
))

# vim: set noet sw=4 ts=4:
