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

$(eval $(call gb_Package_Package,pyuno_zipcore,$(WORKDIR)/CustomTarget/pyuno/zipcore))

$(eval $(call gb_Package_add_file,pyuno_zipcore,bin/python-core-$(PYVERSION).zip,python-core-$(PYVERSION).zip))



ifneq ($(GUI),UNX)
ifeq ($(COM),GCC)
PYTHONBINARY := $(WORKDIR)/CustomTarget/pyuno/zipcore/python-core-$(PYVERSION)/bin/python.bin
else
PYTHONBINARY := $(WORKDIR)/CustomTarget/pyuno/zipcore/python-core-$(PYVERSION)/bin/python$(gb_Executable_EXT)
endif
endif

PYTHON_CORE_FILES := $(PYTHONBINARY) \
	$(foreach lib,$(shell find $(OUTDIR)/lib/python -type f | grep -v "\.pyc" | grep -v "\.py~" | grep -v .orig | grep -v _failed),$(subst $(OUTDIR)/lib/python,$(WORKDIR)/CustomTarget/pyuno/zipcore/python-core-$(PYVERSION)/lib,$(lib))) \
	$(foreach inc,$(shell find $(OUTDIR)/inc/python -type f | grep -v "\.h~" | grep -v _failed),$(subst $(OUTDIR)/inc/python,$(WORKDIR)/CustomTarget/pyuno/zipcore/python-core-$(PYVERSION)/include/python$(PYMAJOR).$(PYMINOR),$(inc)))

$(WORKDIR)/CustomTarget/pyuno/zipcore/python-core-$(PYVERSION).zip : $(PYTHON_CORE_FILES)
	mkdir -p $(dir $@) && \
	cd $(dir $@) && \
	$(if $(filter-out MACOSX,$(OS)),$(if $(filter UNX,$(GUI)),find . -name "*$(gb_Library_PLAINEXT)" | xargs strip &&)) \
	zip -r python-core-$(PYVERSION).zip python-core-$(PYVERSION)

ifneq ($(GUI),UNX)
$(PYTHONBINARY) : $(OUTDIR)/bin/python$(gb_Executable_EXT)
	mkdir -p $(dir $@) && \
	cp $< $@
endif

$(WORKDIR)/CustomTarget/pyuno/zipcore/python-core-$(PYVERSION)/lib/% : $(OUTDIR)/lib/python/%
	mkdir -p $(dir $@) && \
	cp $< $@

$(WORKDIR)/CustomTarget/pyuno/zipcore/python-core-$(PYVERSION)/include/python$(PYMAJOR).$(PYMINOR)/% : $(OUTDIR)/inc/python/%
	mkdir -p $(dir $@) && \
	cp $< $@
	
