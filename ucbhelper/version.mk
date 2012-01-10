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


UCBHELPER_TARGET=ucbhelper

# the major
UCBHELPER_MAJOR=4
# the minor
UCBHELPER_MINOR=0
# the micro
UCBHELPER_MICRO=0

# this is a c++ compatible library
UCBHELPER_CPP=1

UCBHELPER=$(UCBHELPER_TARGET)_$(CMPEXT)

LIBUCBHELPER_UNX=lib$(UCBHELPER).a.$(UCBHELPER_MAJOR)
LIBUCBHELPER_WIN=$(UCBHELPER_TARGET)$(UCBHELPER_MAJOR)$(CMPEXT).dll
