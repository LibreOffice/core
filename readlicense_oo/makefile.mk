PRJ=.
PRJNAME=readlicense
TARGET=source

# ------------------------------------------------------------------
.INCLUDE: settings.mk
# ------------------------------------------------------------------
.IF "$(OUTPATH)"!="unxsoli2"
PRODUCT_TYPE=ada ipl fat srv fam web
.ELSE
PRODUCT_TYPE=ipl fat srv fam web
.ENDIF

PRODUCT_TYPE1=ada
PRODUCT_TYPE2=fam
PRODUCT_TYPE3=fat
PRODUCT_TYPE4=ipl
PRODUCT_TYPE5=srv
PRODUCT_TYPE6=web

.IF "$(GUI)"=="UNX"
$MAKEALLTARGETS: mkalldir convert2unx copyall zipall
.ENDIF

.IF "$(GUI)"=="WNT"
$MAKEALLTARGETS: mkalldir copyall zipall
.ENDIF

mkalldir: $(MISC)$/mkdir $(MISC)$/mkdir/ada $(MISC)$/mkdir/fam $(MISC)$/mkdir/fat $(MISC)$/mkdir/ipl $(MISC)$/mkdir/srv $(MISC)$/mkdir/web

.IF "$(GUI)"=="UNX"
convert2unx:
    @+echo $(foreach,i,$(alllangext) $(foreach,j,$(PRODUCT_TYPE) $(shell $(@+-any2all -u source$/license$/unx$/$i$/$j$/LICENSE ))))
    @+echo $(foreach,i,$(alllangext) $(foreach,j,$(PRODUCT_TYPE) $(shell $(@+-any2all -u source$/readme$/$(OUTPATH)$/$i$/$j$/README ))))
.ENDIF

.IF "$(GUI)"=="UNX"
copyall:
    @+echo $(foreach,i,$(alllangext) $(foreach,j,$(PRODUCT_TYPE) $(shell $(COPY) source$/license$/unx$/$i$/$j$/LICENSE $(MISC)$/setzip$/$j$/LICENSE_$i)))
    @+echo $(foreach,i,$(alllangext) $(foreach,j,$(PRODUCT_TYPE) $(shell $(COPY) source$/readme$/$(OUTPATH)$/$i$/$j$/README $(MISC)$/setzip$/$j$/README_$i)))
.ENDIF
.IF "$(GUI)"=="WNT"
copyall:
    @+echo $(foreach,i,{$(alllangext)} $(shell +-$(COPY) source$/license$/wnt$/$i$/$(PRODUCT_TYPE1)$/license.txt $(MISC)$/setzip$/$(PRODUCT_TYPE1)$/license_$i.txt))))
    @+echo $(foreach,i,$(alllangext) $(shell +-$(COPY) source$/readme$/$(OUTPATH)$/$i$/$(PRODUCT_TYPE1)$/readme.txt $(MISC)$/setzip$/$(PRODUCT_TYPE1)$/readme_$i.txt))))
    @+echo $(foreach,i,$(alllangext) $(shell +-$(COPY) source$/license$/wnt$/$i$/$(PRODUCT_TYPE2)$/license.txt $(MISC)$/setzip$/$(PRODUCT_TYPE2)$/license_$i.txt))))
    @+echo $(foreach,i,$(alllangext) $(shell +-$(COPY) source$/readme$/$(OUTPATH)$/$i$/$(PRODUCT_TYPE2)$/readme.txt $(MISC)$/setzip$/$(PRODUCT_TYPE2)$/readme_$i.txt))))
    @+echo $(foreach,i,$(alllangext) $(shell +-$(COPY) source$/license$/wnt$/$i$/$(PRODUCT_TYPE3)$/license.txt $(MISC)$/setzip$/$(PRODUCT_TYPE3)$/license_$i.txt))))
    @+echo $(foreach,i,$(alllangext) $(shell +-$(COPY) source$/readme$/$(OUTPATH)$/$i$/$(PRODUCT_TYPE3)$/readme.txt $(MISC)$/setzip$/$(PRODUCT_TYPE3)$/readme_$i.txt))))
    @+echo $(foreach,i,$(alllangext) $(shell +-$(COPY) source$/license$/wnt$/$i$/$(PRODUCT_TYPE4)$/license.txt $(MISC)$/setzip$/$(PRODUCT_TYPE4)$/license_$i.txt))))
    @+echo $(foreach,i,$(alllangext) $(shell +-$(COPY) source$/readme$/$(OUTPATH)$/$i$/$(PRODUCT_TYPE4)$/readme.txt $(MISC)$/setzip$/$(PRODUCT_TYPE4)$/readme_$i.txt))))
    @+echo $(foreach,i,$(alllangext) $(shell +-$(COPY) source$/license$/wnt$/$i$/$(PRODUCT_TYPE5)$/license.txt $(MISC)$/setzip$/$(PRODUCT_TYPE5)$/license_$i.txt))))
    @+echo $(foreach,i,$(alllangext) $(shell +-$(COPY) source$/readme$/$(OUTPATH)$/$i$/$(PRODUCT_TYPE5)$/readme.txt $(MISC)$/setzip$/$(PRODUCT_TYPE5)$/readme_$i.txt))))
    @+echo $(foreach,i,$(alllangext) $(shell +-$(COPY) source$/license$/wnt$/$i$/$(PRODUCT_TYPE6)$/license.txt $(MISC)$/setzip$/$(PRODUCT_TYPE6)$/license_$i.txt))))
    @+echo $(foreach,i,$(alllangext) $(shell +-$(COPY) source$/readme$/$(OUTPATH)$/$i$/$(PRODUCT_TYPE6)$/readme.txt $(MISC)$/setzip$/$(PRODUCT_TYPE6)$/readme_$i.txt))))
