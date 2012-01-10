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
PRJNAME=packimages
TARGET=packimages

.INCLUDE:  settings.mk
# point to an existing directory as default that poses no threat
# to a "find" that looks for "*.png"
RSCCUSTOMIMG*=$(PRJ)
.INCLUDE: target.mk

IMAGES := $(COMMONBIN)$/images.zip
SORTED_LIST=$(RES)$/img$/sorted.lst
# Custom sets, at 24x24 & 16x16 fall-back to industrial preferentially
CUSTOM_IMAGE_SETS=hicontrast industrial tango classic
CUSTOM_IMAGES+=$(foreach,i,$(CUSTOM_IMAGE_SETS) images_$i)
CUSTOM_PREFERRED_FALLBACK_1*=-c $(SOLARSRC)$/ooo_custom_images$/tango
CUSTOM_PREFERRED_FALLBACK_2*=-c $(SOLARSRC)$/ooo_custom_images$/industrial

CLASSIC_TARBALL=$(SOLARSRC)$/ooo_custom_images$/classic/classic_images.tar.gz

ALLTAR : $(IMAGES) $(CUSTOM_IMAGES) $(COMMONBIN)$/images_brand.zip

$(IMAGES) $(CUSTOM_IMAGES) : $(SORTED_LIST)

$(SORTED_LIST) : image-sort.lst
    +-$(MKDIR) $(RES)$/img
    +$(PERL) $(SOLARENV)$/bin$/image-sort.pl image-sort.lst $(SOLARXMLDIR) $@

$(RES)$/img$/commandimagelist.ilst .PHONY : $(SORTED_LIST)
    @@-$(MKDIR) $(RES)$/img
#+-$(RM) $@ $@.$(INPATH)
    $(FIND) $(SOLARSRC)$/$(RSCDEFIMG)/res/commandimagelist -name "*.png" | sed "s#$(SOLARSRC)$/$(RSCDEFIMG)/res#%GLOBALRES%#" | $(PERL) $(SOLARENV)$/bin$/sort.pl > $@.$(INPATH)
    $(PERL) $(SOLARENV)$/bin$/diffmv.pl $@.$(INPATH) $@

$(COMMONBIN)$/images.zip .PHONY: $(RES)$/img$/commandimagelist.ilst
    $(PERL) $(SOLARENV)$/bin$/packimages.pl -g $(SOLARSRC)$/$(RSCDEFIMG) -m $(SOLARSRC)$/$(RSCDEFIMG) -c $(RSCCUSTOMIMG) -l $(SOLARCOMMONRESDIR)$/img -s $(SORTED_LIST) -l $(RES)$/img -o $@

images_% : $(RES)$/img$/commandimagelist.ilst
    $(PERL) $(SOLARENV)$/bin$/packimages.pl -g $(SOLARSRC)$/$(RSCDEFIMG) -m $(SOLARSRC)$/$(RSCDEFIMG) -c $(RSCCUSTOMIMG) -c $(SOLARSRC)$/ooo_custom_images$/$(@:s/images_//) -c $(MISC)$/$(@:s/images_//) $(CUSTOM_PREFERRED_FALLBACK_1) $(CUSTOM_PREFERRED_FALLBACK_2) -l $(SOLARCOMMONRESDIR)$/img -l $(RES)$/img -s $(SORTED_LIST) -o $(COMMONBIN)$/$@.zip

# make sure to have one to keep packing happy
$(COMMONBIN)$/images_brand.zip:
    @$(TOUCH) $@

# generate the HiContrast icon set
$(MISC)$/hicontrast.flag .PHONY :
    $(PERL) $(SOLARENV)$/bin$/hicontrast-to-theme.pl $(SOLARSRC)$/default_images $(MISC)$/hicontrast && $(TOUCH) $@

# unpack the classic icon set
$(MISC)$/classic.flag : $(CLASSIC_TARBALL)
    cd $(MISC) && gunzip -c $(CLASSIC_TARBALL) | ( tar -xf - ) && $(TOUCH) $(@:f)
.IF "$(GUI)"=="UNX"
    chmod -R g+w $(MISC)$/classic
.ENDIF
    @$(TYPE) $@ || echo "ERROR: unpacking $(CLASSIC_TARBALL) failed"

# dependencies
images_hicontrast : $(MISC)$/hicontrast.flag $(RES)$/img$/commandimagelist.ilst

images_crystal : $(MISC)$/crystal.flag $(RES)$/img$/commandimagelist.ilst

images_classic : $(MISC)$/classic.flag $(RES)$/img$/commandimagelist.ilst

