/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salcursors.h,v $
 * $Revision: 1.6 $
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

#include "nodrop_curs.h"
#include "nodrop_mask.h"
#include "magnify_curs.h"
#include "magnify_mask.h"
#include "rotate_curs.h"
#include "rotate_mask.h"
#include "hshear_curs.h"
#include "hshear_mask.h"
#include "vshear_curs.h"
#include "vshear_mask.h"
#include "drawline_curs.h"
#include "drawline_mask.h"
#include "drawrect_curs.h"
#include "drawrect_mask.h"
#include "drawpolygon_curs.h"
#include "drawpolygon_mask.h"
#include "drawbezier_curs.h"
#include "drawbezier_mask.h"
#include "drawarc_curs.h"
#include "drawarc_mask.h"
#include "drawpie_curs.h"
#include "drawpie_mask.h"
#include "drawcirclecut_curs.h"
#include "drawcirclecut_mask.h"
#include "drawellipse_curs.h"
#include "drawellipse_mask.h"
#include "drawconnect_curs.h"
#include "drawconnect_mask.h"
#include "drawtext_curs.h"
#include "drawtext_mask.h"
#include "mirror_curs.h"
#include "mirror_mask.h"
#include "crook_curs.h"
#include "crook_mask.h"
#include "crop_curs.h"
#include "crop_mask.h"
#include "movepoint_curs.h"
#include "movepoint_mask.h"
#include "movebezierweight_curs.h"
#include "movebezierweight_mask.h"
#include "drawfreehand_curs.h"
#include "drawfreehand_mask.h"
#include "drawcaption_curs.h"
#include "drawcaption_mask.h"
#include "movedata_curs.h"
#include "movedata_mask.h"
#include "copydata_curs.h"
#include "copydata_mask.h"
#include "linkdata_curs.h"
#include "linkdata_mask.h"
#include "movedlnk_curs.h"
#include "movedlnk_mask.h"
#include "copydlnk_curs.h"
#include "copydlnk_mask.h"
#include "movefile_curs.h"
#include "movefile_mask.h"
#include "copyfile_curs.h"
#include "copyfile_mask.h"
#include "linkfile_curs.h"
#include "linkfile_mask.h"
#include "moveflnk_curs.h"
#include "moveflnk_mask.h"
#include "copyflnk_curs.h"
#include "copyflnk_mask.h"
#include "movefiles_curs.h"
#include "movefiles_mask.h"
#include "copyfiles_curs.h"
#include "copyfiles_mask.h"

#include "chart_curs.h"
#include "chart_mask.h"
#include "detective_curs.h"
#include "detective_mask.h"
#include "pivotcol_curs.h"
#include "pivotcol_mask.h"
#include "pivotfld_curs.h"
#include "pivotfld_mask.h"
#include "pivotrow_curs.h"
#include "pivotrow_mask.h"
#include "pivotdel_curs.h"
#include "pivotdel_mask.h"

#include "chain_curs.h"
#include "chain_mask.h"
#include "chainnot_curs.h"
#include "chainnot_mask.h"

#include "timemove_curs.h"
#include "timemove_mask.h"
#include "timesize_curs.h"
#include "timesize_mask.h"

#include "ase_curs.h"
#include "ase_mask.h"
#include "asn_curs.h"
#include "asn_mask.h"
#include "asne_curs.h"
#include "asne_mask.h"
#include "asns_curs.h"
#include "asns_mask.h"
#include "asnswe_curs.h"
#include "asnswe_mask.h"
#include "asnw_curs.h"
#include "asnw_mask.h"
#include "ass_curs.h"
#include "ass_mask.h"
#include "asse_curs.h"
#include "asse_mask.h"
#include "assw_curs.h"
#include "assw_mask.h"
#include "asw_curs.h"
#include "asw_mask.h"
#include "aswe_curs.h"
#include "aswe_mask.h"
#include "null_curs.h"
#include "null_mask.h"

#include "airbrush_curs.h"
#include "airbrush_mask.h"
#include "fill_curs.h"
#include "fill_mask.h"
#include "vertcurs_curs.h"
#include "vertcurs_mask.h"
#include "tblsele_curs.h"
#include "tblsele_mask.h"
#include "tblsels_curs.h"
#include "tblsels_mask.h"
#include "tblselse_curs.h"
#include "tblselse_mask.h"
#include "tblselw_curs.h"
#include "tblselw_mask.h"
#include "tblselsw_curs.h"
#include "tblselsw_mask.h"
#include "paintbrush_curs.h"
#include "paintbrush_mask.h"
