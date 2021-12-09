#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#

$(eval $(call gb_Jar_Jar,XMergeBridge))

$(eval $(call gb_Jar_set_componentfile,XMergeBridge,xmerge/source/bridge/XMergeBridge,OOO,services))

$(eval $(call gb_Jar_set_manifest,XMergeBridge,$(SRCDIR)/xmerge/source/bridge/manifest.mf))

$(eval $(call gb_Jar_set_packageroot,XMergeBridge,*.class))

$(eval $(call gb_Jar_use_jars,XMergeBridge,\
	xmerge \
	libreoffice \
))

$(eval $(call gb_Jar_add_sourcefiles,XMergeBridge,\
	xmerge/source/bridge/java/XMergeBridge \
))
