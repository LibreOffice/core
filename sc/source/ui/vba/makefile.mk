PRJ=../..$/..

PRJNAME=
TARGET=vbaobj
.IF "$(ENABLE_VBA)"!="YES"
dummy:
        @echo "not building vba..."
.ENDIF


NO_BSYMBOLIC=   TRUE
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  sv.mk
DLLPRE =

ALLTAR : \
        $(MISC)$/$(TARGET).don \
        $(SLOTARGET)

$(MISC)$/$(TARGET).don : $(SOLARBINDIR)$/oovbaapi.rdb
        +$(CPPUMAKER) -O$(OUT)$/inc -BUCR $(SOLARBINDIR)$/oovbaapi.rdb -X$(SOLARBINDIR)$/types.rdb && echo > $@
        echo $@

# ------------------------------------------------------------------

SLOFILES= \
        $(SLO)$/vbaglobals.obj \
        $(SLO)$/vbaworkbook.obj \
        $(SLO)$/vbaworksheets.obj \
        $(SLO)$/vbaapplication.obj \
        $(SLO)$/vbarange.obj \
        $(SLO)$/vbacomment.obj \
        $(SLO)$/vbacomments.obj \
        $(SLO)$/vbaworkbooks.obj \
        $(SLO)$/vbaworksheet.obj \
        $(SLO)$/vbaoutline.obj \
        $(SLO)$/vbafont.obj\
        $(SLO)$/vbahelper.obj\
        $(SLO)$/vbainterior.obj\
        $(SLO)$/vbawsfunction.obj\
        $(SLO)$/vbawindow.obj\
        $(SLO)$/vbachart.obj\
        $(SLO)$/vbachartobject.obj\
        $(SLO)$/vbachartobjects.obj\
        $(SLO)$/vbaseriescollection.obj\
        $(SLO)$/vbacollectionimpl.obj\
        $(SLO)$/vbadialogs.obj \
        $(SLO)$/vbadialog.obj	\
        $(SLO)$/vbapivottable.obj \
        $(SLO)$/vbapivotcache.obj \
        $(SLO)$/vbapivottables.obj \
        $(SLO)$/vbawindows.obj \
        $(SLO)$/vbapalette.obj \
        $(SLO)$/vbaborders.obj \
        $(SLO)$/vbacharacters.obj \
        $(SLO)$/vbacombobox.obj \
        $(SLO)$/vbavalidation.obj \
        $(SLO)$/service.obj
 

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
