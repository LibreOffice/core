/*************************************************************************
 *
 *  $RCSfile: attrdesc.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-16 21:30:40 $
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
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen wg. Application
#include <vcl/svapp.hxx>
#endif
#ifndef _SVX_ITEMTYPE_HXX
#include <svx/itemtype.hxx>
#endif

#ifndef _GRFMGR_HXX
#include <goodies/grfmgr.hxx>
#endif

#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _FCHRFMT_HXX //autogen
#include <fchrfmt.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTTSPLT_HXX //autogen
#include <fmtlsplt.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _FMTEIRO_HXX //autogen
#include <fmteiro.hxx>
#endif
#ifndef _FMTURL_HXX //autogen
#include <fmturl.hxx>
#endif
#ifndef _FMTHDFT_HXX //autogen
#include <fmthdft.hxx>
#endif
#ifndef _FMTCNCT_HXX //autogen
#include <fmtcnct.hxx>
#endif
#ifndef _FMTLINE_HXX
#include <fmtline.hxx>
#endif
#ifndef _FMTRUBY_HXX
#include <fmtruby.hxx>
#endif
#ifndef _SWATRSET_HXX
#include <swatrset.hxx>
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _GRFATR_HXX
#include <grfatr.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _CHARFMT_HXX
#include <charfmt.hxx>
#endif
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>
#endif
#ifndef _TOX_HXX
#include <tox.hxx>
#endif
#ifndef _ATTRDESC_HRC
#include <attrdesc.hrc>
#endif
#ifndef _FMTFTNTX_HXX //autogen
#include <fmtftntx.hxx>
#endif


TYPEINIT2(SwFmtCharFmt,SfxPoolItem,SwClient);

// erfrage die Attribut-Beschreibung
void SwAttrSet::GetPresentation(
        SfxItemPresentation ePres,
        SfxMapUnit eCoreMetric,
        SfxMapUnit ePresMetric,
        String &rText ) const
{
static sal_Char __READONLY_DATA sKomma[] = ", ";

    rText.Erase();
    String aStr;
    if( Count() )
    {
        SfxItemIter aIter( *this );
        const International& rInt = Application::GetAppInternational();
        while( TRUE )
        {
#if SUPD>546
            aIter.GetCurItem()->GetPresentation( ePres, eCoreMetric,
                                                 ePresMetric, aStr,
                                                 &rInt );
#else
            aIter.GetCurItem()->GetPresentation( ePres, eCoreMetric,
                                                 ePresMetric, aStr );
#endif
            if( rText.Len() && aStr.Len() )
                rText += String::CreateFromAscii(sKomma);
            rText += aStr;
            if( aIter.IsAtEnd() )
                break;
            aIter.NextItem();
        }
    }
}


void SwPageDesc::GetPresentation(
        SfxItemPresentation ePres,
        SfxMapUnit eCoreMetric,
        SfxMapUnit ePresMetric,
        String &rText ) const
{
    rText = GetName();
}


// ATT_CHARFMT *********************************************


SfxItemPresentation SwFmtCharFmt::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    String&             rText,
    const International*        pIntl
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            break;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            const SwCharFmt *pCharFmt = GetCharFmt();
            if ( pCharFmt )
            {
                String aStr;
                rText = SW_RESSTR(STR_CHARFMT );
                pCharFmt->GetPresentation( ePres, eCoreUnit, ePresUnit, aStr );
                rText += '(';
                rText += aStr;
                rText += ')';
            }
            else
                rText = SW_RESSTR( STR_NO_CHARFMT );
            return ePres;
        }
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// ATT_INETFMT *********************************************


SfxItemPresentation SwFmtINetFmt::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    String&             rText,
    const International*        pIntl
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            break;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            rText = GetValue();
            return ePres;
        }
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

SfxItemPresentation SwFmtRuby::GetPresentation( SfxItemPresentation ePres,
                            SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric,
                            String &rText, const International* pIntl ) const
{
    switch ( ePres )
    {
    case SFX_ITEM_PRESENTATION_NONE:
        rText.Erase();
        break;
    case SFX_ITEM_PRESENTATION_NAMELESS:
    case SFX_ITEM_PRESENTATION_COMPLETE:
    {
        rText = aEmptyStr;
        return ePres;
    }
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

/*************************************************************************
|*    class     SwFmtDrop
*************************************************************************/


SfxItemPresentation SwFmtDrop::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    String&             rText,
    const International*        pIntl
)   const
{
    rText.Erase();
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            break;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            if ( GetLines() > 1 )
            {
                if ( GetChars() > 1 )
                {
                    rText = String::CreateFromInt32( GetChars() );
                    rText += ' ';
                }
                rText += SW_RESSTR( STR_DROP_OVER );
                rText += ' ';
                rText += String::CreateFromInt32( GetLines() );
                rText += ' ';
                rText += SW_RESSTR( STR_DROP_LINES );
            }
            else
                rText = SW_RESSTR( STR_NO_DROP_LINES );
            return ePres;
        }
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

/*************************************************************************
|*    class     SwRegisterItem
*************************************************************************/


SfxItemPresentation SwRegisterItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText,
    const International*        pIntl
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return SFX_ITEM_PRESENTATION_NONE;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            USHORT nId = GetValue() ? STR_REGISTER_ON : STR_REGISTER_OFF;
            rText = SW_RESSTR( nId );
            return ePres;
        }
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

/*************************************************************************
|*    class     SwNumRuleItem
*************************************************************************/


SfxItemPresentation SwNumRuleItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText,
    const International*        pIntl
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return SFX_ITEM_PRESENTATION_NONE;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            if( GetValue().Len() )
                (((rText = SW_RESSTR( STR_NUMRULE_ON )) +=
                    '(' ) += GetValue() ) += ')';
            else
                rText = SW_RESSTR( STR_NUMRULE_OFF );
            return ePres;
        }
    }
    return SFX_ITEM_PRESENTATION_NONE;
}


/******************************************************************************
 *  Frame-Attribute:
 ******************************************************************************/


SfxItemPresentation SwFmtFrmSize::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    String&             rText,
    const International*        pIntl
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
        {
            rText.Erase();
            break;
        }
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            rText = SW_RESSTR( STR_FRM_WIDTH );
            rText += ' ';
            if ( GetWidthPercent() )
            {
                rText += String::CreateFromInt32(GetWidthPercent());
                rText += '%';
            }
            else
            {
                rText += ::GetMetricText( GetWidth(), eCoreUnit, ePresUnit );
                rText += ::GetSvxString( ::GetMetricId( ePresUnit ) );
            }
            if ( ATT_VAR_SIZE != GetSizeType() )
            {
                rText += ',';
                rText += ' ';
                const USHORT nId = ATT_FIX_SIZE == eFrmSize ?
                                        STR_FRM_FIXEDHEIGHT : STR_FRM_MINHEIGHT;
                rText += SW_RESSTR( nId );
                rText += ' ';
                if ( GetHeightPercent() )
                {
                    rText += String::CreateFromInt32(GetHeightPercent());
                    rText += '%';
                }
                else
                {
                    rText += ::GetMetricText( GetHeight(), eCoreUnit, ePresUnit );
                    rText += ::GetSvxString( ::GetMetricId( ePresUnit ) );
                }
            }
            return ePres;
        }
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

//Kopfzeile, fuer Seitenformate
//Client von FrmFmt das den Header beschreibt.


SfxItemPresentation SwFmtHeader::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    String&             rText,
    const International*        pIntl
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            break;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            const USHORT nId = GetHeaderFmt() ? STR_HEADER : STR_NO_HEADER;
            rText = SW_RESSTR( nId );
            return ePres;
        }
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

//Fusszeile, fuer Seitenformate
//Client von FrmFmt das den Footer beschreibt.


SfxItemPresentation SwFmtFooter::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    String&             rText,
    const International*        pIntl
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            break;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            const USHORT nId = GetFooterFmt() ? STR_FOOTER : STR_NO_FOOTER;
            rText = SW_RESSTR( nId );
            return ePres;
        }
    }
    return SFX_ITEM_PRESENTATION_NONE;
}


SfxItemPresentation SwFmtSurround::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    String&             rText,
    const International*        pIntl
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            break;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            USHORT nId = 0;
            switch ( (SwSurround)GetValue() )
            {
                case SURROUND_NONE:
                    nId = STR_SURROUND_NONE;
                break;
                case SURROUND_THROUGHT:
                    nId = STR_SURROUND_THROUGHT;
                break;
                case SURROUND_PARALLEL:
                    nId = STR_SURROUND_PARALLEL;
                break;
                case SURROUND_IDEAL:
                    nId = STR_SURROUND_IDEAL;
                break;
                case SURROUND_LEFT:
                    nId = STR_SURROUND_LEFT;
                break;
                case SURROUND_RIGHT:
                    nId = STR_SURROUND_RIGHT;
                break;
            }
            if ( nId )
                rText = SW_RESSTR( nId );

            if ( IsAnchorOnly() )
            {
                rText += ' ';
                rText += SW_RESSTR( STR_SURROUND_ANCHORONLY );
            }
            return ePres;
        }
    }
    return SFX_ITEM_PRESENTATION_NONE;
}


//VertOrientation, wie und woran orientiert --
//  sich der FlyFrm in der Vertikalen -----------


SfxItemPresentation SwFmtVertOrient::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    String&             rText,
    const International*        pIntl
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            break;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            USHORT nId = 0;
            switch ( GetVertOrient() )
            {
                case VERT_NONE:
                {
                    rText += SW_RESSTR( STR_POS_Y );
                    rText += ' ';
                    rText += ::GetMetricText( GetPos(), eCoreUnit, ePresUnit );
                    rText += ::GetSvxString( ::GetMetricId( ePresUnit ) );
                }
                break;
                case VERT_TOP:
                    nId = STR_VERT_TOP;
                    break;
                case VERT_CENTER:
                    nId = STR_VERT_CENTER;
                    break;
                case VERT_BOTTOM:
                    nId = STR_VERT_BOTTOM;
                    break;
                case VERT_LINE_TOP:
                    nId = STR_LINE_TOP;
                    break;
                case VERT_LINE_CENTER:
                    nId = STR_LINE_CENTER;
                    break;
                case VERT_LINE_BOTTOM:
                    nId = STR_LINE_BOTTOM;
                    break;
            }
            if ( nId )
                rText += SW_RESSTR( nId );
            return ePres;
        }
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

//HoriOrientation, wie und woran orientiert --
//  sich der FlyFrm in der Hoizontalen ----------


SfxItemPresentation SwFmtHoriOrient::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    String&             rText,
    const International*        pIntl
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            break;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            USHORT nId = 0;
            switch ( GetHoriOrient() )
            {
                case HORI_NONE:
                {
                    rText += SW_RESSTR( STR_POS_X );
                    rText += ' ';
                    rText += ::GetMetricText( GetPos(), eCoreUnit, ePresUnit );
                    rText += ::GetSvxString( ::GetMetricId( ePresUnit ) );
                }
                break;
                case HORI_RIGHT:
                    nId = STR_HORI_RIGHT;
                break;
                case HORI_CENTER:
                    nId = STR_HORI_CENTER;
                break;
                case HORI_LEFT:
                    nId = STR_HORI_LEFT;
                break;
                case HORI_INSIDE:
                    nId = STR_HORI_INSIDE;
                break;
                case HORI_OUTSIDE:
                    nId = STR_HORI_OUTSIDE;
                break;
                case HORI_FULL:
                    nId = STR_HORI_FULL;
                break;
            }
            if ( nId )
                rText += SW_RESSTR( nId );
            return ePres;
        }
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

//FlyAnchor, Anker des Freifliegenden Rahmen ----


SfxItemPresentation SwFmtAnchor::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    String&             rText,
    const International*        pIntl
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            break;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            USHORT nId = 0;
            switch ( GetAnchorId() )
            {
                case FLY_AT_CNTNT:      //Absatzgebundener Rahmen
                    nId = STR_FLY_AT_CNTNT;
                    break;
                case FLY_IN_CNTNT:      //Zeichengebundener Rahmen
                    nId = STR_FLY_IN_CNTNT;
                    break;
                case FLY_PAGE:          //Seitengebundener Rahmen
                    nId = STR_FLY_PAGE;
                    break;
            }
            if ( nId )
                rText += SW_RESSTR( nId );
            return ePres;
        }
    }
    return SFX_ITEM_PRESENTATION_NONE;
}


SfxItemPresentation SwFmtPageDesc::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    String&             rText,
    const International*        pIntl
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            break;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            const SwPageDesc *pPageDesc = GetPageDesc();
            if ( pPageDesc )
                rText = pPageDesc->GetName();
            else
                rText = SW_RESSTR( STR_NO_PAGEDESC );
            return ePres;
        }
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

//Der ColumnDescriptor --------------------------


SfxItemPresentation SwFmtCol::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    String&             rText,
    const International*        pIntl
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            break;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            USHORT nCnt = GetNumCols();
            if ( nCnt > 1 )
            {
                rText = String::CreateFromInt32(nCnt);
                rText += ' ';
                rText += SW_RESSTR( STR_COLUMNS );
                if ( COLADJ_NONE != GetLineAdj() )
                {
                    USHORT nWdth = USHORT(GetLineWidth());
                    rText += ' ';
                    rText += SW_RESSTR( STR_LINE_WIDTH );
                    rText += ' ';
                    rText += ::GetMetricText( nWdth, eCoreUnit,
                                              SFX_MAPUNIT_POINT );
                }
            }
            else
                rText.Erase();
            return ePres;
        }
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

//URL's und Maps


SfxItemPresentation SwFmtURL::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    String&             rText,
    const International*        pIntl
)   const
{
    rText.Erase();
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            break;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            if ( pMap )
                rText.AppendAscii( RTL_CONSTASCII_STRINGPARAM("Client-Map"));
            if ( sURL.Len() )
            {
                if ( pMap )
                    rText.AppendAscii( RTL_CONSTASCII_STRINGPARAM(" - "));
                rText.AppendAscii( RTL_CONSTASCII_STRINGPARAM("URL: "));
                rText += sURL;
                if ( bIsServerMap )
                    rText.AppendAscii( RTL_CONSTASCII_STRINGPARAM(" (Server-Map)"));
            }
            if ( sTargetFrameName.Len() )
            {
                rText.AppendAscii( RTL_CONSTASCII_STRINGPARAM(", Target: "));
                rText += sTargetFrameName;
            }
            return ePres;
        }
    }
    return SFX_ITEM_PRESENTATION_NONE;
}


//SwFmtEditInReadonly


SfxItemPresentation SwFmtEditInReadonly::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    String&             rText,
    const International*        pIntl
)   const
{
    rText.Erase();
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            break;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            if ( GetValue() )
                rText = SW_RESSTR(STR_EDIT_IN_READONLY);
            return ePres;
        }
    }
    return SFX_ITEM_PRESENTATION_NONE;
}


SfxItemPresentation SwFmtLayoutSplit::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    String&             rText,
    const International*        pIntl
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return SFX_ITEM_PRESENTATION_NONE;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            if ( GetValue() )
                rText = SW_RESSTR(STR_LAYOUT_SPLIT);
            return ePres;
        }
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

SfxItemPresentation SwFmtFtnEndAtTxtEnd::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    String&             rText,
    const International*        pIntl
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            break;

        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            {
                switch( GetValue() )
                {
                case FTNEND_ATPGORDOCEND:
//                  rText = SW_RESSTR( STR_LAYOUT_FTN );
                    break;

                case FTNEND_ATTXTEND:
                    break;

                case FTNEND_ATTXTEND_OWNNUMSEQ:
                    {
//  String      sPrefix;
//  String      sSuffix;
//  SwNumType   aFmt;
//  USHORT      nOffset;
//                      rText +=
                    }
                    break;
                }
            }
            break;

        default:
            ePres = SFX_ITEM_PRESENTATION_NONE;
            break;
    }
    return ePres;
}

SfxItemPresentation SwFmtChain::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    String&             rText,
    const International*        pIntl
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return SFX_ITEM_PRESENTATION_NONE;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            if ( GetPrev() || GetNext() )
            {
                rText = SW_RESSTR(STR_CONNECT1);
                if ( GetPrev() )
                {
                    rText += GetPrev()->GetName();
                    if ( GetNext() )
                        rText += SW_RESSTR(STR_CONNECT2);
                }
                if ( GetNext() )
                    rText += GetNext()->GetName();
            }
            return ePres;
        }
    }
    return SFX_ITEM_PRESENTATION_NONE;
}


SfxItemPresentation SwFmtLineNumber::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    String&             rText,
    const International*    pIntl
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return SFX_ITEM_PRESENTATION_NONE;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            if ( IsCount() )
                rText += SW_RESSTR(STR_LINECOUNT);
            else
                rText += SW_RESSTR(STR_DONTLINECOUNT);
            if ( GetStartValue() )
            {
                rText += ' ';
                rText += SW_RESSTR(STR_LINCOUNT_START);
                rText += String::CreateFromInt32(GetStartValue());
            }
            return ePres;
        }
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// ---------------------- Grafik-Attribute --------------------------

