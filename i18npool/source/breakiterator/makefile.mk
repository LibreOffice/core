#*************************************************************************
#*
#*  $RCSfile: makefile.mk,v $
#*
#*  $Revision: 1.4 $
#*
#*  last change: $Author: hr $ $Date: 2003-11-07 15:14:05 $
#*
#*  The Contents of this file are made available subject to the terms of
#*  either of the following licenses
#*
#*         - GNU Lesser General Public License Version 2.1
#*         - Sun Industry Standards Source License Version 1.1
#*
#*  Sun Microsystems Inc., October, 2000
#*
#*  GNU Lesser General Public License Version 2.1
#*  =============================================
#*  Copyright 2000 by Sun Microsystems, Inc.
#*  901 San Antonio Road, Palo Alto, CA 94303, USA
#*
#*  This library is free software; you can redistribute it and/or
#*  modify it under the terms of the GNU Lesser General Public
#*  License version 2.1, as published by the Free Software Foundation.
#*
#*  This library is distributed in the hope that it will be useful,
#*  but WITHOUT ANY WARRANTY; without even the implied warranty of
#*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#*  Lesser General Public License for more details.
#*
#*  You should have received a copy of the GNU Lesser General Public
#*  License along with this library; if not, write to the Free Software
#*  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#*  MA  02111-1307  USA
#*
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
#************************************************************************/
PRJ=..$/..

PRJNAME=i18npool
TARGET=breakiterator

TARGETTYPE=CUI
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# --- Files --------------------------------------------------------

MY_MISC_CXXFILES = \
        $(MISC)$/dict_word_brk.c \
        $(MISC)$/dict_word_ca_brk.c \
        $(MISC)$/edit_word_brk.c \
        $(MISC)$/count_word_brk.c \
        $(MISC)$/line_brk.c

SLOFILES=   \
        $(SLO)$/breakiteratorImpl.obj \
        $(SLO)$/breakiterator_cjk.obj \
        $(SLO)$/breakiterator_ctl.obj \
        $(SLO)$/breakiterator_th.obj \
        $(SLO)$/breakiterator_hi.obj \
        $(SLO)$/breakiterator_unicode.obj \
        $(SLO)$/xdictionary.obj \
        $(SLO)$/dict_word_brk.obj \
        $(SLO)$/dict_word_ca_brk.obj \
        $(SLO)$/edit_word_brk.obj \
        $(SLO)$/count_word_brk.obj \
        $(SLO)$/line_brk.obj

APP1TARGET = gendict

APP1OBJS   = $(OBJ)$/gendict.obj

APP1STDLIBS = $(SALLIB) \
        $(TOOLSLIB)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk


# 'genbrk' and 'genccode' are tools generated and delivered by icu project to process icu breakiterator rules.
$(MISC)$/%_brk.c : data/%.txt
    +$(WRAPCMD) $(SOLARBINDIR)$/genbrk -r $< -o $(MISC)$/$*.brk
    +$(WRAPCMD) $(SOLARBINDIR)$/genccode -d $(MISC)$ $(MISC)$/$*.brk
