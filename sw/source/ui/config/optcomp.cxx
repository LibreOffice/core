/*************************************************************************
 *
 *  $RCSfile: optcomp.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2003-10-15 10:00:28 $
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

#include "optcomp.hxx"

#include "docsh.hxx"
#include "swmodule.hxx"
#include "swtypes.hxx"
#include "uiitems.hxx"
#include "view.hxx"
#include "wrtsh.hxx"

#include "optcomp.hrc"
#include "globals.hrc"

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX
#include <sfx2/docfilt.hxx>
#endif
#ifndef _SFX_FCONTNR_HXX
#include <sfx2/fcontnr.hxx>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_PRINTERINDEPENDENTLAYOUT_HPP_
#include <com/sun/star/document/PrinterIndependentLayout.hpp>
#endif

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::uno;
using namespace ::rtl;
using namespace ::std;

#define ASCII_STR(s)        OUString( RTL_CONSTASCII_USTRINGPARAM( s ) )
#define DEFAULT_ENTRY       COMPATIBILITY_DEFAULT_NAME
#define USER_ENTRY          ASCII_STR( "_user" )

// struct CompatibilityItem ----------------------------------------------

struct CompatibilityItem
{
    String      m_sName;
    String      m_sModule;
    sal_Int32   m_nOptions;
    bool        m_bIsDefault;
    bool        m_bIsUser;

    CompatibilityItem( const String& _rName, const String& _rModule,
                       sal_Int32 _nOptions, bool _bIsDefault, bool _bIsUser ) :

        m_sName     ( _rName ),
        m_sModule   ( _rModule ),
        m_nOptions  ( _nOptions ),
        m_bIsDefault( _bIsDefault ),
        m_bIsUser   ( _bIsUser ) {}
};

#include <vector>

struct SwCompatibilityOptPage_Impl
{
    typedef vector< CompatibilityItem > SwCompatibilityItemList;

    SwCompatibilityItemList     m_aList;
};

// class SwCompatibilityOptPage ------------------------------------------

SwCompatibilityOptPage::SwCompatibilityOptPage( Window* pParent, const SfxItemSet& rSet ) :

    SfxTabPage( pParent, SW_RES( TP_OPTCOMPATIBILITY_PAGE ), rSet ),

    m_aMainFL           ( this, ResId( FL_MAIN ) ),
    m_aFormattingFT     ( this, ResId( FT_FORMATTING ) ),
    m_aFormattingLB     ( this, ResId( LB_FORMATTING ) ),
    m_aOptionsFT        ( this, ResId( FT_OPTIONS ) ),
    m_aOptionsLB        ( this, ResId( LB_OPTIONS ) ),
    m_aResetPB          ( this, ResId( PB_RESET ) ),
    m_aDefaultPB        ( this, ResId( PB_DEFAULT ) ),
    m_sUserEntry        (       ResId( STR_USERENTRY ) ),
    m_sUseAsDefaultQuery(       ResId( STR_QRYBOX_USEASDEFAULT ) ),
    m_pWrtShell         ( NULL ),
    m_pImpl             ( new SwCompatibilityOptPage_Impl ),
    m_nSavedOptions     ( 0 )

{
    // init options strings with local resource ids -> so do it before FreeResource()
    for ( USHORT nResId = STR_COMP_OPTIONS_START; nResId < STR_COMP_OPTIONS_END; ++nResId )
    {
        SvLBoxEntry* pEntry = m_aOptionsLB.SvTreeListBox::InsertEntry( String( ResId( nResId ) ) );
        if ( pEntry )
        {
            m_aOptionsLB.SetCheckButtonState( pEntry, SV_BUTTON_UNCHECKED );
            pEntry->SetUserData( (void*)(ULONG)nResId );
        }
    }
    m_aOptionsLB.SetWindowBits( m_aOptionsLB.GetStyle() | WB_HSCROLL | WB_HIDESELECTION );
    m_aOptionsLB.SetHighlightRange();
    m_aOptionsLB.SelectEntryPos( 0 );

    FreeResource();

    InitControls( rSet );

    // set handler
    m_aFormattingLB.SetSelectHdl( LINK( this, SwCompatibilityOptPage, SelectHdl ) );
    m_aOptionsLB.SetCheckButtonHdl( LINK( this, SwCompatibilityOptPage, CheckHdl ) );
    m_aDefaultPB.SetClickHdl( LINK( this, SwCompatibilityOptPage, UseAsDefaultHdl ) );

    // hide some controls, will be implemented later!!!
    m_aFormattingFT.Hide();
    m_aFormattingLB.Hide();
    m_aResetPB.Hide();
}

// -----------------------------------------------------------------------

SwCompatibilityOptPage::~SwCompatibilityOptPage()
{
    delete m_pImpl;
}

// -----------------------------------------------------------------------

void SwCompatibilityOptPage::InitControls( const SfxItemSet& rSet )
{
    // init objectshell and detect document name
    String sDocTitle;
    const SfxPoolItem* pItem = NULL;
    SfxObjectShell* pObjShell = NULL;
    if ( SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_WRTSHELL, FALSE, &pItem ) )
        m_pWrtShell = (SwWrtShell*)( (const SwPtrItem*)pItem )->GetValue();
    if ( m_pWrtShell )
    {
        pObjShell = m_pWrtShell->GetView().GetDocShell();
        if ( pObjShell )
            sDocTitle = pObjShell->GetTitle( SFX_TITLE_TITLE );
    }
    else
    {
        m_aMainFL.Disable();
        m_aFormattingFT.Disable();
        m_aFormattingLB.Disable();
        m_aOptionsFT.Disable();
        m_aOptionsLB.Disable();
        m_aResetPB.Disable();
        m_aDefaultPB.Disable();
    }
    String sText = m_aMainFL.GetText();
    sText.SearchAndReplace( String::CreateFromAscii("%DOCNAME"), sDocTitle );
    m_aMainFL.SetText( sText );

    // loading file formats
    Sequence< Sequence< PropertyValue > > aList = m_aConfigItem.GetList();
    OUString sName;
    OUString sModule;
    sal_Int32 nOptions;
    int i, j, nCount = aList.getLength();
    for ( i = 0; i < nCount; ++i )
    {
        String sNewEntry;
        const Sequence< PropertyValue >& rEntry = aList[i];
        for ( j = 0; j < rEntry.getLength(); j++ )
        {
            PropertyValue aValue = rEntry[j];
            if ( aValue.Name == COMPATIBILITY_PROPERTYNAME_NAME )
                aValue.Value >>= sName;
            else if ( aValue.Name == COMPATIBILITY_PROPERTYNAME_MODULE )
                aValue.Value >>= sModule;
            else if ( aValue.Name == COMPATIBILITY_PROPERTYNAME_OPTIONS )
                aValue.Value >>= nOptions;
        }

        CompatibilityItem aItem(
            sName, sModule, nOptions,
            ( sName.equals( DEFAULT_ENTRY ) != sal_False ),
            ( sName.equals( USER_ENTRY ) != sal_False ) );
        m_pImpl->m_aList.push_back( aItem );

        if ( aItem.m_bIsDefault )
            continue;

        if ( sName.equals( USER_ENTRY ) )
            sNewEntry = m_sUserEntry;
        else if ( pObjShell && sName.getLength() > 0 )
        {
            SfxFilterContainer* pFacCont = pObjShell->GetFactory().GetFilterContainer();
            const SfxFilter* pFilter = pFacCont->GetFilter4FilterName( sName );
            if ( pFilter )
                sNewEntry = pFilter->GetUIName();
        }

        if ( sNewEntry.Len() == 0 )
            sNewEntry = sName;

        USHORT nPos = m_aFormattingLB.InsertEntry( sNewEntry );
        m_aFormattingLB.SetEntryData( nPos, (void*)(long)nOptions );
    }

    m_aFormattingLB.SetDropDownLineCount( m_aFormattingLB.GetEntryCount() );
}

// -----------------------------------------------------------------------

IMPL_LINK( SwCompatibilityOptPage, SelectHdl, ListBox*, EMPTYARG )
{
    USHORT nPos = m_aFormattingLB.GetSelectEntryPos();
    ULONG nOptions = (ULONG)(void*)m_aFormattingLB.GetEntryData( nPos );
    SetCurrentOptions( nOptions );

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SwCompatibilityOptPage, CheckHdl, SvxCheckListBox*, EMPTYARG )
{
    USHORT nPos = m_aOptionsLB.GetSelectEntryPos();
    //...
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SwCompatibilityOptPage, UseAsDefaultHdl, PushButton*, EMPTYARG )
{
    QueryBox aBox( this, WinBits( WB_YES_NO | WB_DEF_YES ), m_sUseAsDefaultQuery );
    if ( aBox.Execute() == RET_YES )
    {
        for ( vector< CompatibilityItem >::iterator pItem = m_pImpl->m_aList.begin();
              pItem != m_pImpl->m_aList.end(); ++pItem )
        {
            if ( pItem->m_bIsDefault )
            {
                pItem->m_nOptions = GetCurrentOptions();
                break;
            }
        }

        WriteOptions();
    }

    return 0;
}

// -----------------------------------------------------------------------

void SwCompatibilityOptPage::SetCurrentOptions( ULONG nOptions )
{
    ULONG nCount = m_aOptionsLB.GetEntryCount();
    DBG_ASSERT( nCount <= 32, "SwCompatibilityOptPage::Reset(): entry overflow" );
    for ( USHORT i = 0; i < nCount; ++i )
    {
        BOOL bChecked = ( ( nOptions & 0x00000001 ) == 0x00000001 );
        if ( COPT_USE_VIRTUALDEVICE == i || COPT_EXTLEADING == i )
            // swap the value because this checklistbox entry means the opposite!
            bChecked = !bChecked;
        m_aOptionsLB.CheckEntryPos( i, bChecked );
        nOptions = nOptions >> 1;
    }
}

// -----------------------------------------------------------------------

ULONG SwCompatibilityOptPage::GetCurrentOptions() const
{
    ULONG nRet = 0;
    ULONG nSetBit = 1;
    ULONG nCount = m_aOptionsLB.GetEntryCount();
    DBG_ASSERT( nCount <= 32, "SwCompatibilityOptPage::GetCurrentOptions(): entry overflow" );
    for ( USHORT i = 0; i < nCount; ++i )
    {
        BOOL bChecked = m_aOptionsLB.IsChecked(i);
        if ( COPT_USE_VIRTUALDEVICE == i || COPT_EXTLEADING == i )
            // swap the value because this checklistbox entry means the opposite!
            bChecked = !bChecked;
        if ( bChecked )
            nRet |= nSetBit;
        nSetBit = nSetBit << 1;
    }
    return nRet;
}

// -----------------------------------------------------------------------

void SwCompatibilityOptPage::SetDocumentOptions( ULONG nOptions )
{
    //?!?
}

// -----------------------------------------------------------------------

ULONG SwCompatibilityOptPage::GetDocumentOptions() const
{
    ULONG nRet = 0;
    ULONG nSetBit = 1;

    if ( m_pWrtShell )
    {
        short nPrtLayout = m_pWrtShell->IsUseVirtualDevice();
        if ( PrinterIndependentLayout::DISABLED != nPrtLayout )
            nRet |= nSetBit;
        nSetBit = nSetBit << 1;
        if ( m_pWrtShell->IsParaSpaceMax() )
            nRet |= nSetBit;
        nSetBit = nSetBit << 1;
        if ( m_pWrtShell->IsParaSpaceMaxAtPages() )
            nRet |= nSetBit;
        nSetBit = nSetBit << 1;
        if ( m_pWrtShell->IsTabCompat() )
            nRet |= nSetBit;
        nSetBit = nSetBit << 1;
        if ( m_pWrtShell->IsAddExtLeading() )
            nRet |= nSetBit;
    }

    return nRet;
}

// -----------------------------------------------------------------------

void SwCompatibilityOptPage::WriteOptions()
{
    m_aConfigItem.Clear();
    for ( vector< CompatibilityItem >::const_iterator pItem = m_pImpl->m_aList.begin();
          pItem != m_pImpl->m_aList.end(); ++pItem )
        m_aConfigItem.AppendItem( pItem->m_sName, pItem->m_sModule, pItem->m_nOptions );
}

// -----------------------------------------------------------------------

SfxTabPage* SwCompatibilityOptPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return new SwCompatibilityOptPage( pParent, rAttrSet );
}

// -----------------------------------------------------------------------

BOOL SwCompatibilityOptPage::FillItemSet( SfxItemSet& rSet )
{
    BOOL bModified = FALSE;
    if ( m_pWrtShell )
    {
        ULONG nSavedOptions = m_nSavedOptions;
        ULONG nCount = m_aOptionsLB.GetEntryCount();
        DBG_ASSERT( nCount <= 32, "SwCompatibilityOptPage::Reset(): entry overflow" );

        bool bSetParaSpaceMax = false;

        for ( USHORT i = 0; i < nCount; ++i )
        {
            CompatibilityOptions nOption = static_cast< CompatibilityOptions >(i);
            BOOL bChecked = m_aOptionsLB.IsChecked(i);
            if ( COPT_USE_VIRTUALDEVICE == i || COPT_EXTLEADING == i )
                // swap the value because this checklistbox entry means the opposite!
                bChecked = !bChecked;
            BOOL bSavedChecked = ( ( nSavedOptions & 0x00000001 ) == 0x00000001 );
            if ( bChecked != bSavedChecked )
            {
                if ( COPT_USE_VIRTUALDEVICE == nOption )
                {
                    short nUseVirtualDev = !bChecked
                        ? PrinterIndependentLayout::DISABLED
                        : PrinterIndependentLayout::HIGH_RESOLUTION;
                    m_pWrtShell->SetUseVirtualDevice( nUseVirtualDev );
                    bModified = TRUE;
                }
                else if ( ( COPT_PARASPACE_MAX == nOption || COPT_PARASPACE_MAXATPAGES == nOption ) && !bSetParaSpaceMax )
                    bSetParaSpaceMax = true;
                else if ( COPT_TABALIGNMENT == nOption )
                {
                    m_pWrtShell->SetTabCompat( bChecked );
                    bModified = TRUE;
                }
                else if ( COPT_EXTLEADING == nOption )
                {
                    m_pWrtShell->SetAddExtLeading( bChecked );
                    bModified = TRUE;
                }
            }

            nSavedOptions = nSavedOptions >> 1;
        }

        if ( bSetParaSpaceMax )
        {
            m_pWrtShell->SetParaSpaceMax( m_aOptionsLB.IsChecked( (USHORT)COPT_PARASPACE_MAX ),
                                          m_aOptionsLB.IsChecked( (USHORT)COPT_PARASPACE_MAXATPAGES ) );
            bModified = TRUE;
        }
    }

    if ( bModified )
        WriteOptions();

    return bModified;
}

// -----------------------------------------------------------------------

void SwCompatibilityOptPage::Reset( const SfxItemSet& rSet )
{
    ULONG nOptions = GetDocumentOptions();
    SetCurrentOptions( nOptions );
    m_nSavedOptions = nOptions;
}

