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

$(eval $(call gb_Zip_Zip,mediawiki_develop,$(WORKDIR)/Zip/mediawiki_develop))

$(eval $(call gb_Zip_add_file,mediawiki_develop,Addons.xcu,$(SRCDIR)/swext/mediawiki/src/registry/data/org/openoffice/Office/Addons.xcu))
$(eval $(call gb_Zip_add_file,mediawiki_develop,Filter.xcu,$(SRCDIR)/swext/mediawiki/src/registry/data/org/openoffice/TypeDetection/Filter.xcu))
$(eval $(call gb_Zip_add_file,mediawiki_develop,OptionsDialog.xcu,$(SRCDIR)/swext/mediawiki/src/registry/data/org/openoffice/Office/OptionsDialog.xcu))
$(eval $(call gb_Zip_add_file,mediawiki_develop,Paths.xcu,$(SRCDIR)/swext/mediawiki/src/registry/data/org/openoffice/Office/Paths.xcu))
$(eval $(call gb_Zip_add_file,mediawiki_develop,ProtocolHandler.xcu,$(SRCDIR)/swext/mediawiki/src/registry/data/org/openoffice/Office/ProtocolHandler.xcu))
$(eval $(call gb_Zip_add_file,mediawiki_develop,Types.xcu,$(SRCDIR)/swext/mediawiki/src/registry/data/org/openoffice/TypeDetection/Types.xcu))
$(eval $(call gb_Zip_add_file,mediawiki_develop,WikiExtension.xcs,$(SRCDIR)/swext/mediawiki/src/registry/schema/org/openoffice/Office/Custom/WikiExtension.xcs))
$(eval $(call gb_Zip_add_file,mediawiki_develop,WikiExtension.xcu,$(SRCDIR)/swext/mediawiki/src/registry/data/org/openoffice/Office/Custom/WikiExtension.xcu))
$(eval $(call gb_Zip_add_file,mediawiki_develop,description.xml,$(SRCDIR)/swext/mediawiki/src/description.xml))
$(eval $(call gb_Zip_add_file,mediawiki_develop,mediawiki.jar,$(OUTDIR)/bin/mediawiki.jar))
$(eval $(call gb_Zip_add_file,mediawiki_develop,META-INF/manifest.xml,$(SRCDIR)/swext/mediawiki/src/manifest.xml))
$(eval $(call gb_Zip_add_file,mediawiki_develop,components.rdb,$(SRCDIR)/swext/mediawiki/src/components.rdb))
$(eval $(call gb_Zip_add_file,mediawiki_develop,WikiEditor/EditSetting.xdl,$(SRCDIR)/swext/mediawiki/dialogs/EditSetting.xdl))
$(eval $(call gb_Zip_add_file,mediawiki_develop,WikiEditor/Settings.xdl,$(SRCDIR)/swext/mediawiki/dialogs/Settings.xdl))
$(eval $(call gb_Zip_add_file,mediawiki_develop,WikiEditor/Module1.xba,$(SRCDIR)/swext/mediawiki/dialogs/Module1.xba))
$(eval $(call gb_Zip_add_file,mediawiki_develop,WikiEditor/script.xlb,$(SRCDIR)/swext/mediawiki/dialogs/script.xlb))
$(eval $(call gb_Zip_add_file,mediawiki_develop,WikiEditor/dialog.xlb,$(SRCDIR)/swext/mediawiki/dialogs/dialog.xlb))
$(eval $(call gb_Zip_add_file,mediawiki_develop,WikiEditor/SendToMediaWiki.xdl,$(SRCDIR)/swext/mediawiki/dialogs/SendToMediaWiki.xdl))
$(eval $(call gb_Zip_add_file,mediawiki_develop,filter/odt2mediawiki.xsl,$(SRCDIR)/swext/mediawiki/src/filter/odt2mediawiki.xsl))
$(eval $(call gb_Zip_add_file,mediawiki_develop,description-en-US.txt,$(SRCDIR)/swext/mediawiki/src/description-en-US.txt))
$(eval $(call gb_Zip_add_file,mediawiki_develop,help/en-US/com.sun.wiki-publisher/wikisend.xhp,$(SRCDIR)/swext/mediawiki/help/wikisend.xhp))
$(eval $(call gb_Zip_add_file,mediawiki_develop,help/en-US/com.sun.wiki-publisher/wikiformats.xhp,$(SRCDIR)/swext/mediawiki/help/wikiformats.xhp))
$(eval $(call gb_Zip_add_file,mediawiki_develop,help/en-US/com.sun.wiki-publisher/wikiaccount.xhp,$(SRCDIR)/swext/mediawiki/help/wikiaccount.xhp))
$(eval $(call gb_Zip_add_file,mediawiki_develop,help/en-US/com.sun.wiki-publisher/wiki.xhp,$(SRCDIR)/swext/mediawiki/help/wiki.xhp))
$(eval $(call gb_Zip_add_file,mediawiki_develop,help/en-US/com.sun.wiki-publisher/wikisettings.xhp,$(SRCDIR)/swext/mediawiki/help/wikisettings.xhp))
$(eval $(call gb_Zip_add_file,mediawiki_develop,license/THIRDPARTYLICENSEREADME.html,$(SRCDIR)/swext/mediawiki/src/THIRDPARTYLICENSEREADME.html))
$(eval $(call gb_Zip_add_file,mediawiki_develop,templates/MediaWiki/mediawiki.ott,$(SRCDIR)/swext/mediawiki/src/filter/mediawiki.ott))

