#************************************************************************* 
# 
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: rt $ $Date: 2005-09-09 11:29:07 $
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
PRJ	= ..$/.. 
PRJNAME = filter 
#PACKAGE = com$/sun$/star$/documentconversion$/DBFilter
TARGET  =DBFilter  
# --- Settings ----------------------------------------------------- 
.INCLUDE: settings.mk
CLASSDIR!:=$(CLASSDIR)$/$(TARGET)  
#USE_UDK_EXTENDED_MANIFESTFILE=TRUE
#USE_EXTENDED_MANIFESTFILE=TRUE
JARFILES 		= sandbox.jar ridl.jar unoil.jar jurt.jar juh.jar 
JAVAFILES		= $(subst,$(CLASSDIR)$/, $(subst,.class,.java $(JAVACLASSFILES))) 
CUSTOMMANIFESTFILE = Manifest  
#JARMANIFEST = Manifest

#JARDIR=$(CLASSDIR)

JARCOMPRESS		= TRUE  
JARCLASSDIRS	= DBFilter*.class
JARTARGET		= $(TARGET).jar


# --- Files --------------------------------------------------------  
JAVACLASSFILES=$(CLASSDIR)$/DBFilter.class  
#---Manifest -------------------------------------------------------
#$(OUT)$/class$/$(TARGET)$/META-INF: META-INF
#        + $(COPY) $(COPYRECURSE) META-INF $(OUT)$/class$/DBFilter$/META-INF
# --- Targets ------------------------------------------------------  
.INCLUDE :  target.mk 
$(JAVACLASSFILES) : $(CLASSDIR)
  
$(CLASSDIR) :
    $(MKDIR) $(CLASSDIR)


