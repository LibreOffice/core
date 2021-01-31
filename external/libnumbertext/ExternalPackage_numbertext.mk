# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,libnumbertext_numbertext,libnumbertext))

$(eval $(call gb_ExternalPackage_add_unpacked_files,libnumbertext_numbertext,$(LIBO_SHARE_FOLDER)/numbertext,\
	data/af.sor \
	data/bg.sor \
	data/ca.sor \
	data/cs.sor \
	data/da.sor \
	data/de.sor \
	data/el.sor \
	data/en.sor \
	data/eo.sor \
	data/es.sor \
	data/et.sor \
	data/fi.sor \
	data/fr.sor \
	data/fr.sor \
	data/ga.sor \
	data/gl.sor \
	data/he.sor \
	data/hr.sor \
	data/hu.sor \
	data/hu_Hung.sor \
	data/id.sor \
	data/is.sor \
	data/it.sor \
	data/ja.sor \
	data/ko.sor \
	data/lb.sor \
	data/lg.sor \
	data/lt.sor \
	data/lv.sor \
	data/mr.sor \
	data/ms.sor \
	data/mt.sor \
	data/no.sor \
	data/nl.sor \
	data/pl.sor \
	data/pt.sor \
	data/Roman.sor \
	data/ro.sor \
	data/ru.sor \
	data/sh.sor \
	data/sl.sor \
	data/sq.sor \
	data/sr.sor \
	data/Suzhou.sor \
	data/sv.sor \
	data/th.sor \
	data/tr.sor \
	data/uk.sor \
	data/vi.sor \
	data/zh.sor \
))

# vim: set noet sw=4 ts=4:
