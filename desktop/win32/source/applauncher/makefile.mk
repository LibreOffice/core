#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: rt $ $Date: 2004-09-09 09:17:03 $
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

PRJ=..$/..$/..

PRJNAME=desktop
TARGET=applauncher
LIBTARGET=NO
TARGETTYPE=GUI

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

APP1TARGET=so$/swriter
APP1NOSAL=TRUE
APP1LINKRES=$(MISC)$/$(TARGET).res
APP1ICON=$(SOLARRESDIR)$/icons/002_text_document.ico
APP1OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/swriter.obj


APP2TARGET=so$/scalc
APP2NOSAL=TRUE
APP2LINKRES=$(MISC)$/$(TARGET).res
APP2ICON=$(SOLARRESDIR)$/icons/004_spreadsheet_document.ico
APP2OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/scalc.obj

APP3TARGET=so$/sdraw
APP3NOSAL=TRUE
APP3LINKRES=$(MISC)$/$(TARGET).res
APP3ICON=$(SOLARRESDIR)$/icons/006_drawing_document.ico
APP3OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/sdraw.obj

APP4TARGET=so$/simpress
APP4NOSAL=TRUE
APP4LINKRES=$(MISC)$/$(TARGET).res
APP4ICON=$(SOLARRESDIR)$/icons/008_presentation_document.ico
APP4OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/simpress.obj

APP5TARGET=so$/sbase
APP5NOSAL=TRUE
APP5LINKRES=$(MISC)$/$(TARGET).res
APP5ICON=$(SOLARRESDIR)$/icons/014_database_document.ico
APP5OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/sbase.obj

APP6TARGET=so$/smath
APP6NOSAL=TRUE
APP6LINKRES=$(MISC)$/$(TARGET).res
APP6ICON=$(SOLARRESDIR)$/icons/015_math_document.ico
APP6OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/smath.obj

APP7TARGET=swriter
APP7NOSAL=TRUE
APP7LINKRES=$(MISC)$/$(TARGET).res
APP7ICON=$(SOLARRESDIR)$/icons/ooo_writer.ico
APP7OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/swriter.obj


APP8TARGET=scalc
APP8NOSAL=TRUE
APP8LINKRES=$(MISC)$/$(TARGET).res
APP8ICON=$(SOLARRESDIR)$/icons/ooo_calc.ico
APP8OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/scalc.obj

APP9TARGET=sdraw
APP9NOSAL=TRUE
APP9LINKRES=$(MISC)$/$(TARGET).res
APP9ICON=$(SOLARRESDIR)$/icons/ooo_draw.ico
APP9OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/sdraw.obj

APP10TARGET=simpress
APP10NOSAL=TRUE
APP10LINKRES=$(MISC)$/$(TARGET).res
APP10ICON=$(SOLARRESDIR)$/icons/ooo_impress.ico
APP10OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/simpress.obj

#APP11TARGET=sbase
#APP11NOSAL=TRUE
#APP11LINKRES=$(MISC)$/$(TARGET).res
#APP11ICON=$(SOLARRESDIR)$/icons/ooo_base.ico
#APP11OBJS = \
#		$(OBJ)$/launcher.obj\
#       $(OBJ)$/sbase.obj

APP12TARGET=smath
APP12NOSAL=TRUE
APP12LINKRES=$(MISC)$/$(TARGET).res
APP12ICON=$(SOLARRESDIR)$/icons/ooo_math.ico
APP12OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/smath.obj

# --- Targets ------------------------------------------------------


.INCLUDE :  target.mk

