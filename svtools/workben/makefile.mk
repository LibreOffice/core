#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 16:59:07 $
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

PRJNAME=SVTOOLS
TARGET=svdem
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :	svpre.mk
.INCLUDE :	settings.mk
.INCLUDE :	sv.mk

# --- Files --------------------------------------------------------

OBJFILES=	$(OBJ)$/svdem.obj
# removed, because these files aren't ported to Unicode
#			 $(OBJ)$/browser.obj  \
#			 $(OBJ)$/stest.obj	  \
#			 $(OBJ)$/grptest.obj

APP1TARGET= 	$(TARGET)
.IF "$(GUI)" != "MAC"
APP1STDLIBS=	$(SVTOOLLIB)	\
                $(SVLIB)		\
                $(TOOLSLIB) 	\
                $(SALLIB)		\
                $(VOSLIB)		\
                $(CPPULIB)
.ELSE
APP1STDLIBS=	$(SOLARBINDIR)$/TL$(UPD)$(DLLPOSTFIX).DLL \
                $(SOLARBINDIR)$/VCL$(UPD)$(DLLPOSTFIX).DLL \
                $(SOLARBINDIR)$/SVT$(UPD)$(DLLPOSTFIX).DLL
APP1LIBS=		$(SOLARLIBDIR)$/SALMAIN.MAC.LIB \
                $(SOLARLIBDIR)$/NOSHAREDMAIN.LIB
.ENDIF
.IF "$(GUI)" != "MAC"
APP1DEPN=		$(L)$/itools.lib $(SVLIBDEPEND) $(LB)$/_svt.lib
.ENDIF
APP1OBJS=		$(OBJ)$/svdem.obj
APP1STACK=		32768

# removed, because these files aren't ported to Unicode
#APP2TARGET= browser
.IF "$(GUI)" != "MAC"
#APP2STDLIBS=	 $(SVTOOLLIB)	 \
#				 $(SVLIB)		 \
#				 $(SALLIB)		 \
#				 $(TOOLSLIB)	 \
#				 $(VOSLIB)		 \
#				 $(CPPULIB)
.ELSE
#APP2STDLIBS=	 $(SOLARBINDIR)$/TL$(UPD)$(DLLPOSTFIX).DLL \
#				 $(SOLARBINDIR)$/VCL$(UPD)$(DLLPOSTFIX).DLL \
#				 $(SOLARBINDIR)$/SVT$(UPD)$(DLLPOSTFIX).DLL
#APP2LIBS=		 $(SOLARLIBDIR)$/SALMAIN.MAC.LIB \
#				 $(SOLARLIBDIR)$/NOSHAREDMAIN.LIB
.ENDIF
.IF "$(GUI)" != "MAC"
#APP2DEPN=		 $(LB)$/_svt.lib $(L)$/itools.lib $(SVLIBDEPEND)
.ENDIF
#APP2OBJS=		 $(OBJ)$/browser.obj
#APP2STACK= 	 32768

# removed, because these files aren't ported to Unicode
#APP3TARGET=	 stest
.IF "$(GUI)" != "MAC"
#APP3STDLIBS=	 $(SVMEMLIB)	 \
#				 $(SVTOOLLIB)	 \
#				 $(SVLIB)		 \
#				 $(SALLIB)		 \
#				 $(TOOLSLIB)	 \
#				 $(VOSLIB)		 \
#				 $(SVLLIB)		 \
#				 $(CPPULIB)
.ELSE
#APP3STDLIBS=	 $(SOLARBINDIR)$/TL$(UPD)$(DLLPOSTFIX).DLL \
#				 $(SOLARBINDIR)$/VCL$(UPD)$(DLLPOSTFIX).DLL \
#				 $(SOLARBINDIR)$/SVT$(UPD)$(DLLPOSTFIX).DLL
#APP3LIBS=		 $(SOLARLIBDIR)$/SALMAIN.MAC.LIB \
#				 $(SOLARLIBDIR)$/NOSHAREDMAIN.LIB
.ENDIF
.IF "$(GUI)" != "MAC"
#APP3DEPN=		 $(L)$/itools.lib $(SVLIBDEPEND) $(LB)$/svmem.lib $(LB)$/_svt.lib
.ENDIF
#APP3OBJS=		 $(OBJ)$/stest.obj
#APP3STACK= 	 32768

# removed, because these files aren't ported to Unicode
#APP4TARGET=	 grptest
.IF "$(GUI)" != "MAC"
#APP4STDLIBS=	 $(SVMEMLIB)	 \
#				 $(SVTOOLLIB)	 \
#				 $(SVLIB)		 \
#				 $(SALLIB)		 \
#				 $(TOOLSLIB)	 \
#				 $(VOSLIB)		 \
#				 $(CPPULIB)
.ELSE
#APP4STDLIBS=	 $(SOLARBINDIR)$/TL$(UPD)$(DLLPOSTFIX).DLL \
#				 $(SOLARBINDIR)$/VCL$(UPD)$(DLLPOSTFIX).DLL \
#				 $(SOLARBINDIR)$/SVT$(UPD)$(DLLPOSTFIX).DLL
#APP4LIBS=		 $(SOLARLIBDIR)$/SALMAIN.MAC.LIB \
#				 $(SOLARLIBDIR)$/NOSHAREDMAIN.LIB
.ENDIF
.IF "$(GUI)" != "MAC"
#APP4DEPN=		 $(L)$/itools.lib $(SVLIBDEPEND) $(LB)$/svmem.lib $(LB)$/_svt.lib
.ENDIF
#APP4OBJS=		 $(OBJ)$/grptest.obj
#APP4STACK= 	 32768


.IF "$(GUI)" == "MAC"
MACRES = $(SV_RES)SV.R $(SV_RES)SV_DEMO.R $(SV_RES)SV_POWER.R
.ENDIF

# --- Targets -------------------------------------------------------

.INCLUDE :	target.mk
