#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.74 $
#
#   last change: $Author: vg $ $Date: 2003-10-06 17:01:49 $
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
GEN_HID=            TRUE
GEN_HID_OTHER=      TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- internal import -------------------------------------------------

LIB1TARGET=     $(SLB)$/fwiobj.lib

LIB1OBJFILES=   $(SLO)$/wildcard.obj                            \
                $(SLO)$/argumentanalyzer.obj                    \
                $(SLO)$/converter.obj                           \
                $(SLO)$/lockhelper.obj                          \
                $(SLO)$/transactionmanager.obj                  \
                $(SLO)$/filtercache.obj                         \
                $(SLO)$/filtercachedata.obj                     \
                $(SLO)$/protocolhandlercache.obj				\
                $(SLO)$/networkdomain.obj                       \
                $(SLO)$/configaccess.obj                        \
                $(SLO)$/framelistanalyzer.obj

# --- export library for sfx2 -------------------------------------------------

LIB2TARGET=     $(SLB)$/fweobj.lib

LIB2OBJFILES=   $(SLO)$/attributelist.obj                       \
                $(SLO)$/bmkmenu.obj                             \
                $(SLO)$/eventsconfiguration.obj                 \
                $(SLO)$/eventsdocumenthandler.obj               \
                $(SLO)$/imageproducer.obj                       \
                $(SLO)$/lockhelper.obj                          \
                $(SLO)$/menuconfiguration.obj                   \
                $(SLO)$/menudocumenthandler.obj                 \
                $(SLO)$/saxnamespacefilter.obj                  \
                $(SLO)$/statusbarconfiguration.obj              \
                $(SLO)$/statusbardocumenthandler.obj            \
                $(SLO)$/toolboxconfiguration.obj                \
                $(SLO)$/toolboxdocumenthandler.obj              \
                $(SLO)$/toolboxlayoutdocumenthandler.obj        \
                $(SLO)$/imagesconfiguration.obj                 \
                $(SLO)$/imagesdocumenthandler.obj               \
                $(SLO)$/xmlnamespaces.obj                       \
                $(SLO)$/actiontriggerpropertyset.obj            \
                $(SLO)$/actiontriggerseparatorpropertyset.obj   \
                $(SLO)$/actiontriggercontainer.obj              \
                $(SLO)$/propertysetcontainer.obj                \
                $(SLO)$/rootactiontriggercontainer.obj          \
                $(SLO)$/actiontriggerhelper.obj                 \
                $(SLO)$/imagewrapper.obj        \
        $(SLO)$/interaction.obj		\
        $(SLO)$/addonmenu.obj		\
        $(SLO)$/addonsoptions.obj	\
        $(SLO)$/fwkresid.obj		\
        $(SLO)$/acceleratorinfo.obj


# --- import classes library ---------------------------------------------------

SHL1TARGET=     fwi$(UPD)$(DLLPOSTFIX)

SHL1IMPLIB=     ifwi

SHL1LIBS=       $(LIB1TARGET)

SHL1STDLIBS=    \
                $(UNOTOOLSLIB)	\
                $(TOOLSLIB)		\
                $(VOSLIB)		\
                $(CPPULIB)		\
                $(SALLIB)

.IF "$(GUI)"=="WNT"
SHL1STDLIBS+=\
        uwinapi.lib \
        unicows.lib \
        advapi32.lib \
        kernel32.lib
.ENDIF

SHL1DEF=        $(MISC)$/$(SHL1TARGET).def

DEF1NAME=       $(SHL1TARGET)

DEFLIB1NAME=    fwiobj
DEF1DEPN=       $(MISC)$/$(SHL1TARGET).flt


# --- export classes library ---------------------------------------------------

SHL2TARGET=     fwe$(UPD)$(DLLPOSTFIX)

SHL2IMPLIB=     ifwe

SHL2LIBS=       $(LIB2TARGET)

SHL2STDLIBS=    \
                $(VCLLIB)                           \
                $(SVLLIB)							\
                $(UNOTOOLSLIB)                      \
                $(TOOLSLIB)							\
                $(COMPHELPERLIB)					\
                $(CPPUHELPERLIB)					\
                $(CPPULIB)							\
                $(VOSLIB)							\
                $(SALLIB)

SHL2DEF=        $(MISC)$/$(SHL2TARGET).def

DEF2NAME=       $(SHL2TARGET)

DEFLIB2NAME=    fweobj
DEF2DEPN=       $(MISC)$/$(SHL2TARGET).flt

# --- light services library ----------------------------------------------------

SHL3TARGET=		fwl$(UPD)$(DLLPOSTFIX)

SHL3IMPLIB=		ifwl

SHL3OBJS=       $(SLO)$/contenthandlerfactory.obj   \
                $(SLO)$/detectorfactory.obj         \
                $(SLO)$/filterfactory.obj			\
                $(SLO)$/frameloaderfactory.obj		\
                $(SLO)$/mediatypedetectionhelper.obj\
                $(SLO)$/registertemp.obj			\
                $(SLO)$/typedetection.obj			\
                $(SLO)$/substitutepathvars.obj		\
                $(SLO)$/pathsettings.obj

SHL3STDLIBS=	\
                $(FWILIB)							\
                $(SVLLIB)							\
                $(UNOTOOLSLIB)						\
                $(TOOLSLIB) 						\
                $(CPPUHELPERLIB)					\
                $(CPPULIB)							\
                $(VOSLIB)							\
                $(SALLIB)

SHL3DEF=        $(MISC)$/$(SHL3TARGET).def
SHL3DEPN=       $(SHL1IMPLIBN) $(SHL1TARGETN)

DEF3NAME=       $(SHL3TARGET)

SHL3VERSIONMAP= exports.map

# --- services library ----------------------------------------------------

SHL4TARGET=		fwk$(UPD)$(DLLPOSTFIX)

SHL4IMPLIB=		ifwk

SHL4OBJS=       $(SLO)$/basedispatcher.obj          \
                $(SLO)$/blankdispatcher.obj			\
                $(SLO)$/createdispatcher.obj		\
                $(SLO)$/desktop.obj					\
                $(SLO)$/dispatchprovider.obj		\
                $(SLO)$/documentproperties.obj		\
                $(SLO)$/droptargetlistener.obj		\
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
                $(SLO)$/pluginframe.obj				\
                $(SLO)$/registerservices.obj		\
                $(SLO)$/selfdispatcher.obj			\
                $(SLO)$/closedispatcher.obj         \
                $(SLO)$/soundhandler.obj			\
                $(SLO)$/statusindicator.obj			\
                $(SLO)$/statusindicatorfactory.obj	\
                $(SLO)$/targetfinder.obj		  	\
                $(SLO)$/taskcreator.obj		  		\
                $(SLO)$/timerhelper.obj				\
                $(SLO)$/urltransformer.obj			\
                $(SLO)$/documentlist.obj            \
                $(SLO)$/xmldocproperties.obj        \
                $(SLO)$/configaccess.obj            \
                $(SLO)$/job.obj                     \
                $(SLO)$/jobdata.obj                 \
                $(SLO)$/jobdispatch.obj             \
                $(SLO)$/jobexecutor.obj             \
                $(SLO)$/jobresult.obj               \
                $(SLO)$/joburl.obj                  \
                $(SLO)$/dispatchrecordersupplier.obj\
                $(SLO)$/dispatchrecorder.obj		\
                $(SLO)$/servicehandler.obj          \
                $(SLO)$/stillinteraction.obj		\
                $(SLO)$/restricteduiinteraction.obj \
                $(SLO)$/loadeventlistener.obj		\
                $(SLO)$/asyncloadthread.obj         \
                $(SLO)$/componentloader.obj         \
                $(SLO)$/persistentwindowstate.obj   \
                $(SLO)$/colorlistener.obj           \
                $(SLO)$/backingcomp.obj				\
                $(SLO)$/dispatchhelper.obj

SHL4STDLIBS=	\
                $(FWELIB)							\
                $(FWILIB)                           \
                $(SVTOOLLIB)						\
                $(TKLIB)							\
                $(VCLLIB)							\
                $(SVLLIB)							\
                $(SOTLIB)							\
                $(UNOTOOLSLIB)						\
                $(TOOLSLIB) 						\
                $(COMPHELPERLIB)					\
                $(CPPUHELPERLIB)					\
                $(CPPULIB)							\
                $(VOSLIB)							\
                $(SALLIB)

SHL4DEF=		$(MISC)$/$(SHL4TARGET).def
SHL4DEPN=		$(SHL1IMPLIBN) $(SHL1TARGETN) $(SHL2IMPLIBN) $(SHL2TARGETN)

DEF4NAME=		$(SHL4TARGET)

SHL4VERSIONMAP= exports.map

# --- login service library ----------------------------------------------------

SHL5TARGET=		lgd$(UPD)$(DLLPOSTFIX)

SHL5IMPLIB=		ilgd

SHL5OBJS=		$(SLO)$/registerlogindialog.obj		\
                $(SLO)$/logindialog.obj

SHL5STDLIBS=	$(CPPULIB)							\
                $(CPPUHELPERLIB)					\
                $(VOSLIB)							\
                $(SALLIB)							\
                $(SVLIB)							\
                $(TOOLSLIB)                         \
                $(FWILIB)

SHL5DEF=		$(MISC)$/$(SHL5TARGET).def
SHL5DEPN=       $(SHL1IMPLIBN) $(SHL1TARGETN)

DEF5NAME=		$(SHL5TARGET)

SHL5VERSIONMAP= exports.map

# --- Targets -----------------------------------------------------------------

.INCLUDE :	target.mk

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
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

$(MISC)$/$(SHL2TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo _Impl>$@
    @echo WEP>>$@
    @echo m_pLoader>$@
    @echo _TI2>>$@
    @echo LIBMAIN>>$@
    @echo LibMain>>$@
