PRJ=..

INCPRE=$(MISC)

PRJNAME=transex3
TARGET=tralay
#TARGETTYPE=GUI
TARGETTYPE=CUI
LIBTARGET=no

# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=TRUE

.INCLUDE :  settings.mk

.IF "$(SYSTEM_EXPAT)" == "YES"
CFLAGS+=-DSYSTEM_EXPAT
.ENDIF

# --- Files --------------------------------------------------------

CXXFILES=\
    layoutparse.cxx\
    tralay.cxx

APP1TARGET=$(TARGET)

.IF "$(ENABLE_LAYOUT)" == "TRUE"

OBJFILES =\
    $(OBJ)/export2.obj\
    $(OBJ)/helpmerge.obj\
    $(OBJ)/layoutparse.obj\
    $(OBJ)/merge.obj\
    $(OBJ)/tralay.obj\
    $(OBJ)/xmlparse.obj

APP1OBJS = $(OBJFILES)

APP1STDLIBS =\
    $(EXPATASCII3RDLIB)\
    $(TOOLSLIB)\
    $(VOSLIB)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

test .PHONY:
    ../$(INPATH)/bin/tralay -l en-US zoom.xml > out.sdf
    cat out.sdf > trans.sdf
    sed 's/en-US\t/de\tde:/' out.sdf >> trans.sdf 
    ../$(INPATH)/bin/tralay -m trans.sdf -l de zoom.xml #> zoom-DE.xml

.ELSE # ENABLE_LAYOUT != TRUE

$(BIN)$/$(APP1TARGET)$(EXECPOST):
    touch $@

ALLTAR:
all:
check:
clean:

.ENDIF # ENABLE_LAYOUT != TRUE

