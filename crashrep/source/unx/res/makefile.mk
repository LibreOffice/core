PRJ=..$/..$/..

PRJNAME=crashrep
TARGET=unxcrashres
LIBTARGET=NO
TARGETTYPE=CUI

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(OS)"=="MACOSX"

dummy:
    @echo "Nothing to build for OS $(OS)"

.ELSE		# "$(OS)"=="MACOSX"

# --- Files --------------------------------------------------------

SOLARLIB!:=$(SOLARLIB:s/jre/jnore/)

APP1TARGET=$(TARGET)
APP1OBJS=$(OBJ)$/unxcrashres.obj
APP1STDLIBS=$(TOOLSLIB) $(SALLIB)

.ENDIF    "$(OS)"=="MACOSX"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

