###############################################################
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
###############################################################



# this is a subset of gbuild functionality, for use in custom makefiles

include $(GBUILDDIR)/Output.mk

# BuildDirs uses the Output functions already
include $(GBUILDDIR)/BuildDirs.mk

include $(GBUILDDIR)/Helper.mk

ifeq ($(OS),LINUX)
include $(GBUILDDIR)/platform/linux.mk
else
ifeq ($(OS),WNT)
ifneq ($(USE_MINGW),)
include $(GBUILDDIR)/platform/winmingw.mk
else
include $(GBUILDDIR)/platform/windows.mk
endif
else
ifeq ($(OS),SOLARIS)
include $(GBUILDDIR)/platform/solaris.mk
else
ifeq ($(OS),MACOSX)
include $(GBUILDDIR)/platform/macosx.mk
ifeq ($(OS),FREEBSD)
include $(GBUILDDIR)/platform/freebsd.mk
else
$(eval $(call gb_Output_error,Unsupported OS: $(OS)))
endif
endif
endif
endif
endif
