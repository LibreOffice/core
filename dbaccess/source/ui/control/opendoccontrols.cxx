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


#include "opendoccontrols.hxx"

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/ui/ModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/frame/UICommandDescription.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/ui/XImageManager.hpp>

#include <comphelper/processfactory.hxx>
#include <vcl/graph.hxx>
#include <vcl/help.hxx>
#include <unotools/historyoptions.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <tools/urlobj.hxx>
#include <svl/filenotation.hxx>
#include <osl/diagnose.h>
#include <vcl/builder.hxx>

//........................................................................
namespace dbaui
{
//........................................................................

    namespace
    {
        using ::com::sun::star::uno::Reference;
        using ::com::sun::star::uno::Exception;
        using ::com::sun::star::uno::Sequence;
        using ::com::sun::star::uno::UNO_QUERY_THROW;
        using ::com::sun::star::uno::XComponentContext;
        using ::com::sun::star::container::XNameAccess;
        using ::com::sun::star::lang::XMultiServiceFactory;
        using ::com::sun::star::beans::PropertyValue;
        using ::com::sun::star::ui::ModuleUIConfigurationManagerSupplier;
        using ::com::sun::star::ui::XModuleUIConfigurationManagerSupplier;
        using ::com::sun::star::ui::XUIConfigurationManager;
        using ::com::sun::star::ui::XImageManager;
        using ::com::sun::star::frame::UICommandDescription;
        using ::com::sun::star::graphic::XGraphic;

        String GetCommandText( const sal_Char* _pCommandURL, const OUString& _rModuleName )
        {
            OUString sLabel;
            if ( !_pCommandURL || !*_pCommandURL )
                return sLabel;

            Reference< XNameAccess > xUICommandLabels;
            OUString sCommandURL = OUString::createFromAscii( _pCommandURL );

            try
            {
                do
                {
                    // Retrieve popup menu labels
                    Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
                    if ( !xContext.is() )
                        break;

                    Reference< XNameAccess> xNameAccess( UICommandDescription::create(xContext) );

                    xNameAccess->getByName( _rModuleName ) >>= xUICommandLabels;
                    if ( !xUICommandLabels.is() )
                        break;

                    Sequence< PropertyValue > aProperties;
                    if ( !( xUICommandLabels->getByName(sCommandURL) >>= aProperties ) )
                        break;

                    sal_Int32 nCount( aProperties.getLength() );
                    for ( sal_Int32 i=0; i<nCount; ++i )
                    {
                        OUString sPropertyName( aProperties[i].Name );
                        if ( sPropertyName == "Label" )
                        {
                            aProperties[i].Value >>= sLabel;
                            break;
                        }
                    }
                }
                while ( false );
            }
            catch( Exception& rException )
            {
                (void)rException;
            }

            return sLabel;
        }

        Image GetCommandIcon( const sal_Char* _pCommandURL, const OUString& _rModuleName )
        {
            Image aIcon;
            if ( !_pCommandURL || !*_pCommandURL )
                return aIcon;

            Reference< XNameAccess > xUICommandLabels;
            OUString sCommandURL = OUString::createFromAscii( _pCommandURL );
            try
            {
                do
                {
                    // Retrieve popup menu labels
                    Reference< com::sun::star::uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
                    if ( !xContext.is() )
                        break;

                    Reference< XModuleUIConfigurationManagerSupplier > xSupplier(
                        ModuleUIConfigurationManagerSupplier::create(xContext) );

                    Reference< XUIConfigurationManager > xManager( xSupplier->getUIConfigurationManager( _rModuleName ) );
                    Reference< XImageManager > xImageManager;
                    if ( xManager.is() )
                        xImageManager = xImageManager.query( xManager->getImageManager() );
                    if ( !xImageManager.is() )
                        break;

                    Sequence< OUString > aCommandList( &sCommandURL, 1 );
                    Sequence<Reference< XGraphic> > xIconList( xImageManager->getImages( 0, aCommandList ) );
                    if ( !xIconList.hasElements() )
                        break;

                    aIcon = Graphic( xIconList[0] ).GetBitmapEx();
                }
                while ( false );
            }
            catch ( Exception& rException )
            {
                (void)rException;
            }

            return aIcon;
        }


    }

