# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Gallery_Gallery,diagrams,extras/source/gallery/diagrams))

$(eval $(call gb_Gallery_add_files,diagrams,$(LIBO_SHARE_FOLDER)/gallery/diagrams,\
	extras/source/gallery/diagrams/Component-Circle01-Transparent-DarkBlue.svg \
	extras/source/gallery/diagrams/Component-Circle02-Transparent-Bule.svg \
	extras/source/gallery/diagrams/Component-Circle03-Transparent-Green.svg \
	extras/source/gallery/diagrams/Component-Circle04-Transparent-Orange.svg \
	extras/source/gallery/diagrams/Component-Circle05-Transparent-Red.svg \
	extras/source/gallery/diagrams/Component-Cube01-DarkBlue.svg \
	extras/source/gallery/diagrams/Component-Cube02-LightBlue.svg \
	extras/source/gallery/diagrams/Component-Cube03-Green.svg \
	extras/source/gallery/diagrams/Component-Cube04-Orange.svg \
	extras/source/gallery/diagrams/Component-Cube05-DarkRed.svg \
	extras/source/gallery/diagrams/Component-Cuboid01-DarkBlue.svg \
	extras/source/gallery/diagrams/Component-Cuboid02-Blue.svg \
	extras/source/gallery/diagrams/Component-Cuboid03-Green.svg \
	extras/source/gallery/diagrams/Component-Cuboid04-Orange.svg \
	extras/source/gallery/diagrams/Component-Cuboid05-Red.svg \
	extras/source/gallery/diagrams/Component-Gear01-DarkBlue.svg \
	extras/source/gallery/diagrams/Component-Gear02-LightBlue.svg \
	extras/source/gallery/diagrams/Component-Gear03-Green.svg \
	extras/source/gallery/diagrams/Component-Gear04-DarkRed.svg \
	extras/source/gallery/diagrams/Component-Gear05-Orange.svg \
	extras/source/gallery/diagrams/Component-Person01-DarkBlue.svg \
	extras/source/gallery/diagrams/Component-Person02-Blue.svg \
	extras/source/gallery/diagrams/Component-Person03-Green.svg \
	extras/source/gallery/diagrams/Component-Person04-DarkRed.svg \
	extras/source/gallery/diagrams/Component-Person05-Orange.svg \
	extras/source/gallery/diagrams/Component-PuzzlePiece01-DarkBlue.svg \
	extras/source/gallery/diagrams/Component-PuzzlePiece02-Blue.svg \
	extras/source/gallery/diagrams/Component-PuzzlePiece03-Green.svg \
	extras/source/gallery/diagrams/Component-PuzzlePiece04-Red.svg \
	extras/source/gallery/diagrams/Component-PuzzlePiece05-Orange.svg \
	extras/source/gallery/diagrams/Component-Sphere01-DarkBlue.svg \
	extras/source/gallery/diagrams/Component-Sphere02-LightBlue.svg \
	extras/source/gallery/diagrams/Component-Sphere03-Green.svg \
	extras/source/gallery/diagrams/Component-Sphere04-DarkRed.svg \
	extras/source/gallery/diagrams/Component-Sphere05-Orange.svg \
	extras/source/gallery/diagrams/Cycle01-Transparent.svg \
	extras/source/gallery/diagrams/Cycle02-Transparent-Blue.svg \
	extras/source/gallery/diagrams/Cycle03-Blue.svg \
	extras/source/gallery/diagrams/Cycle04-Blue.svg \
	extras/source/gallery/diagrams/Cycle05.svg \
	extras/source/gallery/diagrams/Cycle06.svg \
	extras/source/gallery/diagrams/Cycle07.svg \
	extras/source/gallery/diagrams/Cycle08-Blue.svg \
	extras/source/gallery/diagrams/Cycle09-Orange.svg \
	extras/source/gallery/diagrams/Donut01-LightBlue.svg \
	extras/source/gallery/diagrams/Donut02-Blue.svg \
	extras/source/gallery/diagrams/Donut03-Blue.svg \
	extras/source/gallery/diagrams/Donut04-DarkBlue.svg \
	extras/source/gallery/diagrams/Donut05-DarkBlue.svg \
	extras/source/gallery/diagrams/Donut06-Blue.svg \
	extras/source/gallery/diagrams/Donut07-Blue.svg \
	extras/source/gallery/diagrams/Graph.svg \
	extras/source/gallery/diagrams/People01-Blue.svg \
	extras/source/gallery/diagrams/People02.svg \
	extras/source/gallery/diagrams/Pillars01-Orange.svg \
	extras/source/gallery/diagrams/Pillars02-LightBlue.svg \
	extras/source/gallery/diagrams/Process01-Blue.svg \
	extras/source/gallery/diagrams/Process02-Blue.svg \
	extras/source/gallery/diagrams/Process03-Blue.svg \
	extras/source/gallery/diagrams/Process04-GoUp-Blue.svg \
	extras/source/gallery/diagrams/Process05-GoUp-Red.svg \
	extras/source/gallery/diagrams/Process06-GoUp-Yellow.svg \
	extras/source/gallery/diagrams/Process07-Blue.svg \
	extras/source/gallery/diagrams/Pyramid01.svg \
	extras/source/gallery/diagrams/Pyramid02-Blue.svg \
	extras/source/gallery/diagrams/Pyramid03.svg \
	extras/source/gallery/diagrams/Radial01-Green.svg \
	extras/source/gallery/diagrams/Radial02-Green.svg \
	extras/source/gallery/diagrams/Radial03-Sphere.svg \
	extras/source/gallery/diagrams/Radial04-Sphere-Red.svg \
	extras/source/gallery/diagrams/Radial05-Sphere-Blue.svg \
	extras/source/gallery/diagrams/Radial06-Arrows-DarkBlue.svg \
	extras/source/gallery/diagrams/Radial07-Arrows-DarkBlue.svg \
	extras/source/gallery/diagrams/Section-Circle.svg \
	extras/source/gallery/diagrams/Section-Cubes01.svg \
	extras/source/gallery/diagrams/Section-Cubes02-Blue.svg \
	extras/source/gallery/diagrams/Section-Cubes03-Orange.svg \
	extras/source/gallery/diagrams/Section-Cubes04.svg \
	extras/source/gallery/diagrams/Section-Cubes05.svg \
	extras/source/gallery/diagrams/Section-Cuboids01-Blue.svg \
	extras/source/gallery/diagrams/Section-Cuboids02-Blue.svg \
	extras/source/gallery/diagrams/Section-Cuboids03.svg \
	extras/source/gallery/diagrams/Section-Gears01.svg \
	extras/source/gallery/diagrams/Section-Gears02-Blue.svg \
	extras/source/gallery/diagrams/Section-Gears03-Blue.svg \
	extras/source/gallery/diagrams/Section-Hexagons01.svg \
	extras/source/gallery/diagrams/Section-Hexagons02-Blue.svg \
	extras/source/gallery/diagrams/Section-Hexagons03-Blue.svg \
	extras/source/gallery/diagrams/Section-Hexagons04-Orange.svg \
	extras/source/gallery/diagrams/Section-Leaves01-LightBlue.svg \
	extras/source/gallery/diagrams/Section-Leaves02-Green.svg \
	extras/source/gallery/diagrams/Section-Pasters01.svg \
	extras/source/gallery/diagrams/Section-Pasters02-Blue.emf \
	extras/source/gallery/diagrams/Section-Puzzle01.svg \
	extras/source/gallery/diagrams/Section-Puzzle02.svg \
	extras/source/gallery/diagrams/Section-Puzzle03.svg \
	extras/source/gallery/diagrams/Section-Rectangles.svg \
	extras/source/gallery/diagrams/Section-Squares.svg \
	extras/source/gallery/diagrams/Section-Triangle.svg \
	extras/source/gallery/diagrams/Target.svg \
	extras/source/gallery/diagrams/Venn01.svg \
	extras/source/gallery/diagrams/Venn02.svg \
	extras/source/gallery/diagrams/Venn03.svg \
	extras/source/gallery/diagrams/Venn04.svg \
	extras/source/gallery/diagrams/Venn05.svg \
	extras/source/gallery/diagrams/Venn06-Blue.svg \
	extras/source/gallery/diagrams/Venn07-Blue.svg \
	extras/source/gallery/diagrams/Venn08.svg \
))

# vim: set noet sw=4 ts=4: \
