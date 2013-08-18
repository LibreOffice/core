# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Gallery_Gallery,people,extras/source/gallery/people))

$(eval $(call gb_Gallery_add_files,people,$(LIBO_SHARE_FOLDER)/gallery/people,\
	extras/source/gallery/people/Artist-Female1.png \
	extras/source/gallery/people/Artist-Female2.png \
	extras/source/gallery/people/Artist-Male1.png \
	extras/source/gallery/people/Artist-Male2.png \
	extras/source/gallery/people/BusinessPerson-Female1.png \
	extras/source/gallery/people/BusinessPerson-Female2.png \
	extras/source/gallery/people/BusinessPerson-Female3.png \
	extras/source/gallery/people/BusinessPerson-Female4.png \
	extras/source/gallery/people/BusinessPerson-Female5.png \
	extras/source/gallery/people/BusinessPerson-Female6.png \
	extras/source/gallery/people/BusinessPerson-HeadSet1.png \
	extras/source/gallery/people/BusinessPerson-HeadSet2.png \
	extras/source/gallery/people/BusinessPerson-Male1.png \
	extras/source/gallery/people/BusinessPerson-Male2.png \
	extras/source/gallery/people/BusinessPerson-Male3.png \
	extras/source/gallery/people/BusinessPerson-Male4.png \
	extras/source/gallery/people/BusinessPerson-Male5.png \
	extras/source/gallery/people/BusinessPerson-Male6.png \
	extras/source/gallery/people/Chef1.png \
	extras/source/gallery/people/Chef2.png \
	extras/source/gallery/people/Computer-User-Female1.png \
	extras/source/gallery/people/Computer-User-Female2.png \
	extras/source/gallery/people/Computer-User-Male1.png \
	extras/source/gallery/people/Computer-User-Male2.png \
	extras/source/gallery/people/ConstructionWorker2.png \
	extras/source/gallery/people/ConstructionWorker.png \
	extras/source/gallery/people/Detective1.png \
	extras/source/gallery/people/Detective2.png \
	extras/source/gallery/people/Doctor-Female1.png \
	extras/source/gallery/people/Doctor-Female2.png \
	extras/source/gallery/people/Doctor-Male1.png \
	extras/source/gallery/people/Doctor-Male2.png \
	extras/source/gallery/people/Nurse1.png \
	extras/source/gallery/people/Nurse2.png \
	extras/source/gallery/people/PoliceOfficer1.png \
	extras/source/gallery/people/PoliceOfficer2.png \
	extras/source/gallery/people/Presenter-Female1.png \
	extras/source/gallery/people/Presenter-Female2.png \
	extras/source/gallery/people/Presenter-Male1.png \
	extras/source/gallery/people/Presenter-Male2.png \
	extras/source/gallery/people/Student-Female.png \
	extras/source/gallery/people/Student-Male.png \
	extras/source/gallery/people/Surgeon1.png \
	extras/source/gallery/people/Surgeon2.png \
	extras/source/gallery/people/Surgeon-Female1.png \
	extras/source/gallery/people/Surgeon-Female2.png \
	extras/source/gallery/people/Teacher1.png \
	extras/source/gallery/people/Teacher2.png \
	extras/source/gallery/people/Tourist-Female1.png \
	extras/source/gallery/people/Tourist-Female2.png \
	extras/source/gallery/people/Tourist-Male1.png \
	extras/source/gallery/people/Tourist-Male2.png \
))

# vim: set noet sw=4 ts=4:
