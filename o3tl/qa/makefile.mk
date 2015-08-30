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
PRJNAME=o3tl
TARGET=tests

ENABLE_EXCEPTIONS=TRUE


# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(ENABLE_UNIT_TESTS)" != "YES"
all:
    @echo unit tests are disabled. Nothing to do.
 
.ELSE


.IF "$(L10N_framework)"==""
# --- Common ----------------------------------------------------------

# BEGIN ----------------------------------------------------------------
APP1OBJS=  \
    $(SLO)$/cow_wrapper_clients.obj     \
    $(SLO)$/main.obj                    \
    $(SLO)$/test-cow_wrapper.obj	    \
    $(SLO)$/test-vector_pool.obj	\
    $(SLO)$/test-heap_ptr.obj           \
    $(SLO)$/test-range.obj

APP1TARGET= tests
APP1STDLIBS= 	$(SALLIB)		 \
                $(GTESTLIB)

APP1RPATH = NONE
APP1TEST = enabled

# END ------------------------------------------------------------------

# --- Targets ------------------------------------------------------
.ENDIF 		# L10N_framework

.INCLUDE : target.mk

.ENDIF # "$(ENABLE_UNIT_TESTS)" != "YES"
