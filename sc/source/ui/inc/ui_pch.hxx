/*************************************************************************
 *
 *  $RCSfile: ui_pch.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: er $ $Date: 2001-03-14 18:13:19 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

//  ItemID-Defines etc. muessen immer ganz vorne stehen

#include "scitems.hxx"
#include "eetext.hxx"

#define ITEMID_FIELD EE_FEATURE_FIELD

#define _ZFORLIST_DECLARE_TABLE

#define _PRINTFUN_CXX
#define _INPUTHDL_CXX

//  ab hier automatisch per makepch generiert
//  folgende duerfen nicht aufgenommen werden:
//      scslots.hxx

#include <tools/solar.h>
#include <tools/string.hxx>
#include <svtools/solar.hrc>
#include <tools/rtti.hxx>
#include <string.h>
#include <tools/contnr.hxx>
#include <vcl/sv.h>
#include <tools/ref.hxx>
#include <tools/list.hxx>
#include <tools/link.hxx>
#include <tools/lang.hxx>
#include <sfx2/sfx.hrc>
#include <svtools/memberid.hrc>
#include <sfx2/sfxsids.hrc>
#include <svtools/cntwids.hrc>
#include <vcl/resid.hxx>
#include <tools/resid.hxx>
#include <vcl/rc.hxx>
#include <tools/rc.hxx>
#include <tools/resmgr.hxx>
#include <tools/gen.hxx>
#include <tools/fract.hxx>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <vcl/keycod.hxx>
#include <vcl/keycodes.hxx>
#include <tools/globname.hxx>
#include <vcl/color.hxx>
#include <vcl/region.hxx>
#include <vcl/mapmod.hxx>
#include <vcl/bitmap.hxx>
#include <sot/sotref.hxx>
#include <svx/svxids.hrc>
#include <vcl/fonttype.hxx>
#include <sot/object.hxx>
#include <sot/sotdata.hxx>
#include <sot/factory.hxx>
#include <tools/ownlist.hxx>
#include <vcl/font.hxx>
#include <vcl/wall.hxx>
#include <vcl/settings.hxx>
#include <vcl/accel.hxx>
#include <vcl/gdiobj.hxx>
#include <vcl/outdev.hxx>
#include <vcl/pointr.hxx>
#include <vcl/ptrstyle.hxx>
#include <vcl/wintypes.hxx>
#include <vcl/event.hxx>
#include <sot/dtrans.hxx>
#include <limits.h>
#include <tools/errinf.hxx>
#include <tools/errcode.hxx>
#include <tools/stream.hxx>
#include <vcl/color.hxx>
#include <vcl/window.hxx>
#include <global.hxx>
#include <vcl/syswin.hxx>
#include <tools/debug.hxx>
#include <svtools/svarray.hxx>
#include <vcl/ctrl.hxx>
#include <svtools/hint.hxx>
#include <svtools/poolitem.hxx>
#include <svtools/args.hxx>
#include <vcl/image.hxx>
#include <vcl/timer.hxx>
#include <tools/unqidx.hxx>
#include <vcl/symbol.hxx>
#include <vcl/fixed.hxx>
#include <svtools/brdcst.hxx>
#include <vcl/timer.hxx>
#include <sc.hrc>
#include <stdarg.h>
#include <svx/dialogs.hrc>
#include <svtools/sbxdef.hxx>
#include <svtools/lstner.hxx>
#include <sfx2/shell.hxx>
#include <vcl/window.hxx>
#include <vcl/group.hxx>
#include <tools/unqid.hxx>
#include <tools/table.hxx>
#include <svtools/itemset.hxx>
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
#include <svtools/eitem.hxx>
#include <svtools/itempool.hxx>
#include <so3/iface.hxx>
#include <so3/factory.hxx>
#include <so3/so2ref.hxx>
#include <so3/so2dll.hxx>
#include <sfx2/tabdlg.hxx>
#include <vcl/tabdlg.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/tabctrl.hxx>
#include <shellids.hxx>
#include <vcl/event.hxx>
#include <so3/advise.hxx>
#include <so3/dtrans.hxx>
#include <so3/protocol.hxx>
#include <svtools/intitem.hxx>
#include <so3/ipclient.hxx>
#include <so3/client.hxx>
#include <sfx2/cfgitem.hxx>
#include <svtools/confitem.hxx>
#include <markdata.hxx>
#include <svx/optgrid.hxx>
#include <vcl/apptypes.hxx>
#include <viewopti.hxx>
#include <viewdata.hxx>
#include <vcl/resid.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/clientsh.hxx>
#include <document.hxx>
#include <vcl/prntypes.hxx>
#include <table.hxx>
#include <column.hxx>
#include <markarr.hxx>
#include <vcl/mapmod.hxx>
#include <svtools/undo.hxx>
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
#include <svtools/smplhint.hxx>
#include <sfx2/viewfac.hxx>
#include <sot/storage.hxx>
#include <sfx2/objsh.hxx>
#include <so3/so2defs.hxx>
#include <so3/persist.hxx>
#include <target.hxx>
#include <tabvwsh.hxx>
#include <svtools/inethist.hxx>
#include <svtools/inetdef.hxx>
#include <sfx2/sfxdefs.hxx>
#include <svtools/cancel.hxx>
#include <svtools/compat.hxx>
#include <svtools/inetmsg.hxx>
#include <sfx2/app.hxx>
#include <tools/urlobj.hxx>
#include <scdll.hxx>
#include <sfx2/msg.hxx>
#include <svtools/ownlist.hxx>
#include <so3/persist.hxx>
#include <so3/pseudo.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/frame.hxx>
#include <so3/ipobj.hxx>
#include <so3/embobj.hxx>
#include <so3/ipobj.hxx>
#include <scresid.hxx>
#include <sfx2/ipfrm.hxx>
#include <so3/factory.hxx>
#include <docsh.hxx>
#include <sfx2/interno.hxx>
#include <vcl/wintypes.hxx>
#include <sfx2/docfac.hxx>
#include <scitems.hxx>
#include <vcl/virdev.hxx>
#include <globstr.hrc>
#include <segmentc.hxx>
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
#include <svtools/svstdarr.hxx>
#include <vcl/edit.hxx>
#include <svtools/stritem.hxx>
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
#include <vcl/system.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/chalign.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/graph.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/animate.hxx>
#include <vcl/graph.h>
#include <svx/editdata.hxx>
#include <svtools/stdctrl.hxx>
#include <sfx2/basedlgs.hxx>
#include <conditio.hxx>
#include <vcl/combobox.hxx>
#include <vcl/print.hxx>
#include <uiitems.hxx>
#include <svx/svdmodel.hxx>
#include <vcl/gdiobj.hxx>
#include <svx/editeng.hxx>
#include <svx/pageitem.hxx>
#include <anyrefdg.hxx>
#include <svtools/sbxcore.hxx>
#include <vcl/ctrl.hxx>
#include <svx/xenum.hxx>
#include <dbcolect.hxx>
#include <sot/exchange.hxx>
#include <math.h>
#include <vcl/wrkwin.hxx>
#include <svtools/sbxvar.hxx>
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
#include <svx/sdmsitm.hxx>
#include <svx/sdolsitm.hxx>
#include <svx/sdooitm.hxx>
#include <svx/sdprcitm.hxx>
#include <svx/sdshcitm.hxx>
#include <svx/sdshitm.hxx>
#include <svx/sdshsitm.hxx>
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
#include <vcl/poly.hxx>
#include <vcl/gdimtf.hxx>
#include <cell.hxx>
#include <stddef.h>
#include <tools/mempool.hxx>
#include <svtools/style.hrc>
#include <drawview.hxx>
#include <svx/view3d.hxx>
#include <svx/def3d.hxx>
#include <goodies/vector3d.hxx>
#include <vcl/floatwin.hxx>
#include <svtools/sbxobj.hxx>
#include <svx/svxenum.hxx>
#include <svx/xtextit0.hxx>
#include <svx/svdtrans.hxx>
#include <svx/xtxasit.hxx>
#include <svtools/style.hxx>
#include <reffact.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdoattr.hxx>
#include <tools/wldcrd.hxx>
#include <rangenam.hxx>
#include <undobase.hxx>
#include <vcl/dockwin.hxx>
#include <vcl/virdev.hxx>
#include <so3/so2defs.hxx>
#include <so3/svstor.hxx>
#include <rangeutl.hxx>
#include <vcl/toolbox.hxx>
#include <fupoor.hxx>
#include <docpool.hxx>
#include <vcl/button.hxx>
#include <svx/srchitem.hxx>
#include <svx/svdorect.hxx>
#include <svtools/whiter.hxx>
#include <stlpool.hxx>
#include <svx/eeitem.hxx>
#include <inputhdl.hxx>
#include <svx/svdpage.hxx>




