/*************************************************************************
 *
 *  $RCSfile: unoframe.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: os $ $Date: 2000-11-27 11:13:17 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include <swtypes.hxx>
#include <cmdid.h>

#ifndef _HINTS_HXX //autogen
#include <hints.hxx>
#endif
#ifndef _DOC_HXX //autogen
#include <doc.hxx>
#endif
#ifndef _DOCSH_HXX //autogen
#include <docsh.hxx>
#endif
#ifndef _NDINDEX_HXX //autogen
#include <ndindex.hxx>
#endif
#ifndef _PAM_HXX //autogen
#include <pam.hxx>
#endif
#ifndef _NDNOTXT_HXX //autogen
#include <ndnotxt.hxx>
#endif
#ifndef _SVX_UNOMID_HXX
#include <svx/unomid.hxx>
#endif
#ifndef _UNOCRSR_HXX //autogen
#include <unocrsr.hxx>
#endif
#ifndef _DOCSTYLE_HXX //autogen
#include <docstyle.hxx>
#endif
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif
#ifndef _FMTCNCT_HXX //autogen
#include <fmtcnct.hxx>
#endif
#ifndef _NDOLE_HXX //autogen
#include <ndole.hxx>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif
#ifndef _UNOMAP_HXX
#include <unomap.hxx>
#endif
#ifndef _UNOPRNMS_HXX
#include <unoprnms.hxx>
#endif
#ifndef _COM_SUN_STAR_TABLE_BORDERLINE_HPP_
#include <com/sun/star/table/BorderLine.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFYBROADCASTER_HPP_
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_SHADOWFORMAT_HPP_
#include <com/sun/star/table/ShadowFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_GRAPHICLOCATION_HPP_
#include <com/sun/star/style/GraphicLocation.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_GRAPHICCROP_HPP_
#include <com/sun/star/text/GraphicCrop.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_TEXTCONTENTANCHORTYPE_HPP_
#include <com/sun/star/text/TextContentAnchorType.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTCOLUMNS_HPP_
#include <com/sun/star/text/XTextColumns.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POINTSEQUENCESEQUENCE_HPP_
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POINTSEQUENCE_HPP_
#include <com/sun/star/drawing/PointSequence.hpp>
#endif
#ifndef _SV_POLY_HXX //autogen
#include <vcl/poly.hxx>
#endif
#ifndef SW_UNOMID_HXX
#include <unomid.h>
#endif
#ifndef _UNOSTYLE_HXX
#include <unostyle.hxx>
#endif
#ifndef _SVDMODEL_HXX
#include <svx/svdmodel.hxx>
#endif
#ifndef _SVDPAGE_HXX
#include <svx/svdpage.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTURL_HXX //autogen
#include <fmturl.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_OPAQITEM_HXX //autogen
#include <svx/opaqitem.hxx>
#endif
#ifndef _SVX_PRNTITEM_HXX //autogen
#include <svx/prntitem.hxx>
#endif
#ifndef _SVX_SHADITEM_HXX //autogen
#include <svx/shaditem.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _GRFATR_HXX //autogen
#include <grfatr.hxx>
#endif
#ifndef _UNOFRAME_HXX
#include <unoframe.hxx>
#endif
#ifndef _FMTANCHR_HXX
#include <fmtanchr.hxx>
#endif
#ifndef _FMTCLDS_HXX
#include <fmtclds.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _NDGRF_HXX
#include <ndgrf.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif
#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::drawing;
using namespace ::rtl;


// unterscheidet sich von der Rahmenbeschreibung durch eine XTextPosition
//{ UNO_NAME_CLIENT_MAP     ,       RES_URL,                &::getBooleanCppuType(),            PROPERTY_MAYBEVOID ,MID_URL_CLIENTMAP        },
const SfxItemPropertyMap* GetFrameDescMap()
{
    static SfxItemPropertyMap aFrameDescPropertyMap_Impl[] =
    {
        { SW_PROP_NAME(UNO_NAME_ANCHOR_PAGE_NO),            RES_ANCHOR,             &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE, MID_ANCHOR_PAGENUM       },
        { SW_PROP_NAME(UNO_NAME_ANCHOR_TYPE   ),            RES_ANCHOR,             &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE, MID_ANCHOR_ANCHORTYPE},
        { SW_PROP_NAME(UNO_NAME_BACK_COLOR    )  ,      RES_BACKGROUND,         &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE ,MID_BACK_COLOR        },
        { SW_PROP_NAME(UNO_NAME_CHAIN_NEXT_NAME),           RES_CHAIN,              &::getCppuType((const OUString*)0),         PropertyAttribute::MAYBEVOID ,MID_CHAIN_NEXTNAME},
        { SW_PROP_NAME(UNO_NAME_CHAIN_PREV_NAME),           RES_CHAIN,              &::getCppuType((const OUString*)0),         PropertyAttribute::MAYBEVOID ,MID_CHAIN_PREVNAME},
/*n.i.*/{ SW_PROP_NAME(UNO_NAME_CLIENT_MAP     )    ,       RES_URL,                &::getBooleanCppuType(),            PROPERTY_NONE ,MID_URL_CLIENTMAP         },
        { SW_PROP_NAME(UNO_NAME_CONTENT_PROTECTED) ,        RES_PROTECT,            &::getBooleanCppuType(),            PROPERTY_NONE, MID_PROTECT_CONTENT   },
        { SW_PROP_NAME(UNO_NAME_EDIT_IN_READONLY),      RES_EDIT_IN_READONLY,   &::getBooleanCppuType(),            PROPERTY_NONE, 0},
        { SW_PROP_NAME(UNO_NAME_FRAME_STYLE_NAME),          0,                      &::getCppuType((const OUString*)0),         PROPERTY_NONE, 0},
        { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_URL)      ,    RES_BACKGROUND,         &::getCppuType((const OUString*)0),         PROPERTY_NONE ,MID_GRAPHIC_URL    },
        { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_FILTER)  ,     RES_BACKGROUND,         &::getCppuType((const OUString*)0),         PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
        { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_LOCATION)   ,  RES_BACKGROUND,         &::getCppuType((const style::GraphicLocation*)0), PROPERTY_NONE ,MID_GRAPHIC_POSITION},
        { SW_PROP_NAME(UNO_NAME_LEFT_MARGIN),           RES_LR_SPACE,           &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE, MID_L_MARGIN|CONVERT_TWIPS},
        { SW_PROP_NAME(UNO_NAME_RIGHT_MARGIN),          RES_LR_SPACE,           &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},
        { SW_PROP_NAME(UNO_NAME_HEIGHT),                    RES_FRM_SIZE,           &::getCppuType((const sal_Int32*)0)  ,          PROPERTY_NONE, MID_FRMSIZE_HEIGHT|CONVERT_TWIPS         },
        { SW_PROP_NAME(UNO_NAME_SIZE_TYPE),                 RES_FRM_SIZE,           &::getCppuType((const sal_Int16*)0)  ,          PROPERTY_NONE,   MID_FRMSIZE_SIZE_TYPE  },
        { SW_PROP_NAME(UNO_NAME_HORI_ORIENT)     ,          RES_HORI_ORIENT,        &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE ,MID_HORIORIENT_ORIENT    },
        { SW_PROP_NAME(UNO_NAME_HORI_ORIENT_POSITION),  RES_HORI_ORIENT,        &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE ,MID_HORIORIENT_POSITION|CONVERT_TWIPS    },
        { SW_PROP_NAME(UNO_NAME_HORI_ORIENT_RELATION),  RES_HORI_ORIENT,        &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE ,MID_HORIORIENT_RELATION  },
        { SW_PROP_NAME(UNO_NAME_HYPER_LINK_U_R_L  ) ,       RES_URL,                &::getCppuType((const OUString*)0),         PROPERTY_NONE ,MID_URL_URL},
        { SW_PROP_NAME(UNO_NAME_HYPER_LINK_TARGET ) ,       RES_URL,                &::getCppuType((const OUString*)0),         PROPERTY_NONE ,MID_URL_TARGET},
        { SW_PROP_NAME(UNO_NAME_HYPER_LINK_NAME ),      RES_URL,                &::getCppuType((const OUString*)0),         PROPERTY_NONE ,MID_URL_HYPERLINKNAME     },
        { SW_PROP_NAME(UNO_NAME_OPAQUE),                    RES_OPAQUE,             &::getBooleanCppuType(),            PROPERTY_NONE, 0},
        { SW_PROP_NAME(UNO_NAME_PAGE_TOGGLE),               RES_HORI_ORIENT,        &::getBooleanCppuType(),            PROPERTY_NONE ,MID_HORIORIENT_PAGETOGGLE },
        { SW_PROP_NAME(UNO_NAME_POSITION_PROTECTED),    RES_PROTECT,            &::getBooleanCppuType(),            PROPERTY_NONE, MID_PROTECT_POSITION},
        { SW_PROP_NAME(UNO_NAME_PRINT),                     RES_PRINT,              &::getBooleanCppuType(),            PROPERTY_NONE, 0},
        { SW_PROP_NAME(UNO_NAME_RELATIVE_HEIGHT),       RES_FRM_SIZE,           &::getCppuType((const sal_Int8*)0)  ,       PROPERTY_NONE,   MID_FRMSIZE_REL_HEIGHT },
        { SW_PROP_NAME(UNO_NAME_RELATIVE_WIDTH),            RES_FRM_SIZE,           &::getCppuType((const sal_Int8*)0)  ,       PROPERTY_NONE,   MID_FRMSIZE_REL_WIDTH  },
        { SW_PROP_NAME(UNO_NAME_SHADOW_FORMAT),             RES_SHADOW,             &::getCppuType((const table::ShadowFormat*)0),  PROPERTY_NONE, CONVERT_TWIPS},
        { SW_PROP_NAME(UNO_NAME_SERVER_MAP)     ,       RES_URL,                &::getBooleanCppuType(),            PROPERTY_NONE ,MID_URL_SERVERMAP         },
        { SW_PROP_NAME(UNO_NAME_SIZE),                  RES_FRM_SIZE,           &::getCppuType((const awt::Size*)0),            PROPERTY_NONE, MID_FRMSIZE_SIZE|CONVERT_TWIPS},
        { SW_PROP_NAME(UNO_NAME_SIZE_PROTECTED)    ,    RES_PROTECT,            &::getBooleanCppuType(),            PROPERTY_NONE, MID_PROTECT_SIZE    },
        { SW_PROP_NAME(UNO_NAME_SIZE_RELATIVE),             RES_FRM_SIZE,           &::getBooleanCppuType()  ,          PROPERTY_NONE,   MID_FRMSIZE_IS_SYNC_REL_SIZE   },
        { SW_PROP_NAME(UNO_NAME_SURROUND  )               , RES_SURROUND,           &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE, MID_SURROUND_SURROUNDTYPE    },
        { SW_PROP_NAME(UNO_NAME_SURROUND_ANCHORONLY),   RES_SURROUND,           &::getBooleanCppuType(),            PROPERTY_NONE, MID_SURROUND_ANCHORONLY      },
        { SW_PROP_NAME(UNO_NAME_TEXT_COLUMNS),          RES_COL,                &::getCppuType((const uno::Reference<XTextColumns>*)0),    PROPERTY_NONE, MID_COLUMNS},
        //MID_COLUMN_SEPARATOR_LINE ???
        { SW_PROP_NAME(UNO_NAME_TOP_MARGIN),                RES_UL_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},
        { SW_PROP_NAME(UNO_NAME_BOTTOM_MARGIN),             RES_UL_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},
        { SW_PROP_NAME(UNO_NAME_BACK_TRANSPARENT) , RES_BACKGROUND,         &::getBooleanCppuType(),            PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
        { SW_PROP_NAME(UNO_NAME_VERT_ORIENT  ),             RES_VERT_ORIENT,        &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE ,MID_VERTORIENT_ORIENT    },
        { SW_PROP_NAME(UNO_NAME_VERT_ORIENT_POSITION),  RES_VERT_ORIENT,        &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE ,MID_VERTORIENT_POSITION|CONVERT_TWIPS    },
        { SW_PROP_NAME(UNO_NAME_VERT_ORIENT_RELATION),  RES_VERT_ORIENT,        &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE ,MID_VERTORIENT_RELATION  },
        { SW_PROP_NAME(UNO_NAME_WIDTH),                     RES_FRM_SIZE,           &::getCppuType((const sal_Int32*)0)  ,          PROPERTY_NONE, MID_FRMSIZE_WIDTH|CONVERT_TWIPS          },
        { SW_PROP_NAME(UNO_NAME_TEXT_WRAP),                 RES_SURROUND,           &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE, MID_SURROUND_SURROUNDTYPE    },
        { SW_PROP_NAME(UNO_NAME_LEFT_BORDER),               RES_BOX,                &::getCppuType((const table::BorderLine*)0),    0, LEFT_BORDER  |CONVERT_TWIPS },
        { SW_PROP_NAME(UNO_NAME_RIGHT_BORDER),          RES_BOX,                &::getCppuType((const table::BorderLine*)0),    0, RIGHT_BORDER |CONVERT_TWIPS },
        { SW_PROP_NAME(UNO_NAME_TOP_BORDER),                RES_BOX,                &::getCppuType((const table::BorderLine*)0),    0, TOP_BORDER   |CONVERT_TWIPS },
        { SW_PROP_NAME(UNO_NAME_BOTTOM_BORDER),         RES_BOX,                &::getCppuType((const table::BorderLine*)0),    0, BOTTOM_BORDER|CONVERT_TWIPS },
        { SW_PROP_NAME(UNO_NAME_BORDER_DISTANCE),         RES_BOX,              &::getCppuType((const sal_Int32*)0),    0, BORDER_DISTANCE|CONVERT_TWIPS },
        { SW_PROP_NAME(UNO_NAME_Z_ORDER),               FN_UNO_Z_ORDER,         &::getCppuType((const sal_Int32*)0),        PROPERTY_NONE, 0},
        {0,0,0,0}
    };
    #define FRM_PROP_COUNT 50
    return aFrameDescPropertyMap_Impl;
}
// unterscheidet sich von der Rahmenbeschreibung durch eine XTextPosition
const SfxItemPropertyMap* GetGraphicDescMap()
{
    static SfxItemPropertyMap aGraphicDescPropertyMap_Impl[] =
    {
        { SW_PROP_NAME(UNO_NAME_ALTERNATIVE_TEXT),      0,                      &::getCppuType((const OUString*)0),         PROPERTY_NONE , 0   },
        { SW_PROP_NAME(UNO_NAME_ANCHOR_PAGE_NO),            RES_ANCHOR,             &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE, MID_ANCHOR_PAGENUM       },
        { SW_PROP_NAME(UNO_NAME_ANCHOR_TYPE   ),            RES_ANCHOR,             &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE, MID_ANCHOR_ANCHORTYPE},
        { SW_PROP_NAME(UNO_NAME_BACK_COLOR     )     ,      RES_BACKGROUND,         &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE ,MID_BACK_COLOR        },
        { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_FILTER)  ,     RES_BACKGROUND,         &::getCppuType((const OUString*)0),         PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
        { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_LOCATION)   ,  RES_BACKGROUND,         &::getCppuType((const style::GraphicLocation*)0), PROPERTY_NONE ,MID_GRAPHIC_POSITION},
        { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_URL)      ,    RES_BACKGROUND,         &::getCppuType((const OUString*)0),         PROPERTY_NONE ,MID_GRAPHIC_URL    },
        { SW_PROP_NAME(UNO_NAME_CLIENT_MAP     )    ,       RES_URL,                &::getBooleanCppuType(),            PROPERTY_NONE ,MID_URL_CLIENTMAP         },
        { SW_PROP_NAME(UNO_NAME_CONTENT_PROTECTED) ,        RES_PROTECT,            &::getBooleanCppuType(),            PROPERTY_NONE, MID_PROTECT_CONTENT   },
        { SW_PROP_NAME(UNO_NAME_CONTOUR_OUTSIDE),       RES_SURROUND,           &::getBooleanCppuType(),            PROPERTY_NONE, MID_SURROUND_CONTOUROUTSIDE  },
        { SW_PROP_NAME(UNO_NAME_FRAME_STYLE_NAME),          0,                      &::getCppuType((const OUString*)0),         PROPERTY_NONE, 0},
        { SW_PROP_NAME(UNO_NAME_GRAPHIC_CROP),          RES_GRFATR_CROPGRF,     &::getCppuType((const GraphicCrop*)0),  PROPERTY_NONE, CONVERT_TWIPS },
        { SW_PROP_NAME(UNO_NAME_HORI_MIRRORED_ON_EVEN_PAGES),   RES_GRFATR_MIRRORGRF,   &::getBooleanCppuType(),            PROPERTY_NONE,      MID_MIRROR_HORZ_EVEN_PAGES            },
        { SW_PROP_NAME(UNO_NAME_HORI_MIRRORED_ON_ODD_PAGES),    RES_GRFATR_MIRRORGRF,   &::getBooleanCppuType(),            PROPERTY_NONE,      MID_MIRROR_HORZ_ODD_PAGES                 },
        { SW_PROP_NAME(UNO_NAME_HEIGHT),                    RES_FRM_SIZE,           &::getCppuType((const sal_Int32*)0)  ,          PROPERTY_NONE, MID_FRMSIZE_HEIGHT|CONVERT_TWIPS         },
        { SW_PROP_NAME(UNO_NAME_HORI_ORIENT ) ,             RES_HORI_ORIENT,        &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE ,MID_HORIORIENT_ORIENT    },
        { SW_PROP_NAME(UNO_NAME_HORI_ORIENT_POSITION),  RES_HORI_ORIENT,        &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE ,MID_HORIORIENT_POSITION|CONVERT_TWIPS    },
        { SW_PROP_NAME(UNO_NAME_HORI_ORIENT_RELATION),  RES_HORI_ORIENT,        &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE ,MID_HORIORIENT_RELATION  },
        { SW_PROP_NAME(UNO_NAME_HYPER_LINK_U_R_L   ),       RES_URL,                &::getCppuType((const OUString*)0),         PROPERTY_NONE ,MID_URL_URL},
        { SW_PROP_NAME(UNO_NAME_HYPER_LINK_TARGET  ),       RES_URL,                &::getCppuType((const OUString*)0),         PROPERTY_NONE ,MID_URL_TARGET},
        { SW_PROP_NAME(UNO_NAME_HYPER_LINK_NAME ),      RES_URL,                &::getCppuType((const OUString*)0),         PROPERTY_NONE ,MID_URL_HYPERLINKNAME     },
        { SW_PROP_NAME(UNO_NAME_LEFT_MARGIN),           RES_LR_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_L_MARGIN|CONVERT_TWIPS},
        { SW_PROP_NAME(UNO_NAME_RIGHT_MARGIN),          RES_LR_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},
        { SW_PROP_NAME(UNO_NAME_OPAQUE),                    RES_OPAQUE,             &::getBooleanCppuType(),            PROPERTY_NONE, 0},
        { SW_PROP_NAME(UNO_NAME_PAGE_TOGGLE),               RES_VERT_ORIENT,        &::getBooleanCppuType(),            PROPERTY_NONE ,MID_HORIORIENT_PAGETOGGLE },
        { SW_PROP_NAME(UNO_NAME_POSITION_PROTECTED),    RES_PROTECT,            &::getBooleanCppuType(),            PROPERTY_NONE, MID_PROTECT_POSITION},
        { SW_PROP_NAME(UNO_NAME_PRINT),                     RES_PRINT,              &::getBooleanCppuType(),            PROPERTY_NONE, 0},
        { SW_PROP_NAME(UNO_NAME_RELATIVE_HEIGHT),       RES_FRM_SIZE,           &::getCppuType((const sal_Int8*)0)  ,       PROPERTY_NONE,   MID_FRMSIZE_REL_HEIGHT },
        { SW_PROP_NAME(UNO_NAME_RELATIVE_WIDTH),            RES_FRM_SIZE,           &::getCppuType((const sal_Int8*)0)  ,       PROPERTY_NONE,   MID_FRMSIZE_REL_WIDTH  },
        { SW_PROP_NAME(UNO_NAME_SERVER_MAP )        ,       RES_URL,                &::getBooleanCppuType(),            PROPERTY_NONE ,MID_URL_SERVERMAP         },
        { SW_PROP_NAME(UNO_NAME_SHADOW_FORMAT),             RES_SHADOW,             &::getCppuType((const table::ShadowFormat*)0),  PROPERTY_NONE, 0},
        { SW_PROP_NAME(UNO_NAME_SIZE),                  RES_FRM_SIZE,           &::getCppuType((const awt::Size*)0),            PROPERTY_NONE, MID_FRMSIZE_SIZE|CONVERT_TWIPS},
        { SW_PROP_NAME(UNO_NAME_SIZE_RELATIVE),             RES_FRM_SIZE,           &::getBooleanCppuType()  ,          PROPERTY_NONE,   MID_FRMSIZE_IS_SYNC_REL_SIZE   },
        { SW_PROP_NAME(UNO_NAME_SIZE_PROTECTED)    ,    RES_PROTECT,            &::getBooleanCppuType(),            PROPERTY_NONE, MID_PROTECT_SIZE    },
        { SW_PROP_NAME(UNO_NAME_SURROUND    )             , RES_SURROUND,           &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE, MID_SURROUND_SURROUNDTYPE    },
        { SW_PROP_NAME(UNO_NAME_SURROUND_ANCHORONLY),   RES_SURROUND,           &::getBooleanCppuType(),            PROPERTY_NONE, MID_SURROUND_ANCHORONLY      },
        { SW_PROP_NAME(UNO_NAME_SURROUND_CONTOUR )   ,  RES_SURROUND,           &::getBooleanCppuType(),            PROPERTY_NONE, MID_SURROUND_CONTOUR         },
        { SW_PROP_NAME(UNO_NAME_TOP_MARGIN),                RES_UL_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},
        { SW_PROP_NAME(UNO_NAME_BOTTOM_MARGIN),             RES_UL_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},
        { SW_PROP_NAME(UNO_NAME_BACK_TRANSPARENT) , RES_BACKGROUND,         &::getBooleanCppuType(),            PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
        { SW_PROP_NAME(UNO_NAME_VERT_MIRRORED),         RES_GRFATR_MIRRORGRF,   &::getBooleanCppuType(),            PROPERTY_NONE,     MID_MIRROR_VERT            },
        { SW_PROP_NAME(UNO_NAME_VERT_ORIENT  ),             RES_VERT_ORIENT,        &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE ,MID_VERTORIENT_ORIENT    },
        { SW_PROP_NAME(UNO_NAME_VERT_ORIENT_POSITION),  RES_VERT_ORIENT,        &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE ,MID_VERTORIENT_POSITION|CONVERT_TWIPS    },
        { SW_PROP_NAME(UNO_NAME_VERT_ORIENT_RELATION),  RES_VERT_ORIENT,        &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE ,MID_VERTORIENT_RELATION  },
        { SW_PROP_NAME(UNO_NAME_WIDTH),                     RES_FRM_SIZE,           &::getCppuType((const sal_Int32*)0)  ,          PROPERTY_NONE, MID_FRMSIZE_WIDTH            },
        { SW_PROP_NAME(UNO_NAME_TEXT_WRAP),                 RES_SURROUND,           &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE, MID_SURROUND_SURROUNDTYPE    },
        { SW_PROP_NAME(UNO_NAME_LEFT_BORDER),               RES_BOX,                &::getCppuType((const table::BorderLine*)0),    0, LEFT_BORDER  |CONVERT_TWIPS },
        { SW_PROP_NAME(UNO_NAME_RIGHT_BORDER),          RES_BOX,                &::getCppuType((const table::BorderLine*)0),    0, RIGHT_BORDER |CONVERT_TWIPS },
        { SW_PROP_NAME(UNO_NAME_TOP_BORDER),                RES_BOX,                &::getCppuType((const table::BorderLine*)0),    0, TOP_BORDER   |CONVERT_TWIPS },
        { SW_PROP_NAME(UNO_NAME_BOTTOM_BORDER),         RES_BOX,                &::getCppuType((const table::BorderLine*)0),    0, BOTTOM_BORDER|CONVERT_TWIPS },
        { SW_PROP_NAME(UNO_NAME_BORDER_DISTANCE),         RES_BOX,              &::getCppuType((const sal_Int32*)0),    0, BORDER_DISTANCE|CONVERT_TWIPS },
        { SW_PROP_NAME(UNO_NAME_GRAPHIC_URL),               0,                      &::getCppuType((const OUString*)0), 0, 0 },
        { SW_PROP_NAME(UNO_NAME_GRAPHIC_FILTER),            0,                      &::getCppuType((const OUString*)0), 0, 0 },
        { SW_PROP_NAME(UNO_NAME_CONTOUR_POLY_POLYGON), FN_PARAM_COUNTOUR_PP, &::getCppuType((PointSequenceSequence*)0), PropertyAttribute::MAYBEVOID, 0 },
        { SW_PROP_NAME(UNO_NAME_Z_ORDER),               FN_UNO_Z_ORDER,         &::getCppuType((const sal_Int32*)0),        PROPERTY_NONE, 0},
        {0,0,0,0}
    };
    #define GRPH_PROP_COUNT 55
    return aGraphicDescPropertyMap_Impl;
}

/****************************************************************************
    Rahmenbeschreibung
****************************************************************************/
class BaseFrameProperties_Impl
{
    const SfxItemPropertyMap*   _pMap;
public:

    BaseFrameProperties_Impl(const SfxItemPropertyMap*  pMap) :
        _pMap(pMap){}

    virtual sal_Bool        SetProperty(const String& rName, uno::Any aVal) = 0;
    virtual sal_Bool        GetProperty(const String& rName, uno::Any*& pAny )  = 0;

    const SfxItemPropertyMap*   GetMap() const {return _pMap;}
    sal_Bool                        FillBaseProperties(SfxItemSet& rSet);

    virtual sal_Bool                AnyToItemSet(SfxItemSet& rFrmSet, SfxItemSet& rSet) = 0;

};
/* -----------------29.06.98 09:55-------------------
 *
 * --------------------------------------------------*/
sal_Bool BaseFrameProperties_Impl::FillBaseProperties(SfxItemSet& rSet)
{
    sal_Bool bRet = sal_True;
    //Anker kommt auf jeden Fall in den Set
    SwFmtAnchor aAnchor;
    {
        uno::Any* pAnchorPgNo;
        if(GetProperty(C2S(UNO_NAME_ANCHOR_PAGE_NO), pAnchorPgNo))
            bRet &= ((SfxPoolItem&)aAnchor).PutValue(*pAnchorPgNo, MID_ANCHOR_PAGENUM);
        uno::Any* pAnchorType;
        if(GetProperty(C2S(UNO_NAME_ANCHOR_TYPE), pAnchorType))
            bRet &= ((SfxPoolItem&)aAnchor).PutValue(*pAnchorType, MID_ANCHOR_ANCHORTYPE);
    }
    if(aAnchor.GetAnchorId() == FLY_PAGE && !aAnchor.GetPageNum())
        aAnchor.SetPageNum(1);
    rSet.Put(aAnchor);
    {
        uno::Any* pCol = 0;
        GetProperty(C2S(UNO_NAME_BACK_COLOR), pCol );
        uno::Any* pTrans = 0;
        GetProperty(C2S(UNO_NAME_BACK_TRANSPARENT), pTrans );
        uno::Any* pGrLoc = 0;
        GetProperty(C2S(UNO_NAME_BACK_GRAPHIC_LOCATION), pGrLoc );
        uno::Any* pGrURL = 0;
        GetProperty(C2S(UNO_NAME_BACK_GRAPHIC_URL), pGrURL     );
        uno::Any* pGrFilter = 0;
        GetProperty(C2S(UNO_NAME_BACK_GRAPHIC_FILTER), pGrFilter     );

        if(pCol || pTrans || pGrURL || pGrFilter || pGrLoc)
        {
            SvxBrushItem aBrush(RES_BACKGROUND);
            if(pCol )
                bRet &= ((SfxPoolItem&)aBrush).PutValue(*pCol,MID_BACK_COLOR    );
            if(pTrans)
                bRet &= ((SfxPoolItem&)aBrush).PutValue(*pTrans, MID_GRAPHIC_TRANSPARENT);
            if(pGrURL)
                bRet &= ((SfxPoolItem&)aBrush).PutValue(*pGrURL, MID_GRAPHIC_URL);
            if(pGrFilter)
                bRet &= ((SfxPoolItem&)aBrush).PutValue(*pGrFilter, MID_GRAPHIC_FILTER);
            if(pGrLoc)
                bRet &= ((SfxPoolItem&)aBrush).PutValue(*pGrLoc, MID_GRAPHIC_POSITION);
            rSet.Put(aBrush);
        }
    }
    {
        uno::Any* pCont = 0;
        GetProperty(C2S(UNO_NAME_CONTENT_PROTECTED), pCont );
        uno::Any* pPos = 0;
        GetProperty(C2S(UNO_NAME_POSITION_PROTECTED), pPos );
        uno::Any* pName = 0;
        GetProperty(C2S(UNO_NAME_SIZE_PROTECTED), pName );
        if(pCont||pPos||pName)
        {
            SvxProtectItem aProt(RES_PROTECT);
            if(pCont)
                bRet &= ((SfxPoolItem&)aProt).PutValue(*pCont, MID_PROTECT_CONTENT);
            if(pPos )
                bRet &= ((SfxPoolItem&)aProt).PutValue(*pPos, MID_PROTECT_POSITION);
            if(pName)
                bRet &= ((SfxPoolItem&)aProt).PutValue(*pName, MID_PROTECT_SIZE);
            rSet.Put(aProt);
        }
    }
    {
        uno::Any* pHori  = 0;
        GetProperty(C2S(UNO_NAME_HORI_ORIENT), pHori );
        uno::Any* pHoriP = 0;
        GetProperty(C2S(UNO_NAME_HORI_ORIENT_POSITION), pHoriP );
        uno::Any* pHoriR = 0;
        GetProperty(C2S(UNO_NAME_HORI_ORIENT_RELATION), pHoriR );
        uno::Any* pPageT = 0;
        GetProperty(C2S(UNO_NAME_PAGE_TOGGLE), pPageT);
        if(pHori||pHoriP||pHoriR||pPageT)
        {
            SwFmtHoriOrient aOrient;
            if(pHori )
                bRet &= ((SfxPoolItem&)aOrient).PutValue(*pHori, MID_HORIORIENT_ORIENT);
            if(pHoriP)
                bRet &= ((SfxPoolItem&)aOrient).PutValue(*pHoriP, MID_HORIORIENT_POSITION|CONVERT_TWIPS);
            if(pHoriR)
                bRet &= ((SfxPoolItem&)aOrient).PutValue(*pHoriR, MID_HORIORIENT_RELATION);
            if(pPageT)
                bRet &= ((SfxPoolItem&)aOrient).PutValue(*pPageT, MID_HORIORIENT_PAGETOGGLE);
            rSet.Put(aOrient);
        }
    }

    {
        uno::Any* pVert  = 0;
        GetProperty(C2S(UNO_NAME_VERT_ORIENT), pVert);
        uno::Any* pVertP = 0;
        GetProperty(C2S(UNO_NAME_VERT_ORIENT_POSITION), pVertP );
        uno::Any* pVertR = 0;
        GetProperty(C2S(UNO_NAME_VERT_ORIENT_RELATION), pVertR );
        if(pVert||pVertP||pVertR)
        {
            SwFmtVertOrient aOrient;
            if(pVert )
                bRet &= ((SfxPoolItem&)aOrient).PutValue(*pVert, MID_VERTORIENT_ORIENT);
            if(pVertP)
                bRet &= ((SfxPoolItem&)aOrient).PutValue(*pVertP, MID_VERTORIENT_POSITION|CONVERT_TWIPS);
            if(pVertR)
                bRet &= ((SfxPoolItem&)aOrient).PutValue(*pVertR, MID_VERTORIENT_RELATION);
            rSet.Put(aOrient);
        }
    }
    {
        uno::Any* pURL = 0;
        GetProperty(C2S(UNO_NAME_HYPER_LINK_U_R_L), pURL );
        uno::Any* pTarget = 0;
        GetProperty(C2S(UNO_NAME_HYPER_LINK_TARGET), pTarget );
        uno::Any* pHyLNm = 0;
        GetProperty(C2S(UNO_NAME_HYPER_LINK_NAME), pHyLNm );
        uno::Any* pHySMp = 0;
        GetProperty(C2S(UNO_NAME_SERVER_MAP), pHySMp );
        if(pURL||pTarget||pHyLNm||pHySMp)
        {
            SwFmtURL aURL;
            if(pURL)
                bRet &= ((SfxPoolItem&)aURL).PutValue(*pURL, MID_URL_URL);
            if(pTarget)
                bRet &= ((SfxPoolItem&)aURL).PutValue(*pTarget, MID_URL_TARGET);
            if(pHyLNm)
                bRet &= ((SfxPoolItem&)aURL).PutValue(*pHyLNm, MID_URL_HYPERLINKNAME  );
            if(pHySMp)
                bRet &= ((SfxPoolItem&)aURL).PutValue(*pHySMp, MID_URL_SERVERMAP);
            rSet.Put(aURL);
        }
    }
    uno::Any* pL = 0;
    GetProperty(C2S(UNO_NAME_LEFT_MARGIN), pL );
    uno::Any* pR = 0;
    GetProperty(C2S(UNO_NAME_RIGHT_MARGIN), pR );
    if(pL||pR)
    {
        SvxLRSpaceItem aLR(RES_LR_SPACE);
        if(pL)
            bRet &= ((SfxPoolItem&)aLR).PutValue(*pL, MID_L_MARGIN|CONVERT_TWIPS);
        if(pR)
            bRet &= ((SfxPoolItem&)aLR).PutValue(*pR, MID_R_MARGIN|CONVERT_TWIPS);
        rSet.Put(aLR);
    }
    uno::Any* pT = 0;
    GetProperty(C2S(UNO_NAME_TOP_MARGIN), pT );
    uno::Any* pB = 0;
    GetProperty(C2S(UNO_NAME_BOTTOM_MARGIN), pB );
    if(pT||pB)
    {
        SvxULSpaceItem aTB(RES_UL_SPACE);
        if(pT)
            bRet &= ((SfxPoolItem&)aTB).PutValue(*pT, MID_UP_MARGIN|CONVERT_TWIPS);
        if(pB)
            bRet &= ((SfxPoolItem&)aTB).PutValue(*pB, MID_LO_MARGIN|CONVERT_TWIPS);
        rSet.Put(aTB);
    }
    uno::Any* pOp;
    if(GetProperty(C2S(UNO_NAME_OPAQUE), pOp))
    {
        SvxOpaqueItem aOp(RES_OPAQUE);
        bRet &= ((SfxPoolItem&)aOp).PutValue(*pOp, 0);
        rSet.Put(aOp);
    }
    uno::Any* pPrt;
    if(GetProperty(C2S(UNO_NAME_PRINT), pPrt))
    {
        SvxPrintItem aPrt(RES_PRINT);
        bRet &= ((SfxPoolItem&)aPrt).PutValue(*pPrt, 0);
        rSet.Put(aPrt);
    }
    uno::Any* pSh;
    if(GetProperty(C2S(UNO_NAME_SHADOW_FORMAT), pSh))
    {
        SvxShadowItem aSh(RES_SHADOW);
        bRet &= ((SfxPoolItem&)aSh).PutValue(*pSh, CONVERT_TWIPS);
        rSet.Put(aSh);
    }
    uno::Any* pSur   = 0;
    GetProperty(C2S(UNO_NAME_SURROUND), pSur);
    if( !pSur )
        GetProperty(C2S(UNO_NAME_TEXT_WRAP), pSur);
    uno::Any* pSurAnch = 0;
    GetProperty(C2S(UNO_NAME_SURROUND_ANCHORONLY), pSurAnch);
    if(pSur || pSurAnch)
    {
        SwFmtSurround aSrnd;
        if(pSur)
            bRet &= ((SfxPoolItem&)aSrnd).PutValue(*pSur, MID_SURROUND_SURROUNDTYPE );
        if(pSurAnch)
            bRet &= ((SfxPoolItem&)aSrnd).PutValue(*pSurAnch, MID_SURROUND_ANCHORONLY);
        rSet.Put(aSrnd);
    }
    uno::Any* pLeft         = 0;
    GetProperty(C2S(UNO_NAME_LEFT_BORDER)  ,    pLeft  );
    uno::Any* pRight        = 0;
    GetProperty(C2S(UNO_NAME_RIGHT_BORDER) ,    pRight );
    uno::Any* pTop      = 0;
    GetProperty(C2S(UNO_NAME_TOP_BORDER)      , pTop   );
    uno::Any* pBottom   = 0;
    GetProperty(C2S(UNO_NAME_BOTTOM_BORDER),    pBottom);
    uno::Any* pDistance     = 0;
    GetProperty(C2S(UNO_NAME_BORDER_DISTANCE),  pDistance);
    if( pLeft || pRight || pTop ||  pBottom || pDistance)
    {
        SvxBoxItem aBox;
        if( pLeft )
            bRet &= ((SfxPoolItem&)aBox).PutValue(*pLeft, CONVERT_TWIPS|LEFT_BORDER );
        if( pRight )
            bRet &= ((SfxPoolItem&)aBox).PutValue(*pRight, CONVERT_TWIPS|RIGHT_BORDER );
        if( pTop )
            bRet &= ((SfxPoolItem&)aBox).PutValue(*pTop, CONVERT_TWIPS|TOP_BORDER);
        if( pBottom )
            bRet &= ((SfxPoolItem&)aBox).PutValue(*pBottom, CONVERT_TWIPS|BOTTOM_BORDER);
        if( pDistance )
            bRet &= ((SfxPoolItem&)aBox).PutValue(*pDistance, CONVERT_TWIPS|BORDER_DISTANCE);
        rSet.Put(aBox);
    }
    {
        uno::Any* pRelH = 0;
        GetProperty(C2S(UNO_NAME_RELATIVE_HEIGHT), pRelH);
        uno::Any* pRelW = 0;
        GetProperty(C2S(UNO_NAME_RELATIVE_WIDTH), pRelW);
        uno::Any* pSzRel = 0;
        GetProperty(C2S(UNO_NAME_SIZE_RELATIVE), pSzRel);
        uno::Any* pWidth = 0;
        GetProperty(C2S(UNO_NAME_WIDTH), pWidth);
        uno::Any* pHeight = 0;
        GetProperty(C2S(UNO_NAME_HEIGHT), pHeight);
        uno::Any* pSize = 0;
        GetProperty(C2S(UNO_NAME_SIZE), pSize);
        uno::Any* pSizeType = 0;
        GetProperty(C2S(UNO_NAME_SIZE_TYPE), pSizeType);
        if(pWidth || pHeight ||pRelH || pRelW || pSzRel || pSize ||pSizeType)
        {
            SwFmtFrmSize aFrmSz;
            if(pWidth)
                bRet &= ((SfxPoolItem&)aFrmSz).PutValue(*pWidth, MID_FRMSIZE_WIDTH|CONVERT_TWIPS);
            if(pHeight)
                bRet &= ((SfxPoolItem&)aFrmSz).PutValue(*pHeight, MID_FRMSIZE_HEIGHT|CONVERT_TWIPS);
            if(pRelH )
                bRet &= ((SfxPoolItem&)aFrmSz).PutValue(*pRelH, MID_FRMSIZE_REL_HEIGHT);
            if(pRelW )
                bRet &= ((SfxPoolItem&)aFrmSz).PutValue(*pRelW, MID_FRMSIZE_REL_WIDTH);
            if(pSzRel)
                bRet &= ((SfxPoolItem&)aFrmSz).PutValue(*pSzRel, MID_FRMSIZE_IS_SYNC_REL_SIZE);
            if(pSize)
                bRet &= ((SfxPoolItem&)aFrmSz).PutValue(*pSize, MID_FRMSIZE_SIZE|CONVERT_TWIPS);
            if(pSizeType)
                bRet &= ((SfxPoolItem&)aFrmSz).PutValue(*pSizeType, MID_FRMSIZE_SIZE_TYPE|CONVERT_TWIPS);
            rSet.Put(aFrmSz);
        }
        else
        {
            SwFmtFrmSize aFrmSz;
            awt::Size aSize;
            aSize.Width = MM50;
            aSize.Height = MM50;
            uno::Any aSizeVal;
            aSizeVal <<= aSize;
            ((SfxPoolItem&)aFrmSz).PutValue(aSizeVal, MID_FRMSIZE_SIZE|CONVERT_TWIPS);
            rSet.Put(aFrmSz);
        }
    }
    return bRet;
}
/* -----------------22.06.98 09:17-------------------
 *
 * --------------------------------------------------*/
class SwFrameProperties_Impl : public BaseFrameProperties_Impl
{
    uno::Any*                   pAnyArr[FRM_PROP_COUNT];
    sal_uInt16                      nArrLen;

public:
    SwFrameProperties_Impl();
    ~SwFrameProperties_Impl();

    virtual sal_Bool        SetProperty(const String& rName, uno::Any aVal);
    virtual sal_Bool        GetProperty(const String& rName, uno::Any*& pAny );

    virtual sal_Bool        AnyToItemSet(SfxItemSet& rFrmSet, SfxItemSet& rSet);
};
/* -----------------22.06.98 09:17-------------------
 *
 * --------------------------------------------------*/
SwFrameProperties_Impl::SwFrameProperties_Impl() :
    BaseFrameProperties_Impl(GetFrameDescMap()),
    nArrLen(FRM_PROP_COUNT)
{
    for(sal_uInt16 i = 0; i < nArrLen; i++)
        pAnyArr[i] = 0;
}
/* -----------------22.06.98 09:17-------------------
 *
 * --------------------------------------------------*/
SwFrameProperties_Impl::~SwFrameProperties_Impl()
{
    for(sal_uInt16 i = 0; i < nArrLen; i++)
        delete pAnyArr[i];
}

/* -----------------22.06.98 09:51-------------------
 *
 * --------------------------------------------------*/
sal_Bool SwFrameProperties_Impl::SetProperty(const String& rName, uno::Any aVal)
{
    sal_uInt16 nPos = 0;
    const SfxItemPropertyMap* pTemp = GetMap();
    while( pTemp->pName )
    {
        if(COMPARE_EQUAL == rName.CompareToAscii(pTemp->pName))
            break;
        ++nPos;
        ++pTemp;
    }
    if(nPos < nArrLen)
    {
        delete pAnyArr[nPos];
        pAnyArr[nPos] = new uno::Any(aVal);
    }
    return nPos < nArrLen;
}
/* -----------------22.06.98 09:51-------------------
 *
 * --------------------------------------------------*/
sal_Bool SwFrameProperties_Impl::GetProperty(const String& rName, uno::Any*& pAny)
{
    sal_uInt16 nPos = 0;
    const SfxItemPropertyMap* pTemp = GetMap();
    while( pTemp->pName )
    {
        if(COMPARE_EQUAL == rName.CompareToAscii(pTemp->pName))
            break;
        ++nPos;
        ++pTemp;
    }
    if(nPos < nArrLen)
        pAny = pAnyArr[nPos];
    return nPos < nArrLen && pAny;
}
/* -----------------22.06.98 11:27-------------------
 *
 * --------------------------------------------------*/
sal_Bool    SwFrameProperties_Impl::AnyToItemSet(SfxItemSet& rSet, SfxItemSet&)
{
    //Properties fuer alle Frames
    sal_Bool bRet = FillBaseProperties(rSet);

    uno::Any* pEdit;
    if(GetProperty(C2S(UNO_NAME_EDIT_IN_READONLY), pEdit))
    {
        SfxBoolItem aBool(RES_EDIT_IN_READONLY);
        ((SfxPoolItem&)aBool).PutValue(*pEdit, 0);
        rSet.Put(aBool);
    }
    uno::Any* pColumns;
    if(GetProperty(C2S(UNO_NAME_TEXT_COLUMNS), pColumns))
    {
        SwFmtCol aCol;
        ((SfxPoolItem&)aCol).PutValue(*pColumns, MID_COLUMNS);
        rSet.Put(aCol);
    }
    return bRet;
}
/****************************************************************************
    Grafik-Descriptor
****************************************************************************/
class SwGraphicProperties_Impl : public BaseFrameProperties_Impl
{
    uno::Any*                   pAnyArr[GRPH_PROP_COUNT];
    sal_uInt16                      nArrLen;

public:
    SwGraphicProperties_Impl();
    ~SwGraphicProperties_Impl();

    virtual sal_Bool        SetProperty(const String& rName, uno::Any aVal);
    virtual sal_Bool        GetProperty(const String& rName, uno::Any*& pAny );

    virtual sal_Bool                AnyToItemSet(SfxItemSet& rFrmSet, SfxItemSet& rSet);
};
/* -----------------27.06.98 14:53-------------------
 *
 * --------------------------------------------------*/
SwGraphicProperties_Impl::SwGraphicProperties_Impl() :
    BaseFrameProperties_Impl(GetGraphicDescMap()),
    nArrLen(GRPH_PROP_COUNT)
{
    for(sal_uInt16 i = 0; i < nArrLen; i++)
        pAnyArr[i] = 0;
}
/* -----------------27.06.98 14:54-------------------
 *
 * --------------------------------------------------*/
SwGraphicProperties_Impl::~SwGraphicProperties_Impl()
{
    for(sal_uInt16 i = 0; i < nArrLen; i++)
        delete pAnyArr[i];
}
/* -----------------27.06.98 14:53-------------------
 *
 * --------------------------------------------------*/
sal_Bool SwGraphicProperties_Impl::SetProperty(const String& rName, uno::Any aVal)
{
    sal_uInt16 nPos = 0;
    const SfxItemPropertyMap* pTemp = GetMap();
    while( pTemp->pName )
    {
        if(rName.CompareToAscii(pTemp->pName) == COMPARE_EQUAL)
            break;
        ++nPos;
        ++pTemp;
    }
    if(nPos < nArrLen)
    {
        delete pAnyArr[nPos];
        pAnyArr[nPos] = new uno::Any(aVal);
    }
    return nPos < nArrLen;
}
/* -----------------27.06.98 14:53-------------------
 *
 * --------------------------------------------------*/
sal_Bool SwGraphicProperties_Impl::GetProperty(const String& rName, uno::Any*& pAny)
{
    sal_uInt16 nPos = 0;
    const SfxItemPropertyMap* pTemp = GetMap();
    while( pTemp->pName )
    {
        if(COMPARE_EQUAL == rName.CompareToAscii(pTemp->pName))
            break;
        ++nPos;
        ++pTemp;
    }
    if(nPos < nArrLen)
        pAny = pAnyArr[nPos];
    return pAny &&nPos < nArrLen;
}
/* -----------------27.06.98 14:40-------------------
 *
 * --------------------------------------------------*/
sal_Bool    SwGraphicProperties_Impl::AnyToItemSet(
            SfxItemSet& rFrmSet,
            SfxItemSet& rGrSet)
{
    //Properties fuer alle Frames
    sal_Bool bRet = FillBaseProperties(rFrmSet);

    uno::Any* pHEvenMirror = 0;
    uno::Any* pHOddMirror = 0;
    uno::Any* pVMirror = 0;
    GetProperty(C2S(UNO_NAME_HORI_MIRRORED_ON_EVEN_PAGES), pHEvenMirror);
    GetProperty(C2S(UNO_NAME_HORI_MIRRORED_ON_ODD_PAGES), pHOddMirror);
    GetProperty(C2S(UNO_NAME_VERT_MIRRORED), pVMirror);
    if(pHEvenMirror || pHOddMirror || pVMirror )
    {
        SwMirrorGrf aMirror;
        if(pHEvenMirror)
            bRet &= ((SfxPoolItem&)aMirror).PutValue(*pHEvenMirror, MID_MIRROR_HORZ_EVEN_PAGES);
        if(pHOddMirror)
            bRet &= ((SfxPoolItem&)aMirror).PutValue(*pHOddMirror, MID_MIRROR_HORZ_ODD_PAGES);
        if(pVMirror)
            bRet &= ((SfxPoolItem&)aMirror).PutValue(*pVMirror, MID_MIRROR_VERT);
        rGrSet.Put(aMirror);
    }
    uno::Any* pCrop;
    if(GetProperty(C2S(UNO_NAME_GRAPHIC_CROP), pCrop ))
    {
        SwCropGrf aCrop;
        bRet &= ((SfxPoolItem&)aCrop).PutValue(*pCrop, CONVERT_TWIPS);
        rGrSet.Put(aCrop);
    }
    return bRet;
}


/******************************************************************
 *  SwXFrame
 ******************************************************************/
/* -----------------------------10.03.00 18:02--------------------------------

 ---------------------------------------------------------------------------*/
const uno::Sequence< sal_Int8 > & SwXFrame::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------10.03.00 18:04--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL SwXFrame::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
            return (sal_Int64)this;
    }
    return 0;
}
/*-----------------24.03.98 14:49-------------------

--------------------------------------------------*/

TYPEINIT1(SwXFrame, SwClient);
/* -----------------------------06.04.00 14:20--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXFrame::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXFrame");
}
/* -----------------------------06.04.00 14:20--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXFrame::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.Frame") == rServiceName;
}
/* -----------------------------06.04.00 14:20--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXFrame::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.Frame");
    return aRet;
}
/*-- 11.12.98 15:05:00---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXFrame::SwXFrame() :
    eType(FLYCNTTYPE_FRM),
    aLstnrCntnr( (container::XNamed*)this),
    aPropSet(0),
    _pMap(0),
    bIsDescriptor(sal_False),
    pProps(0)
{

}
/*-- 14.01.99 11:31:52---------------------------------------------------
    Dieser CTor legt den Frame als Descriptor an
  -----------------------------------------------------------------------*/
SwXFrame::SwXFrame(FlyCntType eSet, const SfxItemPropertyMap*   pMap) :
    aLstnrCntnr( (container::XNamed*)this),
    eType(eSet),
    aPropSet(pMap),
    _pMap(pMap),
    bIsDescriptor(sal_True)
{
    switch(eType)
    {
        case FLYCNTTYPE_FRM:
            pProps = new SwFrameProperties_Impl();
        break;
        case FLYCNTTYPE_GRF:
            pProps = new SwGraphicProperties_Impl();
        break;
        case FLYCNTTYPE_OLE:
            pProps = 0;
        break;
    }
}

/*-- 11.12.98 15:05:01---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXFrame::SwXFrame(SwFrmFmt& rFrmFmt, FlyCntType eSet, const SfxItemPropertyMap* pMap) :
    eType(eSet),
    SwClient( &rFrmFmt ),
    aLstnrCntnr( (container::XNamed*)this),
    aPropSet(pMap),
    _pMap(pMap),
    bIsDescriptor(sal_False),
    pProps(0)
{

}
/*-- 11.12.98 15:05:02---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXFrame::~SwXFrame()
{
    delete pProps;
}
/*-- 11.12.98 15:05:03---------------------------------------------------

  -----------------------------------------------------------------------*/
/*-- 11.12.98 15:05:03---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXFrame::getName(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    String sRet;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
        sRet = pFmt->GetName();
    else if(bIsDescriptor)
        sRet = sName;
    else
        throw RuntimeException();
    return sRet;
}
/*-- 11.12.98 15:05:03---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFrame::setName(const OUString& rName) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwFrmFmt* pFmt = GetFrmFmt();
    String sTmpName(rName);
    if(pFmt)
    {
        pFmt->GetDoc()->SetFlyName((SwFlyFrmFmt&)*pFmt, sTmpName);
        if(pFmt->GetName() != sTmpName)
        {
            throw RuntimeException();
        }
    }
    else if(bIsDescriptor)
        sName = sTmpName;
    else
        throw RuntimeException();
}
/*-- 11.12.98 15:05:03---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XPropertySetInfo >  SwXFrame::getPropertySetInfo(void) throw( RuntimeException )
{
    uno::Reference< XPropertySetInfo >  xRef;
    static uno::Reference< XPropertySetInfo >  xFrmRef;
    static uno::Reference< XPropertySetInfo >  xGrfRef;
    static uno::Reference< XPropertySetInfo >  xOLERef;
    switch(eType)
    {
    case FLYCNTTYPE_FRM:
        if( !xFrmRef.is() )
            xFrmRef = aPropSet.getPropertySetInfo();
        xRef = xFrmRef;
        break;
    case FLYCNTTYPE_GRF:
        if( !xGrfRef.is() )
            xGrfRef = aPropSet.getPropertySetInfo();
        xRef = xGrfRef;
        break;
    case FLYCNTTYPE_OLE:
        if( !xOLERef.is() )
            xOLERef = aPropSet.getPropertySetInfo();
        xRef = xOLERef;
        break;
    }
    return xRef;
}
/*-- 11.12.98 15:05:04---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFrame::setPropertyValue(const OUString& rPropertyName, const uno::Any& aValue)
    throw( UnknownPropertyException, PropertyVetoException, lang::IllegalArgumentException, WrappedTargetException, RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        sal_Bool bNextFrame = sal_False;
        const SfxItemPropertyMap* pCur = SfxItemPropertyMap::GetByName(_pMap, rPropertyName);
        if(!pCur)
            throw UnknownPropertyException();
        SwDoc* pDoc = pFmt->GetDoc();
        if( eType == FLYCNTTYPE_GRF &&
                    (COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_ALTERNATIVE_TEXT)||
                    (pCur &&
                    (pCur->nWID == RES_GRFATR_CROPGRF ||
                        pCur->nWID == RES_GRFATR_MIRRORGRF ||
                            pCur->nWID == FN_PARAM_COUNTOUR_PP))))
        {
            const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
            if(pIdx)
            {
                SwNodeIndex aIdx(*pIdx, 1);
                SwNoTxtNode* pNoTxt = aIdx.GetNode().GetNoTxtNode();
                if(COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_ALTERNATIVE_TEXT))
                {
                    OUString uTemp;
                    aValue >>= uTemp;
                    pNoTxt->SetAlternateText(uTemp);
                }
                else if(pCur->nWID == FN_PARAM_COUNTOUR_PP)
                {
                    PointSequenceSequence aParam;
                    if(!aValue.hasValue())
                        pNoTxt->SetContour(0);
                    else if(aValue >>= aParam)
                    {
                        PolyPolygon aPoly(aParam.getLength());
                        for(sal_Int32 i = 0; i < aParam.getLength(); i++)
                        {
                            const PointSequence* pPointSeq = aParam.getConstArray();
                            Polygon aSet(pPointSeq[i].getLength());
                            for(sal_Int32 j = 0; j < pPointSeq[i].getLength(); j++)
                            {
                                const awt::Point* pPoints = pPointSeq[i].getConstArray();
                                Point aPoint(MM100_TO_TWIP(pPoints[j].X), MM100_TO_TWIP(pPoints[j].Y));
                                aSet.SetPoint(aPoint, j);
                            }
                            aPoly.Insert( aSet );
                        }
                        pNoTxt->SetContour(&aPoly);
                    }
                    else
                        throw IllegalArgumentException();
                }
                else
                {
                    SfxItemSet aSet(pNoTxt->GetSwAttrSet());
                    aPropSet.setPropertyValue(*pCur, aValue, aSet);
                    pNoTxt->SetAttr(aSet);
                }
            }
        }
        else if(COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_FRAME_STYLE_NAME))
        {
            SwDocShell* pDocSh = pFmt->GetDoc()->GetDocShell();
            if(pDocSh)
            {
                OUString uTemp;
                aValue >>= uTemp;
                String sStyle(SwXStyleFamilies::GetUIName(uTemp, SFX_STYLE_FAMILY_FRAME));
                SwDocStyleSheet* pStyle =
                    (SwDocStyleSheet*)pDocSh->GetStyleSheetPool()->Find(sStyle, SFX_STYLE_FAMILY_FRAME);
                if(pStyle)
                {
//                  pSh->SetFrmFmt( pStyle->GetFrmFmt() );
                    UnoActionContext aAction(pFmt->GetDoc());
                    pFmt->GetDoc()->SetFrmFmtToFly( *pFmt, *pStyle->GetFrmFmt());
                }
                else
                    throw IllegalArgumentException();
            }
        }
        else if( COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_GRAPHIC_URL) ||
                COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_GRAPHIC_FILTER))
        {
            String sGrfName, sFltName;
            pFmt->GetDoc()->GetGrfNms( *(SwFlyFrmFmt*)pFmt, &sGrfName, &sFltName );
            OUString uTemp;
            aValue >>= uTemp;
            String sTmp(uTemp);
            UnoActionContext aAction(pFmt->GetDoc());
            if(COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_GRAPHIC_URL))
                sGrfName = sTmp;
            else
                sFltName = sTmp;

            const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
            if(pIdx)
            {
                SwNodeIndex aIdx(*pIdx, 1);
//              SwNoTxtNode* pNoTxt = aIdx.GetNode().GetNoTxtNode();
                SwGrfNode* pGrfNode = aIdx.GetNode().GetGrfNode();
                if(!pGrfNode)
                    throw RuntimeException();
                SwPaM aGrfPaM(*pGrfNode);
                pFmt->GetDoc()->ReRead( aGrfPaM, sGrfName, sFltName, 0 );
            }
        }
        else if(0 != (bNextFrame = (COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_CHAIN_NEXT_NAME)))
            || COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_CHAIN_PREV_NAME))
        {
            OUString uTemp;
            aValue >>= uTemp;
            String sChainName(uTemp);
            if(!sChainName.Len())
            {
                if(bNextFrame)
                    pDoc->Unchain(*pFmt);
                else
                {
                    SwFmtChain aChain( pFmt->GetChain() );
                    SwFrmFmt *pPrev = aChain.GetPrev();
                    if(pPrev)
                        pDoc->Unchain(*pPrev);
                }
            }
            else
            {
                sal_uInt16 nCount = pDoc->GetFlyCount(eType);

                SwFrmFmt* pChain = 0;
                for( sal_uInt16 i = 0; i < nCount; i++)
                {
                    SwFrmFmt* pFmt = pDoc->GetFlyNum(i, FLYCNTTYPE_FRM);
                    if(sChainName == pFmt->GetName() )
                    {
                        pChain = pFmt;
                        break;
                    }
                }
                if(pChain)
                {
                    SwFrmFmt* pSource = bNextFrame ? pFmt : pChain;
                    SwFrmFmt* pDest = bNextFrame ? pChain: pFmt;
                    pDoc->Chain(*pSource, *pDest);
                }
            }
        }
        else if(FN_UNO_Z_ORDER == pCur->nWID)
        {
            sal_Int32 nZOrder = - 1;
            aValue >>= nZOrder;
            const SwContact* pContact = pFmt->FindContactObj();
            if(pContact && nZOrder >= 0)
            {
                const SdrObject* pObj = pContact->GetMaster();
                pFmt->GetDoc()->GetDrawModel()->GetPage(0)->
                            SetObjectOrdNum(pObj->GetOrdNum(), nZOrder);
            }
        }
        else
        {
            SfxItemSet aSet( pDoc->GetAttrPool(),
                RES_FRMATR_BEGIN, RES_FRMATR_END - 1 );

            aSet.SetParent(&pFmt->GetAttrSet());
            aPropSet.setPropertyValue(*pCur, aValue, aSet);
            if(COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_ANCHOR_TYPE))
                pFmt->GetDoc()->SetFlyFrmAttr( *pFmt, aSet );
            else
                pFmt->SetAttr(aSet);
        }
    }
    else if(IsDescriptor())
    {
        if(!pProps->SetProperty(rPropertyName, aValue))
            throw IllegalArgumentException();
    }
    else
        throw RuntimeException();
}
/*-- 11.12.98 15:05:04---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXFrame::getPropertyValue(const OUString& rPropertyName)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Any aAny;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_ANCHOR_TYPES))
    {
        uno::Sequence<TextContentAnchorType> aTypes(5);
         TextContentAnchorType* pArray = aTypes.getArray();
        pArray[0] = TextContentAnchorType_AT_PARAGRAPH;
        pArray[1] = TextContentAnchorType_AS_CHARACTER;
        pArray[2] = TextContentAnchorType_AT_PAGE;
        pArray[3] = TextContentAnchorType_AT_FRAME;
        pArray[4] = TextContentAnchorType_AT_CHARACTER;
        aAny.setValue(&aTypes, ::getCppuType((uno::Sequence<TextContentAnchorType>*)0));
    }
    else if(pFmt)
    {
        const SfxItemPropertyMap* pCur = SfxItemPropertyMap::GetByName(_pMap, rPropertyName);
        if(!pCur)
            throw UnknownPropertyException();
        if(eType == FLYCNTTYPE_GRF &&
                pCur &&
                (pCur->nWID == RES_GRFATR_CROPGRF ||
                    pCur->nWID == RES_GRFATR_MIRRORGRF ||
                        pCur->nWID == FN_PARAM_COUNTOUR_PP ))
        {
            const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
            if(pIdx)
            {
                SwNodeIndex aIdx(*pIdx, 1);
                SwNoTxtNode* pNoTxt = aIdx.GetNode().GetNoTxtNode();
                if(pCur->nWID == FN_PARAM_COUNTOUR_PP)
                {
                    const PolyPolygon* pContour = pNoTxt->HasContour();
                    if(pContour)
                    {
                        PointSequenceSequence aPtSeq(pContour->Count());
                        PointSequence* pPSeq = aPtSeq.getArray();
                        for(USHORT i = 0; i < pContour->Count(); i++)
                        {
                            const Polygon& rPoly = pContour->GetObject(i);
                            pPSeq[i].realloc(rPoly.GetSize());
                            awt::Point* pPoints = pPSeq[i].getArray();
                            for(USHORT j = 0; j < rPoly.GetSize(); j++)
                            {
                                const Point& rPoint = rPoly.GetPoint(j);
                                pPoints[j].X = TWIP_TO_MM100(rPoint.X());
                                pPoints[j].Y = TWIP_TO_MM100(rPoint.Y());
                            }
                        }
                        aAny <<= aPtSeq;
                    }
                }
                else
                {
                    SfxItemSet aSet(pNoTxt->GetSwAttrSet());
                    aAny = aPropSet.getPropertyValue(*pCur, aSet);
                }
            }
        }
        else if( COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_GRAPHIC_URL)||
            COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_GRAPHIC_FILTER))
        {
            String sGrfName, sFltName;
            pFmt->GetDoc()->GetGrfNms( *(SwFlyFrmFmt*)pFmt, &sGrfName, &sFltName );
            if(COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_GRAPHIC_URL))
                aAny <<= OUString(sGrfName);
            else
                aAny <<= OUString(sFltName);
        }
        else if(COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_FRAME_STYLE_NAME))
        {
            aAny <<= OUString(SwXStyleFamilies::GetProgrammaticName(pFmt->DerivedFrom()->GetName(), SFX_STYLE_FAMILY_FRAME));
        }
        else if(eType == FLYCNTTYPE_GRF &&
                (COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_ACTUAL_SIZE) ||
                    COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_ALTERNATIVE_TEXT)))
        {
            const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
            if(pIdx)
            {
                SwNodeIndex aIdx(*pIdx, 1);
                SwNoTxtNode* pNoTxt = aIdx.GetNode().GetNoTxtNode();
                if(COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_ALTERNATIVE_TEXT))
                    aAny <<= OUString(pNoTxt->GetAlternateText());
                else
                {
                     Size aActSize = ((SwGrfNode*)pNoTxt)->GetTwipSize();
                    awt::Size aTmp;
                    aTmp.Width = TWIP_TO_MM100(aActSize.Width());
                    aTmp.Height = TWIP_TO_MM100(aActSize.Height());
                    aAny.setValue(&aTmp, ::getCppuType((const awt::Size*)0));
                }
            }
        }
        else if(FN_PARAM_LINK_DISPLAY_NAME == pCur->nWID)
        {
            aAny <<= OUString(pFmt->GetName());
        }
        else if(FN_UNO_Z_ORDER == pCur->nWID)
        {
            const SwContact* pContact = pFmt->FindContactObj();
            if(pContact)
            {
                const SdrObject* pObj = pContact->GetMaster();
                aAny <<= (sal_Int32)pObj->GetOrdNum();
            }
        }
        else
        {
            const SwAttrSet& rSet = pFmt->GetAttrSet();
            aAny = aPropSet.getPropertyValue(*pCur, rSet);
        }
    }
    else if(IsDescriptor())
    {
        uno::Any* pAny = 0;
        if(!pProps->GetProperty(rPropertyName, pAny))
        {
            throw IllegalArgumentException();
        }
        else if(pAny)
            aAny = *pAny;
    }
    else
        throw RuntimeException();
    return aAny;
}
/*-- 11.12.98 15:05:04---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFrame::addPropertyChangeListener(const OUString& PropertyName,
    const uno::Reference< XPropertyChangeListener > & aListener)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 15:05:05---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFrame::removePropertyChangeListener(const OUString& PropertyName,
    const uno::Reference< XPropertyChangeListener > & aListener)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 15:05:05---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFrame::addVetoableChangeListener(const OUString& PropertyName,
                                const uno::Reference< XVetoableChangeListener > & aListener)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 15:05:05---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFrame::removeVetoableChangeListener(
    const OUString& PropertyName, const uno::Reference< XVetoableChangeListener > & aListener)
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 12.09.00 14:04:53---------------------------------------------------

  -----------------------------------------------------------------------*/
PropertyState SwXFrame::getPropertyState( const OUString& rPropertyName )
    throw(UnknownPropertyException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    Sequence< OUString > aPropertyNames(1);
    OUString* pNames = aPropertyNames.getArray();
    pNames[0] = rPropertyName;
    Sequence< PropertyState > aStates = getPropertyStates(aPropertyNames);
    return aStates.getConstArray()[0];
}
/*-- 12.09.00 14:04:54---------------------------------------------------

  -----------------------------------------------------------------------*/
