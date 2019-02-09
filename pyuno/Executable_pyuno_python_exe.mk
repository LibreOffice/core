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

include $(SRCDIR)/python/pyversion.mk

$(eval $(call gb_Executable_Executable,pyuno_python_exe))

$(eval $(call gb_Executable_set_include,pyuno_python_exe,\
	$$(INCLUDE) \
	-I$(WORKDIR)/CustomTarget/pyuno/zipcore \
))

$(eval $(call gb_Executable_add_linked_libs,pyuno_python_exe,\
	sal \
	stl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Executable_add_linked_libs,pyuno_python_exe,\
	user32 \
))

$(eval $(call gb_Executable_add_linked_static_libs,pyuno_python_exe,\
	ooopathutils \
))

$(eval $(call gb_Executable_add_exception_objects,pyuno_python_exe,\
	pyuno/zipcore/python \
))

# Automatic dependency resolution fails; add it ourselves:
$(SRCDIR)/pyuno/zipcore/python.cxx : $(WORKDIR)/CustomTarget/pyuno/zipcore/pyversion.hxx

$(WORKDIR)/CustomTarget/pyuno/zipcore/pyversion.hxx : $(SRCDIR)/pyuno/zipcore/pyversion.inc
	mkdir -p $(dir $@) && \
	sed "s/@/$(PYVERSION)/g" < $< > $@



# FIXME: cannot specify custom executable path, so must build a wrongly named EXE and then re-deliver it to a different directory!
$(eval $(call gb_Package_Package,pyuno_python_exe,$(OUTDIR)))
$(eval $(call gb_Package_add_file,pyuno_python_exe,bin/pyuno/python.exe,bin/pyuno_python_exe.exe))


# vim: set noet sw=4 ts=4:
