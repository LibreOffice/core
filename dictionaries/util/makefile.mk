PRJ=..
PRJNAME=dictionaries
TARGET=pack_writing_aids

.INCLUDE : settings.mk

DICDEPS:=$(subst,/,$/ $(shell @$(FIND) $(subst,$/,/ $(BIN)) -name "*.line"))


ZIP1TARGET      = writingaids
ZIP1LIST        = $(BIN)$/*.dic $(BIN)$/*.aff $(BIN)$/*.txt $(BIN)$/*.lst $(BIN)$/*.idx $(BIN)$/*.dat $(BIN)$/*.sxw $(SOLARBINDIR)$/hyph_en_US.dic

.INCLUDE : target.mk

.IF "$(ZIP1TARGETN)" != ""
$(ZIP1TARGETN) : $(BIN)$/dictionary.lst
.ENDIF			# "$(ZIP1TARGETN)" != ""

$(BIN)$/dictionary.lst : $(DICDEPS)
    @-$(RM) $@
    $(TYPE) $(BIN)$/dictionary_head > $@
    -$(TYPE) $(BIN)$/dictionary_*.line >> $@

