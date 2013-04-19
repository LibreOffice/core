# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,jvmaccess_inc,$(SRCDIR)/jvmaccess/inc/jvmaccess))

$(eval $(call gb_Package_add_file,jvmaccess_inc,inc/jvmaccess/classpath.hxx,classpath.hxx))
$(eval $(call gb_Package_add_file,jvmaccess_inc,inc/jvmaccess/jvmaccessdllapi.h,jvmaccessdllapi.h))
$(eval $(call gb_Package_add_file,jvmaccess_inc,inc/jvmaccess/unovirtualmachine.hxx,unovirtualmachine.hxx))
$(eval $(call gb_Package_add_file,jvmaccess_inc,inc/jvmaccess/virtualmachine.hxx,virtualmachine.hxx))

# vim:set noet sw=4 ts=4:
