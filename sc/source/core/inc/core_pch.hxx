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

//  ItemID-Defines etc. must be included first

#include "scitems.hxx"


#define _ZFORLIST_DECLARE_TABLE

#define SC_PROGRESS_CXX

#include <tools/solar.h>
#include <string.h>
#include <tools/string.hxx>
#include <tools/rtti.hxx>
#include <limits.h>
#include <tools/ref.hxx>
#include <tools/list.hxx>
#include <tools/contnr.hxx>
#include <tools/link.hxx>
#include <tools/stream.hxx>
#include <tools/errinf.hxx>
#include <tools/errcode.hxx>
#include <vcl/sv.h>
#include <global.hxx>
#include <tools/color.hxx>
#include <i18npool/lang.h>
#include <tools/debug.hxx>
#include <tools/gen.hxx>
#include <svl/svarray.hxx>
#include <markarr.hxx>
#include <vcl/timer.hxx>
#include <rangelst.hxx>
#include <document.hxx>
#include <vcl/prntypes.hxx>
#include <table.hxx>
#include <column.hxx>
#include <svl/hint.hxx>
#include <svl/lstner.hxx>
#include <svl/poolitem.hxx>
#include <tools/time.hxx>
#include <svl/solar.hrc>
#include <tools/date.hxx>
#include <svl/brdcst.hxx>
#include <svx/svxids.hrc>
#include <svl/memberid.hrc>
#include <sfx2/sfx.hrc>
#include <sfx2/sfxsids.hrc>
#include <svl/cntwids.hrc>
#include <tools/resid.hxx>
#include <tools/table.hxx>
#include <stdarg.h>
#include <tools/rc.hxx>
#include <tools/resmgr.hxx>
#include <tools/unqidx.hxx>
#include <rsc/rscsfx.hxx>
#include <basic/sbxdef.hxx>
#include <svl/itemset.hxx>
#include <stddef.h>
#include <collect.hxx>
#include <scitems.hxx>
#include <tools/globname.hxx>
#include <tools/fract.hxx>
#include <sfx2/shell.hxx>
#include <cell.hxx>
#include <tools/mempool.hxx>
#include <vcl/color.hxx>
#include <vcl/region.hxx>
#include <vcl/mapmod.hxx>
#include <vcl/bitmap.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <sot/object.hxx>
#include <sot/factory.hxx>
#include <sot/sotdata.hxx>
#include <vcl/keycod.hxx>
#include <vcl/keycodes.hxx>
#include <rechead.hxx>
#include <tools/unqid.hxx>
#include <vcl/apptypes.hxx>
#include <vcl/vclenum.hxx>
#include <globstr.hrc>
#include <formula/compiler.hrc>
#include <tools/shl.hxx>
#include <compiler.hxx>
#include <vcl/font.hxx>
#include <svl/smplhint.hxx>
#include <vcl/wall.hxx>
#include <vcl/settings.hxx>
#include <vcl/accel.hxx>
#include <patattr.hxx>
#include <svl/zforlist.hxx>
#include <tools/pstm.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <vcl/pointr.hxx>
#include <vcl/ptrstyle.hxx>
#include <vcl/wintypes.hxx>
#include <vcl/event.hxx>
#include <tools/ownlist.hxx>
#include <svl/itempool.hxx>
#include <tools/datetime.hxx>
#include <attrib.hxx>
#include <docpool.hxx>
#include <sot/storage.hxx>
#include <sfx2/objsh.hxx>
#include <vcl/window.hxx>
#include <svtools/confitem.hxx>
#include <vcl/syswin.hxx>
#include <sc.hrc>
#include <svx/dialogs.hrc>
#include <math.h>
#include <svl/style.hxx>
#include <svl/style.hrc>
#include <stdlib.h>
#include <vcl/prntypes.hxx>
#include <vcl/jobset.hxx>
#include <vcl/gdimtf.hxx>
#include <tools/urlobj.hxx>
#include <vcl/print.hxx>
#include <docoptio.hxx>
#include <markdata.hxx>
#include <vcl/wrkwin.hxx>
#include <stlpool.hxx>
#include <sfx2/app.hxx>
#include <svl/inetmsg.hxx>
#include <svtools/compat.hxx>
#include <svl/inetdef.hxx>
#include <svl/inethist.hxx>
#include <vcl/accel.hxx>
#include <sfx2/sfxdefs.hxx>
#include <sfx2/module.hxx>
#include <sfx2/imgdef.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/field.hxx>
#include <vcl/spinfld.hxx>
#include <vcl/edit.hxx>
#include <vcl/timer.hxx>
#include <vcl/combobox.hxx>
#include <vcl/combobox.h>
#include <refupdat.hxx>
#include <editeng/boxitem.hxx>
#include <conditio.hxx>
#include <brdcst.hxx>
#include <editeng/svxenum.hxx>
#include <dociter.hxx>
#include <scdll.hxx>
#include <stdio.h>
#include <stlsheet.hxx>
#include <vcl/gdiobj.hxx>
#include <vcl/mapmod.hxx>
#include <progress.hxx>
#include <sfx2/progress.hxx>
#include <vcl/event.hxx>
#include <vcl/window.hxx>
#include <svx/algitem.hxx>
#include <vcl/field.hxx>
#include <svx/svdtypes.hxx>
#include <vcl/graph.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/animate.hxx>
#include <vcl/graph.h>
#include <drwlayer.hxx>
#include <svx/svdmodel.hxx>
#include <scresid.hxx>
#include <vcl/print.hxx>
#include <attarray.hxx>
#include <svl/ownlist.hxx>
#include <interpre.hxx>
#include <subtotal.hxx>
#include <rangenam.hxx>
#include <scmatrix.hxx>
#include <svx/pageitem.hxx>
#include <dbcolect.hxx>
#include <userlist.hxx>
#include <editeng/editdata.hxx>
#include <basic/sbxvar.hxx>
#include <basic/sbxcore.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdsob.hxx>
#include <svx/svdglue.hxx>
#include <editeng/langitem.hxx>
#include <editeng/eeitem.hxx>
#include <callform.hxx>
#include <validat.hxx>
#include <editeng/brshitem.hxx>
#include <sot/exchange.hxx>
#include <editeng/editeng.hxx>
#include <vcl/fonttype.hxx>
#include <editeng/editobj.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <svl/stritem.hxx>
#include <pivot.hxx>
#include <vcl/gdimtf.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdlayer.hxx>
#include <sfx2/linkmgr.hxx>
#include <ctype.h>
#include <vcl/font.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/postitem.hxx>
#include <svx/svditer.hxx>
#include <editeng/udlnitem.hxx>
#include <adiasync.hxx>
#include <sfx2/bindings.hxx>
#include <ddelink.hxx>
#include <chartlis.hxx>
#include <sfx2/minarray.hxx>
#include <svtools/txtcmp.hxx>
#include <olinetab.hxx>
#include <basic/sbxobj.hxx>
#include <cfgids.hxx>




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
