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

.ELSE

LIBWPD=$(LIBPRE) wpdlib.lib
LIBWPS=$(LIBPRE) wpslib.lib
LIBWPG=$(LIBPRE) wpglib.lib

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
    $(LIBWPS) \
    $(LIBWPG) \
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
    $(LIBWPG) \
    $(LIBWPD)

SHL3TARGET = wpgimport$(DLLPOSTFIX)
SHL3IMPLIB = i$(SHL3TARGET)
SHL3LIBS = $(LIB3TARGET)
SHL3VERSIONMAP = $(SOLARENV)/src/component.map
DEF3NAME = $(SHL3TARGET)

.INCLUDE :  target.mk

ALLTAR : $(MISC)/wpft.component

$(MISC)/wpft.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        wpft.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt wpft.component
