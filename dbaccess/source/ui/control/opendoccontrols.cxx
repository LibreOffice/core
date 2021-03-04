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

#include <opendoccontrols.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/ui/theModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/ui/XImageManager.hpp>

#include <comphelper/processfactory.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <unotools/historyoptions.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <tools/urlobj.hxx>
#include <osl/diagnose.h>

namespace dbaui
{

    namespace
    {
        using ::com::sun::star::uno::Reference;
        using ::com::sun::star::uno::Exception;
        using ::com::sun::star::uno::Sequence;
        using ::com::sun::star::uno::XComponentContext;
        using ::com::sun::star::container::XNameAccess;
        using ::com::sun::star::beans::PropertyValue;
        using ::com::sun::star::ui::theModuleUIConfigurationManagerSupplier;
        using ::com::sun::star::ui::XModuleUIConfigurationManagerSupplier;
        using ::com::sun::star::ui::XUIConfigurationManager;
        using ::com::sun::star::ui::XImageManager;
        using ::com::sun::star::graphic::XGraphic;

        Reference< XGraphic> GetCommandIcon( const char* _pCommandURL, const OUString& _rModuleName )
        {
            if ( !_pCommandURL || !*_pCommandURL )
                return nullptr;

            OUString sCommandURL = OUString::createFromAscii( _pCommandURL );
            try
            {
                do
                {
                    // Retrieve popup menu labels
                    Reference< css::uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
                    if ( !xContext.is() )
                        break;

                    Reference< XModuleUIConfigurationManagerSupplier > xSupplier(
                        theModuleUIConfigurationManagerSupplier::get(xContext) );

                    Reference< XUIConfigurationManager > xManager( xSupplier->getUIConfigurationManager( _rModuleName ) );
                    Reference< XImageManager > xImageManager;
                    if ( xManager.is() )
                        xImageManager.set(xManager->getImageManager(), css::uno::UNO_QUERY);
                    if ( !xImageManager.is() )
                        break;

                    Sequence< OUString > aCommandList( &sCommandURL, 1 );
                    Sequence<Reference< XGraphic> > xIconList( xImageManager->getImages( 0, aCommandList ) );
                    if ( !xIconList.hasElements() )
                        break;

                    return xIconList[0];
                }
                while ( false );
            }
            catch ( Exception& ) {}

            return nullptr;
        }
    }

    // OpenButton

    OpenDocumentButton::OpenDocumentButton(std::unique_ptr<weld::Button> xControl, const char* _pAsciiModuleName)
        : m_xControl(std::move(xControl))
    {
        impl_init( _pAsciiModuleName );
    }

    void OpenDocumentButton::impl_init( const char* _pAsciiModuleName )
    {
        OSL_ENSURE( _pAsciiModuleName, "OpenDocumentButton::impl_init: invalid module name!" );
        m_sModule = OUString::createFromAscii( _pAsciiModuleName );

        // our label should equal the UI text of the "Open" command
        auto aProperties = vcl::CommandInfoProvider::GetCommandProperties(".uno:Open", m_sModule);
        OUString sLabel(vcl::CommandInfoProvider::GetLabelForCommand(aProperties));
        m_xControl->set_label(" " + sLabel.replaceAll("~", ""));

        // Place icon left of text and both centered in the button.
        m_xControl->set_image(GetCommandIcon(".uno:Open", m_sModule));
    }

    // OpenDocumentListBox

    OpenDocumentListBox::OpenDocumentListBox(std::unique_ptr<weld::ComboBox> xControl, const char* _pAsciiModuleName )
        : m_xControl(std::move(xControl))
    {
        // we need to limit the max auto width feature of the filter box
        int nWidth = m_xControl->get_approximate_digit_width() * 50;
        m_xControl->set_size_request(nWidth, -1);

        impl_init( _pAsciiModuleName );
    }

    void OpenDocumentListBox::impl_init( const char* _pAsciiModuleName )
    {
        OSL_ENSURE( _pAsciiModuleName, "OpenDocumentListBox::impl_init: invalid module name!" );

        Sequence< Sequence< PropertyValue> > aHistory = SvtHistoryOptions().GetList( EHistoryType::PickList );
        Reference< XNameAccess > xFilterFactory;
        xFilterFactory.set(::comphelper::getProcessServiceFactory()->createInstance(
            "com.sun.star.document.FilterFactory" ), css::uno::UNO_QUERY);

        sal_uInt32 nCount = aHistory.getLength();
        for ( sal_uInt32 nItem = 0; nItem < nCount; ++nItem )
        {
            try
            {
                //  Get the current history item's properties.
                ::comphelper::SequenceAsHashMap aItemProperties( aHistory[ nItem ] );
                OUString sURL = aItemProperties.getUnpackedValueOrDefault( HISTORY_PROPERTYNAME_URL, OUString() );
                OUString sFilter = aItemProperties.getUnpackedValueOrDefault( HISTORY_PROPERTYNAME_FILTER, OUString() );
                OUString sTitle = aItemProperties.getUnpackedValueOrDefault( HISTORY_PROPERTYNAME_TITLE, OUString() );
                OUString sPassword = aItemProperties.getUnpackedValueOrDefault( HISTORY_PROPERTYNAME_PASSWORD, OUString() );

                //  If the entry is an impress file then insert it into the
                //  history list and the list box.
                Sequence< PropertyValue > aProps;
                xFilterFactory->getByName( sFilter ) >>= aProps;

                ::comphelper::SequenceAsHashMap aFilterProperties( aProps );
                OUString sDocumentService = aFilterProperties.getUnpackedValueOrDefault(
                    "DocumentService", OUString() );
                if ( sDocumentService.equalsAscii( _pAsciiModuleName ) )
                {
                    // yes, it's a Base document
                    INetURLObject aURL;
                    aURL.SetSmartURL( sURL );
                    // The password is set only when it is not empty.
                    if ( !sPassword.isEmpty() )
                        aURL.SetPass( sPassword );

                    if ( sTitle.isEmpty() )
                        sTitle = aURL.getBase( INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::Unambiguous );

                    OUString sDecodedURL = aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );

                    m_xControl->append_text(sTitle);
                    m_aURLs.emplace_back(sDecodedURL, sFilter);
                }
            }
            catch( Exception& ) {}
        }
    }

    OUString OpenDocumentListBox::GetSelectedDocumentURL() const
    {
        OUString sURL;
        sal_Int32 nSelected = m_xControl->get_active();
        if (nSelected != -1)
            sURL = impl_getDocumentAtIndex( nSelected ).first;
        return sURL;
    }

    const OpenDocumentListBox::StringPair & OpenDocumentListBox::impl_getDocumentAtIndex( sal_uInt16 _nListIndex ) const
    {
        return m_aURLs[_nListIndex];
    }

} // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
