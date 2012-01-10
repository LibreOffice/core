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

.IF "$(ZIP1TARGETN)"!=""
$(ZIP1TARGETN) : $(plistfiles) $(binaries) $(resources)

.ENDIF          # "$(ZIP1TARGETN)"!=""

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

