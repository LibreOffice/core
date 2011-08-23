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

//	ItemID-Defines etc. muessen immer ganz vorne stehen

#include "scitems.hxx"
#include "eetext.hxx"

#define ITEMID_FIELD EE_FEATURE_FIELD

#define _ZFORLIST_DECLARE_TABLE

#define _PRINTFUN_CXX
#define _INPUTHDL_CXX

//	ab hier automatisch per makepch generiert
//	folgende duerfen nicht aufgenommen werden:
//		scslots.hxx

#include <tools/solar.h>
#include <tools/string.hxx>
#include <bf_svtools/solar.hrc>
#include <tools/rtti.hxx>
#include <string.h>
#include <tools/contnr.hxx>
#include <vcl/sv.h>
#include <tools/ref.hxx>
#include <tools/list.hxx>
#include <tools/link.hxx>
#include <tools/lang.hxx>
#include <bf_sfx2/sfx.hrc>
#include <bf_svtools/memberid.hrc>
#include <bf_sfx2/sfxsids.hrc>
#include <bf_svtools/cntwids.hrc>
//#include <vcl/resid.hxx>
#include <tools/resid.hxx>
//#include <vcl/rc.hxx>
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
#include <sot/sotref.hxx>
#include <bf_svx/svxids.hrc>
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
//#include <vcl/color.hxx>
#include <vcl/window.hxx>
#include <global.hxx>
#include <vcl/syswin.hxx>
#include <tools/debug.hxx>
#include <bf_svtools/svarray.hxx>
#include <vcl/ctrl.hxx>
#include <bf_svtools/hint.hxx>
#include <bf_svtools/poolitem.hxx>
#include <bf_svtools/args.hxx>
#include <vcl/image.hxx>
#include <vcl/timer.hxx>
#include <tools/unqidx.hxx>
#include <vcl/symbol.hxx>
#include <vcl/fixed.hxx>
#include <bf_svtools/brdcst.hxx>
#include <vcl/timer.hxx>
#include <sc.hrc>
#include <stdarg.h>
#include <bf_svx/dialogs.hrc>
#include "bf_basic/sbxdef.hxx"
#include <bf_svtools/lstner.hxx>
#include <bf_sfx2/shell.hxx>
#include <vcl/window.hxx>
#include <vcl/group.hxx>
#include <tools/unqid.hxx>
#include <tools/table.hxx>
#include <bf_svtools/itemset.hxx>
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
#include <bf_svtools/eitem.hxx>
#include <bf_svtools/itempool.hxx>
#include <bf_so3/so2ref.hxx>
#include <vcl/tabdlg.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/tabctrl.hxx>
#include <shellids.hxx>
#include <vcl/event.hxx>
#include <bf_so3/advise.hxx>
#include <bf_so3/protocol.hxx>
#include <bf_svtools/intitem.hxx>
#include <bf_so3/ipclient.hxx>
#include <bf_sfx2/cfgitem.hxx>
#include <bf_svtools/confitem.hxx>
#include <markdata.hxx>
#include <bf_svx/optgrid.hxx>
#include <vcl/apptypes.hxx>
#include <viewopti.hxx>
#include <viewdata.hxx>
//#include <vcl/resid.hxx>
#include <bf_sfx2/viewsh.hxx>
#include <bf_sfx2/clientsh.hxx>
#include <document.hxx>
#include <vcl/prntypes.hxx>
#include <table.hxx>
#include <column.hxx>
#include <markarr.hxx>
#include <vcl/mapmod.hxx>
#include <bf_svtools/undo.hxx>
#include <vcl/seleng.hxx>
#include <rsc/rscsfx.hxx>
#include <bf_svx/zoomitem.hxx>
#include <vcl/scrbar.hxx>
#include <viewutil.hxx>
#include <vcl/svapp.hxx>
#include <tools/datetime.hxx>
#include <tabview.hxx>
#include <select.hxx>
#include <bf_sfx2/imgdef.hxx>
#include <viewfunc.hxx>
#include <bf_sfx2/module.hxx>
#include <dbfunc.hxx>
#include <vcl/accel.hxx>
#include <bf_svtools/smplhint.hxx>
#include <bf_sfx2/viewfac.hxx>
#include <bf_so3/svstor.hxx>
#include <bf_sfx2/objsh.hxx>
#include <bf_so3/so2defs.hxx>
#include <bf_so3/persist.hxx>
#include <target.hxx>
#include <tabvwsh.hxx>
#include <bf_svtools/inethist.hxx>
#include <bf_svtools/inetdef.hxx>
#include <bf_sfx2/sfxdefs.hxx>
#include <bf_svtools/cancel.hxx>
#include <bf_svtools/inetmsg.hxx>
#include <bf_sfx2/app.hxx>
#include <tools/urlobj.hxx>
#include <scdll.hxx>
#include <bf_sfx2/msg.hxx>
#include <bf_svtools/ownlist.hxx>
#include <bf_so3/persist.hxx>
#include <bf_so3/pseudo.hxx>
#include <bf_sfx2/objface.hxx>
#include <bf_sfx2/viewfrm.hxx>
#include <bf_sfx2/frame.hxx>
#include <bf_so3/ipobj.hxx>
#include <bf_so3/embobj.hxx>
#include <bf_so3/ipobj.hxx>
#include <scresid.hxx>
#include <docsh.hxx>
#include <bf_sfx2/interno.hxx>
#include <vcl/wintypes.hxx>
#include <bf_sfx2/docfac.hxx>
#include <scitems.hxx>
#include <vcl/virdev.hxx>
#include <globstr.hrc>
#include <tcov.hxx>
#include <vcl/dialog.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/lstbox.h>
#include <bf_sfx2/minarray.hxx>
#include <bf_sfx2/bindings.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/pointr.hxx>
#include <vcl/prntypes.hxx>
#include <vcl/jobset.hxx>
#include <bf_svx/svdtypes.hxx>
#include <vcl/btndlg.hxx>
#include <vcl/svapp.hxx>
#include <bf_svtools/svstdarr.hxx>
#include <vcl/edit.hxx>
#include <bf_svtools/stritem.hxx>
#include <collect.hxx>
#include <bf_svx/svdsob.hxx>
#include <vcl/imagebtn.hxx>
#include <bf_svx/svdobj.hxx>
#include <bf_svx/svdglue.hxx>
#include <bf_svx/svdlayer.hxx>
#include <vcl/msgbox.hxx>
#include <bf_svx/svdmrkv.hxx>
#include <bf_svx/svdmark.hxx>
#include <bf_svx/svdhdl.hxx>
#include <bf_svx/svdsnpv.hxx>
#include <bf_svx/svdpntv.hxx>
#include <bf_svx/svddrag.hxx>
#include <bf_svx/svdhlpln.hxx>
#include <scmod.hxx>
#include <bf_sfx2/request.hxx>
#include <vcl/field.hxx>
#include <bf_svx/svdview.hxx>
#include <bf_svx/svdcrtv.hxx>
#include <vcl/print.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/wrkwin.hxx>
//#include <vcl/system.hxx>
#include <bf_sfx2/childwin.hxx>
#include <bf_sfx2/chalign.hxx>
#include <bf_sfx2/dispatch.hxx>
#include <vcl/graph.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/animate.hxx>
#include <vcl/graph.h>
#include <bf_svx/editdata.hxx>
#include <bf_svtools/stdctrl.hxx>
#include <bf_sfx2/basedlgs.hxx>
#include <conditio.hxx>
#include <vcl/combobox.hxx>
#include <vcl/print.hxx>
#include <uiitems.hxx>
#include <bf_svx/svdmodel.hxx>
#include <bf_svx/editeng.hxx>
#include <bf_svx/pageitem.hxx>
#include <anyrefdg.hxx>
#include "bf_basic/sbxcore.hxx"
#include <vcl/ctrl.hxx>
#include <bf_svx/xenum.hxx>
#include <dbcolect.hxx>
#include <sot/exchange.hxx>
#include <math.h>
#include <vcl/wrkwin.hxx>
#include "bf_basic/sbxvar.hxx"
#include <vcl/image.hxx>
#include <bf_svx/xdef.hxx>
#include <drwlayer.hxx>
#include <editutil.hxx>
#include <bf_svx/xit.hxx>
#include <bf_svx/xcolit.hxx>
#include <attrib.hxx>
#include <patattr.hxx>
#include <bf_svx/xfillit0.hxx>
#include <bf_svx/xlineit0.hxx>
#include <bf_svx/xflasit.hxx>
#include <bf_svx/xlnasit.hxx>
#include <bf_svx/svdattr.hxx>
#include <bf_svx/sdangitm.hxx>
#include <bf_svx/sderitm.hxx>
#include <bf_svx/sdmetitm.hxx>
#include <bf_svx/svddef.hxx>
#include <bf_svx/sdmsitm.hxx>
#include <bf_svx/sdolsitm.hxx>
#include <bf_svx/sdooitm.hxx>
#include <bf_svx/sdprcitm.hxx>
#include <bf_svx/sdshcitm.hxx>
#include <bf_svx/sdshitm.hxx>
#include <bf_svx/sdshsitm.hxx>
#include <bf_svx/sdshtitm.hxx>
#include <bf_svx/sdsxyitm.hxx>
#include <bf_svx/sdtaaitm.hxx>
#include <bf_svx/sdtacitm.hxx>
#include <bf_svx/sdtaditm.hxx>
#include <bf_svx/sdtagitm.hxx>
#include <bf_svx/sdtaiitm.hxx>
#include <bf_svx/sdynitm.hxx>
#include <bf_svx/sdtaitm.hxx>
#include <bf_svx/sdtakitm.hxx>
#include <bf_svx/sdtayitm.hxx>
#include <bf_svx/sdtcfitm.hxx>
#include <bf_svx/sdtditm.hxx>
#include <bf_svx/sdtfsitm.hxx>
#include <bf_svx/sdtmfitm.hxx>
#include <tools/poly.hxx>
#include <vcl/gdimtf.hxx>
#include <cell.hxx>
#include <stddef.h>
#include <tools/mempool.hxx>
#include <bf_svtools/style.hrc>
#include <drawview.hxx>
#include <bf_svx/view3d.hxx>
#include <bf_svx/def3d.hxx>
#include <bf_goodies/vector3d.hxx>
#include <vcl/floatwin.hxx>
#include "bf_basic/sbxobj.hxx"
#include <bf_svx/svxenum.hxx>
#include <bf_svx/xtextit0.hxx>
#include <bf_svx/svdtrans.hxx>
#include <bf_svx/xtxasit.hxx>
#include <bf_svtools/style.hxx>
#include <reffact.hxx>
#include <bf_svx/svdotext.hxx>
#include <bf_svx/svdoattr.hxx>
#include <tools/wldcrd.hxx>
#include <rangenam.hxx>
#include <undobase.hxx>
#include <vcl/dockwin.hxx>
#include <vcl/virdev.hxx>
#include <bf_so3/so2defs.hxx>
#include <rangeutl.hxx>
#include <vcl/toolbox.hxx>
#include <fupoor.hxx>
#include <docpool.hxx>
#include <vcl/button.hxx>
#include <bf_svx/srchitem.hxx>
#include <bf_svx/svdorect.hxx>
#include <bf_svtools/whiter.hxx>
#include <stlpool.hxx>
#include <bf_svx/eeitem.hxx>
#include <inputhdl.hxx>
#include <bf_svx/svdpage.hxx>




