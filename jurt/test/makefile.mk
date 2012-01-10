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


# Quick and dirty way to run all tests in sub-directories; probably only works
# on Unix.
# Can be removed once tests are included in regular builds.

all .PHONY:
    cd com/sun/star/comp/bridgefactory && dmake
    cd com/sun/star/comp/connections && dmake
    cd com/sun/star/lib/uno/bridges/java_remote && dmake
    cd com/sun/star/lib/uno/environments/java && dmake
    cd com/sun/star/lib/uno/environments/remote && dmake
    cd com/sun/star/lib/uno/protocols/urp && dmake
    cd com/sun/star/lib/util && dmake
    cd com/sun/star/uno && dmake
