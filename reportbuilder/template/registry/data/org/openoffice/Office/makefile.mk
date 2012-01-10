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



PRJ=..$/..$/..$/..$/..$/..

PRJNAME=reportbuilder
TARGET=template_data_ooOffice
PACKAGE=org.openoffice.Office

.INCLUDE :  $(PRJ)$/util$/makefile.pmk

#
# we overwrite the PROCESSOUT directory, which comes from util/makefile.pmk
# and set it to the templates directory
#
PROCESSOUT=$(MISC)$/templates

# --- Targets ------------------------------------------------------

XCUFILES= \
    Paths.xcu

MODULEFILES=

.INCLUDE :  target.mk
