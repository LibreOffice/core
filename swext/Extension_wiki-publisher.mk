# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#

$(eval $(call gb_Extension_Extension,wiki-publisher,swext/mediawiki/src))

$(eval $(call gb_Extension_use_unpacked,wiki-publisher,xsltml))

$(eval $(call gb_Extension_use_default_description,wiki-publisher,swext/mediawiki/src/description-en-US.txt))
$(eval $(call gb_Extension_use_default_license,wiki-publisher))

$(eval $(call gb_Extension_add_file,wiki-publisher,Addons.xcu,$(call gb_XcuFile_for_extension,swext/mediawiki/src/registry/data/org/openoffice/Office/Addons.xcu)))
$(eval $(call gb_Extension_add_file,wiki-publisher,Filter.xcu,$(SRCDIR)/swext/mediawiki/src/registry/data/org/openoffice/TypeDetection/Filter.xcu))
$(eval $(call gb_Extension_add_file,wiki-publisher,OptionsDialog.xcu,$(call gb_XcuFile_for_extension,swext/mediawiki/src/registry/data/org/openoffice/Office/OptionsDialog.xcu)))
$(eval $(call gb_Extension_add_file,wiki-publisher,Paths.xcu,$(SRCDIR)/swext/mediawiki/src/registry/data/org/openoffice/Office/Paths.xcu))
$(eval $(call gb_Extension_add_file,wiki-publisher,ProtocolHandler.xcu,$(SRCDIR)/swext/mediawiki/src/registry/data/org/openoffice/Office/ProtocolHandler.xcu))
$(eval $(call gb_Extension_add_file,wiki-publisher,Types.xcu,$(SRCDIR)/swext/mediawiki/src/registry/data/org/openoffice/TypeDetection/Types.xcu))
$(eval $(call gb_Extension_add_file,wiki-publisher,WikiExtension.xcs,$(SRCDIR)/swext/mediawiki/src/registry/schema/org/openoffice/Office/Custom/WikiExtension.xcs))
$(eval $(call gb_Extension_add_file,wiki-publisher,WikiExtension.xcu,$(call gb_XcuFile_for_extension,swext/mediawiki/src/registry/data/org/openoffice/Office/Custom/WikiExtension.xcu)))
$(eval $(call gb_Extension_add_file,wiki-publisher,mediawiki.jar,$(call gb_Jar_get_target,mediawiki)))
$(eval $(call gb_Extension_add_file,wiki-publisher,components.rdb,$(SRCDIR)/swext/mediawiki/src/components.rdb))
$(eval $(call gb_Extension_add_file,wiki-publisher,WikiEditor/EditSetting.xdl,$(SRCDIR)/swext/mediawiki/dialogs/EditSetting.xdl))
$(eval $(call gb_Extension_add_file,wiki-publisher,WikiEditor/Settings.xdl,$(SRCDIR)/swext/mediawiki/dialogs/Settings.xdl))
$(eval $(call gb_Extension_add_file,wiki-publisher,WikiEditor/Module1.xba,$(SRCDIR)/swext/mediawiki/dialogs/Module1.xba))
$(eval $(call gb_Extension_add_file,wiki-publisher,WikiEditor/script.xlb,$(SRCDIR)/swext/mediawiki/dialogs/script.xlb))
$(eval $(call gb_Extension_add_file,wiki-publisher,WikiEditor/dialog.xlb,$(SRCDIR)/swext/mediawiki/dialogs/dialog.xlb))
$(eval $(call gb_Extension_add_file,wiki-publisher,WikiEditor/SendToMediaWiki.xdl,$(SRCDIR)/swext/mediawiki/dialogs/SendToMediaWiki.xdl))
$(eval $(call gb_Extension_add_file,wiki-publisher,filter/math/cmarkup.xsl,$(call gb_UnpackedTarball_get_dir,xsltml)/cmarkup.xsl))
$(eval $(call gb_Extension_add_file,wiki-publisher,filter/math/entities.xsl,$(call gb_UnpackedTarball_get_dir,xsltml)/entities.xsl))
$(eval $(call gb_Extension_add_file,wiki-publisher,filter/math/glayout.xsl,$(call gb_UnpackedTarball_get_dir,xsltml)/glayout.xsl))
$(eval $(call gb_Extension_add_file,wiki-publisher,filter/math/mmltex.xsl,$(call gb_UnpackedTarball_get_dir,xsltml)/mmltex.xsl))
$(eval $(call gb_Extension_add_file,wiki-publisher,filter/math/scripts.xsl,$(call gb_UnpackedTarball_get_dir,xsltml)/scripts.xsl))
$(eval $(call gb_Extension_add_file,wiki-publisher,filter/math/tables.xsl,$(call gb_UnpackedTarball_get_dir,xsltml)/tables.xsl))
$(eval $(call gb_Extension_add_file,wiki-publisher,filter/math/tokens.xsl,$(call gb_UnpackedTarball_get_dir,xsltml)/tokens.xsl))
$(eval $(call gb_Extension_add_file,wiki-publisher,filter/odt2mediawiki.xsl,$(SRCDIR)/swext/mediawiki/src/filter/odt2mediawiki.xsl))
$(eval $(call gb_Extension_add_file,wiki-publisher,license/THIRDPARTYLICENSEREADME.html,$(SRCDIR)/swext/mediawiki/src/THIRDPARTYLICENSEREADME.html))
$(eval $(call gb_Extension_add_file,wiki-publisher,templates/MediaWiki/mediawiki.ott,$(SRCDIR)/swext/mediawiki/src/filter/mediawiki.ott))
$(eval $(call gb_Extension_add_helpfile,wiki-publisher,$(SRCDIR)/swext/mediawiki/help,com.sun.wiki-publisher/wikisend.xhp,wikisend.xhp))
$(eval $(call gb_Extension_add_helpfile,wiki-publisher,$(SRCDIR)/swext/mediawiki/help,com.sun.wiki-publisher/wikiformats.xhp,wikiformats.xhp))
$(eval $(call gb_Extension_add_helpfile,wiki-publisher,$(SRCDIR)/swext/mediawiki/help,com.sun.wiki-publisher/wikiaccount.xhp,wikiaccount.xhp))
$(eval $(call gb_Extension_add_helpfile,wiki-publisher,$(SRCDIR)/swext/mediawiki/help,com.sun.wiki-publisher/wiki.xhp,wiki.xhp))
$(eval $(call gb_Extension_add_helpfile,wiki-publisher,$(SRCDIR)/swext/mediawiki/help,com.sun.wiki-publisher/wikisettings.xhp,wikisettings.xhp))

$(eval $(call gb_Extension_add_helptreefile,wiki-publisher,$(SRCDIR)/swext/mediawiki/help,help.tree,help.tree,com.sun.wiki-publisher))

# vim: set noet sw=4 ts=4:
