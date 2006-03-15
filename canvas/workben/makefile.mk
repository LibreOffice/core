PRJ=..

PRJNAME=canvas
TARGET=canvasdemo
TARGETTYPE=GUI
LIBTARGET=NO

ENABLE_EXCEPTIONS=TRUE


# --- Settings -----------------------------------------------------

.INCLUDE :	svpre.mk
.INCLUDE :	settings.mk
.INCLUDE :	sv.mk

# --- Files --------------------------------------------------------

CXXFILES=	canvasdemo.cxx

OBJFILES=	$(OBJ)$/canvasdemo.obj

APP1TARGET= $(TARGET)
APP1OBJS=	$(OBJFILES)
APP1STDLIBS=$(TOOLSLIB) 		\
            $(OSLLIB)			\
            $(ONELIB)			\
            $(VOSLIB)			\
            $(SO2LIB)			\
            $(SVTOOLLIB)		\
            $(SVLLIB)			\
            $(COMPHELPERLIB)	\
            $(CPPULIB)			\
            $(CPPUHELPERLIB)	\
            $(UCBHELPERLIB)		\
            $(SALLIB)			\
            $(VCLLIB)			\
            $(GOODIESLIB)		\
            $(SFXLIB)			\
            $(SOTLIB)			\
            $(DLLIB)			\
            $(SVLIB)			\
            $(SVXLIB)

.IF "$(GUI)"!="UNX"
APP1DEF=	$(MISC)$/$(TARGET).def
.ENDIF


.IF "$(COM)"=="GCC"
ADDOPTFILES=$(OBJ)$/canvasdemo.obj
add_cflagscxx="-frtti -fexceptions"
.ENDIF


# --- Targets ------------------------------------------------------

ALL : \
    ALLTAR \
    $(BIN)$/canvasdemo.rdb 

.INCLUDE :	target.mk

$(BIN)$/canvasdemo.rdb : makefile.mk $(UNOUCRRDB)
    rm -f $@
    $(GNUCOPY) $(UNOUCRRDB) $@
    +cd $(BIN) && \
        regcomp -register -r canvasdemo.rdb \
            -c i18nsearch.uno$(DLLPOST) \
            -c i18npool.uno$(DLLPOST) \
            -c configmgr2.uno$(DLLPOST) \
            -c servicemgr.uno$(DLLPOST) \
            -c $(DLLPRE)fwl$(UPD)$(DLLSUFFIX)$(DLLPOST) \
            -c $(DLLPRE)ucpfile1$(DLLPOST) \
            -c $(DLLPRE)fileacc$(DLLPOST) \
            -c $(DLLPRE)ucb1$(DLLPOST) \
            -c canvasfactory.uno$(DLLPOST) \
            -c vclcanvas.uno$(DLLPOST)
