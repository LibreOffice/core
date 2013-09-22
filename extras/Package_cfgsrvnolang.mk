# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_cfgsrvnolang,$(SRCDIR)/extras/source/misc_config))

$(eval $(call gb_Package_set_outdir,extras_cfgsrvnolang,$(INSTROOT)))

$(eval $(call gb_Package_add_files_with_dir,extras_cfgsrvnolang,$(LIBO_SHARE_FOLDER)/config,\
	wizard/form/styles/beige.css \
	wizard/form/styles/bgr.css \
	wizard/form/styles/dark.css \
	wizard/form/styles/grey.css \
	wizard/form/styles/ibg.css \
	wizard/form/styles/ice.css \
	wizard/form/styles/orange.css \
	wizard/form/styles/red.css \
	wizard/form/styles/violet.css \
	wizard/form/styles/water.css \
	wizard/web/images/calc.gif \
	wizard/web/images/draw.gif \
	wizard/web/images/fls.gif \
	wizard/web/images/graphics.gif \
	wizard/web/images/html.gif \
	wizard/web/images/impress.gif \
	wizard/web/images/media.gif \
	wizard/web/images/other.gif \
	wizard/web/images/pdf.gif \
	wizard/web/images/writer.gif \
	wizard/web/layouts/diagonal/index.html.xsl \
	wizard/web/layouts/frame_bottom/index.html.xsl \
	wizard/web/layouts/frame_bottom/mainframe.html \
	wizard/web/layouts/frame_bottom/tocframe.html.xsl \
	wizard/web/layouts/frame_left/index.html.xsl \
	wizard/web/layouts/frame_left/mainframe.html \
	wizard/web/layouts/frame_left/tocframe.html.xsl \
	wizard/web/layouts/frame_right/index.html.xsl \
	wizard/web/layouts/frame_right/mainframe.html \
	wizard/web/layouts/frame_right/tocframe.html.xsl \
	wizard/web/layouts/frame_top/index.html.xsl \
	wizard/web/layouts/frame_top/mainframe.html \
	wizard/web/layouts/frame_top/tocframe.html.xsl \
	wizard/web/layouts/layoutF.xsl \
	wizard/web/layouts/layout.xsl \
	wizard/web/layouts/layoutX.xsl \
	wizard/web/layouts/simple/index.html.xsl \
	wizard/web/layouts/source.xml.xsl \
	wizard/web/layouts/table_2/index.html.xsl \
	wizard/web/layouts/table_3/index.html.xsl \
	wizard/web/layouts/zigzag/index.html.xsl \
	wizard/web/preview.html \
	wizard/web/styles/beige.css \
	wizard/web/styles/bg.css \
	wizard/web/styles/bgr.css \
	wizard/web/styles/bgrey.css \
	wizard/web/styles/bwb.css \
	wizard/web/styles/bwo.css \
	wizard/web/styles/dark.css \
	wizard/web/styles/dp.css \
	wizard/web/styles/forest.css \
	wizard/web/styles/green.css \
	wizard/web/styles/greenred.css \
	wizard/web/styles/grey.css \
	wizard/web/styles/ibg.css \
	wizard/web/styles/ice.css \
	wizard/web/styles/marine.css \
	wizard/web/styles/orange.css \
	wizard/web/styles/pc_old.css \
	wizard/web/styles/red.css \
	wizard/web/styles/rgb.css \
	wizard/web/styles/strange.css \
	wizard/web/styles/violet.css \
	wizard/web/styles/water.css \
))

# vim: set noet sw=4 ts=4:
