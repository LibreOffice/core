# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
$(eval $(call gb_Package_Package,cosv_inc,$(SRCDIR)/cosv/inc))

$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/csv_precomp.h,cosv/csv_precomp.h))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/bstream.hxx,cosv/bstream.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/comdline.hxx,cosv/comdline.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/comfunc.hxx,cosv/comfunc.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/csv_env.hxx,cosv/csv_env.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/csv_ostream.hxx,cosv/csv_ostream.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/dirchain.hxx,cosv/dirchain.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/file.hxx,cosv/file.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/openclose.hxx,cosv/openclose.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/persist.hxx,cosv/persist.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/ploc_dir.hxx,cosv/ploc_dir.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/ploc.hxx,cosv/ploc.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/plocroot.hxx,cosv/plocroot.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/std_outp.hxx,cosv/std_outp.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/streamstr.hxx,cosv/streamstr.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/stringdata.hxx,cosv/stringdata.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/string.hxx,cosv/string.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/str_types.hxx,cosv/str_types.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/x.hxx,cosv/x.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/tpl/dyn.hxx,cosv/tpl/dyn.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/tpl/processor.hxx,cosv/tpl/processor.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/tpl/swelist.hxx,cosv/tpl/swelist.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/tpl/tpltools.hxx,cosv/tpl/tpltools.hxx))
$(eval $(call gb_Package_add_file,cosv_inc,inc/cosv/tpl/vvector.hxx,cosv/tpl/vvector.hxx))

# vim: set noet sw=4 ts=4:
