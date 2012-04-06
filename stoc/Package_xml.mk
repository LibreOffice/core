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
# Copyright (C) 2012 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
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

$(eval $(call gb_Package_Package,stoc_xml,$(SRCDIR)/stoc/source))

$(eval $(call gb_Package_add_file,stoc_xml,xml/module-description.dtd,module-description.dtd))

$(eval $(call gb_Package_add_file,stoc_xml,xml/reflection.uno.xml,corereflection/corefl.xml))
$(eval $(call gb_Package_add_file,stoc_xml,xml/introspection.uno.xml,inspect/insp.xml))
$(eval $(call gb_Package_add_file,stoc_xml,xml/invocation.uno.xml,invocation/inv.xml))
$(eval $(call gb_Package_add_file,stoc_xml,xml/invocadapt.uno.xml,invocation_adapterfactory/invadp.xml))
$(eval $(call gb_Package_add_file,stoc_xml,xml/javavm.uno.xml,javavm/jen.xml))
$(eval $(call gb_Package_add_file,stoc_xml,xml/javaloader.uno.xml,javaloader/javaloader.xml))
$(eval $(call gb_Package_add_file,stoc_xml,xml/namingservice.uno.xml,namingservice/namingservice.xml))
$(eval $(call gb_Package_add_file,stoc_xml,xml/proxyfac.uno.xml,proxy_factory/proxyfac.xml))
$(eval $(call gb_Package_add_file,stoc_xml,xml/bootstrap.uno.xml,bootstrap/bootstrap.xml))
$(eval $(call gb_Package_add_file,stoc_xml,xml/stocserv.uno.xml,stocservices/stocserv.xml))

# vim:set shiftwidth=4 softtabstop=4 expandtab:
