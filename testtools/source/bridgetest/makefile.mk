PRJ=..$/..

PRJNAME=testtools
TARGET=bridgetest
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE
ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# ------------------------------------------------------------------
.IF "$(GUI)"=="WNT"
MY_DLLPOSTFIX=.dll
MY_DLLPREFIX=
DESTDIR=$(BIN)
BATCH_SUFFIX=.bat
GIVE_EXEC_RIGHTS=@echo
.ELSE
MY_DLLPOSTFIX=.so
MY_DLLPREFIX=lib
DESTDIR=$(OUT)$/lib
BATCH_INPROCESS=bridgetest_inprocess
GIVE_EXEC_RIGHTS=chmod +x 
.ENDIF

UNOUCRDEP=$(SOLARBINDIR)$/udkapi.rdb 
UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb

UNOUCROUT=$(OUT)$/inc
INCPRE+=$(OUT)$/inc

UNOTYPES= \
        com.sun.star.uno.TypeClass		\
        com.sun.star.uno.XAggregation		\
        com.sun.star.uno.XWeak			\
        com.sun.star.lang.XTypeProvider		\
        com.sun.star.lang.XServiceInfo		\
        com.sun.star.lang.XSingleServiceFactory	\
        com.sun.star.lang.XMultiServiceFactory	\
        com.sun.star.lang.XComponent		\
        com.sun.star.lang.XMain			\
        com.sun.star.loader.XImplementationLoader \
        com.sun.star.registry.XRegistryKey	\
        com.sun.star.container.XSet		\
        com.sun.star.test.bridge.XBridgeTest 	\
        com.sun.star.bridge.XUnoUrlResolver		\
        com.sun.star.lang.XSingleComponentFactory	\
        com.sun.star.uno.XComponentContext          

SLOFILES= \
        $(SLO)$/cppobj.obj	\
        $(SLO)$/bridgetest.obj

# ---- test ----

LIB1TARGET=$(SLB)$/cppobj.lib
LIB1OBJFILES= \
        $(SLO)$/cppobj.obj

SHL1TARGET=cppobj
SHL1STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALLIB)

SHL1LIBS=	$(LIB1TARGET)
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1NAME=	$(SHL1TARGET)
SAL1VERSIONMAP=$(SOLARENV)$/src$/component.map

# ---- test object ----

LIB2TARGET=$(SLB)$/bridgetest.lib
LIB2OBJFILES= \
        $(SLO)$/bridgetest.obj

SHL2TARGET=bridgetest
SHL2STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALLIB)

SHL2LIBS=	$(LIB2TARGET)
SHL2DEF=	$(MISC)$/$(SHL2TARGET).def
DEF2NAME=	$(SHL2TARGET)
SAL2VERSIONMAP=$(SOLARENV)$/src$/component.map

# --- Targets ------------------------------------------------------
ALL : \
        ALLTAR \
        $(DESTDIR)$/uno_types.rdb \
        $(DESTDIR)$/uno_services.rdb \
        $(DESTDIR)$/bridgetest_inprocess$(BATCH_SUFFIX) \
        $(DESTDIR)$/bridgetest_server$(BATCH_SUFFIX) \
        $(DESTDIR)$/bridgetest_client$(BATCH_SUFFIX) \


.INCLUDE :	target.mk

$(DESTDIR)$/uno_types.rdb : $(SOLARBINDIR)$/udkapi.rdb
    echo $(DESTDIR)
    $(GNUCOPY) -p $? $@

$(DESTDIR)$/bridgetest_inprocess$(BATCH_SUFFIX) : bridgetest_inprocess
    $(GNUCOPY) -p $? $@
    $(GIVE_EXEC_RIGHTS) $@

$(DESTDIR)$/bridgetest_client$(BATCH_SUFFIX) : bridgetest_client
    $(GNUCOPY) -p $? $@
    $(GIVE_EXEC_RIGHTS) $@

$(DESTDIR)$/bridgetest_server$(BATCH_SUFFIX) : bridgetest_server
    $(GNUCOPY) -p $? $@
    $(GIVE_EXEC_RIGHTS) $@

# I can't make a dependency on shared libraries, because dependent targets
# get the .setdir current directory. AAARGGGGGG !
$(DESTDIR)$/uno_services.rdb .SETDIR=$(DESTDIR) : 
    regcomp -register -r uno_services.rdb \
        -c $(MY_DLLPREFIX)bridgetest$(MY_DLLPOSTFIX)	\
        -c $(MY_DLLPREFIX)cppobj$(MY_DLLPOSTFIX)		\
        -c $(MY_DLLPREFIX)connectr$(MY_DLLPOSTFIX)		\
        -c $(MY_DLLPREFIX)acceptor$(MY_DLLPOSTFIX)		\
        -c $(MY_DLLPREFIX)brdgfctr$(MY_DLLPOSTFIX)		\
        -c $(MY_DLLPREFIX)remotebridge$(MY_DLLPOSTFIX)	\
        -c $(MY_DLLPREFIX)uuresolver$(MY_DLLPOSTFIX)	