Sequence< PropertyState > SwXFrame::getPropertyStates(
    const Sequence< OUString >& aPropertyNames )
        throw(UnknownPropertyException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    Sequence< PropertyState > aStates(aPropertyNames.getLength());
    PropertyState* pStates = aStates.getArray();
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        const OUString* pNames = aPropertyNames.getConstArray();
        const SwAttrSet& rFmtSet = pFmt->GetAttrSet();
        for(int i = 0; i < aPropertyNames.getLength(); i++)
        {
            const SfxItemPropertyMap* pCur = SfxItemPropertyMap::GetByName(_pMap, pNames[i]);
            if(!pCur)
                throw UnknownPropertyException();
            if(pCur->nWID == FN_UNO_ANCHOR_TYPES||
                pCur->nWID == FN_PARAM_LINK_DISPLAY_NAME||
                COMPARE_EQUAL == pNames[i].compareToAscii(UNO_NAME_FRAME_STYLE_NAME)||
                COMPARE_EQUAL == pNames[i].compareToAscii(UNO_NAME_GRAPHIC_URL)||
                COMPARE_EQUAL == pNames[i].compareToAscii(UNO_NAME_GRAPHIC_FILTER)||
                COMPARE_EQUAL == pNames[i].compareToAscii(UNO_NAME_ACTUAL_SIZE) ||
                COMPARE_EQUAL == pNames[i].compareToAscii(UNO_NAME_ALTERNATIVE_TEXT))
                pStates[i] = PropertyState_DIRECT_VALUE;
            else
            {
                if(eType == FLYCNTTYPE_GRF &&
                        pCur &&
                        (pCur->nWID == RES_GRFATR_CROPGRF ||
                            pCur->nWID == RES_GRFATR_MIRRORGRF))
                {
                    const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
                    if(pIdx)
                    {
                        SwNodeIndex aIdx(*pIdx, 1);
                        SwNoTxtNode* pNoTxt = aIdx.GetNode().GetNoTxtNode();
                        SfxItemSet aSet(pNoTxt->GetSwAttrSet());
                        aSet.GetItemState(pCur->nWID);
                        if(SFX_ITEM_SET == aSet.GetItemState( pCur->nWID, FALSE ))
                            pStates[i] = beans::PropertyState_DIRECT_VALUE;
                    }
                }
                else
                {
                    if(SFX_ITEM_SET == rFmtSet.GetItemState( pCur->nWID, FALSE ))
                        pStates[i] = beans::PropertyState_DIRECT_VALUE;
                    else
                        pStates[i] = beans::PropertyState_DEFAULT_VALUE;
                }
            }
        }
    }
    else if(IsDescriptor())
    {
        for(int i = 0; i < aPropertyNames.getLength(); i++)
            pStates[i] = PropertyState_DIRECT_VALUE;
    }
    else
        throw RuntimeException();
    return aStates;
}
/*-- 12.09.00 14:04:54---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFrame::setPropertyToDefault( const OUString& rPropertyName )
    throw(UnknownPropertyException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        const SwAttrSet& rFmtSet = pFmt->GetAttrSet();
        const SfxItemPropertyMap* pCur = SfxItemPropertyMap::GetByName(_pMap, rPropertyName);
        if(!pCur)
            throw UnknownPropertyException();
        BOOL bNextFrame;
        if( pCur->nWID &&
            pCur->nWID != FN_UNO_ANCHOR_TYPES &&
            pCur->nWID != FN_PARAM_LINK_DISPLAY_NAME)
        {
            if( eType == FLYCNTTYPE_GRF &&
                        (COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_ALTERNATIVE_TEXT)||
                        (pCur &&
                        (pCur->nWID == RES_GRFATR_CROPGRF ||
                            pCur->nWID == RES_GRFATR_MIRRORGRF))))
            {
                const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
                if(pIdx)
                {
                    SwNodeIndex aIdx(*pIdx, 1);
                    SwNoTxtNode* pNoTxt = aIdx.GetNode().GetNoTxtNode();
                    if(COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_ALTERNATIVE_TEXT))
                    {
                        pNoTxt->SetAlternateText(aEmptyStr);
                    }
                    else
                    {
                        SfxItemSet aSet(pNoTxt->GetSwAttrSet());
                        aSet.ClearItem(pCur->nWID);
                        pNoTxt->SetAttr(aSet);
                    }
                }
            }
            else
            {
                SwDoc* pDoc = pFmt->GetDoc();
                SfxItemSet aSet( pDoc->GetAttrPool(),
                    RES_FRMATR_BEGIN, RES_FRMATR_END - 1 );
                aSet.SetParent(&pFmt->GetAttrSet());
                aSet.ClearItem(pCur->nWID);
                if(COMPARE_EQUAL != rPropertyName.compareToAscii(UNO_NAME_ANCHOR_TYPE))
                    pFmt->SetAttr(aSet);
            }
        }
        else if(0 != (bNextFrame = (COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_CHAIN_NEXT_NAME)))
                || COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_CHAIN_PREV_NAME))
        {
            SwDoc* pDoc = pFmt->GetDoc();
            if(bNextFrame)
                pDoc->Unchain(*pFmt);
            else
            {
                SwFmtChain aChain( pFmt->GetChain() );
                SwFrmFmt *pPrev = aChain.GetPrev();
                if(pPrev)
                    pDoc->Unchain(*pPrev);
            }
        }
    }
    else if(!IsDescriptor())
        throw RuntimeException();

}
/*-- 12.09.00 14:04:55---------------------------------------------------

  -----------------------------------------------------------------------*/
Any SwXFrame::getPropertyDefault( const OUString& rPropertyName )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    Any aRet;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        const SfxItemPropertyMap* pCur = SfxItemPropertyMap::GetByName(_pMap, rPropertyName);
        if(pCur)
        {
            if(pCur->nWID < RES_FRMATR_END)
            {
                const SfxPoolItem& rDefItem =
                    pFmt->GetDoc()->GetAttrPool().GetDefaultItem(pCur->nWID);
                rDefItem.QueryValue(aRet, pCur->nMemberId);
            }
        }
        else
            throw UnknownPropertyException();
    }
    else if(!IsDescriptor())
        throw RuntimeException();
    return aRet;
}
/* -----------------22.04.99 14:59-------------------
 *
 * --------------------------------------------------*/
void SwXFrame::addEventListener(const uno::Reference< XEventListener > & aListener) throw( RuntimeException )
{
    if(!GetRegisteredIn())
        throw RuntimeException();
    aLstnrCntnr.AddListener(aListener);
}
/* -----------------22.04.99 14:59-------------------
 *
 * --------------------------------------------------*/
void SwXFrame::removeEventListener(const uno::Reference< XEventListener > & aListener) throw( RuntimeException )
{
    if(!GetRegisteredIn() || !aLstnrCntnr.RemoveListener(aListener))
        throw RuntimeException();
}
/*-- 11.12.98 15:05:06---------------------------------------------------

  -----------------------------------------------------------------------*/
void    SwXFrame::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
        aLstnrCntnr.Disposing();
}

/*-- 11.12.98 15:23:05---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFrame::dispose(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        if( pFmt->GetAnchor().GetAnchorId() == FLY_IN_CNTNT )
            {
                const SwPosition &rPos = *(pFmt->GetAnchor().GetCntntAnchor());
                SwTxtNode *pTxtNode = rPos.nNode.GetNode().GetTxtNode();
                const xub_StrLen nIdx = rPos.nContent.GetIndex();
                pTxtNode->Delete( RES_TXTATR_FLYCNT, nIdx, nIdx );
            }
            else
                pFmt->GetDoc()->DelLayoutFmt(pFmt);
    }
    else
        throw RuntimeException();

}
/*-- 11.12.98 16:02:27---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XTextRange >  SwXFrame::getAnchor(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< XTextRange >  aRef;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        if( pFmt->GetAnchor().GetAnchorId() != FLY_PAGE )
        {
            const SwPosition &rPos = *(pFmt->GetAnchor().GetCntntAnchor());
            aRef = ::CreateTextRangeFromPosition(pFmt->GetDoc(), rPos, 0);
        }
    }
    else
        throw RuntimeException();
    return aRef;
}
/* -----------------14.01.99 12:02-------------------
 *
 * --------------------------------------------------*/
void SwXFrame::ResetDescriptor()
{
    bIsDescriptor = sal_False;
    DELETEZ(pProps);
}
/* -----------------18.02.99 13:34-------------------
 *
 * --------------------------------------------------*/
void SwXFrame::attachToRange(const uno::Reference< XTextRange > & xTextRange)
            throw( IllegalArgumentException, RuntimeException )
{
    if(!IsDescriptor())
        throw RuntimeException();
    uno::Reference<XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
    SwXTextRange* pRange = 0;
    SwXTextCursor* pCursor = 0;
    if(xRangeTunnel.is())
    {
        pRange = (SwXTextRange*)xRangeTunnel->getSomething(
                                SwXTextRange::getUnoTunnelId());
        pCursor = (SwXTextCursor*)xRangeTunnel->getSomething(
                                SwXTextCursor::getUnoTunnelId());
    }


    SwDoc* pDoc = pRange ? (SwDoc*)pRange->GetDoc() : pCursor ? (SwDoc*)pCursor->GetDoc() : 0;
    if(pDoc)
    {
        SwUnoInternalPaM aIntPam(*pDoc);
        //das muss jetzt sal_True liefern
        SwXTextRange::XTextRangeToSwPaM(aIntPam, xTextRange);

        SwNode& rNode = pDoc->GetNodes().GetEndOfContent();
        SwPaM aPam(rNode);
        aPam.Move( fnMoveBackward, fnGoDoc );
        static sal_uInt16 __READONLY_DATA aFrmAttrRange[] =
        {
            RES_FRMATR_BEGIN,       RES_FRMATR_END-1,
            SID_ATTR_BORDER_INNER,  SID_ATTR_BORDER_INNER,
            0
        };
        static sal_uInt16 __READONLY_DATA aGrAttrRange[] =
        {
            RES_GRFATR_BEGIN,       RES_GRFATR_END-1,
            0
        };
        SfxItemSet aGrSet(pDoc->GetAttrPool(), aGrAttrRange );

        SfxItemSet aFrmSet(pDoc->GetAttrPool(), aFrmAttrRange );
        //jetzt muessen die passenden Items in den Set
        if(!pProps->AnyToItemSet(aFrmSet, aGrSet))
            throw IllegalArgumentException();
        //der TextRange wird einzeln behandelt
        *aPam.GetPoint() = *aIntPam.GetPoint();
        if(aIntPam.HasMark())
        {
            aPam.SetMark();
            *aPam.GetMark() = *aIntPam.GetMark();
        }

        const SfxPoolItem* pItem;
        if(SFX_ITEM_SET == aFrmSet.GetItemState(RES_ANCHOR, sal_False, &pItem) &&
            FLY_AT_FLY ==((const SwFmtAnchor*)pItem)->GetAnchorId() &&
            !aPam.GetNode()->FindFlyStartNode())
        {
            //rahmengebunden geht nur dort, wo ein Rahmen ist!
            SwFmtAnchor aAnchor(FLY_AT_CNTNT);
            aFrmSet.Put(aAnchor);
        }
        SwFlyFrmFmt* pFmt = 0;
        if( eType == FLYCNTTYPE_FRM)
        {
            UnoActionContext aCont(pDoc);
            pFmt = pDoc->MakeFlySection( FLY_AT_CNTNT, aPam.GetPoint(), &aFrmSet );
            if(pFmt)
            {
                pFmt->Add(this);
                if(sName.Len())
                    pDoc->SetFlyName((SwFlyFrmFmt&)*pFmt, sName);
            }
            //den SwXText wecken
            ((SwXTextFrame*)this)->SetDoc(GetFrmFmt()->GetDoc());
        }
        else if( eType == FLYCNTTYPE_GRF)
        {
            UnoActionContext aCont(pDoc);
            SwFlyFrmFmt* pFmt = 0;

            uno::Any* pGraphicURL;
            String sGraphicURL;
            if(pProps->GetProperty(C2S(UNO_NAME_GRAPHIC_URL), pGraphicURL))
            {
                OUString uTemp;
                (*pGraphicURL) >>= uTemp;
                sGraphicURL = String(uTemp);
            }

            String sFltName;
            uno::Any* pFilter;
            if(pProps->GetProperty(C2S(UNO_NAME_GRAPHIC_FILTER), pFilter))
            {
                OUString uTemp;
                (*pFilter) >>= uTemp;
                sFltName = String(uTemp);
            }

            SwFlyFrmFmt* pGFmt =    pDoc->Insert(aPam,
                                    sGraphicURL,
                                    sFltName, 0, &aFrmSet, &aGrSet);
            if(pGFmt)
            {
                pGFmt->Add(this);
                if(sName.Len())
                    pDoc->SetFlyName((SwFlyFrmFmt&)*pGFmt, sName);

            }
            uno::Any* pSurroundContour;
            if(pProps->GetProperty(C2S(UNO_NAME_SURROUND_CONTOUR), pSurroundContour))
                setPropertyValue(C2U(UNO_NAME_SURROUND_CONTOUR), *pSurroundContour);
            uno::Any* pContourOutside;
            if(pProps->GetProperty(C2S(UNO_NAME_CONTOUR_OUTSIDE), pContourOutside))
                setPropertyValue(C2U(UNO_NAME_CONTOUR_OUTSIDE), *pContourOutside);
            uno::Any* pContourPoly;
            if(pProps->GetProperty(C2S(UNO_NAME_CONTOUR_POLY_POLYGON), pContourPoly))
                setPropertyValue(C2U(UNO_NAME_CONTOUR_POLY_POLYGON), *pContourPoly);
            uno::Any* pAltText;
            if(pProps->GetProperty(C2S(UNO_NAME_ALTERNATIVE_TEXT), pAltText))
                setPropertyValue(C2U(UNO_NAME_ALTERNATIVE_TEXT), *pAltText);
        }
        else
        {
            DBG_ERROR("EmbeddedObject: not implemented")
            throw RuntimeException();
        }
        uno::Any* pOrder;
        if(pProps->GetProperty(C2S(UNO_NAME_Z_ORDER), pOrder))
        {
            setPropertyValue(C2U(UNO_NAME_Z_ORDER), *pOrder);
        }

    }
    else
        throw IllegalArgumentException();
    //setzt das Flag zurueck und loescht den Descriptor-Pointer
    ResetDescriptor();
}

/*-- 22.04.99 08:03:20---------------------------------------------------

  -----------------------------------------------------------------------*/
awt::Point SwXFrame::getPosition(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    RuntimeException aRuntime;
    aRuntime.Message = C2U("position cannot be determined with this method");
    throw aRuntime;
    return awt::Point();
}
/*-- 22.04.99 08:03:21---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFrame::setPosition(const awt::Point& aPosition) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    RuntimeException aRuntime;
    aRuntime.Message = C2U("position cannot be changed with this method");
    throw aRuntime;
}
/*-- 22.04.99 08:03:21---------------------------------------------------

  -----------------------------------------------------------------------*/
awt::Size SwXFrame::getSize(void) throw( RuntimeException )
{
    uno::Any aVal = getPropertyValue(C2U("Size"));
    awt::Size* pRet =  (awt::Size*)aVal.getValue();
    return *pRet;
}
/*-- 22.04.99 08:03:21---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFrame::setSize(const awt::Size& aSize)
    throw( PropertyVetoException, RuntimeException )
{
    uno::Any aVal(&aSize, ::getCppuType((const awt::Size*)0));
    setPropertyValue(C2U("Size"), aVal);
}
/*-- 22.04.99 08:03:21---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXFrame::getShapeType(void) throw( RuntimeException )
{
    return C2U("FrameShape");
}

/******************************************************************
 *  SwXTextFrame
 ******************************************************************/
/*-- 14.01.99 11:27:51---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextFrame::SwXTextFrame() :
    SwXFrame(FLYCNTTYPE_FRM, aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_FRAME)),
    SwXText(0, CURSOR_FRAME)
{
}
/*-- 11.12.98 15:23:01---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextFrame::SwXTextFrame(SwFrmFmt& rFmt) :
    SwXFrame(rFmt, FLYCNTTYPE_FRM, aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_FRAME)),
    SwXText(rFmt.GetDoc(), CURSOR_FRAME)
{

}
/*-- 11.12.98 15:23:02---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextFrame::~SwXTextFrame()
{
}
/* -----------------------------15.03.00 16:30--------------------------------

 ---------------------------------------------------------------------------*/
void SAL_CALL SwXTextFrame::acquire(  )throw()
{
    SwXFrame::acquire();
}
/* -----------------------------15.03.00 16:30--------------------------------

 ---------------------------------------------------------------------------*/
void SAL_CALL SwXTextFrame::release(  )throw()
{
    SwXFrame::release();
}
/* -----------------------------15.03.00 16:30--------------------------------

 ---------------------------------------------------------------------------*/
uno::Any SAL_CALL SwXTextFrame::queryInterface( const uno::Type& aType )
    throw (RuntimeException)
{
    uno::Any aRet = SwXText::queryInterface(aType);
    if(aRet.getValueType() == ::getCppuVoidType())
        aRet = SwXFrame::queryInterface(aType);
    if(aRet.getValueType() == ::getCppuVoidType())
        aRet = SwXTextFrameBaseClass::queryInterface(aType);
    return aRet;
}
/* -----------------------------15.03.00 16:30--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL SwXTextFrame::getTypes(  ) throw(RuntimeException)
{
    uno::Sequence< uno::Type > aTextFrameTypes = SwXTextFrameBaseClass::getTypes();
    uno::Sequence< uno::Type > aFrameTypes = SwXFrame::getTypes();
    uno::Sequence< uno::Type > aTextTypes = SwXText::getTypes();

    long nIndex = aTextFrameTypes.getLength();
    aTextFrameTypes.realloc(
        aTextFrameTypes.getLength() +
        aFrameTypes.getLength() +
        aTextTypes.getLength());

    uno::Type* pTextFrameTypes = aTextFrameTypes.getArray();
    const uno::Type* pFrameTypes = aFrameTypes.getConstArray();
    long nPos;
    for(nPos = 0; nPos <aFrameTypes.getLength(); nPos++)
        pTextFrameTypes[nIndex++] = pFrameTypes[nPos];

    const uno::Type* pTextTypes = aTextTypes.getConstArray();
    for(nPos = 0; nPos <aTextTypes.getLength(); nPos++)
        pTextFrameTypes[nIndex++] = pTextTypes[nPos];

    return aTextFrameTypes;
}
/* -----------------------------15.03.00 16:30--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< sal_Int8 > SAL_CALL SwXTextFrame::getImplementationId(  ) throw(RuntimeException)
{
    static uno::Sequence< sal_Int8 > aId( 16 );
    static BOOL bInit = FALSE;
    if(!bInit)
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    return aId;
}
/*-- 11.12.98 15:23:03---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XText >  SwXTextFrame::getText(void) throw( RuntimeException )
{
    return this;
}
/*-- 11.12.98 15:23:03---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XTextCursor >   SwXTextFrame::createCursor()
{
    return createTextCursor();
}
/*-- 11.12.98 15:23:03---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XTextCursor >  SwXTextFrame::createTextCursor(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< XTextCursor >  aRef;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwPaM aPam(pFmt->GetCntnt().GetCntntIdx()->GetNode());
        aPam.Move(fnMoveForward, fnGoNode);
        SwTableNode* pTblNode = aPam.GetNode()->FindTableNode();
        SwCntntNode* pCont = 0;
        while( pTblNode )
        {
            aPam.GetPoint()->nNode = *pTblNode->EndOfSectionNode();
            pCont = GetDoc()->GetNodes().GoNext(&aPam.GetPoint()->nNode);
            pTblNode = pCont->FindTableNode();
        }
        if(pCont)
            aPam.GetPoint()->nContent.Assign(pCont, 0);

        SwXTextCursor* pXCrsr = new SwXTextCursor(this, *aPam.GetPoint(), CURSOR_FRAME, pFmt->GetDoc());
        aRef =  (XWordCursor*)pXCrsr;
        SwUnoCrsr*  pUnoCrsr = pXCrsr->GetCrsr();
//      // no Cursor in protected sections
//      SwCrsrSaveState aSave( *pUnoCrsr );
//      if(pUnoCrsr->IsInProtectTable( sal_True ) ||
//          pUnoCrsr->IsSelOvr( SELOVER_TOGGLE | SELOVER_CHANGEPOS ))
//          throw  RuntimeException() );
    }
    else
        throw RuntimeException();
    return aRef;
}
/*-- 11.12.98 15:23:03---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XTextCursor >  SwXTextFrame::createTextCursorByRange(const uno::Reference< XTextRange > & aTextPosition) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< XTextCursor >  aRef;
    SwFrmFmt* pFmt = GetFrmFmt();
    SwUnoInternalPaM aPam(*GetDoc());
    if(pFmt && SwXTextRange::XTextRangeToSwPaM(aPam, aTextPosition))
    {
        SwNode& rNode = pFmt->GetCntnt().GetCntntIdx()->GetNode();
#ifdef DEBUG
        const SwStartNode* p1 = aPam.GetNode()->FindFlyStartNode();
        const SwStartNode* p2 = rNode.FindFlyStartNode();
#endif
        if(aPam.GetNode()->FindFlyStartNode() == rNode.FindFlyStartNode())
            aRef =  (XWordCursor*)new SwXTextCursor(this ,
                *aPam.GetPoint(), CURSOR_FRAME, pFmt->GetDoc(), aPam.GetMark());
    }
    else
        throw RuntimeException();
    return aRef;
}
/*-- 11.12.98 15:23:03---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< container::XEnumeration >  SwXTextFrame::createEnumeration(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< container::XEnumeration >  aRef;
    SwFrmFmt* pFmt = GetFrmFmt();
 uno::XInterface* pRet = 0;
    if(pFmt)
    {
        SwPosition aPos(pFmt->GetCntnt().GetCntntIdx()->GetNode());
        SwUnoCrsr* pUnoCrsr = GetDoc()->CreateUnoCrsr(aPos, sal_False);
        pUnoCrsr->Move( fnMoveForward, fnGoNode );
//      // no Cursor in protected sections
//      SwCrsrSaveState aSave( *pUnoCrsr );
//      if(pUnoCrsr->IsInProtectTable( sal_True ) ||
//          pUnoCrsr->IsSelOvr( SELOVER_TOGGLE | SELOVER_CHANGEPOS ))
//          throw  RuntimeException() );
        aRef = new SwXParagraphEnumeration(this, pUnoCrsr, CURSOR_FRAME);
    }
    return aRef;
}
/*-- 11.12.98 15:23:04---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Type  SwXTextFrame::getElementType(void) throw( RuntimeException )
{
    return ::getCppuType((uno::Reference<XTextRange>*)0);
}
/*-- 11.12.98 15:23:04---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextFrame::hasElements(void) throw( RuntimeException )
{
    return sal_True;
}
/*-- 11.12.98 15:23:04---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextFrame::attach(const uno::Reference< XTextRange > & xTextRange)
    throw( IllegalArgumentException, RuntimeException )
{

}
/*-- 11.12.98 15:23:04---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XTextRange >  SwXTextFrame::getAnchor(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return SwXFrame::getAnchor();
}
/*-- 11.12.98 15:23:05---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextFrame::dispose(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwXFrame::dispose();
}
/*-- 11.12.98 15:23:05---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextFrame::addEventListener(const uno::Reference< XEventListener > & aListener) throw( RuntimeException )
{
    SwXFrame::addEventListener(aListener);
}
/*-- 11.12.98 15:23:05---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextFrame::removeEventListener(const uno::Reference< XEventListener > & aListener) throw( RuntimeException )
{
    SwXFrame::removeEventListener(aListener);
}
/* -----------------03.05.99 12:28-------------------
 *
 * --------------------------------------------------*/
OUString SwXTextFrame::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTextFrame");
}
/* -----------------03.05.99 12:28-------------------
 *
 * --------------------------------------------------*/
sal_Bool SwXTextFrame::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return COMPARE_EQUAL == rServiceName.compareToAscii("com.sun.star.text.Text")||
            COMPARE_EQUAL == rServiceName.compareToAscii("com.sun.star.text.TextFrame")||
                COMPARE_EQUAL == rServiceName.compareToAscii("com.sun.star.text.TextContent") ||
                    COMPARE_EQUAL == rServiceName.compareToAscii("com.sun.star.document.LinkTarget");
}
/* -----------------03.05.99 12:28-------------------
 *
 * --------------------------------------------------*/
uno::Sequence< OUString > SwXTextFrame::getSupportedServiceNames(void) throw( RuntimeException )
{
    uno::Sequence < OUString > aRet(4);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.Frame");
    pArray[1] = C2U("com.sun.star.text.TextContent");
    pArray[2] = C2U("com.sun.star.text.Text");
    pArray[3] = C2U("com.sun.star.document.LinkTarget");

    return aRet;
}
/* -----------------------------20.06.00 10:02--------------------------------

 ---------------------------------------------------------------------------*/
void * SAL_CALL SwXTextFrame::operator new( size_t t) throw()
{
    return SwXTextFrameBaseClass::operator new( t);
}
/* -----------------------------20.06.00 10:02--------------------------------

 ---------------------------------------------------------------------------*/
void SAL_CALL SwXTextFrame::operator delete( void * p) throw()
{
    SwXTextFrameBaseClass::operator delete(p);
}
/******************************************************************
 *  SwXTextGraphicObject
 ******************************************************************/
/*-- 14.01.99 11:27:51---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextGraphicObject::SwXTextGraphicObject() :
    SwXFrame(FLYCNTTYPE_GRF, aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_GRAPHIC))
{
}
/*-- 11.12.98 16:02:25---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextGraphicObject::SwXTextGraphicObject(SwFrmFmt& rFmt) :
    SwXFrame(rFmt, FLYCNTTYPE_GRF, aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_GRAPHIC))
{

}
/*-- 11.12.98 16:02:26---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextGraphicObject::~SwXTextGraphicObject()
{

}
/* -----------------------------15.03.00 16:30--------------------------------

 ---------------------------------------------------------------------------*/
void SAL_CALL SwXTextGraphicObject::acquire(  )throw()
{
    SwXFrame::acquire();
}
/* -----------------------------15.03.00 16:30--------------------------------

 ---------------------------------------------------------------------------*/
void SAL_CALL SwXTextGraphicObject::release(  )throw()
{
    SwXFrame::release();
}
/* -----------------------------15.03.00 16:30--------------------------------

 ---------------------------------------------------------------------------*/
uno::Any SAL_CALL SwXTextGraphicObject::queryInterface( const uno::Type& aType )
    throw(RuntimeException)
{
    uno::Any aRet = SwXFrame::queryInterface(aType);
    if(aRet.getValueType() == ::getCppuVoidType())
        aRet = SwXTextGraphicObjectBaseClass::queryInterface(aType);
    return aRet;
}
/* -----------------------------15.03.00 16:30--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL
    SwXTextGraphicObject::getTypes(  ) throw(RuntimeException)
{
    uno::Sequence< uno::Type > aGraphicTypes = SwXTextGraphicObjectBaseClass::getTypes();
    uno::Sequence< uno::Type > aFrameTypes = SwXFrame::getTypes();

    long nIndex = aGraphicTypes.getLength();
    aGraphicTypes.realloc(
        aGraphicTypes.getLength() +
        aFrameTypes.getLength());

    uno::Type* pGraphicTypes = aGraphicTypes.getArray();
    const uno::Type* pFrameTypes = aFrameTypes.getConstArray();
    long nPos;
    for(nPos = 0; nPos <aFrameTypes.getLength(); nPos++)
        pGraphicTypes[nIndex++] = pFrameTypes[nPos];

    return aGraphicTypes;
}
/* -----------------------------15.03.00 16:30--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< sal_Int8 > SAL_CALL SwXTextGraphicObject::getImplementationId(  ) throw(RuntimeException)
{
    static uno::Sequence< sal_Int8 > aId( 16 );
    static BOOL bInit = FALSE;
    if(!bInit)
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    return aId;
}
/*-- 11.12.98 16:02:27---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextGraphicObject::attach(const uno::Reference< XTextRange > & xTextRange) throw( IllegalArgumentException, RuntimeException )
{
}
/*-- 11.12.98 16:02:27---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XTextRange >  SwXTextGraphicObject::getAnchor(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return SwXFrame::getAnchor();
}
/*-- 11.12.98 16:02:28---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextGraphicObject::dispose(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwXFrame::dispose();
}
/*-- 11.12.98 16:02:29---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextGraphicObject::addEventListener(const uno::Reference< XEventListener > & aListener)
                                                    throw( RuntimeException )
{
    SwXFrame::addEventListener(aListener);
}
/*-- 11.12.98 16:02:29---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextGraphicObject::removeEventListener(const uno::Reference< XEventListener > & aListener)
                                                    throw( RuntimeException )
{
    SwXFrame::removeEventListener(aListener);
}
/* -----------------03.05.99 12:28-------------------
 *
 * --------------------------------------------------*/
OUString SwXTextGraphicObject::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTextGraphicObject");
}
/* -----------------03.05.99 12:28-------------------
 *
 * --------------------------------------------------*/
sal_Bool SwXTextGraphicObject::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return COMPARE_EQUAL == rServiceName.compareToAscii("com.sun.star.text.TextGraphicObject") ||
            COMPARE_EQUAL == rServiceName.compareToAscii("com.sun.star.text.TextContent") ||
                COMPARE_EQUAL == rServiceName.compareToAscii("com.sun.star.document.LinkTarget");
}
/* -----------------03.05.99 12:28-------------------
 *
 * --------------------------------------------------*/
uno::Sequence< OUString > SwXTextGraphicObject::getSupportedServiceNames(void)
        throw( RuntimeException )
{
    uno::Sequence < OUString > aRet(3);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextContent");
    pArray[1] = C2U("com.sun.star.document.LinkTarget");
    pArray[2] = C2U("com.sun.star.text.GraphicObject");

    return aRet;
}
/* -----------------------------20.06.00 10:02--------------------------------

 ---------------------------------------------------------------------------*/
void * SAL_CALL SwXTextGraphicObject::operator new( size_t t) throw()
{
    return SwXTextGraphicObjectBaseClass::operator new(t);
}
/* -----------------------------20.06.00 10:02--------------------------------

 ---------------------------------------------------------------------------*/
void SAL_CALL SwXTextGraphicObject::operator delete( void * p) throw()
{
    SwXTextGraphicObjectBaseClass::operator delete(p);
}

/******************************************************************
 *
 ******************************************************************/
/*-- 11.12.98 16:16:53---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextEmbeddedObject::SwXTextEmbeddedObject() :
    SwXFrame(FLYCNTTYPE_OLE, aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_FRAME))
{

}
/*-- 11.12.98 16:16:53---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextEmbeddedObject::SwXTextEmbeddedObject(SwFrmFmt& rFmt) :
    SwXFrame(rFmt, FLYCNTTYPE_OLE, aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_FRAME))
{

}
/*-- 11.12.98 16:16:54---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextEmbeddedObject::~SwXTextEmbeddedObject()
{

}
/*-- 11.12.98 16:16:54---------------------------------------------------

  -----------------------------------------------------------------------*/
/* -----------------------------15.03.00 16:32--------------------------------

 ---------------------------------------------------------------------------*/
void SAL_CALL SwXTextEmbeddedObject::acquire()throw()
{
    SwXFrame::acquire();
}
/* -----------------------------15.03.00 16:32--------------------------------

 ---------------------------------------------------------------------------*/
void SAL_CALL SwXTextEmbeddedObject::release()throw()
{
    SwXFrame::release();
}
/* -----------------------------15.03.00 16:32--------------------------------

 ---------------------------------------------------------------------------*/
uno::Any SAL_CALL SwXTextEmbeddedObject::queryInterface( const uno::Type& aType )
    throw( RuntimeException)
{
    uno::Any aRet = SwXFrame::queryInterface(aType);;
    if(aRet.getValueType() == ::getCppuVoidType())
        aRet = SwXTextEmbeddedObjectBaseClass::queryInterface(aType);
    return aRet;
}
/* -----------------------------15.03.00 16:32--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< uno::Type > SAL_CALL SwXTextEmbeddedObject::getTypes(  ) throw(RuntimeException)
{
    uno::Sequence< uno::Type > aTextEmbeddedTypes = SwXTextEmbeddedObjectBaseClass::getTypes();
    uno::Sequence< uno::Type > aFrameTypes = SwXFrame::getTypes();

    long nIndex = aTextEmbeddedTypes.getLength();
    aTextEmbeddedTypes.realloc(
        aTextEmbeddedTypes.getLength() +
        aFrameTypes.getLength());

    uno::Type* pTextEmbeddedTypes = aTextEmbeddedTypes.getArray();

    const uno::Type* pFrameTypes = aFrameTypes.getConstArray();
    long nPos;
    for(nPos = 0; nPos <aFrameTypes.getLength(); nPos++)
        pTextEmbeddedTypes[nIndex++] = pFrameTypes[nPos];

    return aTextEmbeddedTypes;
}
/* -----------------------------15.03.00 16:32--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< sal_Int8 > SAL_CALL SwXTextEmbeddedObject::getImplementationId(  ) throw(RuntimeException)
{
    static uno::Sequence< sal_Int8 > aId( 16 );
    static BOOL bInit = FALSE;
    if(!bInit)
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    return aId;
}
/*-- 11.12.98 16:16:54---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextEmbeddedObject::attach(const uno::Reference< XTextRange > & xTextRange) throw( IllegalArgumentException, RuntimeException )
{
}
/*-- 11.12.98 16:16:54---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XTextRange >  SwXTextEmbeddedObject::getAnchor(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return SwXFrame::getAnchor();
}
/*-- 11.12.98 16:16:54---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextEmbeddedObject::dispose(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwXFrame::dispose();
}
/*-- 11.12.98 16:16:55---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextEmbeddedObject::addEventListener(const uno::Reference< XEventListener > & aListener) throw( RuntimeException )
{
    SwXFrame::addEventListener(aListener);
}
/*-- 11.12.98 16:16:55---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextEmbeddedObject::removeEventListener(const uno::Reference< XEventListener > & aListener) throw( RuntimeException )
{
    SwXFrame::removeEventListener(aListener);
}
/*-- 11.12.98 16:16:55---------------------------------------------------

  -----------------------------------------------------------------------*/

static uno::Reference< util::XModifyListener >  xSwXOLEListener;

uno::Reference< XComponent >  SwXTextEmbeddedObject::getEmbeddedObject(void) throw( RuntimeException )
{
    uno::Reference< XComponent >  xRet;
    SwFrmFmt*   pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwDoc* pDoc = pFmt->GetDoc();
        const SwFmtCntnt* pCnt = &pFmt->GetCntnt();
        DBG_ASSERT( pCnt->GetCntntIdx() &&
                       pDoc->GetNodes()[ pCnt->GetCntntIdx()->
                                        GetIndex() + 1 ]->GetOLENode(), "kein OLE-Node?")

        SwOLENode* pOleNode =  pDoc->GetNodes()[ pCnt->GetCntntIdx()
                                        ->GetIndex() + 1 ]->GetOLENode();
        SvInPlaceObjectRef xIP( pOleNode->GetOLEObj().GetOleRef() );
        if (xIP.Is())
        {
            SfxInPlaceObjectRef xSfxObj( xIP );
            if(xSfxObj.Is())
            {
                SfxObjectShell* pObjSh = xSfxObj->GetObjectShell();
                if( pObjSh )
                {
                    uno::Reference< frame::XModel > xModel = pObjSh->GetBaseModel();
                    xRet = uno::Reference< XComponent >(xModel, uno::UNO_QUERY);

                    uno::Reference< util::XModifyBroadcaster >  xBrdcst(xModel, uno::UNO_QUERY);
                    if( xBrdcst.is() )
                    {
                        SwXOLEListener* pSwOLEListener = 0;
                        if( !xSwXOLEListener.is() )
                            xSwXOLEListener = pSwOLEListener = new SwXOLEListener;
                        if( pSwOLEListener->AddOLEFmt( *pFmt ) )
                            xBrdcst->addModifyListener( xSwXOLEListener );
                    }
                }
            }
        }
    }
    return xRet;
}
/* -----------------03.05.99 12:28-------------------
 *
 * --------------------------------------------------*/
OUString SwXTextEmbeddedObject::getImplementationName(void) throw( RuntimeException )

{
    return C2U("SwXTextEmbeddedObject");
}
/* -----------------03.05.99 12:28-------------------
 *
 * --------------------------------------------------*/
sal_Bool SwXTextEmbeddedObject::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return  COMPARE_EQUAL == rServiceName.compareToAscii("com.sun.star.text.TextEmbeddedObject")||
            COMPARE_EQUAL == rServiceName.compareToAscii("com.sun.star.text.TextContent")||
            COMPARE_EQUAL == rServiceName.compareToAscii("com.sun.star.document.LinkTarget");
}
/* -----------------03.05.99 12:28-------------------
 *
 * --------------------------------------------------*/
uno::Sequence< OUString > SwXTextEmbeddedObject::getSupportedServiceNames(void)
        throw( RuntimeException )
{
    uno::Sequence < OUString > aRet(3);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextContent");
    pArray[1] = C2U("com.sun.star.document.LinkTarget");
    pArray[2] = C2U("com.sun.star.text.TextEmbeddedObject");

    return aRet;
}
/* -----------------------------20.06.00 10:02--------------------------------

 ---------------------------------------------------------------------------*/
void * SAL_CALL SwXTextEmbeddedObject::operator new( size_t t) throw()
{
    return SwXTextEmbeddedObjectBaseClass::operator new(t);
}
/* -----------------------------20.06.00 10:02--------------------------------

 ---------------------------------------------------------------------------*/
void SAL_CALL SwXTextEmbeddedObject::operator delete( void * p) throw()
{
    SwXTextEmbeddedObjectBaseClass::operator delete(p);
}


/******************************************************************
 *
 ******************************************************************/

void SwXOLEListener::modified( const EventObject& rEvent )
                                        throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    SwOLENode* pNd;
    sal_uInt16 nFndPos = FindEntry( rEvent, &pNd );
    if( USHRT_MAX != nFndPos && ( !pNd->GetOLEObj().IsOleRef() ||
            !pNd->GetOLEObj().GetOleRef()->GetProtocol().IsInPlaceActive() ))
    {
        // if the OLE-Node is UI-Active do nothing
        pNd->SetOLESizeInvalid( sal_True );
        pNd->GetDoc()->SetOLEObjModified();
    }
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
void SwXOLEListener::disposing( const EventObject& rEvent )
                        throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    uno::Reference< util::XModifyListener >  xListener( this );

    SwOLENode* pNd;
    sal_uInt16 nFndPos = FindEntry( rEvent, &pNd );
    if( USHRT_MAX != nFndPos )
    {
        SwDepend* pDepend = (SwDepend*)aFmts[ nFndPos ];
        aFmts.Remove( nFndPos, 1 );

        uno::Reference< frame::XModel >  xModel( rEvent.Source, uno::UNO_QUERY );
        uno::Reference< util::XModifyBroadcaster >  xBrdcst(xModel, uno::UNO_QUERY);

        if( xBrdcst.is() )
            xBrdcst->removeModifyListener( xListener );

        delete pDepend;
        if( !aFmts.Count() )
        {
            // we are the last?
            // then can we delete us
            xSwXOLEListener = 0;
        }
    }
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SwXOLEListener::AddOLEFmt( SwFrmFmt& rFmt )
{
    for( sal_uInt16 n = 0, nCnt = aFmts.Count(); n < nCnt; ++n )
        if( &rFmt == ((SwDepend*)aFmts[ n ])->GetRegisteredIn() )
            return sal_False;       // is in the array

    SwDepend* pNew = new SwDepend( this, &rFmt );
    aFmts.Insert( pNew, aFmts.Count() );
    return sal_True;
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
void SwXOLEListener::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
{
    const SwClient* pClient = 0;

    switch( pOld ? pOld->Which() : 0 )
    {
    case RES_REMOVE_UNO_OBJECT:
    case RES_OBJECTDYING:
        pClient = (SwClient*)((SwPtrMsgPoolItem *)pOld)->pObject;
        break;

    case RES_FMT_CHG:
        // wurden wir an das neue umgehaengt und wird das alte geloscht?
        if( ((SwFmtChg*)pOld)->pChangedFmt->IsFmtInDTOR() )
        {
            pClient = ((SwFmtChg*)pNew)->pChangedFmt;
        }
        break;
    }

    if( pClient )
    {
        uno::Reference< util::XModifyListener >  xListener( this );

        SwDepend* pDepend;
        for( sal_uInt16 n = 0, nCnt = aFmts.Count(); n < nCnt; ++n )
        {
            if( pClient == (pDepend = (SwDepend*)aFmts[ n ])->GetRegisteredIn() )
            {
                aFmts.Remove( n, 1 );

                 uno::Reference<frame::XModel> xModel = GetModel( *(SwFmt*)pClient );
                if( xModel.is() )
                {
                    uno::Reference< util::XModifyBroadcaster >  xBrdcst(xModel, uno::UNO_QUERY);
                    if( xBrdcst.is() )
                        xBrdcst->removeModifyListener( xListener );
                }

                delete pDepend;
                if( !aFmts.Count() )
                {
                    // we are the last?
                    // then can we delete us
                    xSwXOLEListener = 0;
                }
                break;
            }
        }
    }
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
uno::Reference< frame::XModel > SwXOLEListener::GetModel( const SwFmt& rFmt, SwOLENode** ppNd ) const
{
    SfxObjectShell* pObjSh = GetObjShell( rFmt, ppNd );
    return pObjSh ? pObjSh->GetBaseModel() : (uno::Reference< frame::XModel >)0;
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
SfxObjectShell* SwXOLEListener::GetObjShell( const SwFmt& rFmt,
                                            SwOLENode** ppNd ) const
{
    SfxObjectShell* pShell = 0;
    const SwFmtCntnt& rCnt = rFmt.GetCntnt();
    if( rCnt.GetCntntIdx() )
    {
        SwNodeIndex aIdx( *rCnt.GetCntntIdx(), 1 );
        SwOLENode* pOleNode = aIdx.GetNode().GetOLENode();
        if( pOleNode && pOleNode->GetOLEObj().IsOleRef() )
        {
            SfxInPlaceObjectRef xIP( pOleNode->GetOLEObj().GetOleRef() );
            if( xIP.Is() )
                pShell = xIP->GetObjectShell();
        }
        if( ppNd )
            *ppNd = pOleNode;
    }
    return pShell;
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
sal_uInt16 SwXOLEListener::FindEntry( const EventObject& rEvent,SwOLENode** ppNd)
{
    sal_uInt16 nRet = USHRT_MAX;
    uno::Reference< frame::XModel >  xSrch( rEvent.Source, uno::UNO_QUERY );

    for( sal_uInt16 n = 0, nCnt = aFmts.Count(); n < nCnt; ++n )
    {
        SwDepend* pDepend = (SwDepend*)aFmts[ n ];
        SwFrmFmt* pFmt = (SwFrmFmt*)pDepend->GetRegisteredIn();
        if( !pFmt )
        {
            ASSERT( pFmt, "wo ist das Format geblieben?" );
            aFmts.Remove( n, 1 );
            delete pDepend;
            --n;
            --nCnt;
        }
        else
        {
            uno::Reference< frame::XModel >  xFmt( GetModel( *pFmt, ppNd ), uno::UNO_QUERY);
            if( xFmt == xSrch )
            {
                nRet = n;
                break;
            }
        }
    }
    return nRet;
}