SfxItemPresentation SwMirrorGrf::GetPresentation(
    SfxItemPresentation ePres, SfxMapUnit eCoreUnit, SfxMapUnit ePresUnit,
    String& rText, const International* pIntl ) const
{
    switch ( ePres )
    {
    case SFX_ITEM_PRESENTATION_NAMELESS:
    case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            USHORT nId;
            switch( GetValue() )
            {
            case RES_DONT_MIRROR_GRF:   nId = STR_NO_MIRROR;    break;
            case RES_MIRROR_GRF_VERT:   nId = STR_VERT_MIRROR;  break;
            case RES_MIRROR_GRF_HOR:    nId = STR_HORI_MIRROR;  break;
            case RES_MIRROR_GRF_BOTH:   nId = STR_BOTH_MIRROR;  break;
            default:                    nId = 0;    break;
            }
            if ( nId )
            {
                rText = SW_RESSTR( nId );
                if (bGrfToggle)
                    rText += SW_RESSTR( STR_MIRROR_TOGGLE );
            }
        }
        break;
    default:
        ePres = SFX_ITEM_PRESENTATION_NONE;
        rText.Erase();
        break;
    }
    return ePres;
}


SfxItemPresentation SwRotationGrf::GetPresentation(
    SfxItemPresentation ePres, SfxMapUnit eCoreUnit, SfxMapUnit ePresUnit,
    String &rText, const International* pIntl) const
{
    switch( ePres )
    {
    case SFX_ITEM_PRESENTATION_NAMELESS:
    case SFX_ITEM_PRESENTATION_COMPLETE:
        if( SFX_ITEM_PRESENTATION_COMPLETE == ePres )
            rText = SW_RESSTR( STR_ROTATION );
        else if( rText.Len() )
            rText.Erase();
        ( rText += UniString::CreateFromInt32( GetValue() )) += '°';
        break;

    default:
        ePres = SFX_ITEM_PRESENTATION_NONE;
        rText.Erase();
        break;
    }
    return ePres;
}

SfxItemPresentation SwLuminanceGrf::GetPresentation(
    SfxItemPresentation ePres, SfxMapUnit eCoreUnit, SfxMapUnit ePresUnit,
    String &rText, const International* pIntl) const
{
    switch( ePres )
    {
    case SFX_ITEM_PRESENTATION_NAMELESS:
    case SFX_ITEM_PRESENTATION_COMPLETE:
        if( SFX_ITEM_PRESENTATION_COMPLETE == ePres )
            rText = SW_RESSTR( STR_LUMINANCE );
        else if( rText.Len() )
            rText.Erase();
        ( rText += UniString::CreateFromInt32( GetValue() )) += '%';
        break;

    default:
        ePres = SFX_ITEM_PRESENTATION_NONE;
        rText.Erase();
        break;
    }
    return ePres;
}

SfxItemPresentation SwContrastGrf::GetPresentation(
    SfxItemPresentation ePres, SfxMapUnit eCoreUnit, SfxMapUnit ePresUnit,
    String &rText, const International* pIntl) const
{
    switch( ePres )
    {
    case SFX_ITEM_PRESENTATION_NAMELESS:
    case SFX_ITEM_PRESENTATION_COMPLETE:
        if( SFX_ITEM_PRESENTATION_COMPLETE == ePres )
            rText = SW_RESSTR( STR_CONTRAST );
        else if( rText.Len() )
            rText.Erase();
        ( rText += UniString::CreateFromInt32( GetValue() )) += '%';
        break;

    default:
        ePres = SFX_ITEM_PRESENTATION_NONE;
        rText.Erase();
        break;
    }
    return ePres;
}

SfxItemPresentation SwChannelGrf::GetPresentation(
    SfxItemPresentation ePres, SfxMapUnit eCoreUnit, SfxMapUnit ePresUnit,
    String &rText, const International* pIntl) const
{
    switch( ePres )
    {
    case SFX_ITEM_PRESENTATION_NAMELESS:
    case SFX_ITEM_PRESENTATION_COMPLETE:
        if( SFX_ITEM_PRESENTATION_COMPLETE == ePres )
        {
            USHORT nId;
            switch ( Which() )
            {
            case RES_GRFATR_CHANNELR:   nId = STR_CHANNELR; break;
            case RES_GRFATR_CHANNELG:   nId = STR_CHANNELG; break;
            case RES_GRFATR_CHANNELB:   nId = STR_CHANNELB; break;
            default:                    nId = 0; break;
            }
            if( nId )
                rText = SW_RESSTR( nId );
            else if( rText.Len() )
                rText.Erase();
        }
        else if( rText.Len() )
            rText.Erase();
        ( rText += UniString::CreateFromInt32( GetValue() )) += '%';
        break;

    default:
        ePres = SFX_ITEM_PRESENTATION_NONE;
        rText.Erase();
        break;
    }
    return ePres;
}

SfxItemPresentation SwGammaGrf::GetPresentation(
    SfxItemPresentation ePres, SfxMapUnit eCoreUnit, SfxMapUnit ePresUnit,
    String &rText, const International* pIntl) const
{
    switch( ePres )
    {
    case SFX_ITEM_PRESENTATION_NAMELESS:
    case SFX_ITEM_PRESENTATION_COMPLETE:
        if( SFX_ITEM_PRESENTATION_COMPLETE == ePres )
            rText = SW_RESSTR( STR_GAMMA );
        else if( rText.Len() )
            rText.Erase();
        ( rText += UniString::CreateFromDouble( GetValue() )) += '%';
        break;

    default:
        ePres = SFX_ITEM_PRESENTATION_NONE;
        rText.Erase();
        break;
    }
    return ePres;
}

SfxItemPresentation SwInvertGrf::GetPresentation(
    SfxItemPresentation ePres, SfxMapUnit eCoreUnit, SfxMapUnit ePresUnit,
    String &rText, const International* pIntl) const
{
    rText.Erase();
    switch( ePres )
    {
    case SFX_ITEM_PRESENTATION_NAMELESS:
    case SFX_ITEM_PRESENTATION_COMPLETE:
        if( SFX_ITEM_PRESENTATION_COMPLETE == ePres )
        {
            USHORT nId = 0 != GetValue() ? STR_INVERT : STR_INVERT_NOT;
            rText = SW_RESSTR( nId );
        }
        break;

    default:
        ePres = SFX_ITEM_PRESENTATION_NONE;
        break;
    }
    return ePres;
}

SfxItemPresentation SwTransparencyGrf::GetPresentation(
    SfxItemPresentation ePres, SfxMapUnit eCoreUnit, SfxMapUnit ePresUnit,
    String &rText, const International* pIntl) const
{
    switch( ePres )
    {
    case SFX_ITEM_PRESENTATION_NAMELESS:
    case SFX_ITEM_PRESENTATION_COMPLETE:
        if( SFX_ITEM_PRESENTATION_COMPLETE == ePres )
            rText = SW_RESSTR( STR_TRANSPARENCY );
        else if( rText.Len() )
            rText.Erase();
        ( rText += UniString::CreateFromInt32( GetValue() )) += '%';
        break;

    default:
        ePres = SFX_ITEM_PRESENTATION_NONE;
        rText.Erase();
        break;
    }
    return ePres;
}

SfxItemPresentation SwDrawModeGrf::GetPresentation(
    SfxItemPresentation ePres, SfxMapUnit eCoreUnit, SfxMapUnit ePresUnit,
    String &rText, const International* pIntl) const
{
    rText.Erase();
    switch( ePres )
    {
    case SFX_ITEM_PRESENTATION_NAMELESS:
    case SFX_ITEM_PRESENTATION_COMPLETE:
        if( SFX_ITEM_PRESENTATION_COMPLETE == ePres )
        {
            USHORT nId;
            switch ( GetValue() )
            {

            case GRAPHICDRAWMODE_GREYS:     nId = STR_DRAWMODE_GREY; break;
            case GRAPHICDRAWMODE_MONO:      nId = STR_DRAWMODE_BLACKWHITE; break;
            case GRAPHICDRAWMODE_WATERMARK: nId = STR_DRAWMODE_WATERMARK; break;
            default:                        nId = STR_DRAWMODE_STD; break;
            }
            (rText = SW_RESSTR( STR_DRAWMODE ) ) += SW_RESSTR( nId );
        }
        break;

    default:
        ePres = SFX_ITEM_PRESENTATION_NONE;
        break;
    }
    return ePres;
}

/*************************************************************************

      $Log: not supported by cvs2svn $
      Revision 1.2  2000/10/23 11:58:25  jp
      new attributes Ruby and 2Lines

      Revision 1.1.1.1  2000/09/18 17:14:49  hr
      initial import

      Revision 1.48  2000/09/18 16:06:16  willem.vandorp
      OpenOffice header added.

      Revision 1.47  2000/08/25 12:31:48  jp
      Graphic Crop-Attribut exported to SVX

      Revision 1.46  2000/08/02 09:23:10  jp
      GetPresentation for the graphic attributes

      Revision 1.45  2000/07/31 19:25:51  jp
      new attributes for CJK/CTL and graphic

      Revision 1.44  2000/05/23 19:54:12  jp
      Bugfixes for Unicode

      Revision 1.43  2000/05/23 15:34:11  jp
      Bugfixes for Unicode

      Revision 1.42  2000/04/26 15:03:20  os
      GetName() returns const String&

      Revision 1.41  2000/04/18 15:14:08  os
      UNICODE

      Revision 1.40  2000/03/03 15:17:05  os
      StarView remainders removed

      Revision 1.39  2000/02/11 15:00:14  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.38  1999/11/08 17:48:15  jp
      new attribut: Foot-/Endnote at section end

      Revision 1.37  1999/09/01 08:41:58  OS
      #66082# GetPresentation: new Parameter


      Rev 1.36   01 Sep 1999 10:41:58   OS
   #66082# GetPresentation: new Parameter

      Rev 1.35   19 Mar 1999 13:40:00   OS
   #63701# unbenutztes Break-Attribut aus den Sections entfernt

      Rev 1.34   04 Feb 1998 14:09:44   MA
   new: SwFmtLineNumber

      Rev 1.33   20 Jan 1998 15:14:16   OM
   Grafiken auf geraden Seiten spiegeln

      Rev 1.32   15 Dec 1997 16:26:06   AMA
   New: SwSectItem: Soll der uebergeordnete Bereich aufgebrochen werden?

      Rev 1.31   29 Nov 1997 15:08:58   MA
   includes

      Rev 1.30   21 Nov 1997 12:08:58   MA
   includes

      Rev 1.29   20 Nov 1997 12:13:36   AMA
   Opt. SwSurround: GoldCut jetzt als Enum; nicht implementierte Enums entfernt

      Rev 1.28   03 Nov 1997 13:59:28   MA
   precomp entfernt

      Rev 1.27   31 Oct 1997 14:41:20   MA
   Chain

      Rev 1.26   28 Oct 1997 13:56:58   MA
   Connect-Attribut

      Rev 1.25   23 Oct 1997 12:00:30   JP
   Numerierung aufs Attribut/Format-Konzept umstellen

      Rev 1.24   15 Aug 1997 12:16:06   OS
   chartar/frmatr/txtatr aufgeteilt

      Rev 1.23   11 Apr 1997 13:06:34   MA
   new: Layout-Split

      Rev 1.22   18 Feb 1997 13:36:16   MA
   new: Rahmen in Readonly editierbar

      Rev 1.21   11 Nov 1996 10:54:16   MA
   ResMgr

      Rev 1.20   29 Oct 1996 18:33:12   HJS
   includes

      Rev 1.19   01 Oct 1996 16:51:28   MA
   chg: Map und URL gleichzeitig

      Rev 1.18   11 Sep 1996 16:51:36   MA
   new: Umlauf nur Anker

      Rev 1.17   28 Aug 1996 15:42:34   OS
   includes

      Rev 1.16   02 Aug 1996 14:57:28   AMA
   New: INet-Felder werden zu attributiertem Text.

      Rev 1.15   23 Jul 1996 11:02:50   AMA
   Opt: SwRegisterItem statt Svx..Item

      Rev 1.14   23 Jul 1996 10:49:58   AMA
   Opt: SwRegisterItem statt Svx..Item

      Rev 1.13   22 Jul 1996 12:53:50   MA
   new: SwFmtURL

      Rev 1.12   18 Jun 1996 16:59:30   OS
   ui_pch includen

      Rev 1.11   14 Jun 1996 14:31:04   MA
   chg: neue Version Prozentangaben

      Rev 1.10   12 Jun 1996 16:05:12   MA
   new: percent fuer framesize

      Rev 1.9   13 Apr 1996 15:34:12   MA
   chg: Neue Ausrichtung fuer Zeichengebunde Rahmen

      Rev 1.8   22 Mar 1996 15:18:36   HJS
   umstellung 311

      Rev 1.7   24 Nov 1995 16:57:24   OM
   PCH->PRECOMPILED

*************************************************************************/


