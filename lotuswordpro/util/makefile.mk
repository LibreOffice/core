PRJ=..
PRJNAME=lotuswordpro
TARGET=lwpft
VERSION=$(UPD)

.INCLUDE :  settings.mk

LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1FILES= \
    $(SLB)$/filter.lib  \
    $(SLB)$/xfilter.lib
SHL1LIBS=$(LIB1TARGET)
SHL1STDLIBS+= \
    $(TOOLSLIB) \
    $(COMPHELPERLIB) \
    $(UCBHELPERLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB) \
    $(SFXLIB) \
    $(SOTLIB) \
    $(VCLLIB) \
    $(ICUUCLIB) \
    $(ICUINLIB) \
    $(ICULELIB) \
    $(SVXCORELIB) \
    $(GOODIESLIB) \
    $(SVTOOLLIB) \
    $(XMLOFFLIB)

SHL1TARGET = $(TARGET)$(DLLPOSTFIX)
SHL1IMPLIB = i$(SHL1TARGET)
SHL1LIBS = $(LIB1TARGET)
SHL1VERSIONMAP=$(TARGET).map
DEF1NAME=$(SHL1TARGET)

.INCLUDE :  target.mk

ALLTAR : $(MISC)/lwpfilter.component $(MISC)/qa_lwpfilter.component

$(MISC)/lwpfilter.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        lwpfilter.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt lwpfilter.component

$(MISC)/qa_lwpfilter.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        lwpfilter.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_INBUILD_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt lwpfilter.component
