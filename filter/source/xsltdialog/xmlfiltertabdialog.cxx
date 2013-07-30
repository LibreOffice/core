/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <tools/resid.hxx>
#include <vcl/msgbox.hxx>
#include <osl/file.hxx>

#include "xmlfilterdialogstrings.hrc"
#include "xmlfiltertabdialog.hxx"
#include "xmlfiltertabpagebasic.hxx"
#include "xmlfiltertabpagexslt.hxx"
#include "xmlfiltersettingsdialog.hxx"

using namespace com::sun::star::uno;
using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;

XMLFilterTabDialog::XMLFilterTabDialog(Window *pParent, ResMgr& rResMgr,
    const Reference< XComponentContext >& rxContext, const filter_info_impl* pInfo)
    : TabDialog(pParent, "XSLTFilterDialog","filter/ui/xsltfilterdialog.ui")
    , mxContext(rxContext)
    , mrResMgr(rResMgr)
{
    get(m_pOKBtn, "ok");
    get(m_pTabCtrl, "tabcontrol");

    mpOldInfo = pInfo;
    mpNewInfo = new filter_info_impl( *mpOldInfo );

    OUString aTitle( GetText() );
    aTitle = aTitle.replaceAll("%s", mpNewInfo->maFilterName);
    SetText( aTitle );

    m_pOKBtn->SetClickHdl( LINK( this, XMLFilterTabDialog, OkHdl ) );

    m_pTabCtrl->SetActivatePageHdl( LINK( this, XMLFilterTabDialog, ActivatePageHdl ) );
    m_pTabCtrl->SetDeactivatePageHdl( LINK( this, XMLFilterTabDialog, DeactivatePageHdl ) );

    mpBasicPage = new XMLFilterTabPageBasic(m_pTabCtrl);
    mpBasicPage->SetInfo( mpNewInfo );

    m_nBasicPageId = m_pTabCtrl->GetPageId("general");
    m_pTabCtrl->SetTabPage(m_nBasicPageId, mpBasicPage);

    mpXSLTPage = new XMLFilterTabPageXSLT(m_pTabCtrl);
    mpXSLTPage->SetInfo( mpNewInfo );

    m_nXSLTPageId = m_pTabCtrl->GetPageId("transformation");
    m_pTabCtrl->SetTabPage(m_nXSLTPageId, mpXSLTPage);

    ActivatePageHdl(m_pTabCtrl);
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
    OUString aReplace1;
    OUString aReplace2;

    // 1. see if the filter name is ok
    if( (mpNewInfo->maFilterName.isEmpty()) || (mpNewInfo->maFilterName != mpOldInfo->maFilterName) )
    {
        // if the user deleted the filter name, we reset the original filter name
        if( mpNewInfo->maFilterName.isEmpty() )
        {
            mpNewInfo->maFilterName = mpOldInfo->maFilterName;
        }
        else
        {
            try
            {
                Reference< XNameAccess > xFilterContainer( mxContext->getServiceManager()->createInstanceWithContext( "com.sun.star.document.FilterFactory", mxContext ), UNO_QUERY );
                if( xFilterContainer.is() )
                {
                    if( xFilterContainer->hasByName( mpNewInfo->maFilterName ) )
                    {
                        nErrorPage = m_nBasicPageId;
                        nErrorId = STR_ERROR_FILTER_NAME_EXISTS;
                        pFocusWindow = (mpBasicPage->m_pEDFilterName);
                        aReplace1 = mpNewInfo->maFilterName;
                    }

                }
            }
            catch( const Exception& )
            {
                OSL_FAIL( "XMLFilterTabDialog::onOk exception catched!" );
            }
        }
    }

    // 2. see if the interface name is ok
    if( (mpNewInfo->maInterfaceName.isEmpty()) || (mpNewInfo->maInterfaceName != mpOldInfo->maInterfaceName) )
    {
        // if the user deleted the interface name, we reset the original filter name
        if( mpNewInfo->maInterfaceName.isEmpty() )
        {
            mpNewInfo->maInterfaceName = mpOldInfo->maInterfaceName;
        }
        else
        {
            try
            {
                Reference< XNameAccess > xFilterContainer( mxContext->getServiceManager()->createInstanceWithContext( "com.sun.star.document.FilterFactory", mxContext ), UNO_QUERY );
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
                            if ( pValues->Name == "UIName" )
                            {
                                OUString aInterfaceName;
                                pValues->Value >>= aInterfaceName;
                                if( aInterfaceName == mpNewInfo->maInterfaceName )
                                {
                                    nErrorPage = m_nBasicPageId;
                                    nErrorId = STR_ERROR_TYPE_NAME_EXISTS;
                                    pFocusWindow = (mpBasicPage->m_pEDInterfaceName);
                                    aReplace1 = mpNewInfo->maInterfaceName;
                                    aReplace2 = *pFilterName;
                                }
                            }
                        }
                    }
                }
            }
            catch( const Exception& )
            {
                OSL_FAIL( "XMLFilterTabDialog::onOk exception catched!" );
            }
        }
    }

    if( 0 == nErrorId )
    {
        // 4. see if the export xslt is valid
        if( (mpNewInfo->maExportXSLT != mpOldInfo->maExportXSLT) && isFileURL( mpNewInfo->maExportXSLT ) )
        {
            osl::File aFile( mpNewInfo->maExportXSLT );
            osl::File::RC aRC = aFile.open( osl_File_OpenFlag_Read );
            if( aRC != osl::File::E_None )
            {
                nErrorId = STR_ERROR_EXPORT_XSLT_NOT_FOUND;
                nErrorPage = m_nXSLTPageId;
                pFocusWindow = (mpXSLTPage->m_pEDExportXSLT);
            }
        }
    }

    if( 0 == nErrorId )
    {
        // 5. see if the import xslt is valid
        if( (mpNewInfo->maImportXSLT != mpOldInfo->maImportXSLT) && isFileURL( mpNewInfo->maImportXSLT ) )
        {
            osl::File aFile( mpNewInfo->maImportXSLT );
            osl::File::RC aRC = aFile.open( osl_File_OpenFlag_Read );
            if( aRC != osl::File::E_None )
            {
                nErrorId = STR_ERROR_IMPORT_XSLT_NOT_FOUND;
                nErrorPage = m_nXSLTPageId;
                pFocusWindow = (mpXSLTPage->m_pEDImportTemplate);
            }
        }
    }

    // see if we have at least an import or an export xslt
    if((mpNewInfo->maImportXSLT.isEmpty()) && (mpNewInfo->maExportXSLT.isEmpty()) )
    {
        nErrorId = STR_ERROR_EXPORT_XSLT_NOT_FOUND;
        nErrorPage = m_nXSLTPageId;
        pFocusWindow = (mpXSLTPage->m_pEDExportXSLT);
    }

    if( 0 == nErrorId )
    {
        // 6. see if the import template is valid
        if( (mpNewInfo->maImportTemplate != mpOldInfo->maImportTemplate) && isFileURL( mpNewInfo->maImportTemplate ) )
        {
            osl::File aFile( mpNewInfo->maImportTemplate );
            osl::File::RC aRC = aFile.open( osl_File_OpenFlag_Read );
            if( aRC != osl::File::E_None )
            {
                nErrorId = STR_ERROR_IMPORT_TEMPLATE_NOT_FOUND;
                nErrorPage = m_nXSLTPageId;
                pFocusWindow = (mpXSLTPage->m_pEDImportTemplate);
            }
        }
    }

    if( 0 != nErrorId )
    {
        m_pTabCtrl->SetCurPageId((sal_uInt16)nErrorPage);
        ActivatePageHdl(m_pTabCtrl);

        ResId aResId( nErrorId, mrResMgr );
        OUString aMessage( aResId );

        if( aReplace2.getLength() )
        {
            aMessage = aMessage.replaceAll( "%s1", aReplace1 );
            aMessage = aMessage.replaceAll( "%s2", aReplace2 );
        }
        else if( aReplace1.getLength() )
        {
            aMessage = aMessage.replaceAll( "%s", aReplace1 );
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

IMPL_LINK_NOARG(XMLFilterTabDialog, OkHdl)
{
    if( onOk() )
        EndDialog(1);

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( XMLFilterTabDialog, ActivatePageHdl, TabControl *, pTabCtrl )
{
    const sal_uInt16 nId = pTabCtrl->GetCurPageId();
    TabPage* pTabPage = pTabCtrl->GetTabPage( nId );
    pTabPage->Show();

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( XMLFilterTabDialog, DeactivatePageHdl, TabControl *, /* pTabCtrl */ )
{
    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
