#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.82 $
#
#   last change: $Author: kz $ $Date: 2004-08-02 13:17:41 $
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
                $(SLO)$/moduleidentifier.obj



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
                $(SLO)$/uiconfigelementwrapperbase.obj

# --- import classes library ---------------------------------------------------

SHL1TARGET=     fwi$(UPD)$(DLLPOSTFIX)

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
                $(FWILIB)							\
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

SHL3OBJS=       $(SLO)$/mediatypedetectionhelper.obj\
                $(SLO)$/registertemp.obj			\
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

SHL4OBJS=       \
                $(SLO)$/desktop.obj					\
                $(SLO)$/loaddispatcher.obj          \
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
                $(SLO)$/registerservices.obj		\
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
                $(SLO)$/persistentwindowstate.obj   \
                $(SLO)$/colorlistener.obj           \
                $(SLO)$/backingcomp.obj				\
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
                $(SLO)$/license.obj                 \
                $(SLO)$/targethelper.obj            \
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
                $(SLO)$/moduleimagemanager.obj      \
                $(SLO)$/imagemanager.obj            \
                $(SLO)$/graphicnameaccess.obj		\
                $(SLO)$/recentfilesmenucontroller.obj

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
                $(SALLIB)

SHL4DEF=		$(MISC)$/$(SHL4TARGET).def
SHL4DEPN=		$(SHL1IMPLIBN) $(SHL1TARGETN) $(SHL2IMPLIBN) $(SHL2TARGETN)

DEF4NAME=		$(SHL4TARGET)

SHL4VERSIONMAP= exports.map

# --- Targets -----------------------------------------------------------------

.INCLUDE :	target.mk

ALLTAR : $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/commandimagelist.ilst

$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/commandimagelist.ilst .PHONY :
    +-$(RM) $@ $@.$(INPATH)
.IF "$(use_shell)"!="4nt"
    $(FIND) $(SOLARSRC)/res/commandimagelist -name "*.png" | sed s#$(SOLARSRC)#%GLOBAL%# > $@.$(INPATH)
.ELSE			# "$(use_shell)"!="4nt"
    $(FIND) $(SOLARSRC)/res/commandimagelist -name "*.png" | sed "s/$(SOLARSRC:s/\/\\/)/%%GLOBAL%%/" > $@.$(INPATH)
.ENDIF			# "$(use_shell)"!="4nt"
    +$(RENAME) $@.$(INPATH) $@

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
