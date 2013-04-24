#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#

$(eval $(call gb_Jar_Jar,officebean))

$(eval $(call gb_Jar_use_jars,officebean,\
	ridl \
	unoil \
	jurt \
	juh \
))

$(eval $(call gb_Jar_set_packageroot,officebean,com))

$(eval $(call gb_Jar_add_sourcefiles,officebean,\
	bean/com/sun/star/beans/ContainerFactory \
	bean/com/sun/star/beans/JavaWindowPeerFake \
	bean/com/sun/star/beans/LocalOfficeConnection \
	bean/com/sun/star/beans/LocalOfficeWindow \
	bean/com/sun/star/beans/NativeConnection \
	bean/com/sun/star/beans/NativeService \
	bean/com/sun/star/beans/OfficeConnection \
	bean/com/sun/star/beans/OfficeWindow \
	bean/com/sun/star/comp/beans/ContainerFactory \
	bean/com/sun/star/comp/beans/Controller \
	bean/com/sun/star/comp/beans/Frame \
	bean/com/sun/star/comp/beans/HasConnectionException \
	bean/com/sun/star/comp/beans/InvalidArgumentException \
	bean/com/sun/star/comp/beans/JavaWindowPeerFake \
	bean/com/sun/star/comp/beans/LocalOfficeConnection \
	bean/com/sun/star/comp/beans/LocalOfficeWindow \
	bean/com/sun/star/comp/beans/NativeConnection \
	bean/com/sun/star/comp/beans/NativeService \
	bean/com/sun/star/comp/beans/NoConnectionException \
	bean/com/sun/star/comp/beans/NoDocumentException \
	bean/com/sun/star/comp/beans/OfficeConnection \
	bean/com/sun/star/comp/beans/OfficeDocument \
	bean/com/sun/star/comp/beans/OfficeWindow \
	bean/com/sun/star/comp/beans/OOoBean \
	bean/com/sun/star/comp/beans/SystemWindowException \
	bean/com/sun/star/comp/beans/Wrapper \
	bean/com/sun/star/comp/beans/CallWatchThread \
))
