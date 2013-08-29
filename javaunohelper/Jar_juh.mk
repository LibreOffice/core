# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Jar_Jar,juh))

$(eval $(call gb_Jar_use_jars,juh,\
    jurt \
    ridl \
))

$(eval $(call gb_Jar_set_packageroot,juh,com))

$(eval $(call gb_Jar_add_manifest_classpath,juh,ridl.jar jurt.jar ../../lib/ ../bin/))

$(eval $(call gb_Jar_set_manifest,juh,$(SRCDIR)/javaunohelper/util/manifest))

$(eval $(call gb_Jar_add_sourcefiles,juh,\
    javaunohelper/com/sun/star/comp/helper/Bootstrap \
    javaunohelper/com/sun/star/comp/helper/BootstrapException \
    javaunohelper/com/sun/star/comp/helper/ComponentContext \
    javaunohelper/com/sun/star/comp/helper/ComponentContextEntry \
    javaunohelper/com/sun/star/comp/helper/SharedLibraryLoader \
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

# vim:set noet sw=4 ts=4:
