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

$(eval $(call gb_Jar_Jar,juh))

$(eval $(call gb_Jar_use_jars,juh,\
    jurt \
    ridl \
    unoil \
))

$(eval $(call gb_Jar_set_packageroot,juh,com))

$(eval $(call gb_Jar_add_manifest_classpath,juh,ridl.jar jurt.jar ../../lib/ ../bin/))

$(eval $(call gb_Jar_set_manifest,juh,$(SRCDIR)/javaunohelper/util/manifest))

$(eval $(call gb_Jar_set_componentfile,juh,javaunohelper/util/juh,URE))

$(eval $(call gb_Jar_add_sourcefiles,juh,\
    javaunohelper/com/sun/star/comp/JavaUNOHelperServices \
    javaunohelper/com/sun/star/comp/helper/Bootstrap \
    javaunohelper/com/sun/star/comp/helper/BootstrapException \
    javaunohelper/com/sun/star/comp/helper/ComponentContext \
    javaunohelper/com/sun/star/comp/helper/ComponentContextEntry \
    javaunohelper/com/sun/star/comp/helper/RegistryServiceFactory \
    javaunohelper/com/sun/star/comp/helper/SharedLibraryLoader \
    javaunohelper/com/sun/star/comp/helper/UnoInfo \
    javaunohelper/com/sun/star/comp/juhtest/SmoketestCommandEnvironment \
    javaunohelper/com/sun/star/lib/uno/adapter/ByteArrayToXInputStreamAdapter \
    javaunohelper/com/sun/star/lib/uno/adapter/InputStreamToXInputStreamAdapter \
    javaunohelper/com/sun/star/lib/uno/adapter/OutputStreamToXOutputStreamAdapter \
    javaunohelper/com/sun/star/lib/uno/adapter/XInputStreamToInputStreamAdapter \
    javaunohelper/com/sun/star/lib/uno/adapter/XOutputStreamToByteArrayAdapter \
    javaunohelper/com/sun/star/lib/uno/adapter/XOutputStreamToOutputStreamAdapter \
    javaunohelper/com/sun/star/lib/uno/helper/ComponentBase \
    javaunohelper/com/sun/star/lib/uno/helper/Factory \
    javaunohelper/com/sun/star/lib/uno/helper/InterfaceContainer \
    javaunohelper/com/sun/star/lib/uno/helper/MultiTypeInterfaceContainer \
    javaunohelper/com/sun/star/lib/uno/helper/PropertySet \
    javaunohelper/com/sun/star/lib/uno/helper/PropertySetMixin \
    javaunohelper/com/sun/star/lib/uno/helper/UnoUrl \
    javaunohelper/com/sun/star/lib/uno/helper/WeakAdapter \
    javaunohelper/com/sun/star/lib/uno/helper/WeakBase \
))

# vim:set shiftwidth=4 softtabstop=4 expandtab:
