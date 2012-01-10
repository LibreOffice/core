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

PRJNAME=offapi

TARGET=csstextfielddocinfo
PACKAGE=com$/sun$/star$/text$/textfield$/docinfo

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    ChangeAuthor.idl\
    ChangeDateTime.idl\
    CreateAuthor.idl\
    CreateDateTime.idl\
    Custom.idl\
    Description.idl\
    EditTime.idl\
    Info0.idl\
    Info1.idl\
    Info2.idl\
    Info3.idl\
    Keywords.idl\
    PrintAuthor.idl\
    PrintDateTime.idl\
    Revision.idl\
    Subject.idl\
    Title.idl\


# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
