#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#

$(eval $(call gb_Configuration_Configuration,mediawiki,nodeliver))

$(eval $(call gb_Configuration_add_datas,mediawiki,swext/mediawiki/src/registry/data,\
	org/openoffice/Office/ProtocolHandler.xcu \
	org/openoffice/Office/Paths.xcu \
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
