/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: present.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 14:32:49 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif

#include "sdattr.hxx"
#include "present.hxx"
#include "present.hrc"
#include "sdresid.hxx"
#include "cusshow.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;

/*************************************************************************
|* Ctor
\************************************************************************/
SdStartPresentationDlg::SdStartPresentationDlg( Window* pWindow,
                                  const SfxItemSet& rInAttrs,
                                  List& rPageNames, List* pCSList ) :
                ModalDialog     ( pWindow, SdResId( DLG_START_PRESENTATION ) ),
                aGrpRange               ( this, SdResId( GRP_RANGE ) ),
                aRbtAll                 ( this, SdResId( RBT_ALL ) ),
                aRbtAtDia               ( this, SdResId( RBT_AT_DIA ) ),
                aRbtCustomshow          ( this, SdResId( RBT_CUSTOMSHOW ) ),
                aLbDias                 ( this, SdResId( LB_DIAS ) ),
                aLbCustomshow           ( this, SdResId( LB_CUSTOMSHOW ) ),

                aGrpKind                ( this, SdResId( GRP_KIND ) ),
                aRbtStandard            ( this, SdResId( RBT_STANDARD ) ),
                aRbtWindow              ( this, SdResId( RBT_WINDOW ) ),
                aRbtAuto                ( this, SdResId( RBT_AUTO ) ),
                aTmfPause               ( this, SdResId( TMF_PAUSE ) ),
                aCbxAutoLogo            ( this, SdResId( CBX_AUTOLOGO ) ),

                aGrpOptions             ( this, SdResId( GRP_OPTIONS ) ),
                aCbxManuel              ( this, SdResId( CBX_MANUEL ) ),
                aCbxMousepointer        ( this, SdResId( CBX_MOUSEPOINTER ) ),
                aCbxPen                 ( this, SdResId( CBX_PEN ) ),
                aCbxNavigator           ( this, SdResId( CBX_NAVIGATOR ) ),
                aCbxAnimationAllowed    ( this, SdResId( CBX_ANIMATION_ALLOWED ) ),
                aCbxChangePage          ( this, SdResId( CBX_CHANGE_PAGE ) ),
                aCbxAlwaysOnTop         ( this, SdResId( CBX_ALWAYS_ON_TOP ) ),

                maGrpMonitor            ( this, SdResId( GRP_MONITOR ) ),
                maFtMonitor             ( this, SdResId( FT_MONITOR ) ),
                maLBMonitor             ( this, SdResId( LB_MONITOR ) ),

                aBtnOK                  ( this, SdResId( BTN_OK ) ),
                aBtnCancel              ( this, SdResId( BTN_CANCEL ) ),
                aBtnHelp                ( this, SdResId( BTN_HELP ) ),

                pCustomShowList         ( pCSList ),
                rOutAttrs               ( rInAttrs ),
                mnMonitors              ( 0 ),

                msPrimaryMonitor( SdResId(STR_PRIMARY_MONITOR ) ),
                msMonitor( SdResId( STR_MONITOR ) ),
                msAllMonitors( SdResId( STR_ALL_MONITORS ) )
{
    FreeResource();

    Link aLink( LINK( this, SdStartPresentationDlg, ChangeRangeHdl ) );

    aRbtAll.SetClickHdl( aLink );
    aRbtAtDia.SetClickHdl( aLink );
    aRbtCustomshow.SetClickHdl( aLink );

    aLink = LINK( this, SdStartPresentationDlg, ClickWindowPresentationHdl );
    aRbtStandard.SetClickHdl( aLink );
    aRbtWindow.SetClickHdl( aLink );
    aRbtAuto.SetClickHdl( aLink );

    aTmfPause.SetModifyHdl( LINK( this, SdStartPresentationDlg, ChangePauseHdl ) );
    aTmfPause.SetFormat( TIMEF_SEC );

    // Listbox mit Seitennamen fuellen
    rPageNames.First();
    for( UINT16 i = 0;
         i < rPageNames.Count();
         i++ )
    {
        aLbDias.InsertEntry( *( String* ) rPageNames.GetCurObject() );
        rPageNames.Next();
    }

    if( pCustomShowList )
    {
        USHORT nPosToSelect = (USHORT) pCustomShowList->GetCurPos();
        SdCustomShow* pCustomShow;
        // Listbox mit CustomShows fuellen
        for( pCustomShow = (SdCustomShow*) pCustomShowList->First();
             pCustomShow != NULL;
             pCustomShow = (SdCustomShow*) pCustomShowList->Next() )
        {
            aLbCustomshow.InsertEntry( pCustomShow->GetName() );
        }
        aLbCustomshow.SelectEntryPos( nPosToSelect );
        pCustomShowList->Seek( nPosToSelect );
    }
    else
        aRbtCustomshow.Disable();

    if( ( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_CUSTOMSHOW ) ).GetValue() && pCSList )
        aRbtCustomshow.Check();
    else if( ( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_ALL ) ).GetValue() )
        aRbtAll.Check();
    else
        aRbtAtDia.Check();

    aLbDias.SelectEntry( ( ( const SfxStringItem& ) rOutAttrs.Get( ATTR_PRESENT_DIANAME ) ).GetValue() );
    aCbxManuel.Check( ( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_MANUEL ) ).GetValue() );
    aCbxMousepointer.Check( ( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_MOUSE ) ).GetValue() );
    aCbxPen.Check( ( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_PEN ) ).GetValue() );
    aCbxNavigator.Check( ( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_NAVIGATOR ) ).GetValue() );
    aCbxAnimationAllowed.Check( ( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_ANIMATION_ALLOWED ) ).GetValue() );
    aCbxChangePage.Check( ( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_CHANGE_PAGE ) ).GetValue() );
    aCbxAlwaysOnTop.Check( ( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_ALWAYS_ON_TOP ) ).GetValue() );

    const BOOL  bEndless = ( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_ENDLESS ) ).GetValue();
    const BOOL  bWindow = !( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_FULLSCREEN ) ).GetValue();
    const long  nPause = ( ( const SfxUInt32Item& ) rOutAttrs.Get( ATTR_PRESENT_PAUSE_TIMEOUT ) ).GetValue();

    aTmfPause.SetTime( Time( 0, 0, nPause ) );
    // set cursor in timefield
    Edit *pEdit = aTmfPause.GetField();
    Selection aSel( pEdit->GetMaxTextLen(), pEdit->GetMaxTextLen() );
    pEdit->SetSelection( aSel );

    aCbxAutoLogo.Check( ( ( const SfxBoolItem& ) rOutAttrs.Get( ATTR_PRESENT_SHOW_PAUSELOGO ) ).GetValue() );

    if( bWindow )
        aRbtWindow.Check( TRUE );
    else if( bEndless )
        aRbtAuto.Check( TRUE );
    else
        aRbtStandard.Check( TRUE );

    InitMonitorSettings();

    ChangeRangeHdl( this );

    ClickWindowPresentationHdl( NULL );
    ChangePauseHdl( NULL );
}

void SdStartPresentationDlg::InitMonitorSettings()
{
    try
    {
        Reference< XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory(), UNO_QUERY_THROW );
        Reference< XIndexAccess > xMultiMon( xFactory->createInstance(OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.DisplayAccess" ) ) ), UNO_QUERY_THROW );
        maGrpMonitor.Show( true );
        maFtMonitor.Show( true );
        maLBMonitor.Show( true );

        mnMonitors = xMultiMon->getCount();

        if( mnMonitors <= 1 )
        {
            maFtMonitor.Enable( false );
            maLBMonitor.Enable( false );
        }
        else
        {
            sal_Bool bMultiscreen = false;
            sal_Int32 nPrimaryIndex = 0;
            Reference< XPropertySet > xMonProps( xMultiMon, UNO_QUERY );
            if( xMonProps.is() ) try
            {
                const OUString sPropName1( RTL_CONSTASCII_USTRINGPARAM( "MultiDisplay" ) );
                xMonProps->getPropertyValue( sPropName1 ) >>= bMultiscreen;
                const OUString sPropName2( RTL_CONSTASCII_USTRINGPARAM( "DefaultDisplay" ) );
                xMonProps->getPropertyValue( sPropName2 ) >>= nPrimaryIndex;
            }
            catch( Exception& )
            {
            }

            const String sPlaceHolder( RTL_CONSTASCII_USTRINGPARAM( "%1" ) );
            for( sal_Int32 nDisplay = 0; nDisplay < mnMonitors; nDisplay++ )
            {
                String aName( nDisplay == nPrimaryIndex ? msPrimaryMonitor : msMonitor );
                const String aNumber( String::CreateFromInt32( nDisplay + 1 ) );
                aName.SearchAndReplace( sPlaceHolder, aNumber );
                maLBMonitor.InsertEntry( aName );
            }

            if( !bMultiscreen )
                maLBMonitor.InsertEntry( msAllMonitors );

            sal_Int32 nSelected = ( ( const SfxInt32Item& ) rOutAttrs.Get( ATTR_PRESENT_DISPLAY ) ).GetValue();
            if( nSelected <= 0 )
                nSelected = nPrimaryIndex;
            else
                nSelected--;

            maLBMonitor.SelectEntryPos( (USHORT)nSelected );
        }
    }
    catch( Exception& )
    {
    }
}

/*************************************************************************
|* Setzt die ausgewaehlten Attribute des Dialogs
\************************************************************************/
void SdStartPresentationDlg::GetAttr( SfxItemSet& rAttr )
{
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_ALL, aRbtAll.IsChecked() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_CUSTOMSHOW, aRbtCustomshow.IsChecked() ) );
    rAttr.Put( SfxStringItem ( ATTR_PRESENT_DIANAME, aLbDias.GetSelectEntry() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_MANUEL, aCbxManuel.IsChecked() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_MOUSE, aCbxMousepointer.IsChecked() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_PEN, aCbxPen.IsChecked() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_NAVIGATOR, aCbxNavigator.IsChecked() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_ANIMATION_ALLOWED, aCbxAnimationAllowed.IsChecked() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_CHANGE_PAGE, aCbxChangePage.IsChecked() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_ALWAYS_ON_TOP, aCbxAlwaysOnTop.IsChecked() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_FULLSCREEN, !aRbtWindow.IsChecked() ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_ENDLESS, aRbtAuto.IsChecked() ) );
    rAttr.Put( SfxUInt32Item ( ATTR_PRESENT_PAUSE_TIMEOUT, aTmfPause.GetTime().GetMSFromTime() / 1000 ) );
    rAttr.Put( SfxBoolItem ( ATTR_PRESENT_SHOW_PAUSELOGO, aCbxAutoLogo.IsChecked() ) );

    USHORT nPos = maLBMonitor.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
        rAttr.Put( SfxInt32Item ( ATTR_PRESENT_DISPLAY, nPos + 1 ) );

    nPos = aLbCustomshow.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
        pCustomShowList->Seek( nPos );
}

