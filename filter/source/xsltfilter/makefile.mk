#************************************************************************* 
# 
#   $RCSfile: makefile.mk,v $ 
# 
#   $Revision: 1.7 $ 
#
#   last change: $Author: rt $ $Date: 2004-07-23 15:12:49 $ 
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
PRJ	= ..$/.. 
PRJNAME = filter 
#PACKAGE = com$/sun$/star$/documentconversion$/XSLTFilter
TARGET  =XSLTFilter  
ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO

# --- Settings ----------------------------------------------------- 
CLASSDIR!:=$(CLASSDIR)$/$(TARGET)  
.IF "$(XML_CLASSPATH)" != ""
XCLASSPATH+=":$(XML_CLASSPATH)"
.ENDIF
.INCLUDE: settings.mk  
#USE_UDK_EXTENDED_MANIFESTFILE=TRUE
#USE_EXTENDED_MANIFESTFILE=TRUE
JARFILES 		= ridl.jar unoil.jar jurt.jar juh.jar 
JAVAFILES		= $(subst,$(CLASSDIR)$/, $(subst,.class,.java $(JAVACLASSFILES))) 
CUSTOMMANIFESTFILE = Manifest  
#JARMANIFEST = Manifest

#JARDIR=$(CLASSDIR)

JARCOMPRESS		= TRUE  
JARCLASSDIRS	= XSLTFilter*.class XSLTransformer*.class
JARTARGET		= $(TARGET).jar


# --- Files --------------------------------------------------------  
JAVACLASSFILES=$(CLASSDIR)$/XSLTFilter.class  $(CLASSDIR)$/XSLTransformer.class  

#---Manifest -------------------------------------------------------
#$(OUT)$/class$/$(TARGET)$/META-INF: META-INF
#        + $(COPY) $(COPYRECURSE) META-INF $(OUT)$/class$/XSLTFilter$/META-INF
#

SLOFILES=$(SLO)$/XSLTFilter.obj
LIBNAME=xsltfilter
SHL1TARGETDEPN=makefile.mk
SHL1OBJS=$(SLOFILES)
SHL1TARGET=$(LIBNAME)$(UPD)$(DLLPOSTFIX)
SHL1IMPLIB=i$(LIBNAME)
SHL1VERSIONMAP=exports.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

SHL1STDLIBS= \
    $(VCLLIB)           \
    $(UNOTOOLSLIB)      \
    $(TOOLSLIB)         \
    $(VOSLIB)           \
    $(CPPUHELPERLIB)    \
    $(CPPULIB)          \
    $(SALLIB)
    
    
# --- Targets ------------------------------------------------------  
.INCLUDE :  target.mk 
.IF "$(SOLAR_JAVA)"!=""
$(JAVACLASSFILES) : $(CLASSDIR)
.ENDIF
  
$(CLASSDIR) :
    $(MKDIR) $(CLASSDIR)


