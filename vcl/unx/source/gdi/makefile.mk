#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: hdu $ $Date: 2001-02-15 15:30:00 $
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

PRJNAME=vcl
TARGET=salgdi

.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

.IF "$(OS)"=="MACOSX"

dummy:
    @echo "Nothing to build for Mac OS X"

.ELSE		# "$(OS)"=="MACOSX"

.IF "$(remote)"==""
SLOFILES=	\
        $(SLO)$/salgdi2.obj		\
        $(SLO)$/salbmp.obj		\
        $(SLO)$/salgdi.obj		\
        $(SLO)$/salvd.obj		\
        $(SLO)$/salogl.obj		\
        $(SLO)$/charnames.obj	\
        $(SLO)$/dtint.obj		\
        $(SLO)$/kdeint.obj		\
        $(SLO)$/salcvt.obj		\
        $(SLO)$/ansi1252.obj	\
        $(SLO)$/xfont.obj		\
        $(SLO)$/xlfd_attr.obj	\
        $(SLO)$/xlfd_extd.obj	\
        $(SLO)$/xlfd_smpl.obj	\
        $(SLO)$/salgdi3.obj

.IF "$(PSPRINT)" != ""
SLOFILES+=$(SLO)$/salprnpsp.obj
.ELIF "$(USE_XPRINT)" == "TRUE"
CFLAGS+=-D_USE_PRINT_EXTENSION_=1
SLOFILES+=$(SLO)$/xprintext.obj
.ELSE
SLOFILES+=\
        $(SLO)$/salprn.obj 		\
        $(SLO)$/salpimpl.obj 	\
        $(SLO)$/salconfig.obj
.ENDIF

.IF "$(OS)"=="SOLARIS"
SLOFILES+=$(SLO)$/cdeint.obj
ENVCFLAGS+=-DUSE_CDE
.ENDIF

.IF "$(USE_BUILTIN_RASTERIZER)" != ""
SLOFILES+=	$(SLO)$/gcach_xpeer.obj
.ENDIF

.ENDIF

.ENDIF	# "$(OS)"=="MACOSX"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

.INCLUDE :  $(PRJ)$/util$/target.pmk

XSALSETLIBNAME=libspa$(UPD)$(DLLPOSTFIX).so

$(INCCOM)$/rtsname.hxx:
    rm -f $(INCCOM)$/rtsname.hxx ; \
    echo "#define _XSALSET_LIBNAME "\"$(XSALSETLIBNAME)\" > $(INCCOM)$/rtsname.hxx

$(SLO)$/salpimpl.obj : $(INCCOM)$/rtsname.hxx
$(SLO)$/salprnpsp.obj : $(INCCOM)$/rtsname.hxx

