#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
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

EXTENSIONDIR*:=$(MISC)/$(EXTENSIONNAME)

COMPONENT_CONFIGDIR*:=.
COMPONENT_CONFIGDEST*:=registry/data
COMPONENT_CONFIGDESTSCHEMA*:=registry/schema

.IF "$(COMPONENT_COPYONLY)"==""
DESCRIPTION:=$(EXTENSIONDIR)/description.xml
DESCRIPTION_SRC*:=description.xml

COMPONENT_MANIFEST*:=$(EXTENSIONDIR)/META-INF/manifest.xml
MANIFEST_SRC*:=manifest.xml

.IF "$(GUI)" == "WIN" || "$(GUI)" == "WNT"
PACKLICS*:=$(EXTENSIONDIR)/registration/license.txt
COMPONENT_LIC_TEMPL*:=registration/license.txt
.ELSE
PACKLICS*:=$(EXTENSIONDIR)/registration/LICENSE
COMPONENT_LIC_TEMPL*:=registration/LICENSE
.ENDIF

#TODO:  check ZIP9TARGET for previous use!
common_build_zip=
MANIFEST_DEPS=	$(PACKLICS) $(DESCRIPTION) $(COMPONENT_FILES) $(COMPONENT_LIBRARIES) $(COMPONENT_JARFILES) $(COMPONENT_MERGED_XCU) $(EXTENSION_PACKDEPS)
ZIP9DEPS=		$(PACKLICS) $(DESCRIPTION) $(COMPONENT_XCU) $(COMPONENT_FILES) $(COMPONENT_LIBRARIES) $(COMPONENT_MANIFEST) $(COMPONENT_JARFILES) $(COMPONENT_MERGED_XCU) $(EXTENSION_PACKDEPS)
ZIP9TARGET=		$(EXTENSION_ZIPNAME)
ZIP9DIR=		$(EXTENSIONDIR)
ZIP9EXT=		.oxt
ZIP9FLAGS=-r
ZIP9LIST=		*
.ENDIF			# "$(COMPONNENT_COPYONLY)"==""

