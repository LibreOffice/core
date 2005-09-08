#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 14:21:45 $
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
PRJNAME = svtools
TARGET = qa

ENABLE_EXCEPTIONS = true

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# BEGIN ----------------------------------------------------------------
# auto generated Target:job by codegen.pl 
SHL1OBJS=  \
    $(SLO)$/test_URIHelper.obj

SHL1TARGET= URIHelper
SHL1STDLIBS=\
     $(CPPULIB) \
     $(CPPUHELPERLIB) \
     $(SALLIB) \
     $(SVTOOLLIB) \
     $(TOOLSLIB) \
     $(UNOTOOLSLIB) \
   $(CPPUNITLIB) 

SHL1IMPLIB= i$(SHL1TARGET)
DEF1NAME    =$(SHL1TARGET)
SHL1VERSIONMAP= export.map
# auto generated Target:job
# END ------------------------------------------------------------------

#------------------------------- All object files -------------------------------
# do this here, so we get right dependencies
# SLOFILES=$(SHL1OBJS)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE : _cppunit.mk

# LLA: old stuff
# USE_DEFFILE = true
# 
# .INCLUDE: settings.mk
# 
# .IF "$(OS)" == "WNT"
# REGEXP = "s/^[\#].*$$//"
# .ELSE # OS, WNT
# REGEXP = 's/^[\#].*$$//'
# .ENDIF # OS, WNT
# 
# SHL1TARGET = URIHelper
# SHL1OBJS = \
# 	$(SLO)$/test_URIHelper.obj
# SHL1STDLIBS = \
#     $(CPPULIB) \
#     $(CPPUHELPERLIB) \
#     $(SALLIB) \
#     $(SVTOOLLIB) \
#     $(TOOLSLIB) \
#     $(UNOTOOLSLIB)
# 
# DEF1NAME = $(SHL1TARGET)
# DEF1EXPORTFILE = $(MISC)$/$(SHL1TARGET).dxp
# 
# .INCLUDE: target.mk
# 
# $(MISC)$/$(SHL1TARGET).dxp: sce$/$(SHL1TARGET).sce
#     + $(TYPE) $< | sed $(REGEXP) > $@
#     + $(TYPE) $@ | sed "s/^/test_/" > $(MISC)$/$(SHL1TARGET).tst
#     + $(TYPE) $(MISC)$/$(SHL1TARGET).tst | sed "/test_./ w $@"
