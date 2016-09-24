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
    vcl/win/src/salsrc \
))
$(eval $(call gb_WinResTarget_add_dependencies,vcl/salsrc,\
    vcl/win/src/dtext.cur \
    vcl/win/src/50.bmp \
    vcl/win/src/copydata.cur \
    vcl/win/src/dpie.cur \
    vcl/win/src/movedata.cur \
    vcl/win/src/rotate.cur \
    vcl/win/src/50.png \
    vcl/win/src/copydlnk.cur \
    vcl/win/src/dpolygon.cur \
    vcl/win/src/movedlnk.cur \
    vcl/win/src/salsrc.rc \
    vcl/win/src/airbrush.cur \
    vcl/win/src/copyf.cur \
    vcl/win/src/drect.cur \
    vcl/win/src/movef.cur \
    vcl/win/src/ase.cur \
    vcl/win/src/copyf2.cur \
    vcl/win/src/dtext.cur \
    vcl/win/src/movef2.cur \
    vcl/win/src/tblsele.cur \
    vcl/win/src/asn.cur \
    vcl/win/src/copyflnk.cur \
    vcl/win/src/fill.cur \
    vcl/win/src/moveflnk.cur \
    vcl/win/src/tblsels.cur \
    vcl/win/src/asne.cur \
    vcl/win/src/crook.cur \
    vcl/win/src/movept.cur \
    vcl/win/src/tblselse.cur \
    vcl/win/src/asns.cur \
    vcl/win/src/crop.cur \
    vcl/win/src/tblselsw.cur \
    vcl/win/src/asnswe.cur \
    vcl/win/src/hshear.cur \
    vcl/win/src/tblselw.cur \
    vcl/win/src/asnw.cur \
    vcl/win/src/darc.cur \
    vcl/win/src/nullptr.cur \
    vcl/win/src/timemove.cur \
    vcl/win/src/ass.cur \
    vcl/win/src/dbezier.cur \
    vcl/win/src/timesize.cur \
    vcl/win/src/asse.cur \
    vcl/win/src/dcapt.cur \
    vcl/win/src/vshear.cur \
    vcl/win/src/assw.cur \
    vcl/win/src/dcirccut.cur \
    vcl/win/src/linkdata.cur \
    vcl/win/src/pivotcol.cur \
    vcl/win/src/asw.cur \
    vcl/win/src/dconnect.cur \
    vcl/win/src/linkf.cur \
    vcl/win/src/pivotdel.cur \
    vcl/win/src/aswe.cur \
    vcl/win/src/dellipse.cur \
    vcl/win/src/magnify.cur \
    vcl/win/src/pivotfld.cur \
    vcl/win/src/chain.cur \
    vcl/win/src/detectiv.cur \
    vcl/win/src/mirror.cur \
    vcl/win/src/pivotrow.cur \
    vcl/win/src/vtext.cur \
    vcl/win/src/chainnot.cur \
    vcl/win/src/dfree.cur \
    vcl/win/src/pntbrsh.cur \
    vcl/win/src/chart.cur \
    vcl/win/src/dline.cur \
    vcl/win/src/movebw.cur \
))

# vim: set noet sw=4 ts=4:
