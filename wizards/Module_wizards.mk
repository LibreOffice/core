# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2011 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_Module_Module,wizards))

$(eval $(call gb_Module_add_targets,wizards,\
	AllLangResTarget_dbw \
	AllLangResTarget_eur \
	AllLangResTarget_imp \
	AllLangResTarget_tpl \
	AllLangResTarget_wzi \
	Pyuno_fax \
	Pyuno_letter \
	Pyuno_commonwizards \
	Zip_depot \
	Zip_euro \
	Zip_form \
	Zip_gimmicks \
	Zip_import \
	Zip_launcher \
	Zip_share \
	Zip_standard \
	Zip_template \
	Zip_tools \
	Zip_tutorials \
	Zip_usr \
	Zip_web \
))

ifeq ($(SOLAR_JAVA),TRUE)
$(eval $(call gb_Module_add_targets,wizards,\
	Jar_commonwizards \
	Jar_agenda \
	Jar_form \
	Jar_query \
	Jar_report \
	Jar_reportbuilder \
	Jar_table \
	Jar_web \
))
endif

# vim: set noet sw=4 ts=4:
