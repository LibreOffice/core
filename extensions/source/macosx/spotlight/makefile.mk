#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: obo $ $Date: 2008-01-04 14:59:38 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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

