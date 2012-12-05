# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_gallarrows,$(SRCDIR)/extras/source/gallery/arrows))

$(eval $(call gb_Package_set_outdir,extras_gallarrows,$(INSTDIR)))

$(eval $(call gb_Package_add_files,extras_gallarrows,share/gallery/arrows,\
	A01-Arrow-Gray-Left.png \
	A02-Arrow-DarkBlue-Right.png \
	A03-Arrow-Gray-Left.png \
	A04-Arrow-DarkRed-Right.png \
	A05-Arrow-Blue-Left.png \
	A06-Arrow-Red-Right.png \
	A07-Arrow-LightBlue-Left.png \
	A08-Arrow-DarkRed-Right.png \
	A09-Arrow-Gray-Left.png \
	A10-Arrow-Purple-Right.png \
	A11-Arrow-Gray-Left.png \
	A12-Arrow-LightBlue-Right.png \
	A13-Arrow-Gray-Right.png \
	A14-CircleArrow-Green.png \
	A15-CircleArrow.png \
	A16-CircleArrow.png \
	A17-CircleArrow.png \
	A18-CircleArrow.png \
	A19-CircleArrow.png \
	A20-CircleArrow-LightBlue.png \
	A21-CircleArrow-Green.png \
	A22-CircleArrow.png \
	A23-CurvedArrow-Gray-Left.png \
	A24-CurvedArrow-LightBlue-Right.png \
	A25-CurvedArrow-DarkBlue.png \
	A26-CurvedArrow-Orange.png \
	A27-CurvedArrow-DarkRed.png \
	A28-CurvedArrow-DarkBlue.png \
	A29-CurvedArrow-Green.png \
	A30-CurvedArrow-Gray.png \
	A31-CurvedArrow-LightBlue.png \
	A32-CurvedArrow-Orange.png \
	A33-CurvedArrow-LightBlue-TwoDirections.png \
	A34-CurvedArrow-Green-TwoDirections.png \
	A35-CurvedArrow-Brown-Left.png \
	A36-CurvedArrow-LightBlue-Up.png \
	A37-CurvedArrow-Gray-TwoDirections.png \
	A38-CurvedArrow-Gray-TwoDirections.png \
	A39-CurvedArrow-Gray-Left.png \
	A40-CurvedArrow-Gray-Up.png \
	A41-CurvedArrow-Gray-Left.png \
	A42-TrendArrow-Red-GoUp.png \
	A43-TrendArrow-Green-GoDown.png \
	A44-TrendArrow-Orange-GoUp.png \
	A45-TrendArrow-Red-GoUp.png \
	A46-TrendArrow-Orange-GoUp.png \
	A47-TrendArrow-LightBlue.png \
	A48-TrendArrow-Orange-TwoDirections.png \
	A49-TrendArrow-Yellow-ThreeDirections.png \
	A50-TrendArrow-LightBlue-FourDirections.png \
	A51-TrendArrow-Blue-FourDirections.png \
	A52-TrendArrow-Blue-FourDirections.png \
	A53-TrendArrow-LightBlue-TwoDirections.png \
	A54-TrendArrow-Red-TwoDirections.png \
	A55-TrendArrow-TwoDirections.png \
	A56-TrendArrow-Blue-TwoDirections.png \
	A57-Arrow-Yellow-Left.png \
	A58-Arrow-Red-Right.png \
	A59-CurvedArrow-Gray-Left.png \
	A60-CurvedArrow-Purple-Right.png \
	A61-Arrow-StripedOrange-Left.png \
	A62-Arrow-StripedBlue-Right.png \
	A63-Arrow-LightBlue-Left.png \
	A64-Arrow-Green-Right.png \
	A65-Arrow-DarkBlue-Up.png \
	A66-Arrow-Green-Down.png \
	A67-Arrow-Yellow-Left.png \
	A68-Arrow-Gray-Right.png \
))

# vim: set noet sw=4 ts=4:
