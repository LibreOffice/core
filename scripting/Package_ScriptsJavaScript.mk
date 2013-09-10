# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,scripting_ScriptsJavaScript,$(SRCDIR)/scripting/examples))

$(eval $(call gb_Package_set_outdir,scripting_ScriptsJavaScript,$(gb_INSTROOT)))

$(eval $(call gb_Package_add_files_with_dir,scripting_ScriptsJavaScript,$(LIBO_SHARE_FOLDER)/Scripts,\
	javascript/ExportSheetsToHTML/exportsheetstohtml.js \
	javascript/ExportSheetsToHTML/parcel-descriptor.xml \
	javascript/HelloWorld/helloworld.js \
	javascript/HelloWorld/parcel-descriptor.xml \
	javascript/Highlight/ButtonPressHandler.js \
	javascript/Highlight/parcel-descriptor.xml \
	javascript/Highlight/ShowDialog.js \
))

# vim: set noet sw=4 ts=4:
