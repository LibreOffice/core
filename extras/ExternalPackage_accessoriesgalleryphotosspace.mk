# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,extras_accessoriesgalleryphotosspace,gallery-pack))

$(eval $(call gb_ExternalPackage_set_outdir,extras_accessoriesgalleryphotosspace,$(gb_INSTROOT)))

$(eval $(call gb_ExternalPackage_add_unpacked_files,extras_accessoriesgalleryphotosspace,$(LIBO_SHARE_FOLDER)/gallery/Photos/Space,\
	accessories/Photos/Space/Wikimedia_Space_120px-Atlantis_taking_off_on_STS-27.jpg \
	accessories/Photos/Space/Wikimedia_Space_1995-18-b-large_web.jpg \
	accessories/Photos/Space/Wikimedia_Space_2005-06-b-large_web.jpg \
	accessories/Photos/Space/Wikimedia_Space_2005-37-a-large_web.jpg \
	accessories/Photos/Space/Wikimedia_Space_2006-10-a-large_web.jpg \
	accessories/Photos/Space/Wikimedia_Space_395px-Mir_as_seen_from_Discovery_during_STS-63.jpg \
	accessories/Photos/Space/Wikimedia_Space_433px-Baikal_liftoff.jpg \
	accessories/Photos/Space/Wikimedia_Space_477px-Ap11-KSC-69PC-442.jpg \
	accessories/Photos/Space/Wikimedia_Space_516px-Shuttle.jpg \
	accessories/Photos/Space/Wikimedia_Space_594px-Apollo_11_bootprint.jpg \
	accessories/Photos/Space/Wikimedia_Space_595px-Tarantula_nebula_detail.jpg \
	accessories/Photos/Space/Wikimedia_Space_600px-Mars_Valles_Marineris.jpeg \
	accessories/Photos/Space/Wikimedia_Space_682px-Atlantis_Docked_to_Mir.jpg \
	accessories/Photos/Space/Wikimedia_Space_740px-Hst_sts82.jpg \
	accessories/Photos/Space/Wikimedia_Space_792px-NASA_Space_Shuttle_Discovery_STS-92.jpg \
	accessories/Photos/Space/Wikimedia_Space_As08-16-2593.jpg \
	accessories/Photos/Space/Wikimedia_Space_heic0405a.jpg \
	accessories/Photos/Space/Wikimedia_Space_NASA-Apollo8-Dec24-Earthrise.jpg \
	accessories/Photos/Space/Wikimedia_Space_Sputnik.jpg \
))

# vim: set noet sw=4 ts=4:
