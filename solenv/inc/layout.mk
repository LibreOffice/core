TRALAY=$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/tralay
XML_DEST=$(DLLDEST)
XML_LANGS=$(alllangiso)

ALL_XMLS=$(foreach,i,$(XML_FILES) "$(XML_DEST)/$i") $(foreach,i,$(XML_LANGS) $(foreach,j,$(XML_FILES) "$(XML_DEST)/$i/$j"))

# Must remove the -j (no duplicate base file names) flag
ZIPUPDATE=-u
XML_ZIP = $(PRJNAME)-layout

ALLTAR: localize.sdf $(XML_ZIP)

$(XML_ZIP): $(ALL_XMLS)

ZIP1DIR=$(XML_DEST)
ZIP1TARGET=$(XML_ZIP)
ZIP1LIST=$(ALL_XMLS:s@$(XML_DEST)/@@)

$(foreach,i,$(XML_LANGS) $(XML_DEST)/$i/%.xml): %.xml
    $(COMMAND_ECHO)-$(MKDIR) $(@:d)
    @echo $(foreach,i,$(XML_LANGS) $(XML_DEST)/$i/%.xml): %.xml
    $(TRALAY) -m localize.sdf -o "$(XML_DEST)" -l $(XML_LANGS:f:t" -l ") "$<"

$(XML_DEST)/%.xml: %.xml
    $(COMMAND_ECHO)-$(MKDIR) $(@:d)
    $(COMMAND_ECHO)$(COPY) "$<" "$@"

# Don't want to overwrite filled localize.sdf with empty template
template.sdf:
    $(foreach,i,$(XML_FILES) $(TRALAY) -l en-US "$i") > $@

$(ZIP1LIST): localize.sdf
