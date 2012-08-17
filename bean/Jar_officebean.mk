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
# The Initial Developer of the Original Code is
# 	Peter Foley <pefoley2@verizon.net>
# Portions created by the Initial Developer are Copyright (C) 2011 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.
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
