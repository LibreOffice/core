#*************************************************************************
#*
#*  $RCSfile: makefile.mk,v $
#*
#*  $Revision: 1.2 $
#*
#*  last change: $Author: vg $ $Date: 2003-04-24 12:26:02 $
#*
#*  The Contents of this file are made available subject to the terms of
#*  either of the following licenses
#*
#*         - Sun Industry Standards Source License Version 1.1
#*
#*  Sun Microsystems Inc., October, 2000
#*
#*  Sun Industry Standards Source License Version 1.1
#*  =================================================
#*  The contents of this file are subject to the Sun Industry Standards
#*  Source License Version 1.1 (the "License"); You may not use this file
#*  except in compliance with the License. You may obtain a copy of the
#*  License at http://www.openoffice.org/license.html.
#*
#*  Software provided under this License is provided on an "AS IS" basis,
#*  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#*  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#*  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#*  See the License for the specific provisions governing your rights and
#*  obligations concerning the Software.
#*
#*  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#*
#*  Copyright: 2000 by Sun Microsystems, Inc.
#*
#*  All Rights Reserved.
#*
#*  Contributor(s): _______________________________________
#*
#*
#************************************************************************
PRJ=..$/..

PRJNAME=i18nutil
TARGET=i18nutil
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

EXCEPTIONSFILES= \
    $(SLO)$/casefolding.obj

SLOFILES= \
    $(EXCEPTIONSFILES) \
    $(SLO)$/unicode.obj \
    $(SLO)$/widthfolding.obj \
    $(SLO)$/oneToOneMapping.obj

# Unicode utilities 
SHL1TARGET=		$(TARGET)$(COMID)
SHL1IMPLIB=		i$(TARGET)

DEF1DEPN=		$(MISC)$/$(SHL1TARGET).flt
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def
DEF1NAME=		$(SHL1TARGET)
DEFLIB1NAME=	$(SHL1TARGET)

SHL1OBJS=		$(SLOFILES)

LIB1TARGET=	$(SLB)$/$(SHL1TARGET).lib
LIB1OBJFILES=$(SHL1OBJS)

SHL1STDLIBS= \
        $(CPPULIB) \
        $(SALLIB)

# --- Targets ------------------------------------------------------
.INCLUDE :  target.mk

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo CLEAR_THE_FILE > $@
    @echo __CT >> $@

