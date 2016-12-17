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



$(eval $(call gb_Library_Library,sysdtrans))

$(eval $(call gb_Library_add_precompiled_header,sysdtrans,$(SRCDIR)/dtrans/inc/pch/precompiled_dtrans))

$(eval $(call gb_Library_set_componentfile,sysdtrans,dtrans/util/sysdtrans))

$(eval $(call gb_Library_set_include,sysdtrans,\
        $$(INCLUDE) \
	-I$(SRCDIR)/dtrans/inc/pch \
))

$(eval $(call gb_Library_add_api,sysdtrans,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,sysdtrans,\
	advapi32 \
	cppuhelper \
	cppu \
	gdi32 \
	ole32 \
	oleaut32 \
	sal \
	shell32 \
	stl \
	uuid \
	user32 \
	uwinapi \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_cxxobjects,sysdtrans,\
	dtrans/source/win32/clipb/WinClipboard \
	dtrans/source/win32/clipb/WinClipbImpl \
	dtrans/source/win32/clipb/wcbentry \
	, -DUNICODE -D_UNICODE $(gb_LinkTarget_EXCEPTIONFLAGS) \
))

$(eval $(call gb_Library_add_exception_objects,sysdtrans,\
	dtrans/source/win32/misc/ImplHelper \
))

ifneq ($(COM),GCC)
$(eval $(call gb_Library_add_cxxobjects,sysdtrans,\
	dtrans/source/win32/dtobj/DtObjFactory \
	dtrans/source/win32/dtobj/APNDataObject \
	dtrans/source/win32/dtobj/DOTransferable \
	dtrans/source/win32/dtobj/DTransHelper \
	dtrans/source/win32/dtobj/XTDataObject \
	dtrans/source/win32/dtobj/TxtCnvtHlp \
	dtrans/source/win32/dtobj/DataFmtTransl \
	dtrans/source/win32/dtobj/FmtFilter \
	dtrans/source/win32/dtobj/FetcList \
	dtrans/source/win32/dtobj/Fetc \
	dtrans/source/win32/dtobj/XNotifyingDataObject \
	, -GR -Ob0 $(gb_LinkTarget_EXCEPTIONFLAGS) \
))
else
$(eval $(call gb_Library_add_exception_objects,sysdtrans,\
	dtrans/source/win32/dtobj/DtObjFactory \
	dtrans/source/win32/dtobj/APNDataObject \
	dtrans/source/win32/dtobj/DOTransferable \
	dtrans/source/win32/dtobj/DTransHelper \
	dtrans/source/win32/dtobj/XTDataObject \
	dtrans/source/win32/dtobj/TxtCnvtHlp \
	dtrans/source/win32/dtobj/DataFmtTransl \
	dtrans/source/win32/dtobj/FmtFilter \
	dtrans/source/win32/dtobj/FetcList \
	dtrans/source/win32/dtobj/Fetc \
	dtrans/source/win32/dtobj/XNotifyingDataObject \
))
endif

$(eval $(call gb_Library_add_cxxobjects,sysdtrans,\
	dtrans/source/win32/mtaole/MtaOleClipb \
	, -DUNICODE $(gb_LinkTarget_EXCEPTIONFLAGS) \
))

# vim: set noet sw=4 ts=4:
