#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: kz $ $Date: 2005-11-02 17:40:11 $
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
PRJ = ..
PRJNAME = OOoRunner
TARGET  = $(PRJNAME)

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

SUBDIRS_TESTS = mod ifc
SUBDIRS_RUNNER = util share stats lib complexlib helper basicrunner \
            base org$/openoffice convwatch

JARCOMMANDS_TESTS = $(foreach,i,$(SUBDIRS_TESTS) -C $(CLASSDIR) $i)
JARCOMMANDS_RUNNER = $(foreach,i,$(SUBDIRS_RUNNER) -C $(CLASSDIR) $i)

# --- Targets ------------------------------------------------------
.IF "$(SOLAR_JAVA)"!=""
OWNJAR: ALLTAR


.INCLUDE :  target.mk

# LLA: parameter v is only verbose, need too long!
OWNJAR: LIGHT
    +jar cfm $(CLASSDIR)$/$(TARGET).jar manifest -C $(PRJ) objdsc $(JARCOMMANDS_TESTS) $(JARCOMMANDS_RUNNER)

# LLA: parameter v is only verbose, need too long!
LIGHT:
    +jar cfm $(CLASSDIR)$/$(TARGET)Light.jar manifest $(JARCOMMANDS_RUNNER)

.ELSE
all:
        @echo "no java"
.ENDIF
