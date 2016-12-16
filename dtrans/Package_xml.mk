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



$(eval $(call gb_Package_Package,dtrans_xml,$(SRCDIR)/dtrans/source))

$(eval $(call gb_Package_add_file,dtrans_xml,xml/dtrans.xml,generic/dtrans.xml))
$(eval $(call gb_Package_add_file,dtrans_xml,xml/mcnttype.xml,cnttype/mcnttype.xml))

ifeq ($(OS),OS2)
$(eval $(call gb_Package_add_file,dtrans_xml,xml/sysdtrans.xml,os2/clipb/sysdtrans.xml))
endif

ifeq ($(OS),WNT)
$(eval $(call gb_Package_add_file,dtrans_xml,xml/dnd.xml,win32/dnd/dnd.xml))
$(eval $(call gb_Package_add_file,dtrans_xml,xml/ftransl.xml,win32/ftransl/ftransl.xml))
$(eval $(call gb_Package_add_file,dtrans_xml,xml/sysdtrans.xml,win32/clipb/sysdtrans.xml))
endif
