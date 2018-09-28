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



$(eval $(call gb_WinResTarget_WinResTarget,vcl/src))

$(eval $(call gb_WinResTarget_set_include,vcl/src,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

ifeq ($(OS),WNT)

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

else

$(eval $(call gb_WinResTarget_add_file,vcl/src,\
    vcl/os2/source/src/salsrc \
))

$(eval $(call gb_WinResTarget_add_dependency,vcl/src,\
    vcl/os2/source/src/airbrush.ptr \
    vcl/os2/source/src/ase.ptr \
    vcl/os2/source/src/asn.ptr \
    vcl/os2/source/src/asne.ptr \
    vcl/os2/source/src/asns.ptr \
    vcl/os2/source/src/asnswe.ptr \
    vcl/os2/source/src/asnw.ptr \
    vcl/os2/source/src/ass.ptr \
    vcl/os2/source/src/asse.ptr \
    vcl/os2/source/src/assw.ptr \
    vcl/os2/source/src/asw.ptr \
    vcl/os2/source/src/aswe.ptr \
    vcl/os2/source/src/chain.ptr \
    vcl/os2/source/src/chainnot.ptr \
    vcl/os2/source/src/chart.ptr \
    vcl/os2/source/src/copydata.ptr \
    vcl/os2/source/src/copydlnk.ptr \
    vcl/os2/source/src/copyf.ptr \
    vcl/os2/source/src/copyf2.ptr \
    vcl/os2/source/src/copyflnk.ptr \
    vcl/os2/source/src/crook.ptr \
    vcl/os2/source/src/crop.ptr \
    vcl/os2/source/src/cross.ptr \
    vcl/os2/source/src/darc.ptr \
    vcl/os2/source/src/dbezier.ptr \
    vcl/os2/source/src/dcapt.ptr \
    vcl/os2/source/src/dcirccut.ptr \
    vcl/os2/source/src/dconnect.ptr \
    vcl/os2/source/src/dellipse.ptr \
    vcl/os2/source/src/detectiv.ptr \
    vcl/os2/source/src/dfree.ptr \
    vcl/os2/source/src/dline.ptr \
    vcl/os2/source/src/dpie.ptr \
    vcl/os2/source/src/dpolygon.ptr \
    vcl/os2/source/src/drect.ptr \
    vcl/os2/source/src/dtext.ptr \
    vcl/os2/source/src/dtext.ptr \
    vcl/os2/source/src/fill.ptr \
    vcl/os2/source/src/hand.ptr \
    vcl/os2/source/src/help.ptr \
    vcl/os2/source/src/hshear.ptr \
    vcl/os2/source/src/hsizebar.ptr \
    vcl/os2/source/src/hsplit.ptr \
    vcl/os2/source/src/linkdata.ptr \
    vcl/os2/source/src/linkf.ptr \
    vcl/os2/source/src/magnify.ptr \
    vcl/os2/source/src/mirror.ptr \
    vcl/os2/source/src/move.ptr \
    vcl/os2/source/src/movebw.ptr \
    vcl/os2/source/src/movedata.ptr \
    vcl/os2/source/src/movedlnk.ptr \
    vcl/os2/source/src/movef.ptr \
    vcl/os2/source/src/movef2.ptr \
    vcl/os2/source/src/moveflnk.ptr \
    vcl/os2/source/src/movept.ptr \
    vcl/os2/source/src/nullptr.ptr \
    vcl/os2/source/src/pen.ptr \
    vcl/os2/source/src/pivotcol.ptr \
    vcl/os2/source/src/pivotfld.ptr \
    vcl/os2/source/src/pivotrow.ptr \
    vcl/os2/source/src/pntbrsh.ptr \
    vcl/os2/source/src/refhand.ptr \
    vcl/os2/source/src/rotate.ptr \
    vcl/os2/source/src/salsrc.rc \
    vcl/os2/source/src/sd.ico \
    vcl/os2/source/src/tblsele.ptr \
    vcl/os2/source/src/tblsels.ptr \
    vcl/os2/source/src/tblselse.ptr \
    vcl/os2/source/src/tblselsw.ptr \
    vcl/os2/source/src/tblselw.ptr \
    vcl/os2/source/src/timemove.ptr \
    vcl/os2/source/src/timesize.ptr \
    vcl/os2/source/src/vshear.ptr \
    vcl/os2/source/src/vsizebar.ptr \
    vcl/os2/source/src/vsplit.ptr \
    vcl/os2/source/src/vtext.ptr \
))

$(eval $(call gb_WinResTarget_set_include,vcl/src,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/os2/source/src \
))

endif

# vim: set noet sw=4 ts=4:
