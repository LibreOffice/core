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

$(eval $(call gb_Package_Package,filter_inc,$(SRCDIR)/filter/inc))

$(eval $(call gb_Package_add_file,filter_inc,inc/filter/msfilter/msfilterdllapi.h,filter/msfilter/msfilterdllapi.h))
$(eval $(call gb_Package_add_file,filter_inc,inc/filter/msfilter/countryid.hxx,filter/msfilter/countryid.hxx))
$(eval $(call gb_Package_add_file,filter_inc,inc/filter/msfilter/dffpropset.hxx,filter/msfilter/dffpropset.hxx))
$(eval $(call gb_Package_add_file,filter_inc,inc/filter/msfilter/dffrecordheader.hxx,filter/msfilter/dffrecordheader.hxx))
$(eval $(call gb_Package_add_file,filter_inc,inc/filter/msfilter/escherex.hxx,filter/msfilter/escherex.hxx))
$(eval $(call gb_Package_add_file,filter_inc,inc/filter/msfilter/mscodec.hxx,filter/msfilter/mscodec.hxx))
$(eval $(call gb_Package_add_file,filter_inc,inc/filter/msfilter/msdffimp.hxx,filter/msfilter/msdffimp.hxx))
$(eval $(call gb_Package_add_file,filter_inc,inc/filter/msfilter/msocximex.hxx,filter/msfilter/msocximex.hxx))
$(eval $(call gb_Package_add_file,filter_inc,inc/filter/msfilter/msoleexp.hxx,filter/msfilter/msoleexp.hxx))
$(eval $(call gb_Package_add_file,filter_inc,inc/filter/msfilter/mstoolbar.hxx,filter/msfilter/mstoolbar.hxx))
$(eval $(call gb_Package_add_file,filter_inc,inc/filter/msfilter/msvbahelper.hxx,filter/msfilter/msvbahelper.hxx))
$(eval $(call gb_Package_add_file,filter_inc,inc/filter/msfilter/svdfppt.hxx,filter/msfilter/svdfppt.hxx))
$(eval $(call gb_Package_add_file,filter_inc,inc/filter/msfilter/svxmsbas.hxx,filter/msfilter/svxmsbas.hxx))
$(eval $(call gb_Package_add_file,filter_inc,inc/filter/msfilter/rtfutil.hxx,filter/msfilter/rtfutil.hxx))
$(eval $(call gb_Package_add_file,filter_inc,inc/filter/msfilter/util.hxx,filter/msfilter/util.hxx))

# vim: set noet sw=4 ts=4:
