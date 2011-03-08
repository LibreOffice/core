#************************************************************************* 
# 
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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

PRJ	= ..$/.. 
PRJNAME = filter 
TARGET  =XSLTValidate 

# --- Settings ----------------------------------------------------- 

.IF "$(XML_CLASSPATH)" != ""
XCLASSPATH+=":$(XML_CLASSPATH)"
.ENDIF
.INCLUDE: settings.mk  
CLASSDIR!:=$(CLASSDIR)$/$(TARGET)
JARFILES 		= ridl.jar unoil.jar jurt.jar juh.jar crimson.jar

.IF "$(SYSTEM_XALAN)" == "YES"
EXTRAJARFILES += $(XALAN_JAR)
.ELSE
JARFILES += xalan.jar
.ENDIF

.IF "$(SYSTEM_XML_APIS)" == "YES"
EXTRAJARFILES += $(XML_APIS_JAR)
.ELSE
JARFILES += xml-apis.jar
.ENDIF

JAVAFILES		= $(subst,$(CLASSDIR)$/, $(subst,.class,.java $(JAVACLASSFILES))) 
CUSTOMMANIFESTFILE = Manifest  
JARCOMPRESS		= TRUE  
JARCLASSDIRS	= XSLTValidate*.class
JARTARGET		= $(TARGET).jar


# --- Files --------------------------------------------------------  

JAVACLASSFILES=$(CLASSDIR)$/XSLTValidate.class  

# --- Targets ------------------------------------------------------  

.IF "$(SOLAR_JAVA)"!=""
.INCLUDE :  target.mk 
$(JAVACLASSFILES) : $(CLASSDIR)
.IF "$(JARMANIFEST)"!=""
$(JARMANIFEST) : $(CLASSDIR)
.ENDIF			# "$(JARMANIFEST)"!=""
.ENDIF
  
$(CLASSDIR) :
    $(MKDIR) $(CLASSDIR)