.ENDIF
zipall: $(MISC)$/setzip$/ada $(MISC)$/setzip$/fam $(MISC)$/setzip$/fat $(MISC)$/setzip$/ipl $(MISC)$/setzip$/srv $(MISC)$/setzip$/web

$(MISC)$/mkdir:
    @+-$(MKDIR) $(MISC)$/setzip >& $(NULLDEV)
$(MISC)$/mkdir/ada:
    @+-$(MKDIR) $(MISC)$/setzip$/ada >& $(NULLDEV)
$(MISC)$/mkdir/fam:
    @+-$(MKDIR) $(MISC)$/setzip$/fam >& $(NULLDEV)
$(MISC)$/mkdir/fat:
    @+-$(MKDIR) $(MISC)$/setzip$/fat >& $(NULLDEV)
$(MISC)$/mkdir/ipl:
    @+-$(MKDIR) $(MISC)$/setzip$/ipl >& $(NULLDEV)
$(MISC)$/mkdir/srv:
    @+-$(MKDIR) $(MISC)$/setzip$/srv >& $(NULLDEV)
$(MISC)$/mkdir/web:
    @+-$(MKDIR) $(MISC)$/setzip$/web >& $(NULLDEV)

.IF "$(GUI)"=="UNX"
.IF "$(OUTPATH)"!="unxsoli2"
$(MISC)$/setzip$/ada .SETDIR=$(MISC)$/setzip$/ada .PHONY:
    zip ..$/..$/..$/..$/$(BIN)$/readme_ada.zip *
.ENDIF

$(MISC)$/setzip$/fam .SETDIR=$(MISC)$/setzip$/fam .PHONY:
    zip ..$/..$/..$/..$/$(BIN)$/readme_fam.zip *

$(MISC)$/setzip$/fat .SETDIR=$(MISC)$/setzip$/fat .PHONY:
    zip ..$/..$/..$/..$/$(BIN)$/readme_fat.zip *

$(MISC)$/setzip$/ipl .SETDIR=$(MISC)$/setzip$/ipl .PHONY:
    zip ..$/..$/..$/..$/$(BIN)$/readme_ipl.zip *

$(MISC)$/setzip$/srv .SETDIR=$(MISC)$/setzip$/srv .PHONY:
    zip ..$/..$/..$/..$/$(BIN)$/readme_srv.zip *

$(MISC)$/setzip$/web .SETDIR=$(MISC)$/setzip$/web .PHONY:
    zip ..$/..$/..$/..$/$(BIN)$/readme_web.zip *
.ENDIF
.IF "$(GUI)"=="WNT"
$(MISC)$/setzip$/ada .SETDIR=$(MISC)$/setzip$/ada .PHONY:
    @+echo $(shell +-zip ..$/..$/..$/..$/$(BIN)$/readme_ada.zip *)

$(MISC)$/setzip$/fam .SETDIR=$(MISC)$/setzip$/fam .PHONY:
    @+echo $(shell +-zip ..$/..$/..$/..$/$(BIN)$/readme_fam.zip *)

$(MISC)$/setzip$/fat .SETDIR=$(MISC)$/setzip$/fat .PHONY:
    @+echo $(shell +-zip ..$/..$/..$/..$/$(BIN)$/readme_fat.zip *)

$(MISC)$/setzip$/ipl .SETDIR=$(MISC)$/setzip$/ipl .PHONY:
    @+echo $(shell +-zip ..$/..$/..$/..$/$(BIN)$/readme_ipl.zip *)

$(MISC)$/setzip$/srv .SETDIR=$(MISC)$/setzip$/srv .PHONY:
    @+echo $(shell +-zip ..$/..$/..$/..$/$(BIN)$/readme_srv.zip *)

$(MISC)$/setzip$/web .SETDIR=$(MISC)$/setzip$/web .PHONY:
    @+echo $(shell +-zip ..$/..$/..$/..$/$(BIN)$/readme_web.zip *)
.ENDIF

# ------------------------------------------------------------------
.INCLUDE: target.mk
# ------------------------------------------------------------------

