#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.44 $
#
#   last change: $Author: as $ $Date: 2001-07-31 06:58:46 $
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
USE_DEFFILE=		TRUE
ENABLE_EXCEPTIONS=	TRUE
NO_BSYMBOLIC=		TRUE
COMP1TYPELIST=		fwk
COMP2TYPELIST=		fwl
COMP3TYPELIST=		lgd

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

.IF "$(COM)"=="ICC"
LINKFLAGS+=/SEGMENTS:1024 /PACKD:32768
.ENDIF

# --- import library ----------------------------------------------------

LIB1TARGET=		$(SLB)$/fweobj.lib

LIB1OBJFILES=	$(SLO)$/menuconfiguration.obj				\
                $(SLO)$/attributelist.obj					\
                $(SLO)$/imageproducer.obj					\
                $(SLO)$/menudocumenthandler.obj				\
                $(SLO)$/bmkmenu.obj							\
                $(SLO)$/saxnamespacefilter.obj				\
                $(SLO)$/xmlnamespaces.obj					\
                $(SLO)$/toolboxconfiguration.obj			\
                $(SLO)$/toolboxdocumenthandler.obj			\
                $(SLO)$/lockhelper.obj						\
                $(SLO)$/toolboxlayoutdocumenthandler.obj	\
                $(SLO)$/statusbarconfiguration.obj			\
                $(SLO)$/statusbardocumenthandler.obj		\
                $(SLO)$/eventsconfiguration.obj				\
                $(SLO)$/eventsdocumenthandler.obj


# --- services library ----------------------------------------------------

SHL1TARGET=		fwk$(UPD)$(DLLPOSTFIX)

SHL1IMPLIB=		ifwk$(UPD)$(DLLPOSTFIX)

SHL1OBJS=		$(SLO)$/registerservices.obj		\
                $(SLO)$/frame.obj					\
                $(SLO)$/task.obj					\
                $(SLO)$/pluginframe.obj				\
                $(SLO)$/desktop.obj					\
                $(SLO)$/documentproperties.obj		\
                $(SLO)$/urltransformer.obj			\
                $(SLO)$/framecontainer.obj			\
                $(SLO)$/taskcreator.obj		  		\
                $(SLO)$/targetfinder.obj		  	\
                $(SLO)$/argumentanalyzer.obj		\
                $(SLO)$/converter.obj				\
                $(SLO)$/wildcard.obj				\
                $(SLO)$/asyncquit.obj				\
                $(SLO)$/oframes.obj					\
                $(SLO)$/opluginframedispatcher.obj	\
                $(SLO)$/ostatusindicatorfactory.obj	\
                $(SLO)$/ocomponentaccess.obj		\
                $(SLO)$/otasksaccess.obj			\
                $(SLO)$/ostatusindicator.obj		\
                $(SLO)$/otasksenumeration.obj		\
                $(SLO)$/ocomponentenumeration.obj	\
                $(SLO)$/interceptionhelper.obj		\
                $(SLO)$/dispatchprovider.obj		\
                $(SLO)$/basedispatcher.obj			\
                $(SLO)$/blankdispatcher.obj			\
                $(SLO)$/createdispatcher.obj		\
                $(SLO)$/selfdispatcher.obj			\
                $(SLO)$/menudispatcher.obj			\
                $(SLO)$/mailtodispatcher.obj		\
                $(SLO)$/helpagentdispatcher.obj		\
                 $(SLO)$/timerhelper.obj				\
                $(SLO)$/menumanager.obj				\
                $(SLO)$/xmldocproperties.obj		\
                $(SLO)$/fltdlg.obj                  \
                $(SLO)$/droptargetlistener.obj		\
                $(SLO)$/soundhandler.obj

SHL1STDLIBS=	$(CPPULIB)							\
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
                $(SVLLIB)							\
                $(SOTLIB)

SHL1DEF=		$(MISC)$/$(SHL1TARGET).def
SHL1DEPN=		$(SHL4IMPLIBN) $(SHL4TARGETN)

DEF1NAME=		$(SHL1TARGET)

DEF1EXPORTFILE=	exports.dxp

# --- light services library ----------------------------------------------------

SHL2TARGET=		fwl$(UPD)$(DLLPOSTFIX)

SHL2IMPLIB=		ifwl$(UPD)$(DLLPOSTFIX)

SHL2OBJS=		$(SLO)$/registertemp.obj			\
                $(SLO)$/mediatypedetectionhelper.obj\
                $(SLO)$/frameloaderfactory.obj		\
                $(SLO)$/contenthandlerfactory.obj	\
                $(SLO)$/filterfactory.obj			\
                $(SLO)$/typedetection.obj			\
                $(SLO)$/filtercachedata.obj			\
                $(SLO)$/filtercache.obj				\
                $(SLO)$/argumentanalyzer.obj		\
                $(SLO)$/converter.obj				\
                $(SLO)$/wildcard.obj				\
                $(SLO)$/lockhelper.obj

SHL2STDLIBS=	$(CPPULIB)							\
                $(CPPUHELPERLIB)					\
                $(COMPHELPERLIB)					\
                $(UNOTOOLSLIB)						\
                $(TOOLSLIB) 						\
                $(VOSLIB)							\
                $(SVLLIB)							\
                $(SALLIB)

SHL2DEF=		$(MISC)$/$(SHL2TARGET).def

DEF2NAME=		$(SHL2TARGET)

DEF2EXPORTFILE=	exports.dxp

# --- login service library ----------------------------------------------------

SHL3TARGET=		lgd$(UPD)$(DLLPOSTFIX)

SHL3IMPLIB=		ilgd$(UPD)$(DLLPOSTFIX)

SHL3OBJS=		$(SLO)$/registerlogindialog.obj		\
                 $(SLO)$/logindialog.obj				\
                $(SLO)$/lockhelper.obj

SHL3STDLIBS=	$(CPPULIB)							\
                $(CPPUHELPERLIB)					\
                $(VOSLIB)							\
                $(SALLIB)							\
                $(SVLIB)							\
                $(TOOLSLIB)

SHL3DEF=		$(MISC)$/$(SHL3TARGET).def

DEF3NAME=		$(SHL3TARGET)

DEF3EXPORTFILE=	exports.dxp

# --- export classes library ---------------------------------------------------

SHL4TARGET=		fwe$(UPD)$(DLLPOSTFIX)

SHL4IMPLIB=		ifwe

SHL4LIBS= 		$(LIB1TARGET)

SHL4STDLIBS=	$(VCLLIB)							\
                $(CPPULIB)							\
                $(CPPUHELPERLIB)					\
                $(VOSLIB)							\
                $(SALLIB)							\
                $(SVLIB)							\
                $(SVLLIB)							\
                $(TOOLSLIB)							\
                $(COMPHELPERLIB)					\
                $(UNOTOOLSLIB)

SHL4DEF=		$(MISC)$/$(SHL4TARGET).def

DEF4NAME=		$(SHL4TARGET)

DEFLIB4NAME=	fweobj
DEF4DEPN=		$(MISC)$/$(SHL4TARGET).flt

# --- login applikation --------------------------------------------------------

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

# --- framework applikation --------------------------------------------------------
# --- not build yet!        --------------------------------------------------------

#APP2TARGET= 	framework
#
#APP2OBJS=		$(SLO)$/framework.obj
#
#APP2LIBS=		$(SLB)$/fwk_classes.lib				\
#				$(SLB)$/fwk_helper.lib				\
#				$(SLB)$/fwk_services.lib
#
#APP2STDLIBS=	$(CPPULIB)							\
#				$(CPPUHELPERLIB)					\
#				$(OSLLIB)							\
#				$(SALLIB)							\
#				$(VOSLIB)							\
#				$(TOOLSLIB) 						\
#				$(SVTOOLLIB)						\
#				$(SVLLIB)							\
#				$(TKLIB)							\
#				$(COMPHELPERLIB)					\
#				$(SOTLIB)							\
#				$(SVLIB)
#
#APP2DEPN=		$(SLB)$/fwk_helper.lib				\
#				$(SLB)$/fwk_classes.lib
#
#.IF "$(GUI)"=="WIN" || "$(GUI)"=="OS2"
#APP2DEF=		$(MISC)$/framework.def
#.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

$(MISC)$/$(SHL4TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo _Impl>$@
    @echo WEP>>$@
    @echo m_pLoader>$@
    @echo _TI2>>$@
    @echo LIBMAIN>>$@
    @echo LibMain>>$@
