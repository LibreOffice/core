# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_JunitTest_JunitTest,ridljar_typedesc))

$(eval $(call gb_JunitTest_use_jars,ridljar_typedesc,\
    ridl \
))

$(eval $(call gb_JunitTest_add_sourcefiles,ridljar_typedesc,\
    ridljar/test/com/sun/star/lib/uno/typedesc/TypeDescription_Test \
))

$(eval $(call gb_JunitTest_add_classes,ridljar_typedesc,\
    com.sun.star.lib.uno.typedesc.TypeDescription_Test \
))

# vim:set noet sw=4 ts=4:
