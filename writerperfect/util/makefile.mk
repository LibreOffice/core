PRJ=..
PRJNAME=wpft
TARGET=wpft
VERSION=$(UPD)

.INCLUDE :  settings.mk

LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1FILES= \
    $(SLB)$/ooo.lib  \
    $(SLB)$/stream.lib  \
    $(SLB)$/filter.lib
SHL1LIBS=$(LIB1TARGET) 
SHL1STDLIBS+= \
    $(SVLLIB)	\
    $(SOTLIB) \
    $(SVXLIB) \
    $(SO2LIB) \
    $(SVTOOLLIB) \
    $(UNOTOOLSLIB) \
    $(TOOLSLIB) \
    $(COMPHELPERLIB) \
    $(UCBHELPERLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB) \
    -lwpd
SHL1TARGET = $(TARGET)$(UPD)$(DLLPOSTFIX)
SHL1IMPLIB = i$(SHL1TARGET)
SHL1LIBS = $(LIB1TARGET)
SHL1VERSIONMAP=$(TARGET).map
DEF1NAME=$(SHL1TARGET)

.INCLUDE :  target.mk
