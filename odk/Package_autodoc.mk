# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,odk_autodoc,$(call gb_CustomTarget_get_workdir,odk/docs/common/ref)))

$(eval $(call gb_Package_set_outdir,odk_autodoc,$(INSTDIR)))

# indices
$(eval $(call gb_Package_add_files_with_dir,odk_autodoc,$(gb_Package_SDKDIRNAME)/docs/common/ref,\
	index-files/index-1.html \
	index-files/index-10.html \
	index-files/index-11.html \
	index-files/index-12.html \
	index-files/index-13.html \
	index-files/index-14.html \
	index-files/index-15.html \
	index-files/index-16.html \
	index-files/index-17.html \
	index-files/index-18.html \
	index-files/index-19.html \
	index-files/index-2.html \
	index-files/index-20.html \
	index-files/index-21.html \
	index-files/index-22.html \
	index-files/index-23.html \
	index-files/index-24.html \
	index-files/index-25.html \
	index-files/index-26.html \
	index-files/index-27.html \
	index-files/index-3.html \
	index-files/index-4.html \
	index-files/index-5.html \
	index-files/index-6.html \
	index-files/index-7.html \
	index-files/index-8.html \
	index-files/index-9.html \
))

# module lists
$(eval $(call gb_Package_add_files_with_dir,odk_autodoc,$(gb_Package_SDKDIRNAME)/docs/common/ref,\
	module-ix.html \
	$(addsuffix /module-ix.html,\
		com \
		com/sun \
		com/sun/star \
		com/sun/star/accessibility \
		com/sun/star/animations \
		com/sun/star/auth \
		com/sun/star/awt \
		com/sun/star/awt/grid \
		com/sun/star/awt/tab \
		com/sun/star/awt/tree \
		com/sun/star/beans \
		com/sun/star/bridge \
		com/sun/star/bridge/oleautomation \
		com/sun/star/chart \
		com/sun/star/chart2 \
		com/sun/star/chart2/data \
		com/sun/star/configuration \
		com/sun/star/configuration/backend \
		com/sun/star/configuration/backend/xml \
		com/sun/star/configuration/bootstrap \
		com/sun/star/connection \
		com/sun/star/container \
		com/sun/star/cui \
		com/sun/star/datatransfer \
		com/sun/star/datatransfer/clipboard \
		com/sun/star/datatransfer/dnd \
		com/sun/star/deployment \
		com/sun/star/deployment/test \
		com/sun/star/deployment/ui \
		com/sun/star/document \
		com/sun/star/drawing \
		com/sun/star/drawing/framework \
		com/sun/star/embed \
		com/sun/star/form \
		com/sun/star/form/binding \
		com/sun/star/form/component \
		com/sun/star/form/control \
		com/sun/star/form/inspection \
		com/sun/star/form/runtime \
		com/sun/star/form/submission \
		com/sun/star/form/validation \
		com/sun/star/formula \
		com/sun/star/frame \
		com/sun/star/frame/status \
		com/sun/star/gallery \
		com/sun/star/geometry \
		com/sun/star/graphic \
		com/sun/star/i18n \
		com/sun/star/image \
		com/sun/star/inspection \
		com/sun/star/io \
		com/sun/star/java \
		com/sun/star/lang \
		com/sun/star/ldap \
		com/sun/star/linguistic2 \
		com/sun/star/loader \
		com/sun/star/logging \
		com/sun/star/mail \
		com/sun/star/media \
		com/sun/star/mozilla \
		com/sun/star/office \
		com/sun/star/oox \
		com/sun/star/packages \
		com/sun/star/packages/manifest \
		com/sun/star/packages/zip \
		com/sun/star/plugin \
		com/sun/star/presentation \
		com/sun/star/presentation/textfield \
		com/sun/star/qa \
		com/sun/star/rdf \
		com/sun/star/reflection \
		com/sun/star/registry \
		com/sun/star/rendering \
		com/sun/star/report \
		com/sun/star/report/inspection \
		com/sun/star/report/meta \
		com/sun/star/resource \
		com/sun/star/scanner \
		com/sun/star/script \
		com/sun/star/script/browse \
		com/sun/star/script/provider \
		com/sun/star/script/vba \
		com/sun/star/sdb \
		com/sun/star/sdb/application \
		com/sun/star/sdb/tools \
		com/sun/star/sdbc \
		com/sun/star/sdbcx \
		com/sun/star/security \
		com/sun/star/setup \
		com/sun/star/sheet \
		com/sun/star/smarttags \
		com/sun/star/style \
		com/sun/star/svg \
		com/sun/star/system \
		com/sun/star/table \
		com/sun/star/task \
		com/sun/star/text \
		com/sun/star/text/fieldmaster \
		com/sun/star/text/textfield \
		com/sun/star/text/textfield/docinfo \
		com/sun/star/ucb \
		com/sun/star/ui \
		com/sun/star/ui/dialogs \
		com/sun/star/uno \
		com/sun/star/uri \
		com/sun/star/util \
		com/sun/star/view \
		com/sun/star/xforms \
		com/sun/star/xml \
		com/sun/star/xml/crypto \
		com/sun/star/xml/crypto/sax \
		com/sun/star/xml/csax \
		com/sun/star/xml/dom \
		com/sun/star/xml/dom/events \
		com/sun/star/xml/dom/views \
		com/sun/star/xml/input \
		com/sun/star/xml/sax \
		com/sun/star/xml/wrapper \
		com/sun/star/xml/xpath \
		com/sun/star/xml/xslt \
		com/sun/star/xsd \
		org \
		org/freedesktop \
		org/freedesktop/PackageKit \
	) \
))

# vim: set noet sw=4 ts=4:
