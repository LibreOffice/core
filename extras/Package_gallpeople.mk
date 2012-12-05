# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_gallpeople,$(SRCDIR)/extras/source/gallery/people))

$(eval $(call gb_Package_set_outdir,extras_gallpeople,$(INSTDIR)))

$(eval $(call gb_Package_add_files,extras_gallpeople,share/gallery/people,\
	Artist-Female1.png \
	Artist-Female2.png \
	Artist-Male1.png \
	Artist-Male2.png \
	BusinessPerson-Female1.png \
	BusinessPerson-Female2.png \
	BusinessPerson-Female3.png \
	BusinessPerson-Female4.png \
	BusinessPerson-Female5.png \
	BusinessPerson-Female6.png \
	BusinessPerson-HeadSet1.png \
	BusinessPerson-HeadSet2.png \
	BusinessPerson-Male1.png \
	BusinessPerson-Male2.png \
	BusinessPerson-Male3.png \
	BusinessPerson-Male4.png \
	BusinessPerson-Male5.png \
	BusinessPerson-Male6.png \
	Chef1.png \
	Chef2.png \
	Computer-User-Female1.png \
	Computer-User-Female2.png \
	Computer-User-Male1.png \
	Computer-User-Male2.png \
	ConstructionWorker2.png \
	ConstructionWorker.png \
	Detective1.png \
	Detective2.png \
	Doctor-Female1.png \
	Doctor-Female2.png \
	Doctor-Male1.png \
	Doctor-Male2.png \
	Nurse1.png \
	Nurse2.png \
	PoliceOfficer1.png \
	PoliceOfficer2.png \
	Presenter-Female1.png \
	Presenter-Female2.png \
	Presenter-Male1.png \
	Presenter-Male2.png \
	Student-Female.png \
	Student-Male.png \
	Surgeon1.png \
	Surgeon2.png \
	Surgeon-Female1.png \
	Surgeon-Female2.png \
	Teacher1.png \
	Teacher2.png \
	Tourist-Female1.png \
	Tourist-Female2.png \
	Tourist-Male1.png \
	Tourist-Male2.png \
))

# vim: set noet sw=4 ts=4:
