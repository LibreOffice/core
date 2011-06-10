.SUFFIXES:

.SECONDARY:

ifeq "$(DEBUG)" "yes"
debug : ALL
endif

.PHONY: install
install: $(REGISTERFLAG)

$(SDKTYPEFLAG):  #$(URE_TYPES) $(OFFICE_TYPES)
# This is a makefile for a standard Make, so must be TABs below
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(SDKTYPEFLAG)))
	$(CPPUMAKER) -Gc -BUCR -O$(OUT_INC) $(URE_TYPES) $(OFFICE_TYPES)
	echo flagged > $@
