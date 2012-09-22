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



PRJ=..

PRJNAME=desktop
TARGET=zipintro
# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

DEFAULT_FLAVOURS=dev dev_nologo nologo intro

ZIP1LIST= \
    $(null,$(INTRO_BITMAPS) $(MISC)$/ooo_custom_images$/dev$/introabout$/intro.png $(INTRO_BITMAPS)) \
    $(null,$(ABOUT_BITMAPS) $(MISC)$/$(RSCDEFIMG)$/introabout$/about.png $(ABOUT_BITMAPS)) \
    $(MISC)$/$(RSCDEFIMG)$/introabout$/logo.png
ZIP2LIST= \
    $(null,$(INTRO_BITMAPS) $(MISC)$/ooo_custom_images$/dev_nologo$/introabout$/intro.png $(INTRO_BITMAPS)) \
    $(null,$(ABOUT_BITMAPS) $(MISC)$/$(RSCDEFIMG)$/introabout$/about.png $(ABOUT_BITMAPS)) \
    $(MISC)$/$(RSCDEFIMG)$/introabout$/logo.png
ZIP3LIST= \
    $(null,$(INTRO_BITMAPS) $(MISC)$/ooo_custom_images$/nologo$/introabout$/intro.png $(INTRO_BITMAPS)) \
    $(null,$(ABOUT_BITMAPS) $(MISC)$/$(RSCDEFIMG)$/introabout$/about.png $(ABOUT_BITMAPS)) \
    $(MISC)$/$(RSCDEFIMG)$/introabout$/logo.png
ZIP4LIST= \
    $(null,$(INTRO_BITMAPS) $(MISC)$/$(RSCDEFIMG)$/introabout$/intro.png $(INTRO_BITMAPS)) \
    $(null,$(ABOUT_BITMAPS) $(MISC)$/$(RSCDEFIMG)$/introabout$/about.png $(ABOUT_BITMAPS)) \
    $(MISC)$/$(RSCDEFIMG)$/introabout$/logo.png

ZIP1TARGET=dev_intro
ZIP1DEPS=$(ZIP1LIST)

ZIP2TARGET=dev_nologo_intro
ZIP2DEPS=$(ZIP2LIST)

ZIP3TARGET=nologo_intro
ZIP3DEPS=$(ZIP3LIST)

ZIP4TARGET=intro_intro
ZIP4DEPS=$(ZIP4LIST)

.INCLUDE :  target.mk

ALLTAR : $(foreach,i,$(DEFAULT_FLAVOURS) $(COMMONBIN)$/$i$/intro.zip)

# now duplicate for deliver...
# Because of issue 78837 we cannot use a % rule here (Commented out below)
# but have to write individual rules.
#$(COMMONBIN)$/%$/intro.zip : $(COMMONBIN)$/%_intro.zip

$(COMMONBIN)$/dev$/intro.zip : $(COMMONBIN)$/dev_intro.zip
    @@-$(MKDIR) $(@:d)
    @$(COPY) $< $@

$(COMMONBIN)$/dev_nologo$/intro.zip : $(COMMONBIN)$/dev_nologo_intro.zip
    @@-$(MKDIR) $(@:d)
    @$(COPY) $< $@

$(COMMONBIN)$/nologo$/intro.zip : $(COMMONBIN)$/nologo_intro.zip
    @@-$(MKDIR) $(@:d)
    @$(COPY) $< $@

$(COMMONBIN)$/intro$/intro.zip : $(COMMONBIN)$/intro_intro.zip
    @@-$(MKDIR) $(@:d)
    @$(COPY) $< $@

$(MISC)$/%.bmp : $(SOLARSRC)$/%.bmp
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

$(MISC)$/%.png : $(SOLARSRC)$/%.png
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@