/*************************************************************************
|*      Handler: Enabled/Disabled Listbox "Dias"
\************************************************************************/
IMPL_LINK( SdStartPresentationDlg, ChangeRangeHdl, void *, EMPTYARG )
{
    aLbDias.Enable( aRbtAtDia.IsChecked() );
    aLbCustomshow.Enable( aRbtCustomshow.IsChecked() );

    return( 0L );
}

/*************************************************************************
|*      Handler: Enabled/Disabled Checkbox "AlwaysOnTop"
\************************************************************************/
IMPL_LINK( SdStartPresentationDlg, ClickWindowPresentationHdl, void *, EMPTYARG )
{
    const bool bAuto = aRbtAuto.IsChecked();
    const bool bWindow = aRbtWindow.IsChecked();

    // aFtPause.Enable( bAuto );
    aTmfPause.Enable( bAuto );
    aCbxAutoLogo.Enable( bAuto && ( aTmfPause.GetTime().GetMSFromTime() > 0 ) );

    const bool bDisplay = !bWindow && ( mnMonitors > 1 );
    maFtMonitor.Enable( bDisplay );
    maLBMonitor.Enable( bDisplay );

    if( bWindow )
    {
        aCbxAlwaysOnTop.Enable( FALSE );
        aCbxAlwaysOnTop.Check( FALSE );
    }
    else
        aCbxAlwaysOnTop.Enable();

    return( 0L );
}

/*************************************************************************
|*      Handler: Enabled/Disabled Checkbox "AlwaysOnTop"
\************************************************************************/
IMPL_LINK( SdStartPresentationDlg, ChangePauseHdl, void *, EMPTYARG )
{
    aCbxAutoLogo.Enable( aRbtAuto.IsChecked() && ( aTmfPause.GetTime().GetMSFromTime() > 0 ) );
    return( 0L );
}


