PRJ=..$/..

PRJNAME=testtools
TARGET=performancetest
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE
ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# ------------------------------------------------------------------

UNOUCRDEP=$(SOLARBINDIR)$/udkapi.rdb 
UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb

UNOUCROUT=$(OUT)$/inc
INCPRE+=$(OUT)$/inc

UNOTYPES= \
        com.sun.star.uno.TypeClass		\
        com.sun.star.uno.XAggregation		\
        com.sun.star.uno.XWeak			\
        com.sun.star.lang.XInitialization		\
        com.sun.star.lang.XTypeProvider		\
        com.sun.star.lang.XServiceInfo		\
        com.sun.star.lang.XSingleServiceFactory	\
        com.sun.star.lang.XMultiServiceFactory	\
        com.sun.star.lang.XComponent		\
        com.sun.star.lang.XMain			\
        com.sun.star.loader.XImplementationLoader \
        com.sun.star.registry.XRegistryKey	\
        com.sun.star.bridge.XUnoUrlResolver	\
        com.sun.star.container.XSet		\
        com.sun.star.test.performance.XPerformanceTest \
        com.sun.star.lang.XSingleComponentFactory	\
        com.sun.star.uno.XComponentContext          

SLOFILES= \
        $(SLO)$/ubtest.obj	\
        $(SLO)$/ubobject.obj

# ---- test ----

LIB1TARGET=$(SLB)$/perftest.lib
LIB1OBJFILES= \
        $(SLO)$/ubtest.obj

SHL1TARGET=perftest
SHL1STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALLIB)

SHL1LIBS=	$(LIB1TARGET)
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1NAME=	$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

# ---- test object ----

LIB2TARGET=$(SLB)$/perfobj.lib
LIB2OBJFILES= \
        $(SLO)$/ubobject.obj

SHL2TARGET=perfobj
SHL2STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALLIB)

SHL2LIBS=	$(LIB2TARGET)
SHL2DEF=	$(MISC)$/$(SHL2TARGET).def
DEF2NAME=	$(SHL2TARGET)
DEF2EXPORTFILE=	exports.dxp

# ---- pseudo uno bridge ----

LIB3TARGET=$(SLB)$/pseudo.lib
LIB3OBJFILES= \
        $(SLO)$/pseudo.obj

SHL3TARGET=pseudo_uno_uno
SHL3STDLIBS= \
        $(CPPULIB)		\
        $(SALLIB)

SHL3LIBS=	$(LIB3TARGET)
SHL3DEF=	$(MISC)$/$(SHL3TARGET).def
DEF3NAME=	$(SHL3TARGET)
DEF3EXPORTFILE=	pseudo.dxp

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
