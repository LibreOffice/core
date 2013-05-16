# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

$(eval $(call gb_StaticLibrary_StaticLibrary,dtobj))

$(eval $(call gb_StaticLibrary_use_sdk_api,dtobj))

$(eval $(call gb_StaticLibrary_add_exception_objects,dtobj,\
	dtrans/source/win32/dtobj/APNDataObject \
	dtrans/source/win32/dtobj/DataFmtTransl \
	dtrans/source/win32/dtobj/DOTransferable \
	dtrans/source/win32/dtobj/DtObjFactory \
	dtrans/source/win32/dtobj/DTransHelper \
	dtrans/source/win32/dtobj/Fetc \
	dtrans/source/win32/dtobj/FetcList \
	dtrans/source/win32/dtobj/FmtFilter \
	dtrans/source/win32/dtobj/TxtCnvtHlp \
	dtrans/source/win32/dtobj/XNotifyingDataObject \
	dtrans/source/win32/dtobj/XTDataObject \
	dtrans/source/win32/misc/ImplHelper \
))

# vim: set noet sw=4 ts=4:
