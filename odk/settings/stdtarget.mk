.SUFFIXES:

.SECONDARY:

ifeq "$(DEBUG)" "yes"
debug : ALL
endif

.PHONY: install
install: $(REGISTERFLAG)

$(SDKTYPEFLAG):  #$(URE_TYPES) $(OFFICE_TYPES)
    -$(MKDIR) $(subst /,$(PS),$(@D))
    -$(DEL) $(subst \\,\,$(subst /,$(PS),$(SDKTYPEFLAG)))
    $(CPPUMAKER) -Gc -BUCR -O$(OUT_INC) $(URE_TYPES) $(OFFICE_TYPES)
    echo flagged > $@
