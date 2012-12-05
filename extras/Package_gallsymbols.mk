# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_gallsymbols,$(SRCDIR)/extras/source/gallery/symbols))

$(eval $(call gb_Package_set_outdir,extras_gallsymbols,$(INSTDIR)))

$(eval $(call gb_Package_add_files,extras_gallsymbols,share/gallery/symbols,\
	Book.png \
	Box01.png \
	Box02.png \
	Bulb01-Yellow.png \
	Bulb02-Yellow.png \
	Calendar.png \
	Chart.png \
	Clipboard.png \
	Clock.png \
	Compass.png \
	Emotion01-Laughing.png \
	Emotion02-Smiling.png \
	Emotion03-Calm.png \
	Emotion04-Frowning.png \
	Emotion05-Angry.png \
	Emotion06-Crying.png \
	Flag01-Red.png \
	Flag02-Green.png \
	Flag03-Blue.png \
	Gift.png \
	House.png \
	Icon-Computer01-White.png \
	Icon-Computer02-Black.png \
	Icon-Disk01-Blue.png \
	Icon-Disk02-Green.png \
	Icon-Document01-Grey.png \
	Icon-Document02-Grey.png \
	Icon-Document03-Blue.png \
	Icon-Document04-Blue.png \
	Icon-Envelope01-Blue.png \
	Icon-Envelope02-Yellow.png \
	Icon-Envelope03-Open-Yellow.png \
	Icon-Envelope04-Open-Yellow.png \
	Icon-Folder01-Yellow.png \
	Icon-Folder02-Yellow.png \
	Icon-Folder03-Open-Yellow.png \
	Icon-Folder04-Open-Yellow.png \
	Icon-Gear01-Grey.png \
	Icon-Gear02-Blue.png \
	Icon-Network01-Blue.png \
	Icon-Network02.png \
	Icon-Pencil01.png \
	Icon-Pencil02.png \
	Icon-Printer01-White.png \
	Icon-Printer02-Black.png \
	Key01.png \
	Key02.png \
	Lock01-Yellow.png \
	Lock02-Yellow.png \
	Lock03-Blue.png \
	Lock04-Blue.png \
	Magnet.png \
	MagnifyingGlass.png \
	Medal.png \
	Notebook.png \
	Phone.png \
	PieChart.png \
	Pin.png \
	PuzzlePiece.png \
	PuzzlePieces.png \
	Roadblock.png \
	Scissors.png \
	Shield01.png \
	Shield02-Orange.png \
	Shield03-Blue.png \
	Sign-Ban01.png \
	Sign-Ban02.png \
	Sign-CheckBox01.png \
	Sign-CheckBox02-Unchecked.png \
	Sign-Checkmark01-Green.png \
	Sign-Checkmark02-Green.png \
	Sign-DoNotEnter.png \
	Sign-Error01.png \
	Sign-Error02.png \
	Sign-ExclamationPoint01-Red.png \
	Sign-ExclamationPoint02-Orange.png \
	Sign-Help01-Green.png \
	Sign-Help02-Blue.png \
	Sign-Information.png \
	Sign-Null.png \
	Sign-QuestionMark01-Blue.png \
	Sign-QuestionMark02-Red.png \
	Sign-RadioButton01.png \
	Sign-RadioButton02-Unchecked.png \
	Sign-Warning01-Red.png \
	Sign-Warning02-Orange.png \
	Sign-X01-Red.png \
	Sign-X02-Red.png \
	Star-Yellow.png \
	Wrench.png \
))

# vim: set noet sw=4 ts=4:
