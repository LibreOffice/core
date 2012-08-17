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
# Copyright (C) 2010 Red Hat, Inc., David Tardon <dtardon@redhat.com>
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

$(eval $(call gb_JunitTest_JunitTest,jurt_urp))

$(eval $(call gb_JunitTest_use_customtargets,jurt_urp,\
    jurt/test/com/sun/star/lib/uno/protocols/urp \
))

$(eval $(call gb_JunitTest_use_jars,jurt_urp,\
    ridl \
))

$(eval $(call gb_JunitTest_use_jar_classset,jurt_urp,jurt))

$(eval $(call gb_JunitTest_add_sourcefiles,jurt_urp,\
    jurt/test/com/sun/star/lib/uno/protocols/urp/Cache_Test \
    jurt/test/com/sun/star/lib/uno/protocols/urp/Marshaling_Test \
    jurt/test/com/sun/star/lib/uno/protocols/urp/Protocol_Test \
    jurt/test/com/sun/star/lib/uno/protocols/urp/TestBridge \
    jurt/test/com/sun/star/lib/uno/protocols/urp/TestObject \
))

$(eval $(call gb_JunitTest_add_classes,jurt_urp,\
    com.sun.star.lib.uno.protocols.urp.Cache_Test \
    com.sun.star.lib.uno.protocols.urp.Marshaling_Test \
    com.sun.star.lib.uno.protocols.urp.Protocol_Test \
))

# vim:set shiftwidth=4 softtabstop=4 expandtab:
