#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.121 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
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

LIB1OBJFILES=   \
                $(SLO)$/converter.obj                           \
                $(SLO)$/lockhelper.obj                          \
                $(SLO)$/transactionmanager.obj                  \
                $(SLO)$/protocolhandlercache.obj				\
                $(SLO)$/networkdomain.obj                       \
                $(SLO)$/configaccess.obj                        \
                $(SLO)$/framelistanalyzer.obj                   \
                $(SLO)$/shareablemutex.obj                      \
                $(SLO)$/itemcontainer.obj                       \
                $(SLO)$/rootitemcontainer.obj                   \
                $(SLO)$/constitemcontainer.obj                  \
                $(SLO)$/jobconst.obj                            \
                $(SLO)$/propertysethelper.obj



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
                $(SLO)$/imagewrapper.obj                        \
                $(SLO)$/interaction.obj							\
                $(SLO)$/addonmenu.obj							\
                $(SLO)$/addonsoptions.obj						\
                $(SLO)$/fwkresid.obj						    \
                $(SLO)$/acceleratorinfo.obj        	            \
                $(SLO)$/sfxhelperfunctions.obj                  \
                $(SLO)$/uielementwrapperbase.obj                \
                $(SLO)$/uiconfigelementwrapperbase.obj          \
                $(SLO)$/configimporter.obj                      \
                $(SLO)$/menuextensionsupplier.obj               \
                $(SLO)$/preventduplicateinteraction.obj         \
                $(SLO)$/titlehelper.obj

# --- import classes library ---------------------------------------------------

SHL1TARGET=     fwi$(DLLPOSTFIX)

SHL1IMPLIB=     ifwi

SHL1LIBS=       $(LIB1TARGET)

SHL1STDLIBS=    \
                $(UNOTOOLSLIB)	    \
                $(CPPUHELPERLIB)	\
                $(TOOLSLIB)		    \
                $(VOSLIB)		    \
                $(CPPULIB)		    \
                $(SALLIB)

.IF "$(GUI)"=="WNT"
SHL1STDLIBS+=\
         $(UWINAPILIB) \
         $(UNICOWSLIB) \
         $(ADVAPI32LIB) \
         $(KERNEL32LIB)
.ENDIF

SHL1DEF=        $(MISC)$/$(SHL1TARGET).def

DEF1NAME=       $(SHL1TARGET)

DEFLIB1NAME=    fwiobj
DEF1DEPN=       $(MISC)$/$(SHL1TARGET).flt


# --- export classes library ---------------------------------------------------

SHL2TARGET=     fwe$(DLLPOSTFIX)

SHL2IMPLIB=     ifwe

SHL2LIBS=       $(LIB2TARGET)

SHL2STDLIBS=    \
                $(FWILIB)							\
                $(VCLLIB)                           \
                $(SVLLIB)                           \
                $(SVTOOLLIB)						\
                $(UNOTOOLSLIB)                      \
                $(TOOLSLIB)							\
                $(COMPHELPERLIB)					\
                $(CPPUHELPERLIB)					\
                $(CPPULIB)							\
                $(VOSLIB)							\
                $(SALLIB)

SHL2DEF=        $(MISC)$/$(SHL2TARGET).def
SHL2DEPN=       $(SHL1IMPLIBN) $(SHL1TARGETN)

DEF2NAME=       $(SHL2TARGET)

DEFLIB2NAME=    fweobj
DEF2DEPN=       $(MISC)$/$(SHL2TARGET).flt

# --- light services library ----------------------------------------------------

SHL3TARGET=		fwl$(DLLPOSTFIX)

SHL3IMPLIB=		ifwl

SHL3OBJS=       $(SLO)$/mediatypedetectionhelper.obj\
                $(SLO)$/registertemp.obj			\
                $(SLO)$/substitutepathvars.obj		\
                $(SLO)$/pathsettings.obj

SHL3STDLIBS=	\
                $(FWILIB)							\
                $(SVLLIB)							\
                $(UNOTOOLSLIB)						\
                $(TOOLSLIB) 						\
                $(I18NISOLANGLIB)   				\
                $(COMPHELPERLIB)					\
                $(CPPUHELPERLIB)					\
                $(COMPHELPERLIB)					\
                $(CPPULIB)							\
                $(VOSLIB)							\
                $(SALLIB)

SHL3DEF=        $(MISC)$/$(SHL3TARGET).def
SHL3DEPN=       $(SHL1IMPLIBN) $(SHL1TARGETN)

DEF3NAME=       $(SHL3TARGET)

SHL3VERSIONMAP= exports.map

# --- services library ----------------------------------------------------

SHL4TARGET=		fwk$(DLLPOSTFIX)

SHL4IMPLIB=		ifwk

SHL4OBJS=       \
                $(SLO)$/desktop.obj					\
                $(SLO)$/loaddispatcher.obj          \
                $(SLO)$/dispatchprovider.obj		\
                $(SLO)$/dispatchinformationprovider.obj		\
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
                $(SLO)$/registerservices.obj		\
                $(SLO)$/closedispatcher.obj         \
                $(SLO)$/statusindicator.obj			\
                $(SLO)$/statusindicatorfactory.obj	\
                $(SLO)$/targetfinder.obj		  	\
                $(SLO)$/taskcreator.obj		  		\
                $(SLO)$/timerhelper.obj				\
                $(SLO)$/titlebarupdate.obj			\
                $(SLO)$/urltransformer.obj			\
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
                $(SLO)$/persistentwindowstate.obj   \
                $(SLO)$/colorlistener.obj           \
                $(SLO)$/backingcomp.obj				\
                $(SLO)$/backingwindow.obj			\
                $(SLO)$/dispatchhelper.obj          \
                $(SLO)$/layoutmanager.obj           \
                $(SLO)$/menubarmanager.obj          \
                $(SLO)$/menubarwrapper.obj          \
                $(SLO)$/popupmenucontrollerfactory.obj\
                $(SLO)$/uielementfactorymanager.obj \
                $(SLO)$/popupmenucontrollerbase.obj \
                $(SLO)$/fontmenucontroller.obj      \
                $(SLO)$/fontsizemenucontroller.obj  \
                $(SLO)$/objectmenucontroller.obj    \
                $(SLO)$/headermenucontroller.obj    \
                $(SLO)$/footermenucontroller.obj    \
                $(SLO)$/controlmenucontroller.obj   \
                $(SLO)$/macrosmenucontroller.obj   \
                $(SLO)$/dockingareadefaultacceptor.obj  \
                $(SLO)$/uicommanddescription.obj    \
                $(SLO)$/modulemanager.obj           \
                $(SLO)$/uiconfigurationmanager.obj  \
                $(SLO)$/moduleuicfgsupplier.obj     \
                $(SLO)$/moduleuiconfigurationmanager.obj \
                $(SLO)$/menubarfactory.obj          \
                $(SLO)$/filter.obj                  \
                $(SLO)$/frameloader.obj             \
                $(SLO)$/contenthandler.obj          \
                $(SLO)$/containerquery.obj          \
                $(SLO)$/loadenv.obj                 \
                $(SLO)$/targethelper.obj            \
                $(SLO)$/keymapping.obj              \
                $(SLO)$/storageholder.obj                \
                $(SLO)$/acceleratorcache.obj                \
                $(SLO)$/acceleratorconfigurationreader.obj  \
                $(SLO)$/acceleratorconfigurationwriter.obj  \
                $(SLO)$/acceleratorconfiguration.obj        \
                $(SLO)$/presethandler.obj                   \
                $(SLO)$/globalacceleratorconfiguration.obj  \
                $(SLO)$/moduleacceleratorconfiguration.obj  \
                $(SLO)$/documentacceleratorconfiguration.obj \
                $(SLO)$/license.obj                 \
                $(SLO)$/windowstateconfiguration.obj    \
                $(SLO)$/toolboxfactory.obj          \
                $(SLO)$/addonstoolboxfactory.obj    \
                $(SLO)$/toolbarwrapper.obj          \
                $(SLO)$/toolbarmanager.obj          \
                $(SLO)$/generictoolbarcontroller.obj    \
                $(SLO)$/toolbar.obj                 \
                $(SLO)$/addonstoolbarwrapper.obj    \
                $(SLO)$/addonstoolbarmanager.obj    \
                $(SLO)$/toolbarsmenucontroller.obj  \
                $(SLO)$/toolbarcontrollerfactory.obj\
                $(SLO)$/loaddispatchlistener.obj    \
                $(SLO)$/autorecovery.obj			\
                $(SLO)$/statusbarcontrollerfactory.obj\
                $(SLO)$/moduleimagemanager.obj      \
                $(SLO)$/imagemanager.obj            \
                $(SLO)$/graphicnameaccess.obj       \
                $(SLO)$/statusbarwrapper.obj        \
                $(SLO)$/statusbar.obj               \
                $(SLO)$/statusbarmanager.obj        \
                $(SLO)$/statusbarfactory.obj			\
                $(SLO)$/recentfilesmenucontroller.obj   \
                $(SLO)$/progressbarwrapper.obj          \
                $(SLO)$/statusindicatorinterfacewrapper.obj \
                $(SLO)$/uicategorydescription.obj       \
                $(SLO)$/vclstatusindicator.obj          \
                $(SLO)$/wakeupthread.obj               \
                $(SLO)$/sessionlistener.obj         \
                $(SLO)$/newmenucontroller.obj          \
                $(SLO)$/taskcreatorsrv.obj              \
                $(SLO)$/logoimagestatusbarcontroller.obj  \
                $(SLO)$/logotextstatusbarcontroller.obj   \
                $(SLO)$/globalsettings.obj                \
                $(SLO)$/simpletextstatusbarcontroller.obj \
                $(SLO)$/complextoolbarcontroller.obj      \
                $(SLO)$/comboboxtoolbarcontroller.obj     \
                $(SLO)$/imagebuttontoolbarcontroller.obj  \
                $(SLO)$/togglebuttontoolbarcontroller.obj \
                $(SLO)$/buttontoolbarcontroller.obj       \
                $(SLO)$/spinfieldtoolbarcontroller.obj    \
                $(SLO)$/edittoolbarcontroller.obj         \
                $(SLO)$/dropdownboxtoolbarcontroller.obj  \
                $(SLO)$/uriabbreviation.obj               \
                $(SLO)$/popupmenudispatcher.obj           \
                $(SLO)$/windowcommanddispatch.obj         \
                $(SLO)$/menubarmerger.obj                 \
                $(SLO)$/toolbarmerger.obj                 \
                $(SLO)$/langselectionstatusbarcontroller.obj \
                $(SLO)$/langselectionmenucontroller.obj   \
                $(SLO)$/tagwindowasmodified.obj

SHL4STDLIBS=	\
                $(FWILIB)                           \
                $(FWELIB)							\
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
                $(SALLIB)					        \
                $(UCBHELPERLIB)                     \
                $(I18NISOLANGLIB)

SHL4DEF=		$(MISC)$/$(SHL4TARGET).def
SHL4DEPN=		$(SHL1IMPLIBN) $(SHL1TARGETN) $(SHL2IMPLIBN) $(SHL2TARGETN)

DEF4NAME=		$(SHL4TARGET)

SHL4VERSIONMAP= exports.map

# --- services library ----------------------------------------------------

SHL5TARGET=		fwm$(DLLPOSTFIX)

SHL5IMPLIB=		ifwm

SHL5OBJS=       \
                $(SLO)$/helponstartup.obj   \
                $(SLO)$/tabwinfactory.obj   \
                $(SLO)$/tabwindow.obj       \
                $(SLO)$/systemexec.obj      \
                $(SLO)$/shelljob.obj        \
                $(SLO)$/register3rdcomponents.obj

SHL5STDLIBS=	\
                $(FWILIB)			\
                $(TKLIB)			\
                $(VCLLIB)			\
                $(TOOLSLIB) 		\
                $(COMPHELPERLIB)	\
                $(CPPUHELPERLIB)	\
                $(CPPULIB)			\
                $(SALLIB)

SHL5DEF=		$(MISC)$/$(SHL5TARGET).def
SHL5DEPN=		$(SHL1IMPLIBN) $(SHL1TARGETN)

DEF5NAME=		$(SHL5TARGET)

SHL5VERSIONMAP= exports.map
            
RESLIB1NAME=		fwe
RESLIB1IMAGES=		$(PRJ)$/res
RESLIB1SRSFILES= 	$(SRS)$/fwk_classes.srs \
                    $(SRS)$/fwk_services.srs \

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
