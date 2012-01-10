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



PRJ=..$/..

PRJNAME=javainstaller2
TARGET=create_property_files

INPUT=java_ulffiles

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

.IF "$(SOLAR_JAVA)"!=""
ALLTAR: $(MISC)$/setupstrings.properties

$(MISC)$/setupstrings.properties : create_property.pl $(COMMONMISC)$/$(INPUT)$/setupstrings.jlf setupstrings_template.properties setupfiles_template.properties
    $(PERL) create_property.pl $/ $(COMMONMISC)$/$(INPUT) $(MISC)
.ENDIF
