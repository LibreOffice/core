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



# MODULE is the name of the module the makefile is located in

# declare a library
# LIB is the name of the library as it is found in Repository.mk
$(eval $(call gb_Library_Library,LIB))

# declare packages that will be delivered before compilation of LIB
# learn more about TYPE in the Package.mk template
$(eval $(call gb_Library_add_package_headers,LIB,MODULE_TYPE))

# for platforms supporting PCH: declare the location of the pch file
# this is the name of the cxx file (without extension)
$(eval $(call gb_Library_add_precompiled_header,LIB,$(SRCDIR)/MODULE/PCH_FILE))

# in case UNO services are exported: declare location of component file
$(eval $(call gb_Library_set_componentfile,LIB,MODULE/COMPONENT_FILE))

# add any additional include paths for this library here
$(eval $(call gb_Library_set_include,LIB,\
	$$(INCLUDE) \
))

# add any additional definitions to be set for compilation here
# (e.g. -DLIB_DLLIMPLEMENTATION)
$(eval $(call gb_Library_set_defs,LIB,\
	$$(DEFS) \
))

# add libraries to be linked to LIB; again these names need to be given as
# specified in Repository.mk
$(eval $(call gb_Library_add_linked_libs,LIB,\
    $(gb_STDLIBS) \
))

# add all source files that shall be compiled with exceptions enabled
# the name is relative to $(SRCROOT) and must not contain an extension
$(eval $(call gb_Library_add_exception_objects,LIB,\
))

# in case sdi files need to be processed: declare "root" (main) sdi target and sdi header target
# "ROOT_SDI_FILE" is the file that is handed over to svidl (all other sdi files are included in the root file)
$(eval $(call gb_Library_add_sdi_headers,LIB,MODULE/sdi/ROOT_SDI_FILE))
$(eval $(call gb_SdiTarget_SdiTarget,MODULE/sdi/ROOT_SDI_FILE,MODULE/sdi/LIB))

# add any additional include paths for sdi processing here
$(eval $(call gb_SdiTarget_set_include,MODULE/sdi/ROOT_SDI_FILE,\
	$$(INCLUDE) \
))

# this is an example how files can be added that require special compiler settings, e.g. building without optimizing
ifeq ($(OS),WNT)
$(eval $(call gb_Library_add_cxxobjects,LIB,\
    MODULE/source/foo/bar \
    , $(gb_COMPILERNOOPTFLAGS) $(gb_LinkTarget_EXCEPTIONFLAGS) \
))

endif

# vim: set noet sw=4 ts=4:
