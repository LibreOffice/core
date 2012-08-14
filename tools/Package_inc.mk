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

$(eval $(call gb_Package_Package,tools_inc,$(SRCDIR)/tools/inc))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/StringListResource.hxx,tools/StringListResource.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/appendunixshellword.hxx,tools/appendunixshellword.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/b3dtrans.hxx,tools/b3dtrans.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/bigint.hxx,tools/bigint.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/cachestr.hxx,tools/cachestr.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/color.hxx,tools/color.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/config.hxx,tools/config.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/contnr.hxx,tools/contnr.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/date.hxx,tools/date.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/datetime.hxx,tools/datetime.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/debug.hxx,tools/debug.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/diagnose_ex.h,tools/diagnose_ex.h))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/errcode.hxx,tools/errcode.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/errinf.hxx,tools/errinf.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/extendapplicationenvironment.hxx,tools/extendapplicationenvironment.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/fldunit.hxx,tools/fldunit.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/fontenum.hxx,tools/fontenum.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/fract.hxx,tools/fract.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/fsys.hxx,tools/fsys.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/gen.hxx,tools/gen.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/getprocessworkingdir.hxx,tools/getprocessworkingdir.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/globname.hxx,tools/globname.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/helpers.hxx,tools/helpers.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/inetmime.hxx,tools/inetmime.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/inetmsg.hxx,tools/inetmsg.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/inetstrm.hxx,tools/inetstrm.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/line.hxx,tools/line.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/lineend.hxx,tools/lineend.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/link.hxx,tools/link.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/mapunit.hxx,tools/mapunit.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/mempool.hxx,tools/mempool.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/multisel.hxx,tools/multisel.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/pathutils.hxx,tools/pathutils.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/poly.hxx,tools/poly.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/postsys.h,tools/postsys.h))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/postx.h,tools/postx.h))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/presys.h,tools/presys.h))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/prex.h,tools/prex.h))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/pstm.hxx,tools/pstm.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/rc.h,tools/rc.h))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/rc.hxx,tools/rc.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/rcid.h,tools/rcid.h))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/ref.hxx,tools/ref.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/resary.hxx,tools/resary.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/resid.hxx,tools/resid.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/resmgr.hxx,tools/resmgr.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/rtti.hxx,tools/rtti.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/shl.hxx,tools/shl.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/simplerm.hxx,tools/simplerm.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/solar.h,tools/solar.h))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/solarmutex.hxx,tools/solarmutex.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/stream.hxx,tools/stream.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/string.hxx,tools/string.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/svborder.hxx,tools/svborder.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/tempfile.hxx,tools/tempfile.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/tenccvt.hxx,tools/tenccvt.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/time.hxx,tools/time.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/tools.h,tools/tools.h))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/toolsdllapi.h,tools/toolsdllapi.h))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/unqidx.hxx,tools/unqidx.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/urlobj.hxx,tools/urlobj.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/vcompat.hxx,tools/vcompat.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/vector2d.hxx,tools/vector2d.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/weakbase.h,tools/weakbase.h))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/weakbase.hxx,tools/weakbase.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/wintypes.hxx,tools/wintypes.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/wldcrd.hxx,tools/wldcrd.hxx))
$(eval $(call gb_Package_add_file,tools_inc,inc/tools/zcodec.hxx,tools/zcodec.hxx))

# vim: set noet sw=4 ts=4:
