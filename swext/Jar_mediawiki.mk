#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#

$(eval $(call gb_Jar_Jar,mediawiki))

$(eval $(call gb_Jar_set_packageroot,mediawiki,com))

$(eval $(call gb_Jar_set_manifest,mediawiki,$(SRCDIR)/swext/mediawiki/src/com/sun/star/wiki/MANIFEST.MF))

$(eval $(call gb_Jar_use_jars,mediawiki,\
	juh \
	jurt \
	ridl \
	unoil \
))

$(eval $(call gb_Jar_add_sourcefiles,mediawiki,\
	swext/mediawiki/src/com/sun/star/wiki/EditPageParser \
	swext/mediawiki/src/com/sun/star/wiki/Helper \
	swext/mediawiki/src/com/sun/star/wiki/MainThreadDialogExecutor \
	swext/mediawiki/src/com/sun/star/wiki/Settings \
	swext/mediawiki/src/com/sun/star/wiki/WikiArticle \
	swext/mediawiki/src/com/sun/star/wiki/WikiCancelException \
	swext/mediawiki/src/com/sun/star/wiki/WikiDialog \
	swext/mediawiki/src/com/sun/star/wiki/WikiEditorImpl \
	swext/mediawiki/src/com/sun/star/wiki/WikiEditSettingDialog \
	swext/mediawiki/src/com/sun/star/wiki/WikiOptionsEventHandlerImpl \
	swext/mediawiki/src/com/sun/star/wiki/WikiPropDialog \
	swext/mediawiki/src/com/sun/star/wiki/WikiProtocolSocketFactory \
))
