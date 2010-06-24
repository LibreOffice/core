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
PRJ	= ..$/..$/..$/..$/..$/.. 
PRJNAME = connectivity
PACKAGE = com$/sun$/star$/sdbcx$/comp$/hsqldb
TARGET  = sdbc_hsqldb 

SECONDARY_PACKAGE = org$/hsqldb$/lib

# --- Settings ----------------------------------------------------- 
.INCLUDE: settings.mk  

# --- Files --------------------------------------------------------  


.IF "$(SYSTEM_HSQLDB)" == "YES"
EXTRAJARFILES = $(HSQLDB_JAR)
.ELSE
JARFILES = hsqldb.jar
.ENDIF

JAVAFILES =\
    NativeInputStreamHelper.java\
    NativeOutputStreamHelper.java\
    NativeStorageAccess.java\
    StorageAccess.java\
    StorageFileAccess.java\
    StorageNativeInputStream.java\
    StorageNativeOutputStream.java\
    FileSystemRuntimeException.java\
    NativeLibraries.java
    
JAVACLASSFILES  = $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

JARCOMPRESS	= TRUE
JARCLASSDIRS = $(PACKAGE) $(SECONDARY_PACKAGE)
JARTARGET	= $(TARGET).jar
JARCLASSPATH = $(JARFILES) $(EXTRAJARFILES) ..

# --- Targets ------------------------------------------------------  
.INCLUDE :  target.mk 
