PRJ=..$/..$/..

PRJNAME=crashrep
TARGET=unxcrashres
LIBTARGET=NO
TARGETTYPE=CUI

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk


# --- Files --------------------------------------------------------

APP1TARGET=$(TARGET)
APP1OBJS=$(OBJ)$/unxcrashres.obj
APP1STDLIBS=$(TOOLSLIB) $(SALLIB)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

