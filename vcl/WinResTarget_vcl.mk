#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2011 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_WinResTarget_WinResTarget,vcl/src))

$(eval $(call gb_WinResTarget_set_include,vcl/src,\
        $$(INCLUDE) \
        -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_WinResTarget_add_file,vcl/src,\
    vcl/win/source/src/salsrc \
))
$(eval $(call gb_WinResTarget_add_dependency,vcl/src,\
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
    vcl/win/source/src/sd.ico \
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
