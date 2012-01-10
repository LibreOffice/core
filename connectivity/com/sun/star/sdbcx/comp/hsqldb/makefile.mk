#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************

  
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
