#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.49 $
#
#   last change: $Author: hjs $ $Date: 2001-09-03 16:12:05 $
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

PRJNAME=			framework
TARGET=				framework
TARGETTYPE=GUI

USE_DEFFILE=		TRUE
ENABLE_EXCEPTIONS=	TRUE
NO_BSYMBOLIC=		TRUE

COMP3TYPELIST=		fwl
COMP4TYPELIST=		fwk
COMP5TYPELIST=		lgd

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.IF "$(COM)"=="ICC"
LINKFLAGS+=/SEGMENTS:1024 /PACKD:32768
.ENDIF

# --- export library for sfx2 -------------------------------------------------

LIB1TARGET=		$(SLB)$/fweobj.lib

LIB1OBJFILES=	$(SLO)$/attributelist.obj					\
                $(SLO)$/bmkmenu.obj							\
                $(SLO)$/eventsconfiguration.obj				\
                $(SLO)$/eventsdocumenthandler.obj			\
                $(SLO)$/imageproducer.obj					\
                $(SLO)$/lockhelper.obj						\
                $(SLO)$/menuconfiguration.obj				\
                $(SLO)$/menudocumenthandler.obj				\
                $(SLO)$/saxnamespacefilter.obj				\
                $(SLO)$/statusbarconfiguration.obj			\
                $(SLO)$/statusbardocumenthandler.obj		\
                $(SLO)$/toolboxconfiguration.obj			\
                $(SLO)$/toolboxdocumenthandler.obj			\
                $(SLO)$/toolboxlayoutdocumenthandler.obj	\
                $(SLO)$/xmlnamespaces.obj

# --- internal import -------------------------------------------------

LIB2TARGET=		$(SLB)$/fwiobj.lib

LIB2OBJFILES=	$(SLO)$/wildcard.obj						\
                $(SLO)$/lockhelper.obj						\
                $(SLO)$/filtercache.obj						\
                $(SLO)$/filtercachedata.obj

# --- export classes library ---------------------------------------------------

SHL1TARGET=		fwe$(UPD)$(DLLPOSTFIX)

SHL1IMPLIB=		ifwe

SHL1LIBS= 		$(LIB1TARGET)

SHL1STDLIBS=	$(VCLLIB)							\
                $(CPPULIB)							\
                $(CPPUHELPERLIB)					\
                $(VOSLIB)							\
                $(SALLIB)							\
                $(SVLIB)							\
                $(SVLLIB)							\
                $(TOOLSLIB)							\
                $(COMPHELPERLIB)					\
                $(UNOTOOLSLIB)

SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

DEF1NAME=		$(SHL1TARGET)

DEFLIB1NAME=	fweobj
DEF1DEPN=		$(MISC)$/$(SHL1TARGET).flt

# --- import classes library ---------------------------------------------------

SHL2TARGET=		fwi$(UPD)$(DLLPOSTFIX)

SHL2IMPLIB=		ifwi

SHL2LIBS= 		$(LIB2TARGET)

SHL2STDLIBS=	$(CPPULIB)							\
                $(CPPUHELPERLIB)					\
                $(VOSLIB)							\
                $(SALLIB)							\
                $(TOOLSLIB)							\
                $(UNOTOOLSLIB)

SHL2DEF=		$(MISC)$/$(SHL2TARGET).def

DEF2NAME=		$(SHL2TARGET)

DEFLIB2NAME=	fwiobj
DEF2DEPN=		$(MISC)$/$(SHL2TARGET).flt

# --- light services library ----------------------------------------------------

SHL3TARGET=		fwl$(UPD)$(DLLPOSTFIX)

SHL3IMPLIB=		ifwl

SHL3OBJS=		$(SLO)$/argumentanalyzer.obj		\
                $(SLO)$/contenthandlerfactory.obj	\
                $(SLO)$/converter.obj				\
                $(SLO)$/filterfactory.obj			\
                $(SLO)$/frameloaderfactory.obj		\
                $(SLO)$/mediatypedetectionhelper.obj\
                $(SLO)$/registertemp.obj			\
                $(SLO)$/typedetection.obj			\
                $(SLO)$/lockhelper.obj

SHL3STDLIBS=	$(CPPULIB)							\
                $(CPPUHELPERLIB)					\
                $(COMPHELPERLIB)					\
                $(UNOTOOLSLIB)						\
                $(TOOLSLIB) 						\
                $(VOSLIB)							\
                $(SVLLIB)							\
                $(SALLIB)							\
                $(FWILIB)

SHL3DEF=		$(MISC)$/$(SHL3TARGET).def
SHL3DEPN=		$(SHL2IMPLIBN) $(SHL2TARGETN)

DEF3NAME=		$(SHL3TARGET)

DEF3EXPORTFILE=	exports.dxp

# --- services library ----------------------------------------------------

SHL4TARGET=		fwk$(UPD)$(DLLPOSTFIX)

SHL4IMPLIB=		ifwk

SHL4OBJS=       $(SLO)$/argumentanalyzer.obj		\
                $(SLO)$/asyncquit.obj				\
                $(SLO)$/basedispatcher.obj			\
                $(SLO)$/blankdispatcher.obj			\
                $(SLO)$/converter.obj				\
                $(SLO)$/createdispatcher.obj		\
                $(SLO)$/desktop.obj					\
                $(SLO)$/dispatchprovider.obj		\
                $(SLO)$/documentproperties.obj		\
                $(SLO)$/droptargetlistener.obj		\
                $(SLO)$/fltdlg.obj                  \
                $(SLO)$/frame.obj					\
                $(SLO)$/framecontainer.obj			\
                $(SLO)$/helpagentdispatcher.obj		\
                $(SLO)$/interceptionhelper.obj		\
                $(SLO)$/mailtodispatcher.obj		\
                $(SLO)$/menudispatcher.obj			\
                $(SLO)$/menumanager.obj				\
                $(SLO)$/ocomponentaccess.obj		\
                $(SLO)$/ocomponentenumeration.obj	\
                $(SLO)$/oframes.obj					\
                $(SLO)$/opluginframedispatcher.obj	\
                $(SLO)$/otasksaccess.obj			\
                $(SLO)$/otasksenumeration.obj		\
                $(SLO)$/pluginframe.obj				\
                $(SLO)$/registerservices.obj		\
                $(SLO)$/selfdispatcher.obj			\
                $(SLO)$/soundhandler.obj			\
                $(SLO)$/statusindicator.obj			\
                $(SLO)$/statusindicatorfactory.obj	\
                $(SLO)$/targetfinder.obj		  	\
                $(SLO)$/task.obj					\
                $(SLO)$/taskcreator.obj		  		\
                 $(SLO)$/timerhelper.obj				\
                $(SLO)$/urltransformer.obj			\
                $(SLO)$/xmldocproperties.obj

SHL4STDLIBS=	$(CPPULIB)							\
                $(CPPUHELPERLIB)					\
                $(VOSLIB)							\
                $(SALLIB)							\
                $(TOOLSLIB) 						\
                $(SVTOOLLIB)						\
                $(TKLIB)							\
                $(SVLIB)							\
                $(COMPHELPERLIB)					\
                $(UCBHELPERLIB)						\
                $(UNOTOOLSLIB)						\
                $(FWELIB)							\
                $(FWILIB)							\
                $(SVLLIB)							\
                $(SOTLIB)

SHL4DEF=		$(MISC)$/$(SHL4TARGET).def
SHL4DEPN=		$(SHL1IMPLIBN) $(SHL1TARGETN) $(SHL2IMPLIBN) $(SHL2TARGETN)

DEF4NAME=		$(SHL4TARGET)

DEF4EXPORTFILE=	exports.dxp

# --- login service library ----------------------------------------------------

SHL5TARGET=		lgd$(UPD)$(DLLPOSTFIX)

SHL5IMPLIB=		ilgd

SHL5OBJS=		$(SLO)$/registerlogindialog.obj		\
                 $(SLO)$/logindialog.obj				\
                $(SLO)$/lockhelper.obj

SHL5STDLIBS=	$(CPPULIB)							\
                $(CPPUHELPERLIB)					\
                $(VOSLIB)							\
                $(SALLIB)							\
                $(SVLIB)							\
                $(TOOLSLIB)

SHL5DEF=		$(MISC)$/$(SHL5TARGET).def

DEF5NAME=		$(SHL5TARGET)

DEF5EXPORTFILE=	exports.dxp

# --- login applikation -------------------------------------------------------

APP1TARGET= 	login

APP1OBJS=		$(SLO)$/login.obj					\
                $(SLO)$/servicemanager.obj			\
                $(SLO)$/lockhelper.obj

APP1STDLIBS=	$(CPPULIB)							\
                $(CPPUHELPERLIB)					\
                $(OSLLIB)							\
                $(SALLIB)							\
                $(VOSLIB)							\
                $(TOOLSLIB) 						\
                $(SVTOOLLIB)						\
                $(TKLIB)							\
                $(COMPHELPERLIB)					\
                $(SVLIB)

APP1DEPN=		$(SLO)$/servicemanager.obj

.IF "$(GUI)"=="WIN" || "$(GUI)"=="OS2"
APP1DEF=		$(MISC)$/login.def
.ENDIF

# --- Targets -----------------------------------------------------------------

.INCLUDE :	target.mk

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo _Impl>$@
    @echo WEP>>$@
    @echo m_pLoader>$@
    @echo _TI2>>$@
    @echo LIBMAIN>>$@
    @echo LibMain>>$@

$(MISC)$/$(SHL2TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo _Impl>$@
    @echo WEP>>$@
    @echo m_pLoader>$@
    @echo _TI2>>$@
    @echo _TI3>>$@
    @echo LIBMAIN>>$@
    @echo LibMain>>$@
    @echo _STL::pair>>$@
