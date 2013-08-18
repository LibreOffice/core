# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Gallery_Gallery,education,extras/source/gallery/education))

$(eval $(call gb_Gallery_add_files,education,$(LIBO_SHARE_FOLDER)/gallery/education,\
	extras/source/gallery/education/Blackboard.png \
	extras/source/gallery/education/Books.png \
	extras/source/gallery/education/Chalk.png \
	extras/source/gallery/education/Globe.png \
	extras/source/gallery/education/Glue.png \
	extras/source/gallery/education/GraduationCap.png \
	extras/source/gallery/education/Microscope.png \
	extras/source/gallery/education/Notebook.png \
	extras/source/gallery/education/PaperClip-Blue.png \
	extras/source/gallery/education/PaperClip-Red.png \
	extras/source/gallery/education/Pencil.png \
	extras/source/gallery/education/Ruler.png \
	extras/source/gallery/education/TestTubes.png \
))

# vim: set noet sw=4 ts=4:
