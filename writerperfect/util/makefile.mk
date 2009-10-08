PRJ=..
PRJNAME=writerperfect
TARGET=writerperfect

.INCLUDE :  settings.mk

.IF "$(GUI)"=="UNX"
.IF "$(SYSTEM_LIBWPD)" == "YES"
LIBWPD=$(LIBWPD_LIBS)
.ELSE
LIBWPD=-lwpdlib
.ENDIF
.ELSE
.IF "$(GUI)$(COM)"=="WNTGCC"
LIBWPD=-lwpdlib
.ELSE
 LIBWPD=$(LIBPRE) wpdlib.lib
.ENDIF
.ENDIF

LIB1TARGET= $(SLB)$/wpft.lib
LIB1FILES= \
    $(SLB)$/stream.lib  \
    $(SLB)$/filter.lib  \
    $(SLB)$/wpdimp.lib
SHL1LIBS=$(LIB1TARGET)
SHL1STDLIBS+= \
    $(SOTLIB) \
    $(SO2LIB) \
    $(UNOTOOLSLIB) \
    $(TOOLSLIB) \
    $(UCBHELPERLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB) \
    $(XMLOFFLIB) \
    $(LIBWPD)

SHL1TARGET = wpft$(DLLPOSTFIX)
SHL1IMPLIB = i$(SHL1TARGET)
SHL1LIBS = $(LIB1TARGET)
SHL1VERSIONMAP=wpft.map
DEF1NAME=$(SHL1TARGET)

.INCLUDE :  target.mk
