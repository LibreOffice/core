#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.17 $
#
#   last change: $Author: nf $ $Date: 2001-08-03 14:41:09 $
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

PRJNAME=transex
TARGET=transex3
TARGETTYPE=CUI
NO_DEFAULT_STL=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  static.mk

CDEFS+= -DYY_NEVER_INTERACTIVE=1

# --- Files --------------------------------------------------------

OBJFILES=   			\
    $(OBJ)$/export.obj	\
    $(OBJ)$/export2.obj	\
    $(OBJ)$/merge.obj   \
    $(OBJ)$/wrdtrans.obj	\
    $(OBJ)$/wtratree.obj	\
    $(OBJ)$/wtranode.obj    \
    $(OBJ)$/srciter.obj		\
    $(OBJ)$/utf8conv.obj

LIB1TARGET= $(LB)$/transex.lib
LIB1ARCHIV= $(LB)$/libtransex.a
LIB1FILES=  $(LB)$/transex3.lib


# extractor and merger for *.src and *.hrc
APP1TARGET=	$(TARGET)
APP1STACK=	16000
APP1OBJS=   $(OBJ)$/src_yy.obj 
APP1STDLIBS+=$(BTSTRPLIB) $(STATIC_LIBS) 
APP1LIBS+=	$(LB)$/$(TARGET).lib
APP1DEPN=   $(OBJ)$/src_yy.obj $(LB)$/$(TARGET).lib

#APP2TARGET= termilo
#APP2STACK=  16000
#APP2OBJS=   $(OBJ)$/termino.obj
#APP2STDLIBS=$(STATIC_LIBS) $(L)$/bootstrp.lib

# extractor and merger for *.lng and *.lng
APP3TARGET= lngex
APP3STACK=  16000
APP3OBJS=   $(OBJ)$/lngmerge.obj $(OBJ)$/merge.obj $(OBJ)$/export2.obj $(OBJ)$/lngex.obj $(OBJ)$/utf8conv.obj
APP3STDLIBS=$(BTSTRPLIB) $(STATIC_LIBS)

# encoding converter for *.gsi
APP4TARGET= gsiconv
APP4STACK=  16000
APP4OBJS=   $(OBJ)$/utf8conv.obj $(OBJ)$/gsiconv.obj
APP4STDLIBS=$(STATIC_LIBS)

# tag checker for *.gsi
APP5TARGET= gsicheck
APP5STACK=  16000
APP5OBJS=   $(OBJ)$/gsicheck.obj $(OBJ)$/tagtest.obj
APP5STDLIBS=$(STATIC_LIBS)

# extractor and merger for *.xxl and *.xrb
APP6TARGET= xmlex
APP6STACK=  16000
APP6OBJS=   $(OBJ)$/xmlmerge.obj $(OBJ)$/xml_yy.obj $(OBJ)$/merge.obj $(OBJ)$/export2.obj $(OBJ)$/utf8conv.obj
APP6STDLIBS=$(BTSTRPLIB) $(STATIC_LIBS)

# extractor and merger for *.cfg
APP7TARGET= cfgex
APP7STACK=  16000
APP7OBJS=   $(OBJ)$/cfgmerge.obj $(OBJ)$/cfg_yy.obj $(OBJ)$/merge.obj $(OBJ)$/export2.obj $(OBJ)$/utf8conv.obj
APP7STDLIBS=$(BTSTRPLIB) $(STATIC_LIBS)

APP8TARGET= xgfconv
APP8STACK=  16000
APP8OBJS=   $(OBJ)$/utf8conv.obj $(OBJ)$/xgfconv.obj $(OBJ)$/export2.obj
APP8STDLIBS=$(BTSTRPLIB) $(STATIC_LIBS) 

# encoding converter for text files
#APP9TARGET= txtconv
#APP9STACK=  16000
#APP9OBJS=   $(OBJ)$/utf8conv.obj $(OBJ)$/txtconv.obj
#APP9STDLIBS=$(STATIC_LIBS)

# localizer for l10n framework
APP9TARGET= localize
APP9STACK=  16000
APP9OBJS=   $(OBJ)$/localize.obj $(OBJ)$/utf8conv.obj $(OBJ)$/srciter.obj $(OBJ)$/export2.obj
APP9STDLIBS+=$(BTSTRPLIB) $(STATIC_LIBS) 
#APP9LIBS+=	$(LB)$/$(TARGET).lib

DEPOBJFILES=$(APP1OBJS) $(APP2OBJS) $(APP3OBJS) $(APP4OBJS) $(APP5OBJS) $(APP6OBJS) $(APP7OBJS) $(APP8OBJS) $(APP9OBJS)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

ALLTAR : 	$(MISC)$/src_yy.c 	\
            $(MISC)$/xml_yy.c	\
            $(MISC)$/cfg_yy.c

$(MISC)$/src_yy.c : srclex.l
    +flex -l -8 -o$(MISC)$/src_yy.c srclex.l

$(MISC)$/xml_yy.c : xmllex.l
    +flex -l -8 -o$(MISC)$/xml_yy.c xmllex.l

$(MISC)$/cfg_yy.c : cfglex.l
    +flex -l -8 -o$(MISC)$/cfg_yy.c cfglex.l
