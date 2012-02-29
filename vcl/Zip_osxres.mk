# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2012 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Zip_Zip,osxres,$(SRCDIR)/vcl/aqua/source/res))

$(eval $(call gb_Zip_add_files,osxres,\
    MainMenu.nib/classes.nib \
    MainMenu.nib/info.nib \
    MainMenu.nib/keyedobjects.nib \
    cursors/airbrush.png \
    cursors/ase.png \
    cursors/asn.png \
    cursors/asne.png \
    cursors/asns.png \
    cursors/asnswe.png \
    cursors/asnw.png \
    cursors/ass.png \
    cursors/asse.png \
    cursors/assw.png \
    cursors/asw.png \
    cursors/aswe.png \
    cursors/chain.png \
    cursors/chainnot.png \
    cursors/chart.png \
    cursors/copydata.png \
    cursors/copydlnk.png \
    cursors/copyf.png \
    cursors/copyf2.png \
    cursors/copyflnk.png \
    cursors/crook.png \
    cursors/crop.png \
    cursors/darc.png \
    cursors/dbezier.png \
    cursors/dcapt.png \
    cursors/dcirccut.png \
    cursors/dconnect.png \
    cursors/dellipse.png \
    cursors/detectiv.png \
    cursors/dfree.png \
    cursors/dline.png \
    cursors/dpie.png \
    cursors/dpolygon.png \
    cursors/drect.png \
    cursors/dtext.png \
    cursors/fill.png \
    cursors/help.png \
    cursors/hourglass.png \
    cursors/hshear.png \
    cursors/linkdata.png \
    cursors/linkf.png \
    cursors/magnify.png \
    cursors/mirror.png \
    cursors/movebw.png \
    cursors/movedata.png \
    cursors/movedlnk.png \
    cursors/movef.png \
    cursors/movef2.png \
    cursors/moveflnk.png \
    cursors/movept.png \
    cursors/neswsize.png \
    cursors/notallow.png \
    cursors/nullptr.png \
    cursors/nwsesize.png \
    cursors/pen.png \
    cursors/pivotcol.png \
    cursors/pivotdel.png \
    cursors/pivotfld.png \
    cursors/pivotrow.png \
    cursors/pntbrsh.png \
    cursors/rotate.png \
    cursors/tblsele.png \
    cursors/tblsels.png \
    cursors/tblselse.png \
    cursors/tblselsw.png \
    cursors/tblselw.png \
    cursors/timemove.png \
    cursors/timesize.png \
    cursors/vshear.png \
    cursors/vtext.png \
))

# vim:set shiftwidth=4 softtabstop=4 expandtab:
