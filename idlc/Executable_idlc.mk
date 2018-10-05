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



$(eval $(call gb_Executable_Executable,idlc))

$(eval $(call gb_Executable_set_targettype_gui,idlc,NO))

$(eval $(call gb_Executable_add_precompiled_header,idlc,$(SRCDIR)/idlc/inc/pch/precompiled_idlc))

#$(eval $(call gb_Executable_add_api,idlc,\
#	udkapi \
#	offapi \
#))

$(eval $(call gb_Executable_set_include,idlc,\
	$$(INCLUDE) \
	-I$(WORKDIR)/inc/ \
	-I$(OUTDIR)/inc/ \
	-I$(SRCDIR)/idlc/inc \
	-I$(SRCDIR)/idlc/inc/pch \
))

$(eval $(call gb_Executable_add_linked_libs,idlc,\
	reg \
	sal \
	salhelper \
	stl \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Executable_add_bison_files,idlc,\
	idlc/source/parser \
))

$(eval $(call gb_Executable_add_flex_files,idlc,\
	idlc/source/scanner \
))

$(eval $(call gb_Executable_add_exception_objects,idlc,\
	idlc/source/wrap_scanner \
	idlc/source/wrap_parser \
	idlc/source/idlcmain \
	idlc/source/idlc \
	idlc/source/idlccompile \
	idlc/source/idlcproduce \
	idlc/source/errorhandler \
	idlc/source/options \
	idlc/source/fehelper \
	idlc/source/astdeclaration \
	idlc/source/astscope \
	idlc/source/aststack \
	idlc/source/astdump \
	idlc/source/astinterface \
	idlc/source/aststruct \
	idlc/source/aststructinstance \
	idlc/source/astoperation \
	idlc/source/astconstant \
	idlc/source/astenum \
	idlc/source/astarray \
	idlc/source/astunion \
	idlc/source/astexpression \
	idlc/source/astservice \
))

# vim: set noet sw=4 ts=4:
