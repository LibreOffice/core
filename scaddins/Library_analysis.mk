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



$(eval $(call gb_Library_Library,analysis))

$(eval $(call gb_Library_set_componentfile,analysis,scaddins/source/analysis/analysis))

$(eval $(call gb_Library_set_private_api,analysis,$(OUTDIR)/bin/types.rdb,\
	$(SRCDIR)/scaddins/source/analysis/analysisadd.idl,\
        com.sun.star.sheet.addin.XAnalysis \
        com.sun.star.lang.XComponent \
        com.sun.star.lang.XMultiServiceFactory \
        com.sun.star.lang.XSingleComponentFactory \
        com.sun.star.lang.XSingleServiceFactory \
        com.sun.star.uno.TypeClass \
        com.sun.star.uno.XInterface \
        com.sun.star.registry.XImplementationRegistration \
        com.sun.star.sheet.LocalizedName \
        com.sun.star.sheet.XAddIn \
        com.sun.star.sheet.XCompatibilityNames \
        com.sun.star.lang.XServiceName \
        com.sun.star.lang.XServiceInfo \
        com.sun.star.lang.XTypeProvider \
        com.sun.star.uno.XWeak \
        com.sun.star.uno.XAggregation \
        com.sun.star.uno.XComponentContext \
        com.sun.star.util.Date \
        com.sun.star.util.XNumberFormatter \
        com.sun.star.util.XNumberFormatTypes \
        com.sun.star.sheet.NoConvergenceException \
))

$(eval $(call gb_Library_add_linked_libs,analysis,\
	cppu \
	cppuhelper \
	sal \
	stl \
	tl \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,analysis,\
	scaddins/source/analysis/analysis \
	scaddins/source/analysis/financial \
	scaddins/source/analysis/analysishelper \
	scaddins/source/analysis/bessel \
))

# vim: set noet sw=4 ts=4:

