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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.	If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_Package_Package,oox_generated,$(WORKDIR)/CustomTarget/oox/source/token))

$(eval $(call \
	gb_Package_add_customtarget,oox_generated,oox/source/token,SRCDIR))

$(eval $(call \
	gb_CustomTarget_add_dependencies,oox/source/token,\
		oox/source/token/namespaces.hxx.head \
		oox/source/token/namespaces.hxx.tail \
		oox/source/token/namespaces.txt \
		oox/source/token/namespaces.pl \
		oox/source/token/tokens.hxx.head \
		oox/source/token/tokens.hxx.tail \
		oox/source/token/tokens.txt \
		oox/source/token/tokens.pl \
		oox/source/token/properties.hxx.head \
		oox/source/token/properties.hxx.tail \
		oox/source/token/properties.txt \
		oox/source/token/properties.pl \
))

$(eval $(call gb_Package_add_file,oox_generated,inc/oox/token/namespaces.hxx,namespaces.hxx))
$(eval $(call gb_Package_add_file,oox_generated,inc/oox/token/namespaces.txt,namespaces.txt))
$(eval $(call gb_Package_add_file,oox_generated,inc/oox/token/properties.hxx,properties.hxx))
$(eval $(call gb_Package_add_file,oox_generated,inc/oox/token/tokens.hxx,tokens.hxx))

