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



PRJ=.

PRJNAME=so_stlport
TARGET=so_stlport

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# Wrap STL headers to get them into the std namespace:
$(INCCOM)$/stlport$/functional \
$(INCCOM)$/stlport$/hash_map \
$(INCCOM)$/stlport$/hash_set \
$(INCCOM)$/stlport$/numeric \
$(INCCOM)$/stlport$/slist \
$(INCCOM)$/stlport$/list \
$(INCCOM)$/stlport$/map \
$(INCCOM)$/stlport$/set \
$(INCCOM)$/stlport$/vector: systemstl$/$$(@:f)
    $(MKDIRHIER) $(@:d)
    $(COPY) $< $@

