#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: sb $ $Date: 2002-07-23 13:03:09 $
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
#   Copyright: 2002 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ=..$/..

PRJNAME=SVTOOLS
TARGET=edit
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

.IF "$(header)" == ""

UNOUCRDEP = $(SOLARBINDIR)$/applicat.rdb
UNOUCRRDB = $(SOLARBINDIR)$/applicat.rdb
UNOUCROUT = $(OUT)$/inc
UNOTYPES = \
    com.sun.star.awt.FontWeight \
    com.sun.star.awt.XTextArea \
    com.sun.star.awt.XTextComponent \
    com.sun.star.awt.XTextLayoutConstrains \
    com.sun.star.lang.EventObject \
    drafts.com.sun.star.accessibility.AccessibleEventId \
    drafts.com.sun.star.accessibility.AccessibleRole \
    drafts.com.sun.star.accessibility.AccessibleStateType \
    drafts.com.sun.star.accessibility.AccessibleTextType \
    drafts.com.sun.star.accessibility.XAccessible \
    drafts.com.sun.star.accessibility.XAccessibleContext \
    drafts.com.sun.star.accessibility.XAccessibleEditableText \
    drafts.com.sun.star.accessibility.XAccessibleEventBroadcaster

SLOFILES=   \
            $(SLO)$/textdata.obj	\
            $(SLO)$/textdoc.obj		\
            $(SLO)$/texteng.obj		\
            $(SLO)$/textundo.obj	\
            $(SLO)$/textview.obj	\
            $(SLO)$/txtattr.obj		\
            $(SLO)$/xtextedt.obj	\
            $(SLO)$/sychconv.obj	\
            $(SLO)$/svmedit.obj		\
            $(SLO)$/svmedit2.obj \
            $(SLO)$/textwindowaccessibility.obj

OBJFILES=   \
            $(OBJ)$/textdata.obj	\
            $(OBJ)$/textdoc.obj		\
            $(OBJ)$/texteng.obj		\
            $(OBJ)$/textundo.obj	\
            $(OBJ)$/textview.obj	\
            $(OBJ)$/txtattr.obj		\
            $(OBJ)$/xtextedt.obj	\
            $(OBJ)$/sychconv.obj	\
            $(OBJ)$/svmedit.obj		\
            $(OBJ)$/svmedit2.obj \
            $(OBJ)$/textwindowaccessibility.obj

EXCEPTIONSFILES=	\
            $(SLO)$/textview.obj	\
            $(OBJ)$/textview.obj \
            $(SLO)$/textwindowaccessibility.obj \
            $(OBJ)$/textwindowaccessibility.obj

LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1OBJFILES=  $(SLOFILES)

LIB2TARGET= $(LB)$/$(TARGET).lib
LIB2OBJFILES=  $(OBJFILES)

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
