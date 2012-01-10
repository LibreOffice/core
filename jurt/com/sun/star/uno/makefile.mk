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



PRJ		= ..$/..$/..$/..
PRJNAME = jurt
PACKAGE = com$/sun$/star$/uno
TARGET  = com_sun_star_uno

# --- Settings -----------------------------------------------------

.INCLUDE : $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

JAVAFILES = \
    AnyConverter.java \
    Ascii.java \
    AsciiString.java \
    MappingException.java \
    WeakReference.java

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

doc:
    pwd
    cd $(PRJ) && javadoc -sourcepath /usr/local/java/src:unxlngi3.pro/japi:.:../jlibs  com.sun.star.lib.util com.sun.star.uno com.sun.star.lib.uno.typeinfo com.sun.star.lib.uno.environments.java com.sun.star.lib.uno.environments.remote com.sun.star.lib.uno.protocols.iiop com.sun.star.lib.uno.bridges.java_remote com.sun.star.comp.loader com.sun.star.comp.connections -d unxlngi3.pro/doc 
