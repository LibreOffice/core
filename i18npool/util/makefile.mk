#*************************************************************************
#*
#*  $RCSfile: makefile.mk,v $
#*
#*	makefile
#*
#*  Erstellung          HJS 17.07.00
#*
#*  Letzte Aenderung    $Author: bustamam $ $Date: 2001-08-30 23:33:17 $
#*
#*  $Revision: 1.2 $
#*
#*  $Source: /zpool/svn/migration/cvs_rep_09_09_08/code/i18npool/util/makefile.mk,v $
#*
#*  Copyright (c) 1997 - 1998, Star Division GmbH
#*
#*************************************************************************

PRJ=..

PRJNAME=i18npool
TARGET=i18npool
VERSION=$(UPD)
USE_LDUMP2=TRUE

# Make symbol renaming match library name for Mac OS X
.IF "$(OS)"=="MACOSX"
SYMBOLPREFIX=i18n$(UPD)$(DLLPOSTFIX)
.ENDIF  

# --- Settings -----------------------------------------------------------

.INCLUDE :	settings.mk

# --- Allgemein ----------------------------------------------------------

SHL1TARGET= 	localedata_ascii
SHL1IMPLIB= 	i$(SHL1TARGET)
SHL1LIBS=	$(SLB)$/$(SHL1TARGET).lib
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1NAME=	$(SHL1TARGET)
DEFLIB1NAME=	$(SHL1TARGET)
DEF1DEPN=	$(MISC)$/$(SHL1TARGET).flt

SHL2TARGET= 	localedata_CJK
SHL2IMPLIB= 	i$(SHL2TARGET)
SHL2LIBS=	$(SLB)$/$(SHL2TARGET).lib
SHL2DEF=	$(MISC)$/$(SHL2TARGET).def
DEF2NAME=	$(SHL2TARGET)
DEFLIB2NAME=	$(SHL2TARGET)
DEF2DEPN=	$(MISC)$/$(SHL2TARGET).flt

LIB3TARGET= $(SLB)$/$(TARGET).lib
LIB3FILES=	$(SLB)$/defaultnumberingprovider.lib	\
        $(SLB)$/registerservices.lib		\
        $(SLB)$/numberformatcode.lib		\
        $(SLB)$/localedata.lib

SHL3TARGET= $(TARGET)$(VERSION)$(DLLPOSTFIX)
SHL3IMPLIB= i$(TARGET)

SHL3STDLIBS=\
        $(UNOTOOLSLIB)				\
        $(TOOLSLIB)				\
        $(CPPULIB)				\
        $(COMPHELPERLIB)			\
        $(CPPUHELPERLIB)			\
        $(SALLIB)

SHL3LIBS=	$(LIB3TARGET)
SHL3DEF=	$(MISC)$/$(SHL3TARGET).def
DEF3NAME=	$(SHL3TARGET)
DEF3EXPORTFILE=	$(TARGET).dxp

.IF "$(GUI)"=="UNX"
.IF "$(OS)"!="MACOSX"
SHL3STDLIBS +=\
    -lX11 -lXt -lXmu
.ENDIF
.ENDIF

# --- Targets ------------------------------------------------------------

.INCLUDE :	target.mk

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo Provider>> $@

$(MISC)$/$(SHL2TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo Provider>> $@

$(MISC)$/$(SHL3TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo Provider>> $@
