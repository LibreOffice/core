PRJ=..
PRJNAME=odk
TARGET=odk

# ------------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: makefile.pmk
# ------------------------------------------------------------------

ZIP1TARGET=odkexamples
ZIP1FLAGS=-u -r
ZIP1DIR=$(PRJ)
ZIP1LIST=examples -x "*CVS*" -x "*makefile.mk" -x "*Container1*" -x "*Storage*" -x "*register_component*" -x "*examples.html"

.INCLUDE :  target.mk

ALLTAR:\
    $(BIN)$/$(PRODUCTZIPFILE)\
    $(BIN)$/odk_oo.zip


$(BIN)$/$(PRODUCTZIPFILE) : $(SDK_CONTENT_CHECK_FILES) $(SDK_CHECK_FLAGS)
    cd $(BIN)$/$(PRODUCT_NAME) && $(WRAPCMD) zip -urq ..$/$(PRODUCTZIPFILE) . -x "idl/*" $(CHECKZIPRESULT)
    cd $(BIN)$/$(PRODUCT_NAME) && $(WRAPCMD) zip -urq ..$/odkidl.zip idl/* $(CHECKZIPRESULT)

ODK_OO_FILES=\
    $(PRJ)$/index.html \
    $(PRJ)$/docs$/tools.html \
    $(PRJ)$/docs$/notsupported.html \
    $(PRJ)$/docs$/DevelopersGuide_intro.html \
    $(PRJ)$/docs$/install.html \
    $(PRJ)$/docs$/sdk_styles.css \
    $(PRJ)$/docs$/images$/arrow-1.gif \
    $(PRJ)$/docs$/images$/arrow-3.gif \
    $(PRJ)$/docs$/images$/bg_table.gif \
    $(PRJ)$/docs$/images$/bg_table2.gif \
    $(PRJ)$/docs$/images$/bg_table3.gif \
    $(PRJ)$/docs$/images$/nav_down.png \
    $(PRJ)$/docs$/images$/nav_home.png \
    $(PRJ)$/docs$/images$/nav_left.png \
    $(PRJ)$/docs$/images$/nav_right.png \
    $(PRJ)$/docs$/images$/nav_up.png \
    $(PRJ)$/docs$/images$/sdk_head-1.gif \
    $(PRJ)$/docs$/images$/sdk_head-2.gif \
    $(PRJ)$/docs$/images$/sdk_head-3.gif \
    $(PRJ)$/docs$/images$/sdk_line-1.gif \
    $(PRJ)$/docs$/images$/sdk_line-2.gif \
    $(PRJ)$/examples$/examples.html \
    $(PRJ)$/examples$/DevelopersGuide$/examples.html


$(BIN)$/odk_oo.zip : $(ODK_OO_FILES)
    cd $(PRJ) && $(WRAPCMD) zip -urq $(subst,$(PRJ)$/, $(BIN)$/odk_oo.zip) $(subst,$(PRJ)$/, $<) $(CHECKZIPRESULT)

