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



$(eval $(call gb_Package_Package,packimages_packimages,$(WORKDIR)/CustomTarget/packimages,packimages))

# Custom sets, at 24x24 & 16x16 fall-back to industrial preferentially
CUSTOM_IMAGE_SETS := hicontrast industrial classic
CUSTOM_PREFERRED_FALLBACK_1 := -c $(SRCDIR)/ooo_custom_images/industrial
CLASSIC_TARBALL=$(SRCDIR)/ooo_custom_images/classic/classic_images.tar.gz

$(eval $(call gb_Package_add_file,packimages_packimages,bin/images.zip,bin/images.zip))
$(foreach imageset,$(CUSTOM_IMAGE_SETS),$(eval $(call gb_Package_add_file,packimages_packimages,bin/images_$(imageset).zip,bin/images_$(imageset).zip)))
$(eval $(call gb_Package_add_file,packimages_packimages,bin/images_brand.zip,bin/images_brand.zip))

ifeq ($(VERBOSE_PACKIMG),TRUE)
VERBOSESWITCH := -vv
else ifeq ($(VERBOSE),TRUE)
VERBOSESWITCH := -v
else
VERBOSESWITCH :=
endif

RSCDEFIMG := default_images
SORTED_LIST := $(WORKDIR)/CustomTarget/packimages/res/img/sorted.lst
COMMAND_IMAGE_LIST := $(WORKDIR)/CustomTarget/packimages/res/img/commandimagelist.ilst

$(WORKDIR)/CustomTarget/packimages/bin/images.zip : $(COMMAND_IMAGE_LIST)
	mkdir -p $(dir $@) && \
	$(PERL) $(SOLARENV)/bin/packimages.pl \
		$(VERBOSESWITCH) \
		-g $(SOLARSRC)/$(RSCDEFIMG) \
		-m $(SOLARSRC)/$(RSCDEFIMG) \
		-c $(SRCDIR)/packimages \
		-l $(OUTDIR)/res/img \
		-s $(SORTED_LIST) \
		-l $(WORKDIR)/CustomTarget/packimages/res/img \
		-o $@

$(WORKDIR)/CustomTarget/packimages/bin/images_%.zip : $(COMMAND_IMAGE_LIST)
	mkdir -p $(dir $@) && \
	$(PERL) $(SOLARENV)/bin/packimages.pl \
		$(VERBOSESWITCH) \
		-g $(SRCDIR)/$(RSCDEFIMG) \
		-m $(SRCDIR)/$(RSCDEFIMG) \
		-c $(SRCDIR)/packimages \
		-c $(SRCDIR)/ooo_custom_images/$* \
		-c $(WORKDIR)/CustomTarget/packimages/$* \
		$(CUSTOM_PREFERRED_FALLBACK_1) \
		$(CUSTOM_PREFERRED_FALLBACK_2) \
		-l $(OUTDIR)/res/img \
		-l $(WORKDIR)/CustomTarget/packimages/res/img \
		-s $(SORTED_LIST) \
		-o $@

# make sure to have one to keep packing happy
$(WORKDIR)/CustomTarget/packimages/bin/images_brand.zip :
	mkdir -p $(dir $@) && \
	touch $@

$(COMMAND_IMAGE_LIST) : $(SORTED_LIST)
	mkdir -p $(dir $@) && \
	find $(SRCDIR)/$(RSCDEFIMG)/res/commandimagelist -name "*.png" | sed "s#$(SRCDIR)/$(RSCDEFIMG)/res#%GLOBALRES%#" | $(PERL) $(SOLARENV)/bin/sort.pl > $@.$(INPATH) && \
	$(PERL) $(SOLARENV)/bin/diffmv.pl $@.$(INPATH) $@

$(SORTED_LIST) : $(SRCDIR)/packimages/pack/image-sort.lst
	mkdir -p $(dir $@) && \
	$(PERL) $(SOLARENV)/bin/image-sort.pl $^ $(OUTDIR)/xml $@



# generate the HiContrast icon set

$(WORKDIR)/CustomTarget/packimages/bin/images_hicontrast.zip : $(WORKDIR)/CustomTarget/packimages/hicontrast.flag

$(WORKDIR)/CustomTarget/packimages/hicontrast.flag :
	$(PERL) $(SOLARENV)/bin/hicontrast-to-theme.pl \
		$(SRCDIR)/default_images \
		$(WORKDIR)/CustomTarget/packimages/hicontrast && \
	touch $@



# unpack the classic icon set

$(WORKDIR)/CustomTarget/packimages/bin/images_classic.zip : $(WORKDIR)/CustomTarget/packimages/classic.flag

ifeq ($(GUI),OS2)
$(WORKDIR)/CustomTarget/packimages/classic.flag :
	mkdir -p $(dir $@) && \
	cd $(dir $@) && \
	tar zxf $(CLASSIC_TARBALL) && \
	touch $@
	cat $@ || echo "ERROR: unpacking $(CLASSIC_TARBALL) failed"
else
$(WORKDIR)/CustomTarget/packimages/classic.flag :
	mkdir -p $(dir $@) && \
	cd $(dir $@) && \
	gunzip -c $(CLASSIC_TARBALL) | ( tar -xf - ) && \
	touch $@
	$(if $(filter UNX,$(GUI)),chmod -R g+w $(WORKDIR)/CustomTarget/packimages/classic)
	cat $@ || echo "ERROR: unpacking $(CLASSIC_TARBALL) failed"
endif

