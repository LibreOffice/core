#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.4 $
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

PRJ=../../..
PRJNAME=extensions
TARGET=spotlightplugin

# --- Settings ----------------------------------
.INCLUDE : settings.mk

.IF "$(OS)"!="MACOSX"
dummy:
    @echo Not using Mac OS X - nothing to build
.ENDIF

# --- Files -------------------------------------

# ... object files ............................
SLOFILES= \
        $(SLO)$/ioapi.obj                \
        $(SLO)$/unzip.obj                \
        $(SLO)$/main.obj                 \
        $(SLO)$/GetMetadataForFile.obj   \
        $(SLO)$/OOoContentDataParser.obj \
        $(SLO)$/OOoMetaDataParser.obj    \
        $(SLO)$/OOoSpotlightImporter.obj \
        
BUNDLE =    $(MISC)$/OOoSpotlightImporter.mdimporter
CONTENTS =  $(BUNDLE)$/Contents
RESOURCES = $(CONTENTS)$/Resources
MACOS =     $(CONTENTS)$/MacOS

BUNDLELIBS= -framework CoreFoundation \
            -framework Foundation     \
            -framework CoreServices   \
            $(ZLIB3RDLIB)

plistfiles  = $(CONTENTS)$/Info.plist 
binaries =    $(MACOS)$/OOoSpotlightImporter
resources =   $(RESOURCES)/en.lproj/schema.strings \
              $(RESOURCES)/schema.xml

ZIPFLAGS = -r
ZIP1TARGET = mdibundle
ZIP1DIR = $(CONTENTS)
ZIP1LIST = *


# --- Targets ----------------------------------

.INCLUDE : target.mk

$(ZIP1TARGETN) : $(plistfiles) $(binaries) $(resources)

# Info.plist is just versioned and copied into the bundle   
$(CONTENTS)$/%.plist : mdimporter/%.plist
    $(MKDIRHIER) $(@:d)
    $(COPY) "$<" "$@"
    
$(MACOS)$/OOoSpotlightImporter: $(SLOFILES)
    $(MKDIRHIER) $(@:d)
    $(CC) -o $(MACOS)$/OOoSpotlightImporter $(SLOFILES:s/.obj/.o/) $(EXTRA_LINKFLAGS) $(BUNDLELIBS) -bundle
# we have to change the zlib install name, otherwise the plugin will not work
    .IF "$(SYSTEM_ZLIB)"=="NO"
    install_name_tool -change @executable_path/libz.1.dylib @executable_path/../../../../MacOS/libz.1.dylib $(MACOS)$/OOoSpotlightImporter
    .ENDIF

#the resources are just copied
$(RESOURCES)$/% : mdimporter/%
    $(MKDIRHIER) $(@:d)
    $(COPY) "$<" "$@"

