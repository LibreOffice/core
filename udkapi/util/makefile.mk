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



PRJ=..
PRJPCH=

PRJNAME=udkapi
TARGET=udkapi

#use_starjar=true


# --- Settings -----------------------------------------------------

.INCLUDE :  makefile.pmk

# ------------------------------------------------------------------

UNOIDLDBFILES= \
    $(UCR)$/css.db \
    $(UCR)$/cssutil.db \
    $(UCR)$/cssbeans.db \
    $(UCR)$/cssbridge.db \
    $(UCR)$/cssboleautomation.db \
    $(UCR)$/cssconnection.db\
    $(UCR)$/csscontainer.db \
    $(UCR)$/cssio.db \
    $(UCR)$/cssjava.db \
    $(UCR)$/csslang.db \
    $(UCR)$/csssec.db \
    $(UCR)$/cssloader.db \
    $(UCR)$/cssreflection.db \
    $(UCR)$/cssregistry.db \
    $(UCR)$/cssscript.db \
    $(UCR)$/csstest.db \
    $(UCR)$/cssperftest.db \
    $(UCR)$/cssbridgetest.db \
    $(UCR)$/cssuno.db \
    $(UCR)$/cssulog.db \
    $(UCR)$/csscorba.db \
    $(UCR)$/cssiop.db \
    $(UCR)$/cssiiop.db \
    $(UCR)$/cssgiop.db \
    $(UCR)$/csstask.db \
    $(UCR)$/cssuri.db

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk


