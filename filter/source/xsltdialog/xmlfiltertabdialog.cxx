/*************************************************************************
 *
 *  $RCSfile: xmlfiltertabdialog.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-04 16:34:39 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _TOOLS_RESID_HXX
#include <tools/resid.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

#include "xmlfilterdialogstrings.hrc"
#include "xmlfiltertabdialog.hxx"
#include "xmlfiltertabdialog.hrc"
#include "xmlfiltertabpagebasic.hrc"
#include "xmlfiltertabpagexslt.hrc"
#include "xmlfiltertabpagebasic.hxx"
#include "xmlfiltertabpagexslt.hxx"
#include "xmlfiltersettingsdialog.hxx"
#include "xmlfiltersettingsdialog.hrc"
#include "xmlfilterhelpids.hrc"

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;

XMLFilterTabDialog::XMLFilterTabDialog( Window *pParent, ResMgr& rResMgr, const Reference< XMultiServiceFactory >& rxMSF, const filter_info_impl* pInfo ) :
    TabDialog( pParent, ResId( DLG_XML_FILTER_TABDIALOG, &rResMgr ) ),
    mxMSF( rxMSF ),
    mrResMgr( rResMgr ),
    maTabCtrl( this, ResId( 1, &rResMgr ) ),
    maOKBtn( this ),
    maCancelBtn( this ),
    maHelpBtn( this )
{
    FreeResource();

    maTabCtrl.SetHelpId( HID_XML_FILTER_TABPAGE_CTRL );

    mpOldInfo = pInfo;
    mpNewInfo = new filter_info_impl( *mpOldInfo );

    String aTitle( GetText() );
    aTitle.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM("%s") ), mpNewInfo->maFilterName );
    SetText( aTitle );

    maTabCtrl.Show();
    maOKBtn.Show();
    maCancelBtn.Show();
    maHelpBtn.Show();

    maOKBtn.SetClickHdl( LINK( this, XMLFilterTabDialog, OkHdl ) );

    maTabCtrl.SetActivatePageHdl( LINK( this, XMLFilterTabDialog, ActivatePageHdl ) );
    maTabCtrl.SetDeactivatePageHdl( LINK( this, XMLFilterTabDialog, DeactivatePageHdl ) );

    mpBasicPage = new XMLFilterTabPageBasic( &maTabCtrl, mrResMgr );
    mpBasicPage->SetInfo( mpNewInfo );

    maTabCtrl.SetTabPage( RID_XML_FILTER_TABPAGE_BASIC, mpBasicPage );

    Size aSiz = mpBasicPage->GetSizePixel();
    Size aCtrlSiz = maTabCtrl.GetOutputSizePixel();
    // set size on TabControl only if smaller than TabPage
    if ( aCtrlSiz.Width() < aSiz.Width() || aCtrlSiz.Height() < aSiz.Height() )
    {
        maTabCtrl.SetOutputSizePixel( aSiz );
        aCtrlSiz = aSiz;
    }

    mpXSLTPage = new XMLFilterTabPageXSLT( &maTabCtrl, mrResMgr, mxMSF );
    mpXSLTPage->SetInfo( mpNewInfo );

    maTabCtrl.SetTabPage( RID_XML_FILTER_TABPAGE_XSLT, mpXSLTPage );

    aSiz = mpXSLTPage->GetSizePixel();
    if ( aCtrlSiz.Width() < aSiz.Width() || aCtrlSiz.Height() < aSiz.Height() )
    {
        maTabCtrl.SetOutputSizePixel( aSiz );
        aCtrlSiz = aSiz;
    }

    ActivatePageHdl( &maTabCtrl );

    AdjustLayout();
}

// -----------------------------------------------------------------------

XMLFilterTabDialog::~XMLFilterTabDialog()
{
    delete mpBasicPage;
    delete mpXSLTPage;
    delete mpNewInfo;
}

// -----------------------------------------------------------------------

bool XMLFilterTabDialog::onOk()
{
    mpXSLTPage->FillInfo( mpNewInfo );
    mpBasicPage->FillInfo( mpNewInfo );

    sal_uInt16 nErrorPage = 0;
    sal_uInt16 nErrorId = 0;
    Window* pFocusWindow = NULL;
    String aReplace1;
    String aReplace2;

    // 1. see if the filter name is ok
    if( (mpNewInfo->maFilterName.getLength() == 0) || (mpNewInfo->maFilterName != mpOldInfo->maFilterName) )
    {
        // if the user deleted the filter name, we reset the original filter name
        if( mpNewInfo->maFilterName.getLength() == 0 )
        {
            mpNewInfo->maFilterName = mpOldInfo->maFilterName;
        }
        else
        {
            try
            {
                Reference< XNameAccess > xFilterContainer( mxMSF->createInstance( OUString::createFromAscii("com.sun.star.document.FilterFactory" ) ), UNO_QUERY );
                if( xFilterContainer.is() )
                {
                    if( xFilterContainer->hasByName( mpNewInfo->maFilterName ) )
                    {
                        nErrorPage = RID_XML_FILTER_TABPAGE_BASIC;
                        nErrorId = STR_ERROR_FILTER_NAME_EXISTS;
                        pFocusWindow = &(mpBasicPage->maEDFilterName);
                        aReplace1 = mpNewInfo->maFilterName;
                    }

                }
            }
            catch( Exception& )
            {
                DBG_ERROR( "XMLFilterTabDialog::onOk exception catched!" );
            }
        }
    }

    // 2. see if the interface name is ok
    if( (mpNewInfo->maInterfaceName.getLength() == 0) || (mpNewInfo->maInterfaceName != mpOldInfo->maInterfaceName) )
    {
        // if the user deleted the interface name, we reset the original filter name
        if( mpNewInfo->maInterfaceName.getLength() == 0 )
        {
            mpNewInfo->maInterfaceName = mpOldInfo->maInterfaceName;
        }
        else
        {
            try
            {
                Reference< XNameAccess > xFilterContainer( mxMSF->createInstance( OUString::createFromAscii("com.sun.star.document.FilterFactory" ) ), UNO_QUERY );
                if( xFilterContainer.is() )
                {
                    Sequence< OUString > aFilterNames( xFilterContainer->getElementNames() );
                    OUString* pFilterName = aFilterNames.getArray();

                    const sal_Int32 nCount = aFilterNames.getLength();
                    sal_Int32 nFilter;

                    Sequence< PropertyValue > aValues;
                    for( nFilter = 0; (nFilter < nCount) && (nErrorId == 0); nFilter++, pFilterName++ )
                    {
                        Any aAny( xFilterContainer->getByName( *pFilterName ) );
                        if( !(aAny >>= aValues) )
                            continue;

                        const sal_Int32 nValueCount( aValues.getLength() );
                        PropertyValue* pValues = aValues.getArray();
                        sal_Int32 nValue;

                        for( nValue = 0; (nValue < nValueCount) && (nErrorId == 0); nValue++, pValues++ )
                        {
                            if( pValues->Name.equalsAscii( "UIName" ) )
                            {
                                OUString aInterfaceName;
                                pValues->Value >>= aInterfaceName;
                                if( aInterfaceName == mpNewInfo->maInterfaceName )
                                {
                                    nErrorPage = RID_XML_FILTER_TABPAGE_BASIC;
                                    nErrorId = STR_ERROR_TYPE_NAME_EXISTS;
                                    pFocusWindow = &(mpBasicPage->maEDInterfaceName);
                                    aReplace1 = mpNewInfo->maInterfaceName;
                                    aReplace2 = *pFilterName;
                                }
                            }
                        }
                    }
                }
            }
            catch( Exception& )
            {
                DBG_ERROR( "XMLFilterTabDialog::onOk exception catched!" );
            }
        }
    }

    // 3. see if the dtd is valid
    if( 0 == nErrorId )
    {
        if( (mpNewInfo->maDTD != mpOldInfo->maDTD) && isFileURL( mpNewInfo->maDTD ) )
        {
            osl::File aFile( mpNewInfo->maDTD );
            osl::File::RC aRC = aFile.open( OpenFlag_Read );
            if( aRC != osl::File::E_None )
            {
                nErrorId = STR_ERROR_DTD_NOT_FOUND;
                nErrorPage = RID_XML_FILTER_TABPAGE_XSLT;
                pFocusWindow = &(mpXSLTPage->maEDDTDSchema);
            }
        }
    }

    if( 0 == nErrorId )
    {
        // 4. see if the export xslt is valid
        if( (mpNewInfo->maExportXSLT != mpOldInfo->maExportXSLT) && isFileURL( mpNewInfo->maExportXSLT ) )
        {
            osl::File aFile( mpNewInfo->maExportXSLT );
            osl::File::RC aRC = aFile.open( OpenFlag_Read );
            if( aRC != osl::File::E_None )
            {
                nErrorId = STR_ERROR_EXPORT_XSLT_NOT_FOUND;
                nErrorPage = RID_XML_FILTER_TABPAGE_XSLT;
                pFocusWindow = &(mpXSLTPage->maEDExportXSLT);
            }
        }
    }

    if( 0 == nErrorId )
    {
        // 5. see if the import xslt is valid
        if( (mpNewInfo->maImportXSLT != mpOldInfo->maImportXSLT) && isFileURL( mpNewInfo->maImportXSLT ) )
        {
            osl::File aFile( mpNewInfo->maImportXSLT );
            osl::File::RC aRC = aFile.open( OpenFlag_Read );
            if( aRC != osl::File::E_None )
            {
                nErrorId = STR_ERROR_IMPORT_XSLT_NOT_FOUND;
                nErrorPage = RID_XML_FILTER_TABPAGE_XSLT;
                pFocusWindow = &(mpXSLTPage->maEDImportTemplate);
            }
        }
    }

    if( 0 == nErrorId )
    {
        // 6. see if the import template is valid
        if( (mpNewInfo->maImportTemplate != mpOldInfo->maImportTemplate) && isFileURL( mpNewInfo->maImportTemplate ) )
        {
            osl::File aFile( mpNewInfo->maImportTemplate );
            osl::File::RC aRC = aFile.open( OpenFlag_Read );
            if( aRC != osl::File::E_None )
            {
                nErrorId = STR_ERROR_IMPORT_TEMPLATE_NOT_FOUND;
                nErrorPage = RID_XML_FILTER_TABPAGE_XSLT;
                pFocusWindow = &(mpXSLTPage->maEDImportTemplate);
            }
        }
    }

    if( 0 != nErrorId )
    {
        maTabCtrl.SetCurPageId( (USHORT)nErrorPage );
        ActivatePageHdl( &maTabCtrl );

        ResId aResId( nErrorId );
        String aMessage( aResId );

        if( aReplace2.Len() )
        {
            aMessage.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM("%s1") ), aReplace1 );
            aMessage.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM("%s2") ), aReplace2 );
        }
        else if( aReplace1.Len() )
        {
            aMessage.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM("%s") ), aReplace1 );
        }

        ErrorBox aBox(this, (WinBits)(WB_OK), aMessage );
        aBox.Execute();

        if( pFocusWindow )
            pFocusWindow->GrabFocus();

        return false;
    }
    else
    {
        return true;
    }
}

// -----------------------------------------------------------------------

filter_info_impl* XMLFilterTabDialog::getNewFilterInfo() const
{
    return mpNewInfo;
}

// -----------------------------------------------------------------------

IMPL_LINK( XMLFilterTabDialog, CancelHdl, Button*, pButton )
{
    Close();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( XMLFilterTabDialog, OkHdl, Button *, EMPTYARG )
{
    if( onOk() )
        EndDialog(1);

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( XMLFilterTabDialog, ActivatePageHdl, TabControl *, pTabCtrl )
{
    const USHORT nId = pTabCtrl->GetCurPageId();
    TabPage* pTabPage = pTabCtrl->GetTabPage( nId );
    pTabPage->Show();

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( XMLFilterTabDialog, DeactivatePageHdl, TabControl *, pTabCtrl )
{
    return TRUE;
}
