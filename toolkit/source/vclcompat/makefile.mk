PRJ=..$/..
PRJNAME=toolkit
TARGET=vclcompat
ENABLE_EXCEPTIONS=true
TARGETTYPE=GUI

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

.IF "$(ENABLE_LAYOUT)" == "TRUE"

.IF "$(COMNAME)" == "gcc3"
CFLAGS+=-Wall -Wno-non-virtual-dtor
.ENDIF

##CFLAGS += -DENABLE_LAYOUT

CXXFILES= \
    wrapper.cxx \
    wbutton.cxx \
    wcontainer.cxx \
    wfield.cxx

SLOFILES= \
    $(SLO)$/wrapper.obj \
    $(SLO)$/wbutton.obj \
    $(SLO)$/wcontainer.obj \
    $(SLO)$/wfield.obj

.ENDIF # ENABLE_LAYOUT == "TRUE"

.INCLUDE : target.mk
