#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 13:59:42 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

PRJ=..$/..$/..
PRJINC=$(PRJ)$/source
PRJNAME=dbaccess
TARGET=dbacfg

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE :  	svpre.mk
.INCLUDE :  	settings.mk
.INCLUDE :  	sv.mk

# --- Types -------------------------------------

UNOTYPES=\
    com.sun.star.uno.RuntimeException							\
    com.sun.star.uno.TypeClass									\
    com.sun.star.uno.XInterface									\
    com.sun.star.uno.XWeak										\
    com.sun.star.registry.XRegistryKey							\
    com.sun.star.io.XInputStream								\
    com.sun.star.io.XOutputStream								\
    com.sun.star.lang.XComponent								\
    com.sun.star.lang.XInitialization							\
    com.sun.star.lang.XMultiServiceFactory						\
    com.sun.star.lang.XSingleServiceFactory						\
    com.sun.star.lang.XServiceInfo								\
    com.sun.star.loader.XImplementationLoader					\
    com.sun.star.registry.XImplementationRegistration			\
    com.sun.star.registry.XRegistryKey							\
    com.sun.star.registry.XSimpleRegistry						\
    com.sun.star.document.XFilter								\
    com.sun.star.document.XExporter								\
    com.sun.star.container.XIndexAccess
# --- Files -------------------------------------

SLOFILES=	$(SLO)$/cfgimport.obj				\
            $(SLO)$/cfgservices.obj

# --- Library -----------------------------------
LIB1TARGET=$(SLB)$/dba_cfg.lib
LIB1FILES=\
        $(SLB)$/cfgshared.lib	\
        $(SLB)$/$(TARGET).lib
        
SHL1TARGET=$(TARGET)$(UPD)$(DLLPOSTFIX)

SHL1STDLIBS=\
    $(UNOTOOLSLIB)		\
    $(TOOLSLIB)			\
    $(COMPHELPERLIB)	\
    $(CPPUHELPERLIB)	\
    $(CPPULIB)			\
    $(UCBHELPERLIB)		\
    $(SFXLIB)			\
    $(SVLLIB)			\
    $(SALLIB)


SHL1DEPN=
SHL1LIBS=$(LIB1TARGET)
SHL1IMPLIB=	i$(SHL1TARGET)
# SHL1LIBS=	$(SLB)$/$(TARGET).lib
SHL1VERSIONMAP=exports.map
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)

# --- Targets ----------------------------------

.INCLUDE : target.mk
