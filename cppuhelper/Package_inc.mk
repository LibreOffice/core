# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
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
# Copyright (C) 2011 Matúš Kukan <matus.kukan@gmail.com> (initial developer)
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

$(eval $(call gb_Package_Package,cppuhelper_inc,$(SRCDIR)/cppuhelper/inc))

$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/cppuhelperdllapi.h,cppuhelper/cppuhelperdllapi.h))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/access_control.hxx,cppuhelper/access_control.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/basemutex.hxx,cppuhelper/basemutex.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/bootstrap.hxx,cppuhelper/bootstrap.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/compbase10.hxx,cppuhelper/compbase10.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/compbase11.hxx,cppuhelper/compbase11.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/compbase12.hxx,cppuhelper/compbase12.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/compbase1.hxx,cppuhelper/compbase1.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/compbase2.hxx,cppuhelper/compbase2.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/compbase3.hxx,cppuhelper/compbase3.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/compbase4.hxx,cppuhelper/compbase4.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/compbase5.hxx,cppuhelper/compbase5.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/compbase6.hxx,cppuhelper/compbase6.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/compbase7.hxx,cppuhelper/compbase7.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/compbase8.hxx,cppuhelper/compbase8.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/compbase9.hxx,cppuhelper/compbase9.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/compbase_ex.hxx,cppuhelper/compbase_ex.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/compbase.hxx,cppuhelper/compbase.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/component_context.hxx,cppuhelper/component_context.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/component.hxx,cppuhelper/component.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/exc_hlp.hxx,cppuhelper/exc_hlp.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/factory.hxx,cppuhelper/factory.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/findsofficepath.h,cppuhelper/findsofficepath.h))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/implbase10.hxx,cppuhelper/implbase10.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/implbase11.hxx,cppuhelper/implbase11.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/implbase12.hxx,cppuhelper/implbase12.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/implbase1.hxx,cppuhelper/implbase1.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/implbase2.hxx,cppuhelper/implbase2.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/implbase3.hxx,cppuhelper/implbase3.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/implbase4.hxx,cppuhelper/implbase4.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/implbase5.hxx,cppuhelper/implbase5.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/implbase6.hxx,cppuhelper/implbase6.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/implbase7.hxx,cppuhelper/implbase7.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/implbase8.hxx,cppuhelper/implbase8.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/implbase9.hxx,cppuhelper/implbase9.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/implbase_ex.hxx,cppuhelper/implbase_ex.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/implbase_ex_post.hxx,cppuhelper/implbase_ex_post.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/implbase_ex_pre.hxx,cppuhelper/implbase_ex_pre.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/implbase.hxx,cppuhelper/implbase.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/implementationentry.hxx,cppuhelper/implementationentry.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/interfacecontainer.h,cppuhelper/interfacecontainer.h))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/interfacecontainer.hxx,cppuhelper/interfacecontainer.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/propertysetmixin.hxx,cppuhelper/propertysetmixin.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/propshlp.hxx,cppuhelper/propshlp.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/proptypehlp.h,cppuhelper/proptypehlp.h))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/proptypehlp.hxx,cppuhelper/proptypehlp.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/queryinterface.hxx,cppuhelper/queryinterface.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/servicefactory.hxx,cppuhelper/servicefactory.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/shlib.hxx,cppuhelper/shlib.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/stdidlclass.hxx,cppuhelper/stdidlclass.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/supportsservice.hxx,cppuhelper/supportsservice.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/typeprovider.hxx,cppuhelper/typeprovider.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/unourl.hxx,cppuhelper/unourl.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/weakagg.hxx,cppuhelper/weakagg.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/weak.hxx,cppuhelper/weak.hxx))
$(eval $(call gb_Package_add_file,cppuhelper_inc,inc/cppuhelper/weakref.hxx,cppuhelper/weakref.hxx))

# vim: set noet sw=4 ts=4:
