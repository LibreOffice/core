#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************


.IF "$(OS)"=="LINUX"
PKGREV          = $(BUILD)
.ELSE
PKGREV          = $(LAST_MINOR:s/m//:s/s/./)
.ENDIF

ABSLOCALOUT:=$(shell @cd $(PRJ) && pwd)$/$(ROUT)

# create desktop-integration subfolder on linux
.IF "$(OS)"=="LINUX"
# rpm seems to require absolute paths here :(
PKGDIR:=$(ABSLOCALOUT)$/bin$/desktop-integration
RPMMACROS= \
    --define "_rpmdir $(PKGDIR)" \
    --define "_rpmfilename %%{{NAME}}-%%{{VERSION}}-%%{{RELEASE}}.%%{{ARCH}}.rpm"
.ELSE
PKGDIR=$(BIN)
.ENDIF

PRODUCTLIST = openoffice.org

# default values to minimize maintainance effort 
PRODUCTVERSION = 3.4
PRODUCTVERSIONSHORT = 3
PKGVERSION = $(PRODUCTVERSION)
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
UNIXWRAPPERNAME *= '$${{UNIXPRODUCTNAME}}$${{BRANDPACKAGEVERSION}}'

PRODUCTNAME.openoffice.org = OpenOffice.org
PRODUCTVERSION.openoffice.org = $(PRODUCTVERSION)
PRODUCTVERSIONSHORT.openoffice.org = $(PRODUCTVERSIONSHORT)
PKGVERSION.openoffice.org = $(PKGVERSION)
UNIXFILENAME.openoffice.org = $(PRODUCTNAME.openoffice.org:l)$(PRODUCTVERSIONSHORT.openoffice.org)
ICONPREFIX.openoffice.org = $(UNIXFILENAME.openoffice.org:s/.//g)

