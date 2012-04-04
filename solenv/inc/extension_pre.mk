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



EXTENSIONDIR*:=$(MISC)/$(EXTENSIONNAME)

COMPONENT_CONFIGDIR*:=.
COMPONENT_CONFIGDEST*:=registry/data
COMPONENT_CONFIGDESTSCHEMA*:=registry/schema

.IF "$(COMPONENT_COPYONLY)"==""
DESCRIPTION:=$(EXTENSIONDIR)/description.xml
DESCRIPTION_SRC*:=description.xml

COMPONENT_MANIFEST*:=$(EXTENSIONDIR)/META-INF/manifest.xml
MANIFEST_SRC*:=manifest.xml

PACKLICS*:=$(EXTENSIONDIR)/registration/LICENSE
COMPONENT_LIC_TEMPL*:=registration/LICENSE

#TODO:  check ZIP9TARGET for previous use!
common_build_zip=
MANIFEST_DEPS=	$(PACKLICS) $(DESCRIPTION) $(COMPONENT_FILES) $(COMPONENT_LIBRARIES) $(COMPONENT_JARFILES) $(COMPONENT_MERGED_XCU) $(EXTENSION_PACKDEPS)
ZIP9DEPS=		$(PACKLICS) $(DESCRIPTION) $(COMPONENT_XCU) $(COMPONENT_FILES) $(COMPONENT_LIBRARIES) $(COMPONENT_MANIFEST) $(COMPONENT_JARFILES) $(COMPONENT_MERGED_XCU) $(EXTENSION_PACKDEPS)
ZIP9TARGET=		$(EXTENSION_ZIPNAME)
ZIP9DIR=		$(EXTENSIONDIR)
ZIP9EXT=		.oxt
ZIP9FLAGS=-r
ZIP9LIST=		*
.ENDIF			# "$(COMPONNENT_COPYONLY)"==""

