PRJ=..
PRJNAME=writerperfect
TARGET=writerperfect

.INCLUDE :  settings.mk


.IF "$(GUI)"=="UNX" || "$(GUI)$(COM)"=="WNTGCC"

.IF "$(SYSTEM_LIBWPD)" == "YES"
LIBWPD=$(LIBWPD_LIBS)
.ELSE
LIBWPD=-lwpdlib
.ENDIF

.IF "$(SYSTEM_LIBWPS)" == "YES"
LIBWPS=$(LIBWPS_LIBS)
.ELSE
LIBWPS=-lwpslib
.ENDIF

.IF "$(SYSTEM_LIBWPG)" == "YES"
LIBWPG=$(LIBWPG_LIBS)
.ELSE
LIBWPG=-lwpglib
.ENDIF

.IF "$(SYSTEM_LIBVISIO)" == "YES"
LIBVISIO=$(LIBVISIO_LIBS)
.ELSE
LIBVISIO=-lvisiolib
.ENDIF

.ELSE

LIBWPD=$(LIBPRE) wpdlib.lib
LIBWPS=$(LIBPRE) wpslib.lib
LIBWPG=$(LIBPRE) wpglib.lib
LIBVISIO=$(LIBPRE) visiolib.lib

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
    $(SFXLIB) \
    $(VCLLIB) \
    $(UNOTOOLSLIB) \
    $(TOOLSLIB) \
    $(UCBHELPERLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB) \
    $(XMLOFFLIB) \
    $(LIBWPG) \
    $(LIBWPD)

SHL1TARGET = wpft$(DLLPOSTFIX)
SHL1IMPLIB = i$(SHL1TARGET)
SHL1LIBS = $(LIB1TARGET)
SHL1VERSIONMAP=$(SOLARENV)/src/component.map
DEF1NAME=$(SHL1TARGET)


LIB2TARGET= $(SLB)$/msworks.lib
LIB2FILES= \
    $(SLB)$/stream.lib  \
    $(SLB)$/filter.lib  \
    $(SLB)$/wpsimp.lib
SHL2LIBS=$(LIB2TARGET)
SHL2STDLIBS+= \
    $(SVLLIB) \
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
    $(LIBWPS) \
    $(LIBWPD)

SHL2TARGET = msworks$(DLLPOSTFIX)
SHL2IMPLIB = i$(SHL2TARGET)
SHL2LIBS = $(LIB2TARGET)
SHL2VERSIONMAP = $(SOLARENV)/src/component.map
DEF2NAME = $(SHL2TARGET)

LIB3TARGET= $(SLB)$/wpgimport.lib
LIB3FILES= \
    $(SLB)$/stream.lib  \
    $(SLB)$/filter.lib  \
    $(SLB)$/wpgimp.lib
SHL3LIBS=$(LIB3TARGET)
SHL3STDLIBS+= \
    $(SVLLIB) \
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
    $(LIBWPG) \
    $(LIBWPD)

SHL3TARGET = wpgimport$(DLLPOSTFIX)
SHL3IMPLIB = i$(SHL3TARGET)
SHL3LIBS = $(LIB3TARGET)
SHL3VERSIONMAP = $(SOLARENV)/src/component.map
DEF3NAME = $(SHL3TARGET)

LIB4TARGET= $(SLB)$/visioimport.lib
LIB4FILES= \
    $(SLB)$/stream.lib  \
    $(SLB)$/filter.lib  \
    $(SLB)$/vsdimp.lib
SHL4LIBS=$(LIB4TARGET)
SHL4STDLIBS+= \
    $(SVLLIB) \
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
    $(LIBVISIO) \
    $(LIBWPD)

SHL4TARGET = visioimport$(DLLPOSTFIX)
SHL4IMPLIB = i$(SHL4TARGET)
SHL4LIBS = $(LIB4TARGET)
SHL4VERSIONMAP = $(SOLARENV)/src/component.map
DEF4NAME = $(SHL4TARGET)

.INCLUDE :  target.mk

ALLTAR : $(MISC)/wpft.component $(MISC)/wpgfilter.component $(MISC)/msworksfilter.component $(MISC)/visiofilter.component

$(MISC)/wpft.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        wpft.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt wpft.component

$(MISC)/msworksfilter.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        msworksfilter.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL2TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt msworksfilter.component

$(MISC)/wpgfilter.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        wpgfilter.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL3TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt wpgfilter.component

$(MISC)/visiofilter.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        visiofilter.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL4TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt visiofilter.component
