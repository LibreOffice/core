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

$(eval $(call gb_WinResTarget_WinResTarget,vcl/salsrc))

$(eval $(call gb_WinResTarget_set_include,vcl/salsrc,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_WinResTarget_set_rcfile,vcl/salsrc,\
    vcl/win/source/src/salsrc \
))
$(eval $(call gb_WinResTarget_add_dependencies,vcl/salsrc,\
    vcl/win/source/src/dtext.cur \
    vcl/win/source/src/50.bmp \
    vcl/win/source/src/copydata.cur \
    vcl/win/source/src/dpie.cur \
    vcl/win/source/src/movedata.cur \
    vcl/win/source/src/rotate.cur \
    vcl/win/source/src/50.png \
    vcl/win/source/src/copydlnk.cur \
    vcl/win/source/src/dpolygon.cur \
    vcl/win/source/src/movedlnk.cur \
    vcl/win/source/src/salsrc.rc \
    vcl/win/source/src/airbrush.cur \
    vcl/win/source/src/copyf.cur \
    vcl/win/source/src/drect.cur \
    vcl/win/source/src/movef.cur \
    vcl/win/source/src/ase.cur \
    vcl/win/source/src/copyf2.cur \
    vcl/win/source/src/dtext.cur \
    vcl/win/source/src/movef2.cur \
    vcl/win/source/src/tblsele.cur \
    vcl/win/source/src/asn.cur \
    vcl/win/source/src/copyflnk.cur \
    vcl/win/source/src/fill.cur \
    vcl/win/source/src/moveflnk.cur \
    vcl/win/source/src/tblsels.cur \
    vcl/win/source/src/asne.cur \
    vcl/win/source/src/crook.cur \
    vcl/win/source/src/hand.cur \
    vcl/win/source/src/movept.cur \
    vcl/win/source/src/tblselse.cur \
    vcl/win/source/src/asns.cur \
    vcl/win/source/src/crop.cur \
    vcl/win/source/src/help.cur \
    vcl/win/source/src/neswsize.cur \
    vcl/win/source/src/tblselsw.cur \
    vcl/win/source/src/asnswe.cur \
    vcl/win/source/src/cross.cur \
    vcl/win/source/src/hshear.cur \
    vcl/win/source/src/notallow.cur \
    vcl/win/source/src/tblselw.cur \
    vcl/win/source/src/asnw.cur \
    vcl/win/source/src/darc.cur \
    vcl/win/source/src/hsize.cur \
    vcl/win/source/src/nullptr.cur \
    vcl/win/source/src/timemove.cur \
    vcl/win/source/src/ass.cur \
    vcl/win/source/src/dbezier.cur \
    vcl/win/source/src/hsizebar.cur \
    vcl/win/source/src/nwsesize.cur \
    vcl/win/source/src/timesize.cur \
    vcl/win/source/src/asse.cur \
    vcl/win/source/src/dcapt.cur \
    vcl/win/source/src/hsplit.cur \
    vcl/win/source/src/pen.cur \
    vcl/win/source/src/vshear.cur \
    vcl/win/source/src/assw.cur \
    vcl/win/source/src/dcirccut.cur \
    vcl/win/source/src/linkdata.cur \
    vcl/win/source/src/pivotcol.cur \
    vcl/win/source/src/vsize.cur \
    vcl/win/source/src/asw.cur \
    vcl/win/source/src/dconnect.cur \
    vcl/win/source/src/linkf.cur \
    vcl/win/source/src/pivotdel.cur \
    vcl/win/source/src/vsizebar.cur \
    vcl/win/source/src/aswe.cur \
    vcl/win/source/src/dellipse.cur \
    vcl/win/source/src/magnify.cur \
    vcl/win/source/src/pivotfld.cur \
    vcl/win/source/src/vsplit.cur \
    vcl/win/source/src/chain.cur \
    vcl/win/source/src/detectiv.cur \
    vcl/win/source/src/mirror.cur \
    vcl/win/source/src/pivotrow.cur \
    vcl/win/source/src/vtext.cur \
    vcl/win/source/src/chainnot.cur \
    vcl/win/source/src/dfree.cur \
    vcl/win/source/src/move.cur \
    vcl/win/source/src/pntbrsh.cur \
    vcl/win/source/src/chart.cur \
    vcl/win/source/src/dline.cur \
    vcl/win/source/src/movebw.cur \
    vcl/win/source/src/refhand.cur \
))

# vim: set noet sw=4 ts=4:
