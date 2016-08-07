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



$(eval $(call gb_Library_Library,xof))

$(eval $(call gb_Library_set_componentfile,xof,xmloff/source/transform/xof))

$(eval $(call gb_Library_set_include,xof,\
    -I$(SRCDIR)/xmloff/inc/pch \
    -I$(SRCDIR)/xmloff/inc \
    -I$(SRCDIR)/xmloff/source/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_add_api,xof,\
    udkapi \
    offapi \
))

$(eval $(call gb_Library_add_linked_libs,xof,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    salhelper \
    stl \
    xo \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,xof,\
    xmloff/source/transform/ChartOASISTContext \
    xmloff/source/transform/ChartOOoTContext \
    xmloff/source/transform/ChartPlotAreaOASISTContext \
    xmloff/source/transform/ChartPlotAreaOOoTContext \
    xmloff/source/transform/ControlOASISTContext \
    xmloff/source/transform/ControlOOoTContext \
    xmloff/source/transform/CreateElemTContext \
    xmloff/source/transform/DeepTContext \
    xmloff/source/transform/DlgOASISTContext \
    xmloff/source/transform/DocumentTContext \
    xmloff/source/transform/EventMap \
    xmloff/source/transform/EventOASISTContext \
    xmloff/source/transform/EventOOoTContext \
    xmloff/source/transform/FlatTContext \
    xmloff/source/transform/FormPropOASISTContext \
    xmloff/source/transform/FormPropOOoTContext \
    xmloff/source/transform/FrameOASISTContext \
    xmloff/source/transform/FrameOOoTContext \
    xmloff/source/transform/IgnoreTContext \
    xmloff/source/transform/MergeElemTContext \
    xmloff/source/transform/MetaTContext \
    xmloff/source/transform/MutableAttrList \
    xmloff/source/transform/NotesTContext \
    xmloff/source/transform/OOo2Oasis \
    xmloff/source/transform/Oasis2OOo \
    xmloff/source/transform/PersAttrListTContext \
    xmloff/source/transform/PersMixedContentTContext \
    xmloff/source/transform/ProcAddAttrTContext \
    xmloff/source/transform/ProcAttrTContext \
    xmloff/source/transform/PropertyActionsOASIS \
    xmloff/source/transform/PropertyActionsOOo \
    xmloff/source/transform/RenameElemTContext \
    xmloff/source/transform/StyleOASISTContext \
    xmloff/source/transform/StyleOOoTContext \
    xmloff/source/transform/TransformerActions \
    xmloff/source/transform/TransformerBase \
    xmloff/source/transform/TransformerContext \
    xmloff/source/transform/TransformerTokenMap \
    xmloff/source/transform/XMLFilterRegistration \
))

# vim: set noet ts=4 sw=4:
