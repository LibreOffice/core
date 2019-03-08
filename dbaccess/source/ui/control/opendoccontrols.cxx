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
#include <vcl/graph.hxx>
#include <vcl/help.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/event.hxx>
#include <unotools/historyoptions.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <tools/urlobj.hxx>
#include <svl/filenotation.hxx>
#include <osl/diagnose.h>
#include <vcl/builderfactory.hxx>

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

        Image GetCommandIcon( const sal_Char* _pCommandURL, const OUString& _rModuleName )
        {
            Image aIcon;
            if ( !_pCommandURL || !*_pCommandURL )
                return aIcon;

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

                    aIcon = Image(Graphic(xIconList[0]).GetBitmapEx());
                }
                while ( false );
            }
            catch ( Exception& ) {}

            return aIcon;
        }

    }

    // OpenButton

    OpenDocumentButton::OpenDocumentButton( vcl::Window* _pParent, const sal_Char* _pAsciiModuleName )
        :PushButton( _pParent )
    {
        impl_init( _pAsciiModuleName );
    }

    VCL_BUILDER_FACTORY_ARGS( OpenDocumentButton, "com.sun.star.sdb.OfficeDatabaseDocument" );

    void OpenDocumentButton::impl_init( const sal_Char* _pAsciiModuleName )
    {
        OSL_ENSURE( _pAsciiModuleName, "OpenDocumentButton::impl_init: invalid module name!" );
        m_sModule = OUString::createFromAscii( _pAsciiModuleName );

        // our label should equal the UI text of the "Open" command
        OUString sLabel(vcl::CommandInfoProvider::GetLabelForCommand(".uno:Open", m_sModule));
        SetText(" " + sLabel.replaceAll("~", ""));

        // Place icon left of text and both centered in the button.
        SetModeImage( GetCommandIcon( ".uno:Open", m_sModule ) );
        EnableImageDisplay( true );
        EnableTextDisplay( true );
        SetImageAlign( ImageAlign::Left );
        SetStyle( GetStyle() | WB_CENTER );
    }

    // OpenDocumentListBox

    OpenDocumentListBox::OpenDocumentListBox( vcl::Window* _pParent, const sal_Char* _pAsciiModuleName )
        :ListBox( _pParent, WB_BORDER | WB_DROPDOWN )
    {
        impl_init( _pAsciiModuleName );
    }

    VCL_BUILDER_FACTORY_ARGS( OpenDocumentListBox, "com.sun.star.sdb.OfficeDatabaseDocument" );

    void OpenDocumentListBox::impl_init( const sal_Char* _pAsciiModuleName )
    {
        OSL_ENSURE( _pAsciiModuleName, "OpenDocumentListBox::impl_init: invalid module name!" );

        Sequence< Sequence< PropertyValue> > aHistory = SvtHistoryOptions().GetList( ePICKLIST );
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

                    sal_Int32 nPos = InsertEntry( sTitle );
                    m_aURLs.emplace( nPos, StringPair( sDecodedURL, sFilter ) );
                }
            }
            catch( Exception& ) {}
        }
    }

    OUString OpenDocumentListBox::GetSelectedDocumentURL() const
    {
        OUString sURL;
        sal_Int32 nSelected = GetSelectedEntryPos();
        if ( LISTBOX_ENTRY_NOTFOUND != GetSelectedEntryPos() )
            sURL = impl_getDocumentAtIndex( nSelected ).first;
        return sURL;
    }

    OpenDocumentListBox::StringPair OpenDocumentListBox::impl_getDocumentAtIndex( sal_uInt16 _nListIndex, bool _bSystemNotation ) const
    {
        MapIndexToStringPair::const_iterator pos = m_aURLs.find( _nListIndex );
        OSL_ENSURE( pos != m_aURLs.end(), "OpenDocumentListBox::impl_getDocumentAtIndex: invalid index!" );

        StringPair aDocumentDescriptor;
        if ( pos != m_aURLs.end() )
        {
            aDocumentDescriptor = pos->second;
            if ( _bSystemNotation && !aDocumentDescriptor.first.isEmpty() )
            {
                ::svt::OFileNotation aNotation( aDocumentDescriptor.first );
                aDocumentDescriptor.first = aNotation.get( ::svt::OFileNotation::N_SYSTEM );
            }
        }
        return aDocumentDescriptor;
    }

    void  OpenDocumentListBox::RequestHelp( const HelpEvent& _rHEvt )
    {
        if( !( _rHEvt.GetMode() & HelpEventMode::QUICK ) )
            return;
        if ( !IsEnabled() )
            return;

        Point aRequestPos( ScreenToOutputPixel( _rHEvt.GetMousePosPixel() ) );
        sal_Int32 nItemIndex = LISTBOX_ENTRY_NOTFOUND;
        if ( GetIndexForPoint( aRequestPos, nItemIndex ) != -1 )
        {
            tools::Rectangle aItemRect( GetBoundingRectangle( nItemIndex ) );
            aItemRect = tools::Rectangle(
                OutputToScreenPixel( aItemRect.TopLeft() ),
                OutputToScreenPixel( aItemRect.BottomRight() ) );
            OUString sHelpText = impl_getDocumentAtIndex( nItemIndex, true ).first;
            Help::ShowQuickHelp( this, aItemRect, sHelpText, QuickHelpFlags::Left | QuickHelpFlags::VCenter );
        }
    }

} // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
