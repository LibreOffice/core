#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

.IF "$(OS)"=="LINUX"
PKGREV          = $(BUILD)
.ELSE
PKGREV          = $(LAST_MINOR:s/m//:s/s/./)
.ENDIF

ABSLOCALOUT:=$(shell @cd $(PRJ) && pwd)$/$(ROUT)

# create desktop-integration subfolder on linux
.IF "$(OS)"=="LINUX" || "$(OS)"=="AIX"
# rpm seems to require absolute paths here :(
PKGDIR:=$(ABSLOCALOUT)$/bin$/desktop-integration
RPMMACROS= \
    --define "_rpmdir $(PKGDIR)" \
    --define "_rpmfilename %%{{NAME}}-%%{{VERSION}}-%%{{RELEASE}}.%%{{ARCH}}.rpm"
.ELSE
PKGDIR=$(BIN)
.ENDIF

.INCLUDE : productversion.mk

PKGVERSION = $(VERSIONMAJOR).$(VERSIONMINOR).$(VERSIONMICRO)

# gnome-vfs treats everything behind the last '.' as an icon extension, 
# even though the "icon_filename" in '.keys' is specified as filename 
# without extension. Since it also does not know how to handle "2-writer"
# type icons :-), we are stripping all '.' for now.
# ToDo: maybe we use a product major later ??
ICONVERSION = $(PRODUCTVERSIONSHORT:s/.//g)
# UNIXWRAPPERNAME variable is used to generate the common desktop files below share/xdg;
# the default values get replaced by make_installer.pl at (core0x) packaging time;
# another wrapper name can be forced by --with-unix-wrapper configure option
# which is need by other distributors, see http://www.openoffice.org/issues/show_bug.cgi?id=75366
.IF "$(UNIXWRAPPERNAME)" == ""
UNIXWRAPPERNAME = '$${{UNIXPRODUCTNAME}}$${{BRANDPACKAGEVERSION}}'
.ENDIF

# CAUTION! $(UNIXBASISROOTNAME) from openoffice.lst NOT available (!)
PRODUCTNAME.libreoffice = LibreOffice
PRODUCTVERSION.libreoffice = $(PRODUCTVERSION)
PRODUCTVERSIONSHORT.libreoffice = $(PRODUCTVERSIONSHORT)
PKGVERSION.libreoffice = $(PKGVERSION)
UNIXFILENAME.libreoffice = $(PRODUCTNAME.libreoffice:l)$(PRODUCTVERSION.libreoffice)
ICONPREFIX.libreoffice = $(UNIXFILENAME.libreoffice)

PRODUCTNAME.lodev = LibreOfficeDev
PRODUCTVERSION.lodev = $(PRODUCTVERSION)
PRODUCTVERSIONSHORT.lodev = $(PRODUCTVERSIONSHORT)
PKGVERSION.lodev = $(PKGVERSION)
UNIXFILENAME.lodev = lodev$(PRODUCTVERSION.lodev)
ICONPREFIX.lodev = $(UNIXFILENAME.lodev)

PRODUCTNAME.oxygenoffice = OxygenOffice
PRODUCTVERSION.oxygenoffice = $(PRODUCTVERSION)
PRODUCTVERSIONSHORT.oxygenoffice = $(PRODUCTVERSIONSHORT)
PKGVERSION.oxygenoffice = $(PKGVERSION)
UNIXFILENAME.oxygenoffice = $(PRODUCTNAME.oxygenoffice:l)$(PRODUCTVERSION.oxygenoffice)
ICONPREFIX.oxygenoffice = $(UNIXFILENAME.oxygenoffice)
