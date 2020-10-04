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
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/fileurl.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <osl/file.hxx>

#include <strings.hrc>
#include "xmlfiltertabdialog.hxx"
#include "xmlfiltertabpagebasic.hxx"
#include "xmlfiltertabpagexslt.hxx"
#include "xmlfiltercommon.hxx"

using namespace com::sun::star::uno;
using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;

XMLFilterTabDialog::XMLFilterTabDialog(weld::Window *pParent,
    const Reference< XComponentContext >& rxContext, const filter_info_impl* pInfo)
    : GenericDialogController(pParent, "filter/ui/xsltfilterdialog.ui", "XSLTFilterDialog")
    , mxContext(rxContext)
    , m_xTabCtrl(m_xBuilder->weld_notebook("tabcontrol"))
    , m_xOKBtn(m_xBuilder->weld_button("ok"))
    , mpBasicPage(new XMLFilterTabPageBasic(m_xTabCtrl->get_page("general")))
    , mpXSLTPage(new XMLFilterTabPageXSLT(m_xTabCtrl->get_page("transformation"), m_xDialog.get()))
{
    mpOldInfo = pInfo;
    mpNewInfo.reset( new filter_info_impl( *mpOldInfo ) );

    OUString aTitle(m_xDialog->get_title());
    aTitle = aTitle.replaceAll("%s", mpNewInfo->maFilterName);
    m_xDialog->set_title(aTitle);

    m_xOKBtn->connect_clicked( LINK( this, XMLFilterTabDialog, OkHdl ) );

    mpBasicPage->SetInfo( mpNewInfo.get() );
    mpXSLTPage->SetInfo( mpNewInfo.get() );
}

XMLFilterTabDialog::~XMLFilterTabDialog()
{
}

bool XMLFilterTabDialog::onOk()
{
    mpXSLTPage->FillInfo( mpNewInfo.get() );
    mpBasicPage->FillInfo( mpNewInfo.get() );

    OString sErrorPage;
    const char* pErrorId = nullptr;
    weld::Widget* pFocusWindow = nullptr;
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
                        sErrorPage = "general";
                        pErrorId = STR_ERROR_FILTER_NAME_EXISTS;
                        pFocusWindow = mpBasicPage->m_xEDFilterName.get();
                        aReplace1 = mpNewInfo->maFilterName;
                    }

                }
            }
            catch( const Exception& )
            {
                TOOLS_WARN_EXCEPTION("filter.xslt", "");
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
                    for( nFilter = 0; (nFilter < nCount) && (pErrorId == nullptr); nFilter++, pFilterName++ )
                    {
                        Any aAny( xFilterContainer->getByName( *pFilterName ) );
                        if( !(aAny >>= aValues) )
                            continue;

                        const sal_Int32 nValueCount( aValues.getLength() );
                        PropertyValue* pValues = aValues.getArray();
                        sal_Int32 nValue;

                        for( nValue = 0; (nValue < nValueCount) && (pErrorId == nullptr); nValue++, pValues++ )
                        {
                            if ( pValues->Name == "UIName" )
                            {
                                OUString aInterfaceName;
                                pValues->Value >>= aInterfaceName;
                                if( aInterfaceName == mpNewInfo->maInterfaceName )
                                {
                                    sErrorPage = "general";
                                    pErrorId = STR_ERROR_TYPE_NAME_EXISTS;
                                    pFocusWindow = mpBasicPage->m_xEDInterfaceName.get();
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
                TOOLS_WARN_EXCEPTION("filter.xslt", "");
            }
        }
    }

    if (!pErrorId)
    {
        // 4. see if the export xslt is valid
        if( (mpNewInfo->maExportXSLT != mpOldInfo->maExportXSLT) && comphelper::isFileUrl( mpNewInfo->maExportXSLT ) )
        {
            osl::File aFile( mpNewInfo->maExportXSLT );
            osl::File::RC aRC = aFile.open( osl_File_OpenFlag_Read );
            if( aRC != osl::File::E_None )
            {
                pErrorId = STR_ERROR_EXPORT_XSLT_NOT_FOUND;
                sErrorPage = "transformation";
                pFocusWindow = mpXSLTPage->m_xEDExportXSLT->getWidget();
            }
        }
    }

    if (!pErrorId)
    {
        // 5. see if the import xslt is valid
        if( (mpNewInfo->maImportXSLT != mpOldInfo->maImportXSLT) && comphelper::isFileUrl( mpNewInfo->maImportXSLT ) )
        {
            osl::File aFile( mpNewInfo->maImportXSLT );
            osl::File::RC aRC = aFile.open( osl_File_OpenFlag_Read );
            if( aRC != osl::File::E_None )
            {
                pErrorId = STR_ERROR_IMPORT_XSLT_NOT_FOUND;
                sErrorPage = "transformation";
                pFocusWindow = mpXSLTPage->m_xEDImportTemplate->getWidget();
            }
        }
    }

    // see if we have at least an import or an export xslt
    if((mpNewInfo->maImportXSLT.isEmpty()) && (mpNewInfo->maExportXSLT.isEmpty()) )
    {
        pErrorId = STR_ERROR_EXPORT_XSLT_NOT_FOUND;
        sErrorPage = "transformation";
        pFocusWindow = mpXSLTPage->m_xEDExportXSLT->getWidget();
    }

    if (!pErrorId)
    {
        // 6. see if the import template is valid
        if( (mpNewInfo->maImportTemplate != mpOldInfo->maImportTemplate) && comphelper::isFileUrl( mpNewInfo->maImportTemplate ) )
        {
            osl::File aFile( mpNewInfo->maImportTemplate );
            osl::File::RC aRC = aFile.open( osl_File_OpenFlag_Read );
            if( aRC != osl::File::E_None )
            {
                pErrorId = STR_ERROR_IMPORT_TEMPLATE_NOT_FOUND;
                sErrorPage = "transformation";
                pFocusWindow = mpXSLTPage->m_xEDImportTemplate->getWidget();
            }
        }
    }

    if (pErrorId)
    {
        m_xTabCtrl->set_current_page(sErrorPage);

        OUString aMessage(XsltResId(pErrorId));

        if( aReplace2.getLength() )
        {
            aMessage = aMessage.replaceAll( "%s1", aReplace1 );
            aMessage = aMessage.replaceAll( "%s2", aReplace2 );
        }
        else if( aReplace1.getLength() )
        {
            aMessage = aMessage.replaceAll( "%s", aReplace1 );
        }

        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                  VclMessageType::Warning, VclButtonsType::Ok,
                                                  aMessage));
        xBox->run();

        if( pFocusWindow )
            pFocusWindow->grab_focus();

        return false;
    }
    else
    {
        return true;
    }
}

IMPL_LINK_NOARG(XMLFilterTabDialog, OkHdl, weld::Button&, void)
{
    if( onOk() )
        m_xDialog->response(RET_OK);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
