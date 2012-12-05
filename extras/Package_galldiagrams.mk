# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_galldiagrams,$(SRCDIR)/extras/source/gallery/diagrams))

$(eval $(call gb_Package_set_outdir,extras_galldiagrams,$(INSTDIR)))

$(eval $(call gb_Package_add_files,extras_galldiagrams,share/gallery/diagrams,\
	Component-Circle01-Transparent-DarkBlue.png \
	Component-Circle02-Transparent-Bule.png \
	Component-Circle03-Transparent-Green.png \
	Component-Circle04-Transparent-Orange.png \
	Component-Circle05-Transparent-Red.png \
	Component-Cube01-DarkBlue.png \
	Component-Cube02-LightBlue.png \
	Component-Cube03-Green.png \
	Component-Cube04-Orange.png \
	Component-Cube05-DarkRed.png \
	Component-Cuboid01-DarkBlue.png \
	Component-Cuboid02-Blue.png \
	Component-Cuboid03-Green.png \
	Component-Cuboid04-Orange.png \
	Component-Cuboid05-Red.png \
	Component-Gear01-DarkBlue.png \
	Component-Gear02-LightBlue.png \
	Component-Gear03-Green.png \
	Component-Gear04-DarkRed.png \
	Component-Gear05-Orange.png \
	Component-Person01-DarkBlue.png \
	Component-Person02-Blue.png \
	Component-Person03-Green.png \
	Component-Person04-DarkRed.png \
	Component-Person05-Orange.png \
	Component-PuzzlePiece01-DarkBlue.png \
	Component-PuzzlePiece02-Blue.png \
	Component-PuzzlePiece03-Green.png \
	Component-PuzzlePiece04-Red.png \
	Component-PuzzlePiece05-Orange.png \
	Component-Sphere01-DarkBlue.png \
	Component-Sphere02-LightBlue.png \
	Component-Sphere03-Green.png \
	Component-Sphere04-DarkRed.png \
	Component-Sphere05-Orange.png \
	Cycle01-Transparent.png \
	Cycle02-Transparent-Blue.png \
	Cycle03-Blue.png \
	Cycle04-Blue.png \
	Cycle05.png \
	Cycle06.png \
	Cycle07.png \
	Cycle08-Blue.png \
	Cycle09-Orange.png \
	Donut01-LightBlue.png \
	Donut02-Blue.png \
	Donut03-Blue.png \
	Donut04-DarkBlue.png \
	Donut05-DarkBlue.png \
	Donut06-Blue.png \
	Donut07-Blue.png \
	Graph.png \
	People01-Blue.png \
	People02.png \
	Pillars01-Orange.png \
	Pillars02-LightBlue.png \
	Process01-Blue.png \
	Process02-Blue.png \
	Process03-Blue.emf \
	Process04-GoUp-Blue.png \
	Process05-GoUp-Red.png \
	Process06-GoUp-Yellow.png \
	Process07-Blue.png \
	Pyramid01.png \
	Pyramid02-Blue.png \
	Pyramid03.emf \
	Radial01-Green.emf \
	Radial02-Green.emf \
	Radial03-Sphere.png \
	Radial04-Sphere-Red.png \
	Radial05-Sphere-Blue.png \
	Radial06-Arrows-DarkBlue.png \
	Radial07-Arrows-DarkBlue.png \
	Section-Circle.emf \
	Section-Cubes01.png \
	Section-Cubes02-Blue.png \
	Section-Cubes03-Orange.png \
	Section-Cubes04.png \
	Section-Cubes05.png \
	Section-Cuboids01-Blue.png \
	Section-Cuboids02-Blue.png \
	Section-Cuboids03.png \
	Section-Gears01.png \
	Section-Gears02-Blue.emf \
	Section-Gears03.png \
	Section-Hexagons01.png \
	Section-Hexagons02-Blue.png \
	Section-Hexagons03-Blue.png \
	Section-Hexagons04-Orange.emf \
	Section-Leaves01-LightBlue.png \
	Section-Leaves02-Green.png \
	Section-Pasters01.png \
	Section-Pasters02-Blue.emf \
	Section-Puzzle01.emf \
	Section-Puzzle02.png \
	Section-Puzzle03.png \
	Section-Rectangles.png \
	Section-Squares.png \
	Section-Triangle.emf \
	Target.png \
	Venn01.png \
	Venn02.png \
	Venn03.png \
	Venn04.png \
	Venn05.png \
	Venn06-Blue.png \
	Venn07-Blue.emf \
	Venn08.png \
))

# vim: set noet sw=4 ts=4: \