    //====================================================================
    //= OpenButton
    //====================================================================

    //--------------------------------------------------------------------
    OpenDocumentButton::OpenDocumentButton( Window* _pParent, const sal_Char* _pAsciiModuleName )
        :PushButton( _pParent )
    {
        impl_init( _pAsciiModuleName );
    }

    //--------------------------------------------------------------------
    extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeOpenDocumentButton( Window *pParent, VclBuilder::stringmap & )
    {
        return new OpenDocumentButton( pParent, "com.sun.star.sdb.OfficeDatabaseDocument" );
    }

    //--------------------------------------------------------------------
    void OpenDocumentButton::impl_init( const sal_Char* _pAsciiModuleName )
    {
        OSL_ENSURE( _pAsciiModuleName, "OpenDocumentButton::impl_init: invalid module name!" );
        m_sModule = OUString::createFromAscii( _pAsciiModuleName );

        // our label should equal the UI text of the "Open" command
        OUString sLabel(GetCommandText(".uno:Open", m_sModule));
        SetText(OUString(' ') + sLabel.replaceAll("~", OUString()));

        // Place icon left of text and both centered in the button.
        SetModeImage( GetCommandIcon( ".uno:Open", m_sModule ) );
        EnableImageDisplay( sal_True );
        EnableTextDisplay( sal_True );
        SetImageAlign( IMAGEALIGN_LEFT );
        SetStyle( GetStyle() | WB_CENTER );
    }

    //====================================================================
    //= OpenDocumentListBox
    //====================================================================

    //--------------------------------------------------------------------
    OpenDocumentListBox::OpenDocumentListBox( Window* _pParent, const sal_Char* _pAsciiModuleName )
        :ListBox( _pParent, WB_BORDER | WB_DROPDOWN )
    {
        impl_init( _pAsciiModuleName );
    }

    //--------------------------------------------------------------------
    extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeOpenDocumentListBox( Window *pParent, VclBuilder::stringmap & )
    {
        return new OpenDocumentListBox( pParent, "com.sun.star.sdb.OfficeDatabaseDocument" );
    }

    //--------------------------------------------------------------------
    void OpenDocumentListBox::impl_init( const sal_Char* _pAsciiModuleName )
    {
        OSL_ENSURE( _pAsciiModuleName, "OpenDocumentListBox::impl_init: invalid module name!" );

        Sequence< Sequence< PropertyValue> > aHistory = SvtHistoryOptions().GetList( ePICKLIST );
        Reference< XNameAccess > xFilterFactory;
        xFilterFactory = xFilterFactory.query( ::comphelper::getProcessServiceFactory()->createInstance(
            OUString( "com.sun.star.document.FilterFactory" ) ) );

        sal_uInt32 nCount = aHistory.getLength();
        for ( sal_uInt32 nItem = 0; nItem < nCount; ++nItem )
        {
            try
            {
                //  Get the current history item's properties.
                ::comphelper::SequenceAsHashMap aItemProperties( aHistory[ nItem ] );
                OUString sURL = aItemProperties.getUnpackedValueOrDefault( HISTORY_PROPERTYNAME_URL, OUString() );
                OUString sFilter = aItemProperties.getUnpackedValueOrDefault( HISTORY_PROPERTYNAME_FILTER, OUString() );
                String          sTitle = aItemProperties.getUnpackedValueOrDefault( HISTORY_PROPERTYNAME_TITLE, OUString() );
                OUString sPassword = aItemProperties.getUnpackedValueOrDefault( HISTORY_PROPERTYNAME_PASSWORD, OUString() );

                //  If the entry is an impress file then insert it into the
                //  history list and the list box.
                Sequence< PropertyValue > aProps;
                xFilterFactory->getByName( sFilter ) >>= aProps;

                ::comphelper::SequenceAsHashMap aFilterProperties( aProps );
                OUString sDocumentService = aFilterProperties.getUnpackedValueOrDefault(
                    OUString( "DocumentService" ), OUString() );
                if ( sDocumentService.equalsAscii( _pAsciiModuleName ) )
                {
                    // yes, it's a Base document
                    INetURLObject aURL;
                    aURL.SetSmartURL( sURL );
                    // The password is set only when it is not empty.
                    if ( !sPassword.isEmpty() )
                        aURL.SetPass( sPassword );

                    if ( !sTitle.Len() )
                        sTitle = aURL.getBase( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_UNAMBIGUOUS );

                    String sDecodedURL = aURL.GetMainURL( INetURLObject::NO_DECODE );

                    sal_uInt16 nPos = InsertEntry( sTitle );
                    m_aURLs.insert( MapIndexToStringPair::value_type( nPos, StringPair( sDecodedURL, sFilter ) ) );
                }
            }
            catch( Exception& rException )
            {
                (void)rException;
            }
        }
    }

