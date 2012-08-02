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

$(eval $(call gb_Package_Package,svl_inc,$(SRCDIR)/svl/inc))


$(eval $(call gb_Package_add_file,svl_inc,inc/svl/PasswordHelper.hxx,svl/PasswordHelper.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/adrparse.hxx,svl/adrparse.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/aeitem.hxx,svl/aeitem.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/asiancfg.hxx,svl/asiancfg.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/brdcst.hxx,svl/brdcst.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/broadcast.hxx,svl/broadcast.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/broadcast.hxx,svl/broadcast.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/cenumitm.hxx,svl/cenumitm.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/cintitem.hxx,svl/cintitem.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/cjkoptions.hxx,svl/cjkoptions.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/cntwall.hxx,svl/cntwall.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/cntwids.hrc,svl/cntwids.hrc))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/converter.hxx,svl/converter.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/ctloptions.hxx,svl/ctloptions.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/ctypeitm.hxx,svl/ctypeitm.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/custritm.hxx,svl/custritm.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/dateitem.hxx,svl/dateitem.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/documentlockfile.hxx,svl/documentlockfile.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/eitem.hxx,svl/eitem.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/filenotation.hxx,svl/filenotation.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/filerec.hxx,svl/filerec.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/flagitem.hxx,svl/flagitem.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/folderrestriction.hxx,svl/folderrestriction.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/fstathelper.hxx,svl/fstathelper.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/globalnameitem.hxx,svl/globalnameitem.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/hint.hxx,svl/hint.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/ilstitem.hxx,svl/ilstitem.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/imageitm.hxx,svl/imageitm.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/inethist.hxx,svl/inethist.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/inetmsg.hxx,svl/inetmsg.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/inetstrm.hxx,svl/inetstrm.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/inettype.hxx,svl/inettype.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/instrm.hxx,svl/instrm.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/intitem.hxx,svl/intitem.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/isethint.hxx,svl/isethint.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/itemiter.hxx,svl/itemiter.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/itempool.hxx,svl/itempool.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/itemprop.hxx,svl/itemprop.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/itemset.hxx,svl/itemset.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/languageoptions.hxx,svl/languageoptions.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/lckbitem.hxx,svl/lckbitem.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/listener.hxx,svl/listener.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/listeneriter.hxx,svl/listeneriter.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/lngmisc.hxx,svl/lngmisc.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/lockfilecommon.hxx,svl/lockfilecommon.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/lstner.hxx,svl/lstner.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/macitem.hxx,svl/macitem.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/mailenum.hxx,svl/mailenum.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/memberid.hrc,svl/memberid.hrc))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/metitem.hxx,svl/metitem.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/nfkeytab.hxx,svl/nfkeytab.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/nfsymbol.hxx,svl/nfsymbol.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/nfversi.hxx,svl/nfversi.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/nranges.hxx,svl/nranges.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/numuno.hxx,svl/numuno.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/ondemand.hxx,svl/ondemand.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/outstrm.hxx,svl/outstrm.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/ownlist.hxx,svl/ownlist.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/pickerhistoryaccess.hxx,svl/pickerhistoryaccess.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/poolcach.hxx,svl/poolcach.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/poolitem.hxx,svl/poolitem.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/ptitem.hxx,svl/ptitem.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/rectitem.hxx,svl/rectitem.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/restrictedpaths.hxx,svl/restrictedpaths.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/rngitem.hxx,svl/rngitem.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/sfontitm.hxx,svl/sfontitm.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/sharecontrolfile.hxx,svl/sharecontrolfile.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/slstitm.hxx,svl/slstitm.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/smplhint.hxx,svl/smplhint.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/solar.hrc,svl/solar.hrc))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/srchdefs.hxx,svl/srchdefs.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/srchitem.hxx,svl/srchitem.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/stritem.hxx,svl/stritem.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/strmadpt.hxx,svl/strmadpt.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/style.hrc,svl/style.hrc))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/style.hxx,svl/style.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/stylepool.hxx,svl/stylepool.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/svdde.hxx,svl/svdde.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/svl.hrc,svl/svl.hrc))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/svldllapi.h,svl/svldllapi.h))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/szitem.hxx,svl/szitem.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/undo.hxx,svl/undo.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/urihelper.hxx,svl/urihelper.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/urlbmk.hxx,svl/urlbmk.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/urlfilter.hxx,svl/urlfilter.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/visitem.hxx,svl/visitem.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/whiter.hxx,svl/whiter.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/xmlement.hxx,svl/xmlement.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/zforlist.hxx,svl/zforlist.hxx))
$(eval $(call gb_Package_add_file,svl_inc,inc/svl/zformat.hxx,svl/zformat.hxx))

# vim: set noet sw=4 ts=4:
