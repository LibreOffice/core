/*************************************************************************
 *
 *  $RCSfile: optjava.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 13:08:40 $
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

// include ---------------------------------------------------------------

#include "optjava.hxx"
#include "dialmgr.hxx"

#include "optjava.hrc"
#include "dialogs.hrc"
#include "helpid.hrc"

#include <vcl/help.hxx>

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _SVTOOLS_IMAGEMGR_HXX
#include <svtools/imagemgr.hxx>
#endif
#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _UCBHELPER_CONTENTBROKER_HXX
#include <ucbhelper/contentbroker.hxx>
#endif

#ifndef  _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFOLDERPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFolderPicker.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_EXECUTABLEDIALOGRESULTS_HPP_
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDER_HPP_
#include <com/sun/star/ucb/XContentProvider.hpp>
#endif

#ifndef INCLUDED_JVMFWK_FRAMEWORK_H
#include <jvmfwk/framework.h>
#endif

// define ----------------------------------------------------------------

#define CLASSPATH_DELIMITER SAL_PATHSEPARATOR
#define STRIM( s )          s.EraseLeadingChars().EraseTrailingChars()
#define BUTTON_BORDER       2
#define RESET_TIMEOUT       300

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;

// class SvxJavaTable ----------------------------------------------------

SvxJavaTable::SvxJavaTable( Window* _pParent, const ResId& _rId ) :

    SvxSimpleTable( _pParent, _rId )

{
}

SvxJavaTable::~SvxJavaTable()
{
}

void SvxJavaTable::SetTabs()
{
    SvxSimpleTable::SetTabs();
/*
    USHORT nAdjust = SV_LBOXTAB_ADJUST_RIGHT | SV_LBOXTAB_ADJUST_LEFT |
                     SV_LBOXTAB_ADJUST_CENTER | SV_LBOXTAB_ADJUST_NUMERIC | SV_LBOXTAB_FORCE;
    if ( aTabs.Count() > 0 )
    {
        SvLBoxTab* pTab = (SvLBoxTab*)aTabs.GetObject(0);
        pTab->nFlags &= ~nAdjust;
        pTab->nFlags |= SV_LBOXTAB_PUSHABLE | SV_LBOXTAB_ADJUST_CENTER | SV_LBOXTAB_FORCE;
    }
*/
}

void SvxJavaTable::MouseButtonUp( const MouseEvent& _rMEvt )
{
    m_aCurMousePoint = _rMEvt.GetPosPixel();
    SvxSimpleTable::MouseButtonUp( _rMEvt );
}

void SvxJavaTable::KeyInput( const KeyEvent& rKEvt )
{
    if ( !rKEvt.GetKeyCode().GetModifier() && KEY_SPACE == rKEvt.GetKeyCode().GetCode() )
    {
        SvLBoxEntry* pEntry = FirstSelected();
        if ( GetCheckButtonState( pEntry ) == SV_BUTTON_UNCHECKED )
        {
            SetCheckButtonState( pEntry, SV_BUTTON_CHECKED );
            GetCheckButtonHdl().Call( NULL );
            return ;
        }
    }

    SvxSimpleTable::KeyInput( rKEvt );
}

// -----------------------------------------------------------------------

bool areListsEqual( const Sequence< ::rtl::OUString >& rListA, const Sequence< ::rtl::OUString >& rListB )
{
    bool bRet = true;
    const sal_Int32 nLen = rListA.getLength();

    if (  rListB.getLength() != nLen )
        bRet = false;
    else
    {
        const ::rtl::OUString* pStringA = rListA.getConstArray();
        const ::rtl::OUString* pStringB = rListB.getConstArray();

        for ( sal_Int32 i = 0; i < nLen; ++i )
        {
            if ( *pStringA++ != *pStringB++ )
            {
                bRet = false;
                break;
            }
        }
    }

    return bRet;
}

// class SvxJavaOptionsPage ----------------------------------------------

SvxJavaOptionsPage::SvxJavaOptionsPage( Window* pParent, const SfxItemSet& rSet ) :

    SfxTabPage( pParent, SVX_RES( RID_SVXPAGE_OPTIONS_JAVA ), rSet ),

    m_aJavaLine         ( this, ResId( FL_JAVA ) ),
    m_aJavaEnableCB     ( this, ResId( CB_JAVA_ENABLE ) ),
    m_aJavaFoundLabel   ( this, ResId( FT_JAVA_FOUND ) ),
    m_aJavaList         ( this, ResId( LB_JAVA ) ),
    m_aJavaPathText     ( this, ResId( FT_JAVA_PATH ) ),
    m_aAddBtn           ( this, ResId( PB_ADD ) ),
    m_aParameterBtn     ( this, ResId( PB_PARAMETER ) ),
    m_aClassPathBtn     ( this, ResId( PB_CLASSPATH ) ),

    m_pParamDlg         ( NULL ),
    m_pPathDlg          ( NULL ),
    m_parJavaInfo       ( NULL ),
    m_parParameters     ( NULL ),
    m_pClassPath        ( NULL ),
    m_nInfoSize         ( 0 ),
    m_nParamSize        ( 0 ),
    m_sInstallText      (       ResId( STR_INSTALLED_IN ) ),
    m_sAccessibilityText(       ResId( STR_ACCESSIBILITY ) ),
    m_sAddDialogText    (       ResId( STR_ADDDLGTEXT ) )

{
    m_aJavaEnableCB.SetClickHdl( LINK( this, SvxJavaOptionsPage, EnableHdl_Impl ) );
    m_aJavaList.SetCheckButtonHdl( LINK( this, SvxJavaOptionsPage, CheckHdl_Impl ) );
    m_aJavaList.SetSelectHdl( LINK( this, SvxJavaOptionsPage, SelectHdl_Impl ) );
    m_aAddBtn.SetClickHdl( LINK( this, SvxJavaOptionsPage, AddHdl_Impl ) );
    m_aParameterBtn.SetClickHdl( LINK( this, SvxJavaOptionsPage, ParameterHdl_Impl ) );
    m_aClassPathBtn.SetClickHdl( LINK( this, SvxJavaOptionsPage, ClassPathHdl_Impl ) );
    m_aResetTimer.SetTimeoutHdl( LINK( this, SvxJavaOptionsPage, ResetHdl_Impl ) );
    m_aResetTimer.SetTimeout( RESET_TIMEOUT );

    m_aJavaList.EnableCheckButton( new SvLBoxButtonData( &m_aJavaList, true ) );

    static long aStaticTabs[]=
    {
        5, 0, 15, 90, 130, 300
    };

    m_aJavaList.SvxSimpleTable::SetTabs( aStaticTabs );
    String sHeader( '\t' );
    sHeader += String( ResId( STR_HEADER_VENDOR ) );
    sHeader += '\t';
    sHeader += String( ResId( STR_HEADER_VERSION ) );
    sHeader += '\t';
    sHeader += String( ResId( STR_HEADER_FEATURES ) );
    sHeader += '\t';
    m_aJavaList.InsertHeaderEntry( sHeader, HEADERBAR_APPEND, HIB_LEFT );

    FreeResource();

    EnableHdl_Impl( &m_aJavaEnableCB );
    jfw_lock();
}

// -----------------------------------------------------------------------

SvxJavaOptionsPage::~SvxJavaOptionsPage()
{
    delete m_pParamDlg;
    delete m_pPathDlg;
    ClearJavaInfo();
    std::vector< JavaInfo* >::iterator pIter;
    for ( pIter = m_aAddedInfos.begin(); pIter != m_aAddedInfos.end(); ++pIter )
    {
        JavaInfo* pInfo = *pIter;
        jfw_freeJavaInfo( pInfo );
    }
/*
    rtl_uString** pParamArr = m_parParameters;
    for ( sal_Int32 i = 0; i < m_nParamSize; ++i )
        rtl_uString_release( *pParamArr++ );
    rtl_freeMemory( m_parParameters );
    rtl_uString_release( m_pClassPath );
*/
    jfw_unlock();
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxJavaOptionsPage, EnableHdl_Impl, CheckBox *, EMPTYARG )
{
    BOOL bEnable = m_aJavaEnableCB.IsChecked();
    m_aJavaFoundLabel.Enable( bEnable );
    m_aJavaPathText.Enable( bEnable );
    m_aAddBtn.Enable( bEnable );
    m_aParameterBtn.Enable( bEnable );
    m_aClassPathBtn.Enable( bEnable );

    bEnable ? m_aJavaList.Enable() : m_aJavaList.Disable();

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxJavaOptionsPage, CheckHdl_Impl, SvxSimpleTable *, pList )
{
    SvLBoxEntry* pEntry = pList ? m_aJavaList.GetEntry( m_aJavaList.GetCurMousePoint() )
                                : m_aJavaList.FirstSelected();
    HandleCheckEntry( pEntry );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxJavaOptionsPage, SelectHdl_Impl, SvxSimpleTable *, pList )
{
    // set installation directory info
    SvLBoxEntry* pEntry = m_aJavaList.FirstSelected();
    DBG_ASSERT( pEntry, "SvxJavaOptionsPage::SelectHdl_Impl(): no entry" );
    String* pLocation = static_cast< String* >( pEntry->GetUserData() );
    DBG_ASSERT( pLocation, "invalid location string" );
    String sInfo = m_sInstallText;
    sInfo += *pLocation;
    m_aJavaPathText.SetText( sInfo );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxJavaOptionsPage, AddHdl_Impl, PushButton *, EMPTYARG )
{
    rtl::OUString sService( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.dialogs.FolderPicker" ) );
    Reference < XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
    Reference < XFolderPicker > xFolderPicker( xFactory->createInstance( sService ), UNO_QUERY );

    String sWorkFolder = SvtPathOptions().GetWorkPath();
    xFolderPicker->setDisplayDirectory( sWorkFolder );
    xFolderPicker->setDescription( m_sAddDialogText );
    bool bFinished = false;
    while ( !bFinished )
    {
        if ( xFolderPicker->execute() == ExecutableDialogResults::OK )
        {
            sal_Int32 nPos = 0;
            ::rtl::OUString sFolder( xFolderPicker->getDirectory() );
            JavaInfo* pInfo = NULL;
            javaFrameworkError eErr = jfw_getJavaInfoByPath( sFolder.pData, &pInfo );
            if ( JFW_E_NONE == eErr && pInfo )
            {
                bool bFound = false;
                JavaInfo** parInfo = m_parJavaInfo;
                for ( sal_Int32 i = 0; i < m_nInfoSize; ++i )
                {
                    JavaInfo* pCmpInfo = *parInfo++;
                    if ( jfw_areEqualJavaInfo( pCmpInfo, pInfo ) )
                    {
                        bFound = true;
                        nPos = i;
                        break;
                    }
                }

                if ( !bFound )
                {
                    std::vector< JavaInfo* >::iterator pIter;
                    for ( pIter = m_aAddedInfos.begin(); pIter != m_aAddedInfos.end(); ++pIter )
                    {
                        JavaInfo* pCmpInfo = *pIter;
                        if ( jfw_areEqualJavaInfo( pCmpInfo, pInfo ) )
                        {
                            bFound = true;
                            break;
                        }
                    }
                }

                if ( !bFound )
                {
                    jfw_addJRELocation( pInfo->sLocation );
                    AddJRE( pInfo );
                    m_aAddedInfos.push_back( pInfo );
                    nPos = m_aJavaList.GetEntryCount() - 1;
                }
                else
                    jfw_freeJavaInfo( pInfo );

                SvLBoxEntry* pEntry = m_aJavaList.GetEntry( nPos );
                m_aJavaList.Select( pEntry );
                m_aJavaList.SetCheckButtonState( pEntry, SV_BUTTON_CHECKED );
                HandleCheckEntry( pEntry );
                bFinished = true;
            }
            else if ( JFW_E_NOT_RECOGNIZED == eErr )
            {
                ErrorBox aErrBox( this, SVX_RES( RID_SVXERR_JRE_NOT_RECOGNIZED ) );
                aErrBox.Execute();
            }
            else if ( JFW_E_FAILED_VERSION == eErr )
            {
                ErrorBox aErrBox( this, SVX_RES( RID_SVXERR_JRE_FAILED_VERSION ) );
                aErrBox.Execute();
            }

            if ( !bFinished )
                xFolderPicker->setDisplayDirectory( sFolder );
        }
        else
            bFinished = true;
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxJavaOptionsPage, ParameterHdl_Impl, PushButton *, EMPTYARG )
{
    Sequence< ::rtl::OUString > aParameterList;
    if ( !m_pParamDlg )
    {
        m_pParamDlg = new SvxJavaParameterDlg( this );
        javaFrameworkError eErr = jfw_getVMParameters( &m_parParameters, &m_nParamSize );
        if ( JFW_E_NONE == eErr && m_parParameters && m_nParamSize > 0 )
        {
            rtl_uString** pParamArr = m_parParameters;
            aParameterList.realloc( m_nParamSize );
            ::rtl::OUString* pParams = aParameterList.getArray();
            for ( sal_Int32 i = 0; i < m_nParamSize; ++i )
            {
                rtl_uString* pParam = *pParamArr++;
                pParams[i] = ::rtl::OUString( pParam );
            }
            m_pParamDlg->SetParameters( aParameterList );
        }
    }
    else
        aParameterList = m_pParamDlg->GetParameters();

    if ( m_pParamDlg->Execute() == RET_OK )
    {
        if ( !areListsEqual( aParameterList, m_pParamDlg->GetParameters() ) )
        {
            aParameterList = m_pParamDlg->GetParameters();
            sal_Bool bRunning = sal_False;
            javaFrameworkError eErr = jfw_isVMRunning( &bRunning );
            DBG_ASSERT( JFW_E_NONE == eErr,
                        "SvxJavaOptionsPage::ParameterHdl_Impl(): error in jfw_isVMRunning" );
            if ( bRunning )
            {
                WarningBox aWarnBox( this, SVX_RES( RID_SVX_MSGBOX_JAVA_RESTART2 ) );
                aWarnBox.Execute();
            }
        }
    }
    else
        m_pParamDlg->SetParameters( aParameterList );

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxJavaOptionsPage, ClassPathHdl_Impl, PushButton *, EMPTYARG )
{
    String sClassPath;

    if ( !m_pPathDlg )
    {
          m_pPathDlg = new SvxJavaClassPathDlg( this );
        javaFrameworkError eErr = jfw_getUserClassPath( &m_pClassPath );
        if ( JFW_E_NONE == eErr && m_pClassPath )
        {
            sClassPath = String( ::rtl::OUString( m_pClassPath ) );
            m_pPathDlg->SetClassPath( sClassPath );
        }
    }
    else
        sClassPath = m_pPathDlg->GetClassPath();

    m_pPathDlg->SetFocus();
    if ( m_pPathDlg->Execute() == RET_OK )
    {

        if ( m_pPathDlg->GetClassPath() != sClassPath )
        {
            sClassPath = m_pPathDlg->GetClassPath();
            sal_Bool bRunning = sal_False;
            javaFrameworkError eErr = jfw_isVMRunning( &bRunning );
            DBG_ASSERT( JFW_E_NONE == eErr,
                        "SvxJavaOptionsPage::ParameterHdl_Impl(): error in jfw_isVMRunning" );
            if ( bRunning )
            {
                WarningBox aWarnBox( this, SVX_RES( RID_SVX_MSGBOX_JAVA_RESTART2 ) );
                aWarnBox.Execute();
            }
        }
    }
    else
        m_pPathDlg->SetClassPath( sClassPath );

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxJavaOptionsPage, ResetHdl_Impl, Timer *, EMPTYARG )
{
    LoadJREs();
    return 0;
}

// -----------------------------------------------------------------------

void SvxJavaOptionsPage::ClearJavaInfo()
{
    if ( m_parJavaInfo )
    {
        JavaInfo** parInfo = m_parJavaInfo;
        for ( sal_Int32 i = 0; i < m_nInfoSize; ++i )
        {
            JavaInfo* pInfo = *parInfo++;
            jfw_freeJavaInfo( pInfo );
        }

        rtl_freeMemory( m_parJavaInfo );
        m_parJavaInfo = NULL;
        m_nInfoSize = 0;
    }
}

// -----------------------------------------------------------------------

void SvxJavaOptionsPage::ClearJavaList()
{
    SvLBoxEntry* pEntry = m_aJavaList.First();
    while ( pEntry )
    {
        String* pLocation = static_cast< String* >( pEntry->GetUserData() );
        delete pLocation;
        pEntry = m_aJavaList.Next( pEntry );
    }
    m_aJavaList.Clear();
}

// -----------------------------------------------------------------------

void SvxJavaOptionsPage::LoadJREs()
{
    WaitObject aWaitObj( &m_aJavaList );
    javaFrameworkError eErr = jfw_findAllJREs( &m_parJavaInfo, &m_nInfoSize );
    if ( JFW_E_NONE == eErr && m_parJavaInfo )
    {
        JavaInfo** parInfo = m_parJavaInfo;
        for ( sal_Int32 i = 0; i < m_nInfoSize; ++i )
        {
            JavaInfo* pInfo = *parInfo++;
            AddJRE( pInfo );
        }
    }

    std::vector< JavaInfo* >::iterator pIter;
    for ( pIter = m_aAddedInfos.begin(); pIter != m_aAddedInfos.end(); ++pIter )
    {
        JavaInfo* pInfo = *pIter;
        AddJRE( pInfo );
    }

    JavaInfo* pSelectedJava = NULL;
    eErr = jfw_getSelectedJRE( &pSelectedJava );
    if ( JFW_E_NONE == eErr && pSelectedJava )
    {
        JavaInfo** parInfo = m_parJavaInfo;
        for ( sal_Int32 i = 0; i < m_nInfoSize; ++i )
        {
            JavaInfo* pCmpInfo = *parInfo++;
            if ( jfw_areEqualJavaInfo( pCmpInfo, pSelectedJava ) )
            {
                SvLBoxEntry* pEntry = m_aJavaList.GetEntry(i);
                HandleCheckEntry( pEntry );
                break;
            }
        }
    }

    jfw_freeJavaInfo( pSelectedJava );
}

// -----------------------------------------------------------------------

void SvxJavaOptionsPage::AddJRE( JavaInfo* _pInfo )
{
    String sEntry( '\t' );
    sEntry += String( ::rtl::OUString( _pInfo->sVendor ) );
    sEntry += '\t';
    sEntry += String( ::rtl::OUString( _pInfo->sVersion ) );
    sEntry += '\t';
    if ( ( _pInfo->nFeatures & JFW_FEATURE_ACCESSBRIDGE ) == JFW_FEATURE_ACCESSBRIDGE )
        sEntry += m_sAccessibilityText;
    SvLBoxEntry* pEntry = m_aJavaList.InsertEntry( sEntry );
    INetURLObject aLocObj( ::rtl::OUString( _pInfo->sLocation ) );
    String* pLocation = new String( aLocObj.getFSysPath( INetURLObject::FSYS_DETECT ) );
    pEntry->SetUserData( pLocation );
}

// -----------------------------------------------------------------------

void SvxJavaOptionsPage::HandleCheckEntry( SvLBoxEntry* _pEntry )
{
    m_aJavaList.Select( _pEntry, TRUE );
    SvButtonState eState = m_aJavaList.GetCheckButtonState( _pEntry );

    if ( SV_BUTTON_CHECKED == eState )
    {
        // we have radio button behavior -> so uncheck the other entries
        SvLBoxEntry* pEntry = m_aJavaList.First();
        while ( pEntry )
        {
            if ( pEntry != _pEntry )
                m_aJavaList.SetCheckButtonState( pEntry, SV_BUTTON_UNCHECKED );
            pEntry = m_aJavaList.Next( pEntry );
        }
    }
    else
        m_aJavaList.SetCheckButtonState( _pEntry, SV_BUTTON_CHECKED );
}

// -----------------------------------------------------------------------

SfxTabPage* SvxJavaOptionsPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return ( new SvxJavaOptionsPage( pParent, rAttrSet ) );
}

// -----------------------------------------------------------------------

BOOL SvxJavaOptionsPage::FillItemSet( SfxItemSet& rCoreSet )
{
    BOOL bModified = FALSE;
    javaFrameworkError eErr = JFW_E_NONE;
    if ( m_pParamDlg )
    {
        Sequence< ::rtl::OUString > aParamList = m_pParamDlg->GetParameters();
        sal_Int32 i, nSize = aParamList.getLength();
        rtl_uString** pParamArr = (rtl_uString**)rtl_allocateMemory( sizeof(rtl_uString*) * nSize );
        rtl_uString** pParamArrIter = pParamArr;
        const ::rtl::OUString* pList = aParamList.getConstArray();
        for ( i = 0; i < nSize; ++i )
            pParamArr[i] = pList[i].pData;
        eErr = jfw_setVMParameters( pParamArrIter, nSize );
        DBG_ASSERT( JFW_E_NONE == eErr,
                    "SvxJavaOptionsPage::FillItemSet(): error in jfw_setVMParameters" );
        pParamArrIter = pParamArr;
        rtl_freeMemory( pParamArr );
        bModified = TRUE;
    }

    if ( m_pPathDlg )
    {
        ::rtl::OUString sPath( m_pPathDlg->GetClassPath() );
        if ( m_pPathDlg->GetOldPath() != String( sPath ) )
        {
            eErr = jfw_setUserClassPath( sPath.pData );
            DBG_ASSERT( JFW_E_NONE == eErr,
                        "SvxJavaOptionsPage::FillItemSet(): error in jfw_setUserClassPath" );
            bModified = TRUE;
        }
    }

    ULONG nCount = m_aJavaList.GetEntryCount();
    for ( ULONG i = 0; i < nCount; ++i )
    {
        if ( m_aJavaList.GetCheckButtonState( m_aJavaList.GetEntry(i) ) == SV_BUTTON_CHECKED )
        {
            JavaInfo* pInfo = NULL;
            if ( i < static_cast< ULONG >( m_nInfoSize ) )
                pInfo = m_parJavaInfo[i];
            else
                pInfo = m_aAddedInfos[ i - m_nInfoSize ];

            JavaInfo* pSelectedJava = NULL;
            eErr = jfw_getSelectedJRE( &pSelectedJava );
            if ( JFW_E_NONE == eErr || JFW_E_INVALID_SETTINGS == eErr )
            {
                if (pSelectedJava == NULL || !jfw_areEqualJavaInfo( pInfo, pSelectedJava ) )
                {
                    sal_Bool bRunning = sal_False;
                    eErr = jfw_isVMRunning( &bRunning );
                    DBG_ASSERT( JFW_E_NONE == eErr,
                                "SvxJavaOptionsPage::FillItemSet(): error in jfw_isVMRunning" );
                    if ( bRunning ||
                        ( ( pInfo->nRequirements & JFW_REQUIRE_NEEDRESTART ) == JFW_REQUIRE_NEEDRESTART ) )
                    {
                        WarningBox aWarnBox( this, SVX_RES( RID_SVX_MSGBOX_JAVA_RESTART ) );
                        aWarnBox.Execute();
                    }

                    eErr = jfw_setSelectedJRE( pInfo );
                    DBG_ASSERT( JFW_E_NONE == eErr,
                                "SvxJavaOptionsPage::FillItemSet(): error in jfw_setSelectedJRE" );
                    bModified = TRUE;
                }
            }
            jfw_freeJavaInfo( pSelectedJava );
            break;
        }
    }

    sal_Bool bEnabled = sal_False;
    eErr = jfw_getEnabled( &bEnabled );
    DBG_ASSERT( JFW_E_NONE == eErr,
                "SvxJavaOptionsPage::FillItemSet(): error in jfw_getEnabled" );
    if ( bEnabled != m_aJavaEnableCB.IsChecked() )
    {
        eErr = jfw_setEnabled( m_aJavaEnableCB.IsChecked() );
        DBG_ASSERT( JFW_E_NONE == eErr,
                    "SvxJavaOptionsPage::FillItemSet(): error in jfw_setEnabled" );
        bModified = TRUE;
    }

    return bModified;
}

// -----------------------------------------------------------------------

void SvxJavaOptionsPage::Reset( const SfxItemSet& rSet )
{
    ClearJavaInfo();
    ClearJavaList();

    sal_Bool bEnabled = sal_False;
    javaFrameworkError eErr = jfw_getEnabled( &bEnabled );
    if ( eErr != JFW_E_NONE )
        bEnabled = sal_False;
    m_aJavaEnableCB.Check( bEnabled );
    EnableHdl_Impl( &m_aJavaEnableCB );

    m_aResetTimer.Start();
}

// -----------------------------------------------------------------------

void SvxJavaOptionsPage::FillUserData()
{
    String aUserData;
    SetUserData( aUserData );
}

// class SvxJavaParameterDlg ---------------------------------------------

SvxJavaParameterDlg::SvxJavaParameterDlg( Window* pParent ) :

    ModalDialog( pParent, SVX_RES( RID_SVXDLG_JAVA_PARAMETER ) ),

    m_aParameterLabel   ( this, ResId( FT_PARAMETER ) ),
    m_aParameterEdit    ( this, ResId( ED_PARAMETER ) ),
    m_aAssignBtn        ( this, ResId( PB_ASSIGN ) ),
    m_aAssignedLabel    ( this, ResId( FT_ASSIGNED ) ),
    m_aAssignedList     ( this, ResId( LB_ASSIGNED ) ),
    m_aExampleText      ( this, ResId( FT_EXAMPLE ) ),
    m_aRemoveBtn        ( this, ResId( PB_REMOVE ) ),
    m_aButtonsLine      ( this, ResId( FL_BUTTONS ) ),
    m_aOKBtn            ( this, ResId( PB_PARAMETER_OK ) ),
    m_aCancelBtn        ( this, ResId( PB_PARAMETER_ESC ) ),
    m_aHelpBtn          ( this, ResId( PB_PARAMETER_HLP ) )

{
    FreeResource();

    m_aParameterEdit.SetModifyHdl( LINK( this, SvxJavaParameterDlg, ModifyHdl_Impl ) );
    m_aAssignBtn.SetClickHdl( LINK( this, SvxJavaParameterDlg, AssignHdl_Impl ) );
    m_aRemoveBtn.SetClickHdl( LINK( this, SvxJavaParameterDlg, RemoveHdl_Impl ) );
    m_aAssignedList.SetSelectHdl( LINK( this, SvxJavaParameterDlg, SelectHdl_Impl ) );
    m_aAssignedList.SetDoubleClickHdl( LINK( this, SvxJavaParameterDlg, DblClickHdl_Impl ) );

    ModifyHdl_Impl( &m_aParameterEdit );
    EnableRemoveButton();
}

// -----------------------------------------------------------------------

SvxJavaParameterDlg::~SvxJavaParameterDlg()
{
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxJavaParameterDlg, ModifyHdl_Impl, Edit *, EMPTYARG )
{
    String sParam = STRIM( m_aParameterEdit.GetText() );
    m_aAssignBtn.Enable( sParam.Len() > 0 );

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxJavaParameterDlg, AssignHdl_Impl, PushButton *, EMPTYARG )
{
    String sParam = STRIM( m_aParameterEdit.GetText() );
    if ( sParam.Len() > 0 )
    {
        USHORT nPos = m_aAssignedList.GetEntryPos( sParam );
        if ( LISTBOX_ENTRY_NOTFOUND == nPos )
            nPos = m_aAssignedList.InsertEntry( sParam );
        m_aAssignedList.SelectEntryPos( nPos );
        m_aParameterEdit.SetText( String() );
        ModifyHdl_Impl( &m_aParameterEdit );
        EnableRemoveButton();
    }

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxJavaParameterDlg, SelectHdl_Impl, ListBox *, EMPTYARG )
{
    EnableRemoveButton();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxJavaParameterDlg, DblClickHdl_Impl, ListBox *, EMPTYARG )
{
    USHORT nPos = m_aAssignedList.GetSelectEntryPos();
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        m_aParameterEdit.SetText( m_aAssignedList.GetEntry( nPos ) );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxJavaParameterDlg, RemoveHdl_Impl, PushButton *, EMPTYARG )
{
    USHORT nPos = m_aAssignedList.GetSelectEntryPos();
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        m_aAssignedList.RemoveEntry( nPos );
        USHORT nCount = m_aAssignedList.GetEntryCount();
        if ( nCount )
        {
            if ( nPos >= nCount )
                nPos = ( nCount - 1 );
            m_aAssignedList.SelectEntryPos( nPos );
        }
    }
    EnableRemoveButton();

    return 0;
}

// -----------------------------------------------------------------------

short SvxJavaParameterDlg::Execute()
{
    m_aParameterEdit.GrabFocus();
    m_aAssignedList.SetNoSelection();
    return ModalDialog::Execute();
}

// -----------------------------------------------------------------------

Sequence< ::rtl::OUString > SvxJavaParameterDlg::GetParameters() const
{
    USHORT nCount = m_aAssignedList.GetEntryCount();
    Sequence< ::rtl::OUString > aParamList( nCount );
    ::rtl::OUString* pArray = aParamList.getArray();
     for ( USHORT i = 0; i < nCount; ++i )
         pArray[i] = ::rtl::OUString( m_aAssignedList.GetEntry(i) );
    return aParamList;
}

// -----------------------------------------------------------------------

void SvxJavaParameterDlg::SetParameters( Sequence< ::rtl::OUString >& rParams )
{
    m_aAssignedList.Clear();
    ULONG i, nCount = rParams.getLength();
    const ::rtl::OUString* pArray = rParams.getConstArray();
    for ( i = 0; i < nCount; ++i )
    {
        String sParam = String( *pArray++ );
        m_aAssignedList.InsertEntry( sParam );
    }
}

// class SvxJavaClassPathDlg ---------------------------------------------

SvxJavaClassPathDlg::SvxJavaClassPathDlg( Window* pParent ) :

    ModalDialog( pParent, SVX_RES( RID_SVXDLG_JAVA_CLASSPATH ) ),

    m_aPathLabel        ( this, ResId( FT_PATH ) ),
    m_aPathList         ( this, ResId( LB_PATH ) ),
    m_aAddArchiveBtn    ( this, ResId( PB_ADDARCHIVE ) ),
    m_aAddPathBtn       ( this, ResId( PB_ADDPATH ) ),
    m_aRemoveBtn        ( this, ResId( PB_REMOVE_PATH ) ),
    m_aButtonsLine      ( this, ResId( FL_PATH_BUTTONS ) ),
    m_aOKBtn            ( this, ResId( PB_PATH_OK ) ),
    m_aCancelBtn        ( this, ResId( PB_PATH_ESC ) ),
    m_aHelpBtn          ( this, ResId( PB_PATH_HLP ) )

{
    FreeResource();

    m_aAddArchiveBtn.SetClickHdl( LINK( this, SvxJavaClassPathDlg, AddArchiveHdl_Impl ) );
    m_aAddPathBtn.SetClickHdl( LINK( this, SvxJavaClassPathDlg, AddPathHdl_Impl ) );
    m_aRemoveBtn.SetClickHdl( LINK( this, SvxJavaClassPathDlg, RemoveHdl_Impl ) );

    // check if the buttons text are not too wide otherwise we have to stretch the buttons
    // and shrink the listbox
    long nTxtWidth1 = m_aAddArchiveBtn.GetTextWidth( m_aAddArchiveBtn.GetText() );
    long nTxtWidth2 = m_aAddPathBtn.GetTextWidth( m_aAddPathBtn.GetText() );
    Size aBtnSz = m_aAddArchiveBtn.GetSizePixel();
    if ( nTxtWidth1 > aBtnSz.Width() || nTxtWidth2 > aBtnSz.Width() )
    {
        long nW = ( nTxtWidth1 > aBtnSz.Width() ) ? nTxtWidth1 : nTxtWidth2;
        long nDelta = nW - aBtnSz.Width() + 2 * BUTTON_BORDER;
        aBtnSz.Width() += nDelta;
        Point aBtnPnt = m_aAddArchiveBtn.GetPosPixel();
        aBtnPnt.X() -= nDelta;
        m_aAddArchiveBtn.SetPosSizePixel( aBtnPnt, aBtnSz );
        aBtnPnt = m_aAddPathBtn.GetPosPixel();
        aBtnPnt.X() -= nDelta;
        m_aAddPathBtn.SetPosSizePixel( aBtnPnt, aBtnSz );
        aBtnPnt = m_aRemoveBtn.GetPosPixel();
        aBtnPnt.X() -= nDelta;
        m_aRemoveBtn.SetPosSizePixel( aBtnPnt, aBtnSz );
        Size aBoxSz = m_aPathList.GetSizePixel();
        aBoxSz.Width() -= nDelta;
        m_aPathList.SetSizePixel( aBoxSz );
    }

    // set initial focus to path list
    m_aPathList.GrabFocus();
}

