#*************************************************************************
#*
#*  $RCSfile: makefile.mk,v $
#*
#*	makefile
#*
#*  Erstellung          HJS 17.07.00
#*
#*  Letzte Aenderung    $Author: bustamam $ $Date: 2001-08-29 21:25:22 $
#*
#*  $Revision: 1.1 $
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


.IF "$(GUI)"=="UNX"
.IF "$(OS)"!="MACOSX"
SHL1STDLIBS +=\
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
