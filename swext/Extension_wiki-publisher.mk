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

WIKISRC := $(SRCDIR)/swext/mediawiki

$(eval $(call gb_Extension_Extension,wiki-publisher,swext/mediawiki/src))

ifneq ($(SYSTEM_APACHE_COMMONS),YES)
$(eval $(call gb_Extension_add_file,wiki-publisher,commons-codec-1.3.jar,$(OUTDIR)/bin/commons-codec-1.3.jar)) 
$(eval $(call gb_Extension_add_file,wiki-publisher,commons-httpclient-3.1.jar,$(OUTDIR)/bin/commons-httpclient-3.1.jar)) 
$(eval $(call gb_Extension_add_file,wiki-publisher,commons-lang-2.3.jar,$(OUTDIR)/bin/commons-lang-2.3.jar)) 
$(eval $(call gb_Extension_add_file,wiki-publisher,commons-logging-1.1.1.jar,$(OUTDIR)/bin/commons-logging-1.1.1.jar)) 
endif
$(eval $(call gb_Extension_add_file,wiki-publisher,Addons.xcu,$(WORKDIR)/XcuMergeTarget/swext/mediawiki/src/registry/data/org/openoffice/Office/Addons.xcu))
$(eval $(call gb_Extension_add_file,wiki-publisher,Filter.xcu,$(WIKISRC)/src/registry/data/org/openoffice/TypeDetection/Filter.xcu))
$(eval $(call gb_Extension_add_file,wiki-publisher,OptionsDialog.xcu,$(WORKDIR)/XcuMergeTarget/swext/mediawiki/src/registry/data/org/openoffice/Office/OptionsDialog.xcu))
$(eval $(call gb_Extension_add_file,wiki-publisher,Paths.xcu,$(WIKISRC)/src/registry/data/org/openoffice/Office/Paths.xcu))
$(eval $(call gb_Extension_add_file,wiki-publisher,ProtocolHandler.xcu,$(WIKISRC)/src/registry/data/org/openoffice/Office/ProtocolHandler.xcu))
$(eval $(call gb_Extension_add_file,wiki-publisher,Types.xcu,$(WIKISRC)/src/registry/data/org/openoffice/TypeDetection/Types.xcu))
$(eval $(call gb_Extension_add_file,wiki-publisher,WikiExtension.xcs,$(WIKISRC)/src/registry/schema/org/openoffice/Office/Custom/WikiExtension.xcs))
$(eval $(call gb_Extension_add_file,wiki-publisher,WikiExtension.xcu,$(WORKDIR)/XcuMergeTarget/swext/mediawiki/src/registry/data/org/openoffice/Office/Custom/WikiExtension.xcu))
$(eval $(call gb_Extension_add_file,wiki-publisher,mediawiki.jar,$(OUTDIR)/bin/mediawiki.jar))
$(eval $(call gb_Extension_add_file,wiki-publisher,components.rdb,$(WIKISRC)/src/components.rdb))
$(eval $(call gb_Extension_add_file,wiki-publisher,WikiEditor/EditSetting.xdl,$(WIKISRC)/dialogs/EditSetting.xdl))
$(eval $(call gb_Extension_add_file,wiki-publisher,WikiEditor/Settings.xdl,$(WIKISRC)/dialogs/Settings.xdl))
$(eval $(call gb_Extension_add_file,wiki-publisher,WikiEditor/Module1.xba,$(WIKISRC)/dialogs/Module1.xba))
$(eval $(call gb_Extension_add_file,wiki-publisher,WikiEditor/script.xlb,$(WIKISRC)/dialogs/script.xlb))
$(eval $(call gb_Extension_add_file,wiki-publisher,WikiEditor/dialog.xlb,$(WIKISRC)/dialogs/dialog.xlb))
$(eval $(call gb_Extension_add_file,wiki-publisher,WikiEditor/SendToMediaWiki.xdl,$(WIKISRC)/dialogs/SendToMediaWiki.xdl))
$(eval $(call gb_Extension_add_file,wiki-publisher,filter/math/cmarkup.xsl,$(OUTDIR)/bin/xslt/export/xsltml/cmarkup.xsl))
$(eval $(call gb_Extension_add_file,wiki-publisher,filter/math/entities.xsl,$(OUTDIR)/bin/xslt/export/xsltml/entities.xsl))
$(eval $(call gb_Extension_add_file,wiki-publisher,filter/math/glayout.xsl,$(OUTDIR)/bin/xslt/export/xsltml/glayout.xsl))
$(eval $(call gb_Extension_add_file,wiki-publisher,filter/math/mmltex.xsl,$(OUTDIR)/bin/xslt/export/xsltml/mmltex.xsl))
$(eval $(call gb_Extension_add_file,wiki-publisher,filter/math/scripts.xsl,$(OUTDIR)/bin/xslt/export/xsltml/scripts.xsl))
$(eval $(call gb_Extension_add_file,wiki-publisher,filter/math/tables.xsl,$(OUTDIR)/bin/xslt/export/xsltml/tables.xsl))
$(eval $(call gb_Extension_add_file,wiki-publisher,filter/math/tokens.xsl,$(OUTDIR)/bin/xslt/export/xsltml/tokens.xsl))
$(eval $(call gb_Extension_add_file,wiki-publisher,filter/odt2mediawiki.xsl,$(WIKISRC)/src/filter/odt2mediawiki.xsl))
$(eval $(call gb_Extension_add_file,wiki-publisher,help/en-US/com.sun.wiki-publisher/wikisend.xhp,$(WIKISRC)/help/wikisend.xhp))
$(eval $(call gb_Extension_add_file,wiki-publisher,help/en-US/com.sun.wiki-publisher/wikiformats.xhp,$(WIKISRC)/help/wikiformats.xhp))
$(eval $(call gb_Extension_add_file,wiki-publisher,help/en-US/com.sun.wiki-publisher/wikiaccount.xhp,$(WIKISRC)/help/wikiaccount.xhp))
$(eval $(call gb_Extension_add_file,wiki-publisher,help/en-US/com.sun.wiki-publisher/wiki.xhp,$(WIKISRC)/help/wiki.xhp))
$(eval $(call gb_Extension_add_file,wiki-publisher,help/en-US/com.sun.wiki-publisher/wikisettings.xhp,$(WIKISRC)/help/wikisettings.xhp))
$(eval $(call gb_Extension_add_file,wiki-publisher,license/THIRDPARTYLICENSEREADME.html,$(WIKISRC)/src/THIRDPARTYLICENSEREADME.html))
$(eval $(call gb_Extension_add_file,wiki-publisher,templates/MediaWiki/mediawiki.ott,$(WIKISRC)/src/filter/mediawiki.ott))
$(eval $(call gb_Extension_localize_help,wiki-publisher,help/lang/com.sun.wiki-publisher/wikisend.xhp,$(WIKISRC)/help/wikisend.xhp))
$(eval $(call gb_Extension_localize_help,wiki-publisher,help/lang/com.sun.wiki-publisher/wikiformats.xhp,$(WIKISRC)/help/wikiformats.xhp))
$(eval $(call gb_Extension_localize_help,wiki-publisher,help/lang/com.sun.wiki-publisher/wikiaccount.xhp,$(WIKISRC)/help/wikiaccount.xhp))
$(eval $(call gb_Extension_localize_help,wiki-publisher,help/lang/com.sun.wiki-publisher/wiki.xhp,$(WIKISRC)/help/wiki.xhp))
$(eval $(call gb_Extension_localize_help,wiki-publisher,help/lang/com.sun.wiki-publisher/wikisettings.xhp,$(WIKISRC)/help/wikisettings.xhp))
