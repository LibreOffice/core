.SUFFIXES:

gb_Side=host
BUILDDIR=$(shell pwd)
SRCDIR:=$(abspath $(dir $(firstword $(MAKEFILE_LIST)))/../..)
$(info SRCDIR:$(SRCDIR))
include $(SRCDIR)/config_host.mk

.PHONY: init_doxygen generate_tag generate_doc
.DEFAULT: generate_doc

prefered_modules := sw sc sd
gbuild_modules := $(prefered_modules) $(filter-out $(prefered_modules),$(patsubst $(SRCDIR)/%/,%,$(dir $(wildcard $(SRCDIR)/*/Module_*.mk))))
$(info gbuild_modules:$(gbuild_modules))

DOXYGEN_REF_TAGFILES=$(foreach m,$(gbuild_modules), $(BUILDDIR)/docs/$(m)/$(m).tags=./$(m)/html)
export DOXYGEN_REF_TAGFILES

$(BUILDDIR)/docs/%.tag: init_doxygen
	@echo "doxygen tag of $(basename $(notdir $@))"
	@export DOXYGEN_INCLUDE_PATH=`echo $(SOLARINC) | sed -e 's/-I\.//g' -e 's/ -I/ /'g -e 's/ -isystem/ /g' -e 's|/usr/[^ ]*| |g'` ; \
	 $(SRCDIR)/solenv/bin/mkonedoc.sh "$(basename $(notdir $@))" "tag" "$(SRCDIR)" "$(BUILDDIR)/docs"
	@touch $@

$(BUILDDIR)/docs/%.doc: generate_tag
	@echo "doxygen doc of $(basename $(notdir $@))"
	@export DOXYGEN_INCLUDE_PATH=`echo $(SOLARINC) | sed -e 's/-I\.//g' -e 's/ -I/ /'g -e 's/ -isystem/ /g' -e 's|/usr/[^ ]*| |g'` ; \
	 $(SRCDIR)/solenv/bin/mkonedoc.sh "$(basename $(notdir $@))" "doc" "$(SRCDIR)" "$(BUILDDIR)/docs"
	@touch $@



all: generate_doc

generate_tag: $(foreach m,$(gbuild_modules), $(BUILDDIR)/docs/$(m).tag)
	@echo "all tag generated"

generate_doc: $(foreach m,$(gbuild_modules), $(BUILDDIR)/docs/$(m).doc)
	@$(SRCDIR)/solenv/bin/mkdocs_portal.sh "$(SRCDIR)" "$(BUILDDIR)/docs"
	@echo "Done."

init_doxygen:
	@echo "init"
	@rm -fr $(BUILDDIR)/docs
	@mkdir $(BUILDDIR)/docs

