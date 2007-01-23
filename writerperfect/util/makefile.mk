PRJ=..
PRJNAME=writerperfect
TARGET=writerperfect
VERSION=$(UPD)

.INCLUDE :  settings.mk

.IF "$(GUI)"=="UNX" || "$(GUI)"=="MAC"
.IF "$(SYSTEM_LIBWPD)" == "YES"
LIBWPD=$(LIBWPD_LIBS)
.ELSE
LIBWPD=-lwpdlib
.ENDIF
.ELSE
LIBWPD=$(LIBPRE) wpdlib.lib
.ENDIF

LIB1TARGET= $(SLB)$/wpft.lib
LIB1FILES= \
    $(SLB)$/stream.lib  \
    $(SLB)$/filter.lib  \
    $(SLB)$/wpdimp.lib
SHL1LIBS=$(LIB1TARGET)
SHL1STDLIBS+= \
    $(SVLLIB)	\
    $(SOTLIB) \
    $(SO2LIB) \
    $(SVTOOLLIB) \
    $(UNOTOOLSLIB) \
    $(TOOLSLIB) \
    $(COMPHELPERLIB) \
    $(UCBHELPERLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB) \
    $(XMLOFFLIB) \
    $(LIBWPD)

SHL1TARGET = wpft$(UPD)$(DLLPOSTFIX)
SHL1IMPLIB = i$(SHL1TARGET)
SHL1LIBS = $(LIB1TARGET)
SHL1VERSIONMAP=wpft.map
DEF1NAME=$(SHL1TARGET)

.INCLUDE :  target.mk