// -----------------------------------------------------------------------

SvxJavaClassPathDlg::~SvxJavaClassPathDlg()
{
    USHORT i, nCount = m_aPathList.GetEntryCount();
    for ( i = 0; i < nCount; ++i )
        delete static_cast< String* >( m_aPathList.GetEntryData(i) );
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxJavaClassPathDlg, AddArchiveHdl_Impl, PushButton *, EMPTYARG )
{
    sfx2::FileDialogHelper aDlg( ::sfx2::FILEOPEN_SIMPLE, 0 );
    aDlg.SetTitle( SVX_RES( RID_SVXSTR_ARCHIVE_TITLE ) );
    aDlg.AddFilter( SVX_RES( RID_SVXSTR_ARCHIVE_HEADLINE ), String::CreateFromAscii("*.jar;*.zip") );
    if ( aDlg.Execute() == ERRCODE_NONE )
    {
        String sURL = aDlg.GetPath();
        INetURLObject aURL( sURL );
        String sFile = aURL.getFSysPath( INetURLObject::FSYS_DETECT );
        if ( !IsPathDuplicate( sURL ) )
            m_aPathList.InsertEntry( sFile, SvFileInformationManager::GetImage( aURL ) );
        else
        {
            String sMsg( SVX_RES( RID_SVXSTR_MULTIFILE_DBL_ERR ) );
            sMsg.SearchAndReplaceAscii( "%1", sFile );
            ErrorBox( this, WB_OK, sMsg ).Execute();
        }
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxJavaClassPathDlg, AddPathHdl_Impl, PushButton *, EMPTYARG )
{
    rtl::OUString sService( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.dialogs.FolderPicker" ) );
    Reference < XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
    Reference < XFolderPicker > xFolderPicker( xFactory->createInstance( sService ), UNO_QUERY );

    String sWorkFolder = SvtPathOptions().GetWorkPath();
    xFolderPicker->setDisplayDirectory( sWorkFolder );
    if ( xFolderPicker->execute() == ExecutableDialogResults::OK )
    {
        String sFolderURL( xFolderPicker->getDirectory() );
        INetURLObject aURL( sFolderURL );
        String sFolder = aURL.getFSysPath( INetURLObject::FSYS_DETECT );
        if ( !IsPathDuplicate( sFolderURL ) )
            m_aPathList.InsertEntry( sFolder, SvFileInformationManager::GetImage( aURL ) );
        else
        {
            String sMsg( SVX_RES( RID_SVXSTR_MULTIFILE_DBL_ERR ) );
            sMsg.SearchAndReplaceAscii( "%1", sFolder );
            ErrorBox( this, WB_OK, sMsg ).Execute();
        }
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxJavaClassPathDlg, RemoveHdl_Impl, PushButton *, EMPTYARG )
{
    USHORT nPos = m_aPathList.GetSelectEntryPos();
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        m_aPathList.RemoveEntry( nPos );
        USHORT nCount = m_aPathList.GetEntryCount();
        if ( nCount )
        {
            if ( nPos >= nCount )
                nPos = ( nCount - 1 );
            m_aPathList.SelectEntryPos( nPos );
        }
    }

    return 0;
}

// -----------------------------------------------------------------------

bool SvxJavaClassPathDlg::IsPathDuplicate( const String& _rPath )
{
    bool bRet = false;
    INetURLObject aFileObj( _rPath );
    USHORT nCount = m_aPathList.GetEntryCount();
    for ( USHORT i = 0; i < nCount; ++i )
    {
        INetURLObject aOtherObj( m_aPathList.GetEntry(i), INetURLObject::FSYS_DETECT );
        if ( aOtherObj == aFileObj )
        {
            bRet = true;
            break;
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------

String SvxJavaClassPathDlg::GetClassPath() const
{
    String sPath;
    USHORT nCount = m_aPathList.GetEntryCount();
    for ( USHORT i = 0; i < nCount; ++i )
    {
        if ( sPath.Len() > 0 )
            sPath += CLASSPATH_DELIMITER;
        String* pFullPath = static_cast< String* >( m_aPathList.GetEntryData(i) );
        if ( pFullPath )
            sPath += *pFullPath;
        else
            sPath += m_aPathList.GetEntry(i);
    }
    return sPath;
}

// -----------------------------------------------------------------------

void SvxJavaClassPathDlg::SetClassPath( const String& _rPath )
{
    if ( m_sOldPath.Len() == 0 )
        m_sOldPath = _rPath;
    m_aPathList.Clear();
    xub_StrLen i, nIdx = 0;
    xub_StrLen nCount = _rPath.GetTokenCount( CLASSPATH_DELIMITER );
    for ( i = 0; i < nCount; ++i )
    {
        String sToken = _rPath.GetToken( 0, CLASSPATH_DELIMITER, nIdx );
        INetURLObject aURL( sToken, INetURLObject::FSYS_DETECT );
        String sPath = aURL.getFSysPath( INetURLObject::FSYS_DETECT );
        m_aPathList.InsertEntry( sPath, SvFileInformationManager::GetImage( aURL ) );
    }
    // select first entry
    m_aPathList.SelectEntryPos(0);
}

