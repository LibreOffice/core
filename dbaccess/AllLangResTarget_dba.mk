#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2011 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platdbam office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS dba A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 dba more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# dba a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_AllLangResTarget_AllLangResTarget,dba))

$(eval $(call gb_AllLangResTarget_set_reslocation,dba,dbaccess))

$(eval $(call gb_AllLangResTarget_add_srs,dba,dbaccess/dba))

$(eval $(call gb_SrsTarget_SrsTarget,dbaccess/dba))

$(eval $(call gb_SrsTarget_set_include,dbaccess/dba,\
	$$(INCLUDE) \
	-I$(OUTDIR)/inc \
	-I$(SRCDIR)/dbaccess/source/core/inc \
	-I$(SRCDIR)/dbaccess/inc \
))

$(eval $(call gb_SrsTarget_add_files,dbaccess/dba,\
	dbaccess/source/core/resource/strings.src \
))

# vim: set noet sw=4 ts=4:
