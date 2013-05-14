# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Gallery_Gallery,diagrams,extras/source/gallery/diagrams))

$(eval $(call gb_Gallery_add_files,diagrams,share/gallery/diagrams,\
	extras/source/gallery/diagrams/Component-Circle01-Transparent-DarkBlue.png \
	extras/source/gallery/diagrams/Component-Circle02-Transparent-Bule.png \
	extras/source/gallery/diagrams/Component-Circle03-Transparent-Green.png \
	extras/source/gallery/diagrams/Component-Circle04-Transparent-Orange.png \
	extras/source/gallery/diagrams/Component-Circle05-Transparent-Red.png \
	extras/source/gallery/diagrams/Component-Cube01-DarkBlue.png \
	extras/source/gallery/diagrams/Component-Cube02-LightBlue.png \
	extras/source/gallery/diagrams/Component-Cube03-Green.png \
	extras/source/gallery/diagrams/Component-Cube04-Orange.png \
	extras/source/gallery/diagrams/Component-Cube05-DarkRed.png \
	extras/source/gallery/diagrams/Component-Cuboid01-DarkBlue.png \
	extras/source/gallery/diagrams/Component-Cuboid02-Blue.png \
	extras/source/gallery/diagrams/Component-Cuboid03-Green.png \
	extras/source/gallery/diagrams/Component-Cuboid04-Orange.png \
	extras/source/gallery/diagrams/Component-Cuboid05-Red.png \
	extras/source/gallery/diagrams/Component-Gear01-DarkBlue.png \
	extras/source/gallery/diagrams/Component-Gear02-LightBlue.png \
	extras/source/gallery/diagrams/Component-Gear03-Green.png \
	extras/source/gallery/diagrams/Component-Gear04-DarkRed.png \
	extras/source/gallery/diagrams/Component-Gear05-Orange.png \
	extras/source/gallery/diagrams/Component-Person01-DarkBlue.png \
	extras/source/gallery/diagrams/Component-Person02-Blue.png \
	extras/source/gallery/diagrams/Component-Person03-Green.png \
	extras/source/gallery/diagrams/Component-Person04-DarkRed.png \
	extras/source/gallery/diagrams/Component-Person05-Orange.png \
	extras/source/gallery/diagrams/Component-PuzzlePiece01-DarkBlue.png \
	extras/source/gallery/diagrams/Component-PuzzlePiece02-Blue.png \
	extras/source/gallery/diagrams/Component-PuzzlePiece03-Green.png \
	extras/source/gallery/diagrams/Component-PuzzlePiece04-Red.png \
	extras/source/gallery/diagrams/Component-PuzzlePiece05-Orange.png \
	extras/source/gallery/diagrams/Component-Sphere01-DarkBlue.png \
	extras/source/gallery/diagrams/Component-Sphere02-LightBlue.png \
	extras/source/gallery/diagrams/Component-Sphere03-Green.png \
	extras/source/gallery/diagrams/Component-Sphere04-DarkRed.png \
	extras/source/gallery/diagrams/Component-Sphere05-Orange.png \
	extras/source/gallery/diagrams/Cycle01-Transparent.png \
	extras/source/gallery/diagrams/Cycle02-Transparent-Blue.png \
	extras/source/gallery/diagrams/Cycle03-Blue.png \
	extras/source/gallery/diagrams/Cycle04-Blue.png \
	extras/source/gallery/diagrams/Cycle05.png \
	extras/source/gallery/diagrams/Cycle06.png \
	extras/source/gallery/diagrams/Cycle07.png \
	extras/source/gallery/diagrams/Cycle08-Blue.png \
	extras/source/gallery/diagrams/Cycle09-Orange.png \
	extras/source/gallery/diagrams/Donut01-LightBlue.png \
	extras/source/gallery/diagrams/Donut02-Blue.png \
	extras/source/gallery/diagrams/Donut03-Blue.png \
	extras/source/gallery/diagrams/Donut04-DarkBlue.png \
	extras/source/gallery/diagrams/Donut05-DarkBlue.png \
	extras/source/gallery/diagrams/Donut06-Blue.png \
	extras/source/gallery/diagrams/Donut07-Blue.png \
	extras/source/gallery/diagrams/Graph.png \
	extras/source/gallery/diagrams/People01-Blue.png \
	extras/source/gallery/diagrams/People02.png \
	extras/source/gallery/diagrams/Pillars01-Orange.png \
	extras/source/gallery/diagrams/Pillars02-LightBlue.png \
	extras/source/gallery/diagrams/Process01-Blue.png \
	extras/source/gallery/diagrams/Process02-Blue.png \
	extras/source/gallery/diagrams/Process03-Blue.emf \
	extras/source/gallery/diagrams/Process04-GoUp-Blue.png \
	extras/source/gallery/diagrams/Process05-GoUp-Red.png \
	extras/source/gallery/diagrams/Process06-GoUp-Yellow.png \
	extras/source/gallery/diagrams/Process07-Blue.png \
	extras/source/gallery/diagrams/Pyramid01.png \
	extras/source/gallery/diagrams/Pyramid02-Blue.png \
	extras/source/gallery/diagrams/Pyramid03.emf \
	extras/source/gallery/diagrams/Radial01-Green.emf \
	extras/source/gallery/diagrams/Radial02-Green.emf \
	extras/source/gallery/diagrams/Radial03-Sphere.png \
	extras/source/gallery/diagrams/Radial04-Sphere-Red.png \
	extras/source/gallery/diagrams/Radial05-Sphere-Blue.png \
	extras/source/gallery/diagrams/Radial06-Arrows-DarkBlue.png \
	extras/source/gallery/diagrams/Radial07-Arrows-DarkBlue.png \
	extras/source/gallery/diagrams/Section-Circle.emf \
	extras/source/gallery/diagrams/Section-Cubes01.png \
	extras/source/gallery/diagrams/Section-Cubes02-Blue.png \
	extras/source/gallery/diagrams/Section-Cubes03-Orange.png \
	extras/source/gallery/diagrams/Section-Cubes04.png \
	extras/source/gallery/diagrams/Section-Cubes05.png \
	extras/source/gallery/diagrams/Section-Cuboids01-Blue.png \
	extras/source/gallery/diagrams/Section-Cuboids02-Blue.png \
	extras/source/gallery/diagrams/Section-Cuboids03.png \
	extras/source/gallery/diagrams/Section-Gears01.png \
	extras/source/gallery/diagrams/Section-Gears02-Blue.emf \
	extras/source/gallery/diagrams/Section-Gears03.png \
	extras/source/gallery/diagrams/Section-Hexagons01.png \
	extras/source/gallery/diagrams/Section-Hexagons02-Blue.png \
	extras/source/gallery/diagrams/Section-Hexagons03-Blue.png \
	extras/source/gallery/diagrams/Section-Hexagons04-Orange.emf \
	extras/source/gallery/diagrams/Section-Leaves01-LightBlue.png \
	extras/source/gallery/diagrams/Section-Leaves02-Green.png \
	extras/source/gallery/diagrams/Section-Pasters01.png \
	extras/source/gallery/diagrams/Section-Pasters02-Blue.emf \
	extras/source/gallery/diagrams/Section-Puzzle01.emf \
	extras/source/gallery/diagrams/Section-Puzzle02.png \
	extras/source/gallery/diagrams/Section-Puzzle03.png \
	extras/source/gallery/diagrams/Section-Rectangles.png \
	extras/source/gallery/diagrams/Section-Squares.png \
	extras/source/gallery/diagrams/Section-Triangle.emf \
	extras/source/gallery/diagrams/Target.png \
	extras/source/gallery/diagrams/Venn01.png \
	extras/source/gallery/diagrams/Venn02.png \
	extras/source/gallery/diagrams/Venn03.png \
	extras/source/gallery/diagrams/Venn04.png \
	extras/source/gallery/diagrams/Venn05.png \
	extras/source/gallery/diagrams/Venn06-Blue.png \
	extras/source/gallery/diagrams/Venn07-Blue.emf \
	extras/source/gallery/diagrams/Venn08.png \
))

# vim: set noet sw=4 ts=4: \
