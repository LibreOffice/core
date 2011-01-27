PRJ=..

PRJNAME=canvas
TARGET=canvasdemo
TARGETTYPE=GUI
LIBTARGET=NO

ENABLE_EXCEPTIONS=TRUE


# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

APP1TARGET= $(TARGET)

APP1OBJS=	\
    $(OBJ)$/canvasdemo.obj

APP1STDLIBS=$(TOOLSLIB) 		\
            $(COMPHELPERLIB)	\
            $(CPPULIB)			\
            $(CPPUHELPERLIB)	\
            $(UCBHELPERLIB)		\
            $(SALLIB)			\
            $(VCLLIB)			\
            $(BASEGFXLIB)

.IF "$(GUI)"!="UNX"
APP1DEF=	$(MISC)$/$(TARGET).def
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
