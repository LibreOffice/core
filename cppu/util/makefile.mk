#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 15:25:54 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************
PRJ=..

PRJNAME=cppu
TARGET=cppu
USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE
NO_BSYMBOLIC=TRUE

# --- Settings -----------------------------------------------------
.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

.INCLUDE :  ..$/version.mk

# --- Files --------------------------------------------------------

SHL1LIBS= \
        $(SLB)$/cppu_typelib.lib	\
        $(SLB)$/cppu_uno.lib		\
        $(SLB)$/cppu_threadpool.lib

SHL1TARGET=	$(CPPU_TARGET)$(CPPU_MAJOR)

SHL1STDLIBS=	$(SALLIB)

SHL1DEPN=
SHL1IMPLIB=	i$(TARGET)
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

DOCPPFILES= \
        $(INC)$/typelib$/uik.h			\
        $(INC)$/typelib$/typeclass.h		\
        $(INC)$/typelib$/typedescription.h	\
        $(INC)$/typelib$/typedescription.hxx	\
        $(INC)$/uno$/any2.h			\
        $(INC)$/uno$/sequence2.h		\
         $(INC)$/uno$/data.h			\
         $(INC)$/uno$/environment.h		\
         $(INC)$/uno$/environment.hxx		\
         $(INC)$/uno$/mapping.h			\
         $(INC)$/uno$/mapping.hxx		\
        $(INC)$/uno$/dispatcher.h		\
        $(INC)$/uno$/threadpool.h		\
        $(INC)$/com$/sun$/star$/uno$/genfunc.h	\
        $(INC)$/com$/sun$/star$/uno$/Reference.h\
        $(INC)$/com$/sun$/star$/uno$/Type.h	\
        $(INC)$/com$/sun$/star$/uno$/Any.h	\
        $(INC)$/com$/sun$/star$/uno$/Sequence.h

docpp: $(DOCPPFILES)
    +doc++ -H -m -f -u -d $(OUT)$/doc$/$(PRJNAME) $(DOCPPFILES)
