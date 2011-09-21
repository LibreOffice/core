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

$(eval $(call gb_Configuration_Configuration,mediawiki,SRCDIR))

$(eval $(call gb_Configuration_add_datas,mediawiki,swext/mediawiki/src/registry/data,\
	org/openoffice/Office/Addons.xcu \
	org/openoffice/Office/OptionsDialog.xcu \
	org/openoffice/Office/ProtocolHandler.xcu \
	org/openoffice/Office/Paths.xcu \
	org/openoffice/Office/Custom/WikiExtension.xcu \
	org/openoffice/TypeDetection/Filter.xcu \
	org/openoffice/TypeDetection/Types.xcu \
))

$(eval $(call gb_Configuration_add_localized_datas,mediawiki,swext/mediawiki/src/registry/data,\
	org/openoffice/Office/Addons.xcu \
	org/openoffice/Office/OptionsDialog.xcu \
	org/openoffice/Office/Custom/WikiExtension.xcu \
))

$(eval $(call gb_Configuration_add_schemas,mediawiki,swext/mediawiki/src/registry/schema,\
	org/openoffice/Office/Custom/WikiExtension.xcs \
))	
