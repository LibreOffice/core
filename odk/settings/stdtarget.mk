.SECONDARY:

$(OUT) : 
    @echo Building for $(OS)
    $(MKDIR) $(subst /,$(PS),$(OUT))
    $(MKDIR) $(subst /,$(PS),$(OUT_BIN))
    $(MKDIR) $(subst /,$(PS),$(OUT_INC))
    $(MKDIR) $(subst /,$(PS),$(OUT_LIB))
    $(MKDIR) $(subst /,$(PS),$(OUT_SLO))
    $(MKDIR) $(subst /,$(PS),$(OUT_SLB))
    $(MKDIR) $(subst /,$(PS),$(OUT_MISC))
    $(MKDIR) $(subst /,$(PS),$(OUT_OBJ))
    $(MKDIR) $(subst /,$(PS),$(OUT_CLASS))

ifeq "$(DEBUG)" "yes"
debug : ALL
endif