    //--------------------------------------------------------------------
    String OpenDocumentListBox::GetSelectedDocumentURL() const
    {
        String sURL;
        sal_uInt16 nSelected = GetSelectEntryPos();
        if ( LISTBOX_ENTRY_NOTFOUND != GetSelectEntryPos() )
            sURL = impl_getDocumentAtIndex( nSelected ).first;
        return sURL;
    }

    //--------------------------------------------------------------------
    String OpenDocumentListBox::GetSelectedDocumentFilter() const
    {
        String sFilter;
        sal_uInt16 nSelected = GetSelectEntryPos();
        if ( LISTBOX_ENTRY_NOTFOUND != GetSelectEntryPos() )
            sFilter = impl_getDocumentAtIndex( nSelected ).second;
        return sFilter;
    }

    //--------------------------------------------------------------------
    OpenDocumentListBox::StringPair OpenDocumentListBox::impl_getDocumentAtIndex( sal_uInt16 _nListIndex, bool _bSystemNotation ) const
    {
        MapIndexToStringPair::const_iterator pos = m_aURLs.find( _nListIndex );
        OSL_ENSURE( pos != m_aURLs.end(), "OpenDocumentListBox::impl_getDocumentAtIndex: invalid index!" );

        StringPair aDocumentDescriptor;
        if ( pos != m_aURLs.end() )
        {
            aDocumentDescriptor = pos->second;
            if ( _bSystemNotation && aDocumentDescriptor.first.Len() )
            {
                ::svt::OFileNotation aNotation( aDocumentDescriptor.first );
                aDocumentDescriptor.first = aNotation.get( ::svt::OFileNotation::N_SYSTEM );
            }
        }
        return aDocumentDescriptor;
    }

    //--------------------------------------------------------------------
    void  OpenDocumentListBox::RequestHelp( const HelpEvent& _rHEvt )
    {
        if( !( _rHEvt.GetMode() & HELPMODE_QUICK ) )
            return;
        if ( !IsEnabled() )
            return;

        Point aRequestPos( ScreenToOutputPixel( _rHEvt.GetMousePosPixel() ) );
        sal_uInt16 nItemIndex = LISTBOX_ENTRY_NOTFOUND;
        if ( GetIndexForPoint( aRequestPos, nItemIndex ) != -1 )
        {
            Rectangle aItemRect( GetBoundingRectangle( nItemIndex ) );
            aItemRect = Rectangle(
                OutputToScreenPixel( aItemRect.TopLeft() ),
                OutputToScreenPixel( aItemRect.BottomRight() ) );
            String sHelpText = impl_getDocumentAtIndex( nItemIndex, true ).first;
            Help::ShowQuickHelp( this, aItemRect, sHelpText, QUICKHELP_LEFT | QUICKHELP_VCENTER );
        }
    }

//........................................................................
} // namespace dbaui
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
