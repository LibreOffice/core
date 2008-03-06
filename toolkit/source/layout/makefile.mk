PRJ=..$/..
PRJNAME=toolkit
TARGET=layoutcore
ENABLE_EXCEPTIONS=true
TARGETTYPE=GUI

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

.IF "$(ENABLE_LAYOUT)" == "TRUE"

.IF "$(COMNAME)" == "gcc3"
CFLAGS+=-I../$(PRJ)/xmlscript/source
CFLAGS+=-Wall -Wno-non-virtual-dtor
.ENDIF

##CFLAGS += -DENABLE_LAYOUT=1

CXXFILES= \
    bin.cxx \
    box.cxx \
    byteseq.cxx \
    container.cxx \
    dialogbuttonhbox.cxx \
    factory.cxx \
    flow.cxx \
    helper.cxx \
    import.cxx \
    proplist.cxx \
    root.cxx \
    table.cxx \
    timer.cxx \
    translate.cxx

SLOFILES= \
    $(SLO)$/bin.obj \
    $(SLO)$/box.obj \
    $(SLO)$/byteseq.obj \
    $(SLO)$/container.obj \
    $(SLO)$/dialogbuttonhbox.obj \
    $(SLO)$/factory.obj \
    $(SLO)$/flow.obj \
    $(SLO)$/helper.obj \
    $(SLO)$/import.obj \
    $(SLO)$/proplist.obj \
    $(SLO)$/root.obj \
    $(SLO)$/table.obj \
    $(SLO)$/timer.obj \
    $(SLO)$/translate.obj

.ENDIF # ENABLE_LAYOUT == "TRUE"

.INCLUDE : target.mk
