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

$(eval $(call gb_Library_Library,icg))

$(eval $(call gb_Library_use_sdk_api,icg))

$(eval $(call gb_Library_set_include,icg,\
	$$(INCLUDE) \
))

$(eval $(call gb_Library_use_libraries,icg,\
        comphelper \
	tk \
	vcl \
	utl \
	tl \
	cppu \
	sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_standard_system_libs,icg))

$(eval $(call gb_Library_add_exception_objects,icg,\
	filter/source/graphicfilter/icgm/actimpr \
	filter/source/graphicfilter/icgm/bitmap \
	filter/source/graphicfilter/icgm/bundles \
	filter/source/graphicfilter/icgm/cgm \
	filter/source/graphicfilter/icgm/chart \
	filter/source/graphicfilter/icgm/class0 \
	filter/source/graphicfilter/icgm/class1 \
	filter/source/graphicfilter/icgm/class2 \
	filter/source/graphicfilter/icgm/class3 \
	filter/source/graphicfilter/icgm/class4 \
	filter/source/graphicfilter/icgm/class5 \
	filter/source/graphicfilter/icgm/class7 \
	filter/source/graphicfilter/icgm/classx \
	filter/source/graphicfilter/icgm/elements \
	filter/source/graphicfilter/icgm/outact \
))

# vim: set noet sw=4 ts=4:
