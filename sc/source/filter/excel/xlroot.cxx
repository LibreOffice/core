/*************************************************************************
 *
 *  $RCSfile: xlroot.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 13:36:39 $
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

// ============================================================================

#ifndef SC_XLROOT_HXX
#include "xlroot.hxx"
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
#endif
#ifndef _SFX_PRINTER_HXX
#include <sfx2/printer.hxx>
#endif
#ifndef _SV_FONT_HXX
#include <vcl/font.hxx>
#endif
#ifndef _EDITSTAT_HXX
#include <svx/editstat.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef SC_ITEMS_HXX
#include "scitems.hxx"
#endif
#ifndef _EEITEM_HXX
#include <svx/eeitem.hxx>
#endif

#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_SCDOCPOL_HXX
#include "docpool.hxx"
#endif
#ifndef SC_DOCUNO_HXX
#include "docuno.hxx"
#endif
#ifndef SC_EDITUTIL_HXX
#include "editutil.hxx"
#endif
#ifndef SC_DRWLAYER_HXX
#include "drwlayer.hxx"
#endif
#ifndef SC_SCPATATR_HXX
#include "patattr.hxx"
#endif

#ifndef SC_XLSTYLE_HXX
#include "xlstyle.hxx"
#endif
#ifndef SC_XLTRACER_HXX
#include "xltracer.hxx"
#endif

#include "root.hxx"


namespace com { namespace sun { namespace star { namespace frame { class XModel; } } } }

using ::com::sun::star::uno::Reference;
using ::com::sun::star::frame::XModel;


// Global data ================================================================

XclRootData::XclRootData( XclBiff eBiff, ScDocument& rDocument, const String& rDocUrl, CharSet eCharSet ) :
    meBiff( eBiff ),
    mrDoc( rDocument ),
    maDocUrl( rDocUrl ),
    maBasePath( rDocUrl, 0, rDocUrl.SearchBackward( '/' ) + 1 ),
    meCharSet( eCharSet ),
    meSysLang( Application::GetSettings().GetLanguage() ),
    meDocLang( Application::GetSettings().GetLanguage() ),
    meUILang( Application::GetSettings().GetUILanguage() ),
    maScMaxPos( MAXCOL, MAXROW, MAXTAB ),
    maXclMaxPos( EXC_MAXCOL_BIFF2, EXC_MAXROW_BIFF2, EXC_MAXTAB_BIFF2 ),
    mnCharWidth( 110 ),
    mnScTab( 0 ),
    mbTruncated( false ),
    mpRDP( new RootData )//!
{
#ifdef DBG_UTIL
    mnObjCnt = 0;
#endif
}

XclRootData::~XclRootData()
{
#ifdef DBG_UTIL
    DBG_ASSERT( mnObjCnt == 0, "XclRootData::~XclRootData - wrong object count" );
#endif
}


// ----------------------------------------------------------------------------

XclRoot::XclRoot( XclRootData& rRootData ) :
    mrData( rRootData ),
    mpRD( rRootData.mpRDP.get() )//!
{
#ifdef DBG_UTIL
    ++mrData.mnObjCnt;
#endif
    if( GetBiff() != xlBiffUnknown )
        SetMaxPos();
}

XclRoot::XclRoot( const XclRoot& rRoot ) :
    mrData( rRoot.mrData ),
    mpRD( rRoot.mpRD )//!
{
#ifdef DBG_UTIL
    ++mrData.mnObjCnt;
#endif
}

XclRoot::~XclRoot()
{
#ifdef DBG_UTIL
    --mrData.mnObjCnt;
#endif
}

XclRoot& XclRoot::operator=( const XclRoot& rRoot )
{
    // allowed for assignment in derived classes - but test if the same root data is used
    DBG_ASSERT( &mrData == &rRoot.mrData, "XclRoot::operator= - incompatible root data" );
    return *this;
}

void XclRoot::SetBiff( XclBiff eBiff )
{
    mrData.meBiff = eBiff;
    if( eBiff != xlBiffUnknown )
        SetMaxPos();
}

void XclRoot::SetCharWidth( const XclFontData& rFontData )
{
    if( SfxPrinter* pPrinter = GetPrinter() )
    {
        Font aFont( rFontData.maName, Size( 0, rFontData.mnHeight ) );
        aFont.SetFamily( rFontData.GetScFamily( GetCharSet() ) );
        aFont.SetCharSet( rFontData.GetScCharSet() );
        aFont.SetWeight( rFontData.GetScWeight() );
        pPrinter->SetFont( aFont );
        mrData.mnCharWidth = pPrinter->GetTextWidth( String( '0' ) );
    }
    else
        mrData.mnCharWidth = 11 * rFontData.mnHeight / 20;
}

void XclRoot::SetMaxPos()
{
    switch( GetBiff() )
    {
        case xlBiff2:
        case xlBiff3:   mrData.maXclMaxPos.Set( EXC_MAXCOL_BIFF2, EXC_MAXROW_BIFF2, EXC_MAXTAB_BIFF2 );    break;
        case xlBiff4:
        case xlBiff5:
        case xlBiff7:   mrData.maXclMaxPos.Set( EXC_MAXCOL_BIFF4, EXC_MAXROW_BIFF4, EXC_MAXTAB_BIFF4 );    break;
        case xlBiff8:   mrData.maXclMaxPos.Set( EXC_MAXCOL_BIFF8, EXC_MAXROW_BIFF8, EXC_MAXTAB_BIFF8 );    break;
        default:        DBG_ERROR_BIFF();
    }
}

SfxObjectShell* XclRoot::GetDocShell() const
{
    return GetDoc().GetDocumentShell();
}

ScModelObj* XclRoot::GetDocModelObj() const
{
    SfxObjectShell* pDocShell = GetDocShell();
    return pDocShell ? ScModelObj::getImplementation( Reference< XModel >( pDocShell->GetModel() ) ) : NULL;
}

SfxPrinter* XclRoot::GetPrinter() const
{
    return GetDoc().GetPrinter();
}

SvNumberFormatter& XclRoot::GetFormatter() const
{
    return *GetDoc().GetFormatTable();
}

ScStyleSheetPool& XclRoot::GetStyleSheetPool() const
{
    return *GetDoc().GetStyleSheetPool();
}

ScRangeName& XclRoot::GetNamedRanges() const
{
    return *GetDoc().GetRangeName();
}

SvStorage* XclRoot::GetRootStorage() const
{
    return mpRD->pRootStorage;
}

ScEditEngineDefaulter& XclRoot::GetEditEngine() const
{
    if( !mrData.mpEditEngine.get() )
    {
        mrData.mpEditEngine.reset( new ScEditEngineDefaulter( GetDoc().GetEnginePool() ) );
        ScEditEngineDefaulter& rEE = *mrData.mpEditEngine;
        rEE.SetRefMapMode( MAP_100TH_MM );
        rEE.SetEditTextObjectPool( GetDoc().GetEditPool() );
        rEE.SetUpdateMode( FALSE );
        rEE.EnableUndo( FALSE );
        rEE.SetControlWord( rEE.GetControlWord() & ~EE_CNTRL_ALLOWBIGOBJS );
    }
    return *mrData.mpEditEngine;
}

ScHeaderEditEngine& XclRoot::GetHFEditEngine() const
{
    if( !mrData.mpHFEditEngine.get() )
    {
        mrData.mpHFEditEngine.reset( new ScHeaderEditEngine( EditEngine::CreatePool(), TRUE ) );
        ScHeaderEditEngine& rEE = *mrData.mpHFEditEngine;
        rEE.SetRefMapMode( MAP_TWIP );  // headers/footers use twips as default metric
        rEE.SetUpdateMode( FALSE );
        rEE.EnableUndo( FALSE );
        rEE.SetControlWord( rEE.GetControlWord() & ~EE_CNTRL_ALLOWBIGOBJS );

        // set Calc header/footer defaults
        SfxItemSet* pEditSet = new SfxItemSet( rEE.GetEmptyItemSet() );
        SfxItemSet aItemSet( *GetDoc().GetPool(), ATTR_PATTERN_START, ATTR_PATTERN_END );
        ScPatternAttr::FillToEditItemSet( *pEditSet, aItemSet );
        // FillToEditItemSet() adjusts font height to 1/100th mm, we need twips
        pEditSet->Put( aItemSet.Get( ATTR_FONT_HEIGHT ), EE_CHAR_FONTHEIGHT );
        pEditSet->Put( aItemSet.Get( ATTR_CJK_FONT_HEIGHT ), EE_CHAR_FONTHEIGHT_CJK );
        pEditSet->Put( aItemSet.Get( ATTR_CTL_FONT_HEIGHT ), EE_CHAR_FONTHEIGHT_CTL );
        rEE.SetDefaults( pEditSet );    // takes ownership
   }
    return *mrData.mpHFEditEngine;
}

EditEngine& XclRoot::GetDrawEditEngine() const
{
    if( !mrData.mpDrawEditEng.get() )
    {
        mrData.mpDrawEditEng.reset( new EditEngine( &GetDoc().GetDrawLayer()->GetItemPool() ) );
        EditEngine& rEE = *mrData.mpDrawEditEng;
        rEE.SetRefMapMode( MAP_100TH_MM );
        rEE.SetUpdateMode( FALSE );
        rEE.EnableUndo( FALSE );
        rEE.SetControlWord( rEE.GetControlWord() & ~EE_CNTRL_ALLOWBIGOBJS );
    }
    return *mrData.mpDrawEditEng;
}

XclTracer& XclRoot::GetTracer() const
{
    return *mrData.mpTracer;
}

bool XclRoot::CheckCellAddress( const ScAddress& rPos, const ScAddress& rMaxPos ) const
{
    bool bValid = (rPos.Col() <= rMaxPos.Col()) && (rPos.Row() <= rMaxPos.Row()) && (rPos.Tab() <= rMaxPos.Tab());
    if( !bValid )
    {
        mrData.mbTruncated = true;
        GetTracer().TraceInvalidAddress(rPos, rMaxPos);
    }
    return bValid;
}

bool XclRoot::CheckCellRange( ScRange& rRange, const ScAddress& rMaxPos ) const
{
    rRange.Justify();

    // check start position
    bool bValidStart = CheckCellAddress( rRange.aStart, rMaxPos );

    // check & correct end position
    if( bValidStart )
    {
        CheckCellAddress( rRange.aEnd, rMaxPos );

        if( rRange.aEnd.Col() > rMaxPos.Col() )
            rRange.aEnd.SetCol( rMaxPos.Col() );
        if( rRange.aEnd.Row() > rMaxPos.Row() )
            rRange.aEnd.SetRow( rMaxPos.Row() );
        if( rRange.aEnd.Tab() > rMaxPos.Tab() )
            rRange.aEnd.SetTab( rMaxPos.Tab() );
    }

    return bValidStart;
}

void XclRoot::CheckCellRangeList( ScRangeList& rRanges, const ScAddress& rMaxPos ) const
{
    sal_uInt32 nIndex = rRanges.Count();
    while( nIndex )
    {
        --nIndex;   // backwards to keep nIndex valid
        ScRange* pRange = rRanges.GetObject( nIndex );
        if( pRange && !CheckCellRange( *pRange, rMaxPos ) )
            delete rRanges.Remove( nIndex );
    }
}


// ============================================================================

