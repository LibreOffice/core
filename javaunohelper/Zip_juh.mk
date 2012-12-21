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

$(eval $(call gb_Zip_Zip,juh_src,$(SRCDIR)/javaunohelper))

$(eval $(call gb_Zip_add_files,juh_src,\
    com/sun/star/comp/JavaUNOHelperServices.java \
    com/sun/star/comp/helper/Bootstrap.java \
    com/sun/star/comp/helper/BootstrapException.java \
    com/sun/star/comp/helper/ComponentContext.java \
    com/sun/star/comp/helper/ComponentContextEntry.java \
    com/sun/star/comp/helper/SharedLibraryLoader.java \
    com/sun/star/comp/helper/UnoInfo.java \
    com/sun/star/comp/juhtest/SmoketestCommandEnvironment.java \
    com/sun/star/lib/uno/adapter/ByteArrayToXInputStreamAdapter.java \
    com/sun/star/lib/uno/adapter/InputStreamToXInputStreamAdapter.java \
    com/sun/star/lib/uno/adapter/OutputStreamToXOutputStreamAdapter.java \
    com/sun/star/lib/uno/adapter/XInputStreamToInputStreamAdapter.java \
    com/sun/star/lib/uno/adapter/XOutputStreamToByteArrayAdapter.java \
    com/sun/star/lib/uno/adapter/XOutputStreamToOutputStreamAdapter.java \
    com/sun/star/lib/uno/helper/ComponentBase.java \
    com/sun/star/lib/uno/helper/Factory.java \
    com/sun/star/lib/uno/helper/InterfaceContainer.java \
    com/sun/star/lib/uno/helper/MultiTypeInterfaceContainer.java \
    com/sun/star/lib/uno/helper/PropertySet.java \
    com/sun/star/lib/uno/helper/PropertySetMixin.java \
    com/sun/star/lib/uno/helper/UnoUrl.java \
    com/sun/star/lib/uno/helper/WeakAdapter.java \
    com/sun/star/lib/uno/helper/WeakBase.java \
))

# vim:set shiftwidth=4 softtabstop=4 expandtab:
