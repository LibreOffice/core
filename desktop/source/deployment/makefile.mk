#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: hr $ $Date: 2004-07-23 14:19:27 $
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
#   Copyright: 2002 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ = ..$/..

PRJNAME = desktop
TARGET = deployment
ENABLE_EXCEPTIONS = TRUE
#USE_DEFFILE = TRUE
NO_BSYMBOLIC = TRUE

.INCLUDE : settings.mk
DLLPRE =

LIB1TARGET = $(SLB)$/$(TARGET).lib

LIB1FILES = \
        $(SLB)$/deployment_registry_configuration.lib \
        $(SLB)$/deployment_registry_component.lib \
        $(SLB)$/deployment_registry_script.lib \
        $(SLB)$/deployment_registry_sfwk.lib                  \
        $(SLB)$/deployment_registry_package.lib \
        $(SLB)$/deployment_registry.lib \
        $(SLB)$/deployment_manager.lib \
        $(SLB)$/deployment_misc.lib

SHL1TARGET = $(TARGET)$(UPD)$(DLLPOSTFIX).uno
SHL1VERSIONMAP = $(TARGET).map

SHL1LIBS = $(LIB1TARGET)

SHL1STDLIBS = \
        $(SALLIB) \
        $(CPPULIB) \
        $(CPPUHELPERLIB) \
        $(UCBHELPERLIB) \
        $(COMPHELPERLIB) \
        $(TOOLSLIB) \
        $(VOSLIB) \
        $(UNOTOOLSLIB) \
        $(XMLSCRIPTLIB) \
        $(BERKELEYLIB) \
        $(BERKELEYCPPLIB) \
        $(SVLLIB)

SHL1DEPN =
SHL1IMPLIB = i$(TARGET)
SHL1DEF = $(MISC)$/$(SHL1TARGET).def

DEF1NAME = $(SHL1TARGET)
#DEFLIB1NAME = $(TARGET)
#DEF1DEPN =

RESLIB1NAME = $(TARGET)

RESLIB1SRSFILES = \
        $(SRS)$/deployment_registry_configuration.srs \
        $(SRS)$/deployment_registry_component.srs \
        $(SRS)$/deployment_registry_script.srs \
        $(SRS)$/deployment_registry_sfwk.srs \
        $(SRS)$/deployment_registry_package.srs \
        $(SRS)$/deployment_registry.srs \
        $(SRS)$/deployment_manager.srs \
        $(SRS)$/deployment_misc.srs

.INCLUDE : target.mk

