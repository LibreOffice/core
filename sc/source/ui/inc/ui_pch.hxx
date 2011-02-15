/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

//  ItemID-Defines etc. muessen immer ganz vorne stehen

#include "scitems.hxx"

#define _ZFORLIST_DECLARE_TABLE

#define _PRINTFUN_CXX
#define _INPUTHDL_CXX

#include <tools/solar.h>
#include <tools/string.hxx>
#include <svl/solar.hrc>
#include <tools/rtti.hxx>
#include <string.h>
#include <tools/contnr.hxx>
#include <vcl/sv.h>
#include <tools/ref.hxx>
#include <tools/list.hxx>
#include <tools/link.hxx>
#include <i18npool/lang.h>
#include <sfx2/sfx.hrc>
#include <svl/memberid.hrc>
#include <sfx2/sfxsids.hrc>
#include <svl/cntwids.hrc>
#include <tools/resid.hxx>
#include <tools/rc.hxx>
#include <tools/resmgr.hxx>
#include <tools/gen.hxx>
#include <tools/fract.hxx>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <vcl/keycod.hxx>
#include <vcl/keycodes.hxx>
#include <tools/globname.hxx>
#include <tools/color.hxx>
#include <vcl/region.hxx>
#include <vcl/mapmod.hxx>
#include <vcl/bitmap.hxx>
#include <svx/svxids.hrc>
#include <vcl/vclenum.hxx>
#include <sot/object.hxx>
#include <sot/sotdata.hxx>
#include <sot/factory.hxx>
#include <tools/ownlist.hxx>
#include <vcl/font.hxx>
#include <vcl/wall.hxx>
#include <vcl/settings.hxx>
#include <vcl/accel.hxx>
#include <vcl/outdev.hxx>
#include <vcl/pointr.hxx>
#include <vcl/ptrstyle.hxx>
#include <vcl/wintypes.hxx>
#include <vcl/event.hxx>
#include <limits.h>
#include <tools/errinf.hxx>
#include <tools/errcode.hxx>
#include <tools/stream.hxx>
#include <vcl/color.hxx>
#include <vcl/window.hxx>
#include <global.hxx>
#include <vcl/syswin.hxx>
#include <tools/debug.hxx>
#include <svl/svarray.hxx>
#include <vcl/ctrl.hxx>
#include <svl/hint.hxx>
#include <svl/poolitem.hxx>
#include <vcl/image.hxx>
#include <vcl/timer.hxx>
#include <tools/unqidx.hxx>
#include <vcl/symbol.hxx>
#include <vcl/fixed.hxx>
#include <svl/brdcst.hxx>
#include <vcl/timer.hxx>
#include <sc.hrc>
#include <stdarg.h>
#include <svx/dialogs.hrc>
#include <basic/sbxdef.hxx>
#include <svl/lstner.hxx>
#include <sfx2/shell.hxx>
#include <vcl/window.hxx>
#include <vcl/group.hxx>
#include <tools/unqid.hxx>
#include <tools/table.hxx>
#include <svl/itemset.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <rangelst.hxx>
#include <vcl/edit.hxx>
#include <vcl/combobox.h>
#include <tools/shl.hxx>
#include <vcl/field.hxx>
#include <vcl/spinfld.hxx>
#include <vcl/combobox.hxx>
#include <tools/pstm.hxx>
#include <svl/eitem.hxx>
#include <svl/itempool.hxx>
#include <sfx2/tabdlg.hxx>
#include <vcl/tabdlg.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/tabctrl.hxx>
#include <shellids.hxx>
#include <vcl/event.hxx>
#include <svl/intitem.hxx>
#include <svtools/confitem.hxx>
#include <markdata.hxx>
#include <svx/optgrid.hxx>
#include <vcl/apptypes.hxx>
#include <viewopti.hxx>
#include <viewdata.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/clientsh.hxx>
#include <document.hxx>
#include <vcl/prntypes.hxx>
#include <table.hxx>
#include <column.hxx>
#include <markarr.hxx>
#include <vcl/mapmod.hxx>
#include <svl/undo.hxx>
#include <vcl/seleng.hxx>
#include <rsc/rscsfx.hxx>
#include <svx/zoomitem.hxx>
#include <vcl/scrbar.hxx>
#include <viewutil.hxx>
#include <vcl/svapp.hxx>
#include <tools/datetime.hxx>
#include <tabview.hxx>
#include <select.hxx>
#include <sfx2/imgdef.hxx>
#include <viewfunc.hxx>
#include <sfx2/module.hxx>
#include <dbfunc.hxx>
#include <vcl/accel.hxx>
#include <svl/smplhint.hxx>
#include <sfx2/viewfac.hxx>
#include <sot/storage.hxx>
#include <sfx2/objsh.hxx>
#include <target.hxx>
#include <tabvwsh.hxx>
#include <svl/inethist.hxx>
#include <svl/inetdef.hxx>
#include <sfx2/sfxdefs.hxx>
#include <svtools/compat.hxx>
#include <svl/inetmsg.hxx>
#include <sfx2/app.hxx>
#include <tools/urlobj.hxx>
#include <scdll.hxx>
#include <sfx2/msg.hxx>
#include <svl/ownlist.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/frame.hxx>
#include <scresid.hxx>
#include <sfx2/ipfrm.hxx>
#include <docsh.hxx>
#include <vcl/wintypes.hxx>
#include <sfx2/docfac.hxx>
#include <scitems.hxx>
#include <vcl/virdev.hxx>
#include <globstr.hrc>
#include <tcov.hxx>
#include <vcl/dialog.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/lstbox.h>
#include <sfx2/minarray.hxx>
#include <sfx2/bindings.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/pointr.hxx>
#include <vcl/prntypes.hxx>
#include <vcl/jobset.hxx>
#include <svx/svdtypes.hxx>
#include <vcl/btndlg.hxx>
#include <vcl/svapp.hxx>
#include <svl/svstdarr.hxx>
#include <vcl/edit.hxx>
#include <svl/stritem.hxx>
#include <collect.hxx>
#include <svx/svdsob.hxx>
#include <vcl/imagebtn.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdglue.hxx>
#include <svx/svdlayer.hxx>
#include <vcl/msgbox.hxx>
#include <svx/svdmrkv.hxx>
#include <svx/svdmark.hxx>
#include <svx/svdhdl.hxx>
#include <svx/svdsnpv.hxx>
#include <svx/svdpntv.hxx>
#include <svx/svddrag.hxx>
#include <svx/svdhlpln.hxx>
#include <scmod.hxx>
#include <sfx2/request.hxx>
#include <vcl/field.hxx>
#include <svx/svdview.hxx>
#include <svx/svdcrtv.hxx>
#include <vcl/print.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/wrkwin.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/chalign.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/graph.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/animate.hxx>
#include <vcl/graph.h>
#include <editeng/editdata.hxx>
#include <svtools/stdctrl.hxx>
#include <sfx2/basedlgs.hxx>
#include <conditio.hxx>
#include <vcl/combobox.hxx>
#include <vcl/print.hxx>
#include <uiitems.hxx>
#include <svx/svdmodel.hxx>
#include <vcl/gdiobj.hxx>
#include <editeng/editeng.hxx>
#include <svx/pageitem.hxx>
#include <anyrefdg.hxx>
#include <basic/sbxcore.hxx>
#include <vcl/ctrl.hxx>
#include <svx/xenum.hxx>
#include <dbcolect.hxx>
#include <sot/exchange.hxx>
#include <math.h>
#include <vcl/wrkwin.hxx>
#include <basic/sbxvar.hxx>
#include <vcl/image.hxx>
#include <svx/xdef.hxx>
#include <drwlayer.hxx>
#include <editutil.hxx>
#include <svx/xit.hxx>
#include <svx/xcolit.hxx>
#include <attrib.hxx>
#include <patattr.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xflasit.hxx>
#include <svx/xlnasit.hxx>
#include <svx/svdattr.hxx>
#include <svx/sdangitm.hxx>
#include <svx/sderitm.hxx>
#include <svx/sdmetitm.hxx>
#include <svx/svddef.hxx>
#include <svx/sdooitm.hxx>
#include <svx/sdprcitm.hxx>
#include <svx/sdshcitm.hxx>
#include <svx/sdshitm.hxx>
#include <svx/sdshtitm.hxx>
#include <svx/sdsxyitm.hxx>
#include <svx/sdtaaitm.hxx>
#include <svx/sdtacitm.hxx>
#include <svx/sdtaditm.hxx>
#include <svx/sdtagitm.hxx>
#include <svx/sdtaiitm.hxx>
#include <svx/sdynitm.hxx>
#include <svx/sdtaitm.hxx>
#include <svx/sdtakitm.hxx>
#include <svx/sdtayitm.hxx>
#include <svx/sdtcfitm.hxx>
#include <svx/sdtditm.hxx>
#include <svx/sdtfsitm.hxx>
#include <svx/sdtmfitm.hxx>
#include <tools/poly.hxx>
#include <vcl/gdimtf.hxx>
#include <cell.hxx>
#include <stddef.h>
#include <tools/mempool.hxx>
#include <svl/style.hrc>
#include <drawview.hxx>
#include <svx/view3d.hxx>
#include <svx/def3d.hxx>
#include <vcl/floatwin.hxx>
#include <basic/sbxobj.hxx>
#include <editeng/svxenum.hxx>
#include <svx/xtextit0.hxx>
#include <svx/svdtrans.hxx>
#include <svl/style.hxx>
#include <reffact.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdoattr.hxx>
#include <tools/wldcrd.hxx>
#include <rangenam.hxx>
#include <undobase.hxx>
#include <vcl/dockwin.hxx>
#include <vcl/virdev.hxx>
#include <rangeutl.hxx>
#include <vcl/toolbox.hxx>
#include <fupoor.hxx>
#include <docpool.hxx>
#include <vcl/button.hxx>
#include <svl/srchitem.hxx>
#include <svx/svdorect.hxx>
#include <svl/whiter.hxx>
#include <stlpool.hxx>
#include <editeng/eeitem.hxx>
#include <inputhdl.hxx>
#include <svx/svdpage.hxx>




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
