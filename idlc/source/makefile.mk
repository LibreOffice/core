#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1 $
#
#   last change: $Author: jsc $ $Date: 2001-03-15 12:30:43 $
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

PRJNAME=idlc
TARGET=idlc
TARGETTYPE=CUI
LIBTARGET=NO

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------


.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk


# --- Files --------------------------------------------------------

CXXFILES=   \
            $(MISC)$/scanner.cxx \
            $(MISC)$/parser.cxx	\
            idlcmain.cxx	\
            idlc.cxx	\
            idlccompile.cxx	\
            idlcproduce.cxx	\
            errorhandler.cxx	\
            options.cxx	\
            fehelper.cxx	\
            astdeclaration.cxx \
            astscope.cxx \
            aststack.cxx \
            astdump.cxx \
            astinterface.cxx \
            aststruct.cxx \
            astoperation.cxx \
            astconstant.cxx \
            astenum.cxx \
            astarray.cxx \
            astunion.cxx \
            astexpression.cxx

OBJFILES=   \
            $(OBJ)$/scanner.obj	\
            $(OBJ)$/parser.obj	\
            $(OBJ)$/idlcmain.obj	\
            $(OBJ)$/idlc.obj	\
            $(OBJ)$/idlccompile.obj	\
            $(OBJ)$/idlcproduce.obj	\
            $(OBJ)$/errorhandler.obj	\
            $(OBJ)$/options.obj	\
            $(OBJ)$/fehelper.obj	\
            $(OBJ)$/astdeclaration.obj	\
            $(OBJ)$/astscope.obj	\
            $(OBJ)$/aststack.obj	\
            $(OBJ)$/astdump.obj	\
            $(OBJ)$/astinterface.obj	\
            $(OBJ)$/aststruct.obj	\
            $(OBJ)$/astoperation.obj	\
            $(OBJ)$/astconstant.obj	\
            $(OBJ)$/astenum.obj	\
            $(OBJ)$/astarray.obj	\
            $(OBJ)$/astunion.obj	\
            $(OBJ)$/astexpression.obj

APP1TARGET= $(TARGET)
APP1OBJS=   $(OBJFILES)

APP1STDLIBS=\
        $(SALLIB)	\
        $(SALHELPERLIB)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk


$(MISC)$/scanner.cxx:	scanner.ll
    +flex -o$(MISC)$/scanner.cxx scanner.ll

$(MISC)$/parser.cxx: parser.yy
    +bison -v -d -o$(MISC)$/parser.cxx parser.yy
    +$(COPY) $(MISC)$/parser.cxx.h $(OUT)$/inc$/parser.h
# with line statements (for debugging)
#	+bison -v -d -o$(MISC)$/parser.cxx parser.yy
