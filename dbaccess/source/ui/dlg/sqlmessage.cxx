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

#include "sqlmessage.hxx"
#include "dbu_dlg.hrc"
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <vcl/fixed.hxx>
#include <osl/diagnose.h>
#include <svtools/treelistbox.hxx>
#include "svtools/treelistentry.hxx"
#include <svtools/svmedit.hxx>
#include <connectivity/dbexception.hxx>
#include <connectivity/sqlerror.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/configmgr.hxx>
#include <sfx2/sfxuno.hxx>
#include "dbaccess_helpid.hrc"
#include "UITools.hxx"
#include "moduledbu.hxx"

#include <tools/urlobj.hxx>

#define RET_MORE   RET_RETRY + 1

#define DIALOG_WIDTH    220
#define OUTER_MARGIN    6
#define IMAGE_SIZE      20
#define INNER_PADDING   3
#define TEXT_POS_X      ( OUTER_MARGIN + IMAGE_SIZE + INNER_PADDING )

using namespace dbtools;
using namespace com::sun::star::uno;
using namespace com::sun::star::sdb;
using namespace com::sun::star::sdbc;

namespace dbaui
{

namespace
{
    class ImageProvider
    {
    private:
        sal_uInt16  m_defaultImageID;

        mutable Image   m_defaultImage;

    public:
        explicit ImageProvider( sal_uInt16 _defaultImageID )
            :m_defaultImageID( _defaultImageID )
        {
        }

        Image const & getImage() const
        {
            if ( !m_defaultImage )
                m_defaultImage = Image( ModuleRes( m_defaultImageID ) );
            return m_defaultImage;
        }
    };

    class LabelProvider
    {
    private:
        OUString  m_label;
    public:
        explicit LabelProvider( sal_uInt16 _labelResourceID )
            :m_label( ModuleRes( _labelResourceID ) )
        {
        }

        const OUString&  getLabel() const
        {
            return m_label;
        }
    };

    class ProviderFactory
    {
    private:
        mutable std::shared_ptr< ImageProvider >   m_pErrorImage;
        mutable std::shared_ptr< ImageProvider >   m_pWarningsImage;
        mutable std::shared_ptr< ImageProvider >   m_pInfoImage;
        mutable std::shared_ptr< LabelProvider >   m_pErrorLabel;
        mutable std::shared_ptr< LabelProvider >   m_pWarningsLabel;
        mutable std::shared_ptr< LabelProvider >   m_pInfoLabel;

    public:
        ProviderFactory()
        {
        }

        std::shared_ptr< ImageProvider > const & getImageProvider( SQLExceptionInfo::TYPE _eType ) const
        {
            std::shared_ptr< ImageProvider >* ppProvider( &m_pErrorImage );
            sal_uInt16 nNormalImageID( BMP_EXCEPTION_ERROR );

            switch ( _eType )
            {
            case SQLExceptionInfo::TYPE::SQLWarning:
                ppProvider = &m_pWarningsImage;
                nNormalImageID = BMP_EXCEPTION_WARNING;
                break;

            case SQLExceptionInfo::TYPE::SQLContext:
                ppProvider = &m_pInfoImage;
                nNormalImageID = BMP_EXCEPTION_INFO;
                break;

            default:
                break;
            }

            if ( !ppProvider->get() )
                ppProvider->reset( new ImageProvider( nNormalImageID ) );
            return *ppProvider;
        }

        std::shared_ptr< LabelProvider > const & getLabelProvider( SQLExceptionInfo::TYPE _eType, bool _bSubLabel ) const
        {
            std::shared_ptr< LabelProvider >* ppProvider( &m_pErrorLabel );
            sal_uInt16 nLabelID( STR_EXCEPTION_ERROR );

            switch ( _eType )
            {
            case SQLExceptionInfo::TYPE::SQLWarning:
                ppProvider = &m_pWarningsLabel;
                nLabelID = STR_EXCEPTION_WARNING;
                break;

            case SQLExceptionInfo::TYPE::SQLContext:
                ppProvider = &m_pInfoLabel;
                nLabelID = _bSubLabel ? STR_EXCEPTION_DETAILS : STR_EXCEPTION_INFO;
                break;
            default:
                break;
            }

            if ( !ppProvider->get() )
                ppProvider->reset( new LabelProvider( nLabelID ) );
            return *ppProvider;
        }

    };

    /// a stripped version of the SQLException, packed for displaying
    struct ExceptionDisplayInfo
    {
        SQLExceptionInfo::TYPE                  eType;

        std::shared_ptr< ImageProvider >        pImageProvider;
        std::shared_ptr< LabelProvider >        pLabelProvider;

        bool                                    bSubEntry;

        OUString                                sMessage;
        OUString                                sSQLState;
        OUString                                sErrorCode;

        ExceptionDisplayInfo() : eType( SQLExceptionInfo::TYPE::Undefined ), bSubEntry( false ) { }
        explicit ExceptionDisplayInfo( SQLExceptionInfo::TYPE _eType ) : eType( _eType ), bSubEntry( false ) { }
    };

    bool lcl_hasDetails( const ExceptionDisplayInfo& _displayInfo )
    {
        return  ( !_displayInfo.sErrorCode.isEmpty() )
                ||  (   !_displayInfo.sSQLState.isEmpty()
                    &&  _displayInfo.sSQLState != "S1000"
                    );
    }

    typedef ::std::vector< ExceptionDisplayInfo >   ExceptionDisplayChain;

    /// strips the [OOoBase] vendor identifier from the given error message, if applicable
    OUString lcl_stripOOoBaseVendor( const OUString& _rErrorMessage )
    {
        OUString sErrorMessage( _rErrorMessage );

        const OUString sVendorIdentifier( ::connectivity::SQLError::getMessagePrefix() );
        if ( sErrorMessage.startsWith( sVendorIdentifier ) )
        {
            // characters to strip
            sal_Int32 nStripLen( sVendorIdentifier.getLength() );
            // usually, there should be a whitespace between the vendor and the real message
            while   (   ( sErrorMessage.getLength() > nStripLen )
                    &&  ( sErrorMessage[nStripLen] == ' ' )
                    )
                    ++nStripLen;
            sErrorMessage = sErrorMessage.copy( nStripLen );
        }

        return sErrorMessage;
    }

    void lcl_buildExceptionChain( const SQLExceptionInfo& _rErrorInfo, const ProviderFactory& _rFactory, ExceptionDisplayChain& _out_rChain )
    {
        {
            ExceptionDisplayChain empty;
            _out_rChain.swap( empty );
        }

        SQLExceptionIteratorHelper iter( _rErrorInfo );
        while ( iter.hasMoreElements() )
        {
            // current chain element
            SQLExceptionInfo aCurrentElement;
            iter.next( aCurrentElement );

            const SQLException* pCurrentError = static_cast<const SQLException*>(aCurrentElement);
            OSL_ENSURE( pCurrentError, "lcl_buildExceptionChain: iterator failure!" );
                // hasMoreElements should not have returned <TRUE/> in this case

            ExceptionDisplayInfo aDisplayInfo( aCurrentElement.getType() );

            aDisplayInfo.sMessage = pCurrentError->Message.trim();
            aDisplayInfo.sSQLState = pCurrentError->SQLState;
            if ( pCurrentError->ErrorCode )
                aDisplayInfo.sErrorCode = OUString::number( pCurrentError->ErrorCode );

            if  (   aDisplayInfo.sMessage.isEmpty()
                &&  !lcl_hasDetails( aDisplayInfo )
                )
            {
                OSL_FAIL( "lcl_buildExceptionChain: useless exception: no state, no error code, no message!" );
                continue;
            }

            aDisplayInfo.pImageProvider = _rFactory.getImageProvider( aCurrentElement.getType() );
            aDisplayInfo.pLabelProvider = _rFactory.getLabelProvider( aCurrentElement.getType(), false );

            _out_rChain.push_back( aDisplayInfo );

            if ( aCurrentElement.getType() == SQLExceptionInfo::TYPE::SQLContext )
            {
                const SQLContext* pContext = static_cast<const SQLContext*>(aCurrentElement);
                if ( !pContext->Details.isEmpty() )
                {
                    ExceptionDisplayInfo aSubInfo( aCurrentElement.getType() );

                    aSubInfo.sMessage = pContext->Details;
                    aSubInfo.pImageProvider = _rFactory.getImageProvider( aCurrentElement.getType() );
                    aSubInfo.pLabelProvider = _rFactory.getLabelProvider( aCurrentElement.getType(), true );
                    aSubInfo.bSubEntry = true;

                    _out_rChain.push_back( aSubInfo );
                }
            }
        }
    }

    void lcl_insertExceptionEntry( SvTreeListBox& _rList, size_t _nElementPos, const ExceptionDisplayInfo& _rEntry )
    {
        Image aEntryImage( _rEntry.pImageProvider->getImage() );
        SvTreeListEntry* pListEntry =
            _rList.InsertEntry( _rEntry.pLabelProvider->getLabel(), aEntryImage, aEntryImage );
        pListEntry->SetUserData( reinterpret_cast< void* >( _nElementPos ) );
    }
}

class OExceptionChainDialog : public ModalDialog
{
    VclPtr<SvTreeListBox>    m_pExceptionList;
    VclPtr<VclMultiLineEdit> m_pExceptionText;

    OUString        m_sStatusLabel;
    OUString        m_sErrorCodeLabel;

    ExceptionDisplayChain   m_aExceptions;

public:
    OExceptionChainDialog( vcl::Window* pParent, const ExceptionDisplayChain& _rExceptions );
    virtual ~OExceptionChainDialog() { disposeOnce(); }
    virtual void dispose() override
    {
        m_pExceptionList.clear();
        m_pExceptionText.clear();
        ModalDialog::dispose();
    }

protected:
    DECL_LINK_TYPED(OnExceptionSelected, SvTreeListBox*, void);
};

OExceptionChainDialog::OExceptionChainDialog(vcl::Window* pParent, const ExceptionDisplayChain& _rExceptions)
    : ModalDialog(pParent, "SQLExceptionDialog", "dbaccess/ui/sqlexception.ui")
    , m_aExceptions(_rExceptions)
{
    get(m_pExceptionList, "list");
    Size aListSize(LogicToPixel(Size(85, 93), MAP_APPFONT));
    m_pExceptionList->set_width_request(aListSize.Width());
    m_pExceptionList->set_height_request(aListSize.Height());
    get(m_pExceptionText, "description");
    Size aTextSize(LogicToPixel(Size(125 , 93), MAP_APPFONT));
    m_pExceptionText->set_width_request(aTextSize.Width());
    m_pExceptionText->set_height_request(aTextSize.Height());

    m_sStatusLabel = ModuleRes( STR_EXCEPTION_STATUS );
    m_sErrorCodeLabel = ModuleRes( STR_EXCEPTION_ERRORCODE );

    m_pExceptionList->SetSelectionMode(SelectionMode::Single);
    m_pExceptionList->SetDragDropMode(DragDropMode::NONE);
    m_pExceptionList->EnableInplaceEditing(false);
    m_pExceptionList->SetStyle(m_pExceptionList->GetStyle() | WB_HASLINES | WB_HASBUTTONS | WB_HASBUTTONSATROOT | WB_HSCROLL);

    m_pExceptionList->SetSelectHdl(LINK(this, OExceptionChainDialog, OnExceptionSelected));
    m_pExceptionList->SetNodeDefaultImages( );

    bool bHave22018 = false;
    size_t elementPos = 0;

    for (   ExceptionDisplayChain::const_iterator loop = m_aExceptions.begin();
            loop != m_aExceptions.end();
            ++loop, ++elementPos
        )
    {
        lcl_insertExceptionEntry( *m_pExceptionList, elementPos, *loop );
        bHave22018 = loop->sSQLState == "22018";
    }

    // if the error has the code 22018, then add an additional explanation
    // #i24021#
    if ( bHave22018 )
    {
        ProviderFactory aProviderFactory;

        ExceptionDisplayInfo aInfo22018;
        aInfo22018.sMessage = ModuleRes( STR_EXPLAN_STRINGCONVERSION_ERROR );
        aInfo22018.pLabelProvider = aProviderFactory.getLabelProvider( SQLExceptionInfo::TYPE::SQLContext, false );
        aInfo22018.pImageProvider = aProviderFactory.getImageProvider( SQLExceptionInfo::TYPE::SQLContext );
        m_aExceptions.push_back( aInfo22018 );

        lcl_insertExceptionEntry( *m_pExceptionList, m_aExceptions.size() - 1, aInfo22018 );
    }
}

IMPL_LINK_NOARG_TYPED(OExceptionChainDialog, OnExceptionSelected, SvTreeListBox*, void)
{
    SvTreeListEntry* pSelected = m_pExceptionList->FirstSelected();
    OSL_ENSURE(!pSelected || !m_pExceptionList->NextSelected(pSelected), "OExceptionChainDialog::OnExceptionSelected : multi selection ?");

    OUString sText;

    if ( pSelected )
    {
        size_t pos = reinterpret_cast< size_t >( pSelected->GetUserData() );
        const ExceptionDisplayInfo& aExceptionInfo( m_aExceptions[ pos ] );

        if ( !aExceptionInfo.sSQLState.isEmpty() )
        {
            sText += m_sStatusLabel;
            sText += ": ";
            sText += aExceptionInfo.sSQLState;
            sText += "\n";
        }

        if ( !aExceptionInfo.sErrorCode.isEmpty() )
        {
            sText += m_sErrorCodeLabel;
            sText += ": ";
            sText += aExceptionInfo.sErrorCode;
            sText += "\n";
        }

        if ( !sText.isEmpty() )
            sText += "\n";

        sText += aExceptionInfo.sMessage;
    }

    m_pExceptionText->SetText(sText);
}

// SQLMessageBox_Impl
struct SQLMessageBox_Impl
{
    ExceptionDisplayChain   aDisplayInfo;

    explicit SQLMessageBox_Impl( const SQLExceptionInfo& _rExceptionInfo )
    {
        // transform the exception chain to a form more suitable for displaying it here
        ProviderFactory aProviderFactory;
        lcl_buildExceptionChain( _rExceptionInfo, aProviderFactory, aDisplayInfo );
    }
};

namespace
{
    void lcl_positionInAppFont( const vcl::Window& _rParent, vcl::Window& _rChild, long _nX, long _nY, long Width, long Height )
    {
        Point aPos = _rParent.LogicToPixel( Point( _nX, _nY ), MAP_APPFONT );
        Size aSize = _rParent.LogicToPixel( Size( Width, Height ), MAP_APPFONT );
        _rChild.SetPosSizePixel( aPos, aSize );
    }

    void lcl_addButton( ButtonDialog& _rDialog, StandardButtonType _eType, bool _bDefault )
    {
        sal_uInt16 nButtonID = 0;
        switch ( _eType )
        {
        case StandardButtonType::Yes:    nButtonID = RET_YES; break;
        case StandardButtonType::No:     nButtonID = RET_NO; break;
        case StandardButtonType::OK:     nButtonID = RET_OK; break;
        case StandardButtonType::Cancel: nButtonID = RET_CANCEL; break;
        case StandardButtonType::Retry:  nButtonID = RET_RETRY; break;
        case StandardButtonType::Help:   nButtonID = RET_HELP; break;
        default:
            OSL_FAIL( "lcl_addButton: invalid button id!" );
            break;
        }
        _rDialog.AddButton( _eType, nButtonID, _bDefault ? ButtonDialogFlags::Default | ButtonDialogFlags::Focus : ButtonDialogFlags::NONE );
    }
}

void OSQLMessageBox::impl_positionControls()
{
    OSL_PRECOND( !m_pImpl->aDisplayInfo.empty(), "OSQLMessageBox::impl_positionControls: nothing to display at all?" );

    if ( m_pImpl->aDisplayInfo.empty() )
        return;
    const ExceptionDisplayInfo* pSecondInfo = nullptr;

    const ExceptionDisplayInfo& rFirstInfo = *m_pImpl->aDisplayInfo.begin();
    if ( m_pImpl->aDisplayInfo.size() > 1 )
        pSecondInfo = &m_pImpl->aDisplayInfo[1];
    OUString sPrimary, sSecondary;
    sPrimary = rFirstInfo.sMessage;
    // one or two texts to display?
    if ( pSecondInfo )
    {
        // we show two elements in the main dialog if and only if one of
        // - the first element in the chain is an SQLContext, and the second
        //   element denotes its sub entry
        // - the first and the second element are both independent (i.e. the second
        //   is no sub entry), and none of them is a context.
        bool bFirstElementIsContext = ( rFirstInfo.eType == SQLExceptionInfo::TYPE::SQLContext );
        bool bSecondElementIsContext = ( pSecondInfo->eType == SQLExceptionInfo::TYPE::SQLContext );

        if ( bFirstElementIsContext && pSecondInfo->bSubEntry )
            sSecondary = pSecondInfo->sMessage;
        if ( !bFirstElementIsContext && !bSecondElementIsContext )
            sSecondary = pSecondInfo->sMessage;
    }

    // image
    lcl_positionInAppFont( *this, *m_aInfoImage.get(), OUTER_MARGIN, OUTER_MARGIN, IMAGE_SIZE, IMAGE_SIZE );
    m_aInfoImage->Show();

    // primary text
    lcl_positionInAppFont( *this, *m_aTitle.get(), TEXT_POS_X, OUTER_MARGIN, DIALOG_WIDTH - TEXT_POS_X - 2 * OUTER_MARGIN, 16 );
    sPrimary = lcl_stripOOoBaseVendor( sPrimary );
    m_aTitle->SetText( sPrimary );
    m_aTitle->Show();

    Rectangle aPrimaryRect( m_aTitle->GetPosPixel(), m_aTitle->GetSizePixel() );

    // secondary text (if applicable)
    m_aMessage->SetStyle( m_aMessage->GetStyle() | WB_NOLABEL );
    sSecondary = lcl_stripOOoBaseVendor( sSecondary );
    m_aMessage->SetText( sSecondary );

    lcl_positionInAppFont( *this, *m_aMessage.get(), TEXT_POS_X, OUTER_MARGIN + 16 + 3, DIALOG_WIDTH - TEXT_POS_X - 2 * OUTER_MARGIN, 8 );
    Rectangle aSecondaryRect( m_aMessage->GetPosPixel(), m_aMessage->GetSizePixel() );

    bool bHaveSecondaryText = !sSecondary.isEmpty();

    // determine which space the secondary text would occupy
    if ( bHaveSecondaryText )
        aSecondaryRect = GetTextRect( aSecondaryRect, sSecondary, DrawTextFlags::WordBreak | DrawTextFlags::MultiLine | DrawTextFlags::Left );
    else
        aSecondaryRect.Bottom() = aSecondaryRect.Top() - 1;

    // adjust secondary control height accordingly
    m_aMessage->SetSizePixel( aSecondaryRect.GetSize() );
    m_aMessage->Show( aSecondaryRect.GetHeight() > 0 );

    // if there's no secondary text ...
    if ( !bHaveSecondaryText )
    {   // then give the primary text as much horizontal space as it needs
        Rectangle aSuggestedRect( GetTextRect( aPrimaryRect, sPrimary, DrawTextFlags::WordBreak | DrawTextFlags::MultiLine | DrawTextFlags::Center ) );
        aPrimaryRect.Right() = aPrimaryRect.Left() + aSuggestedRect.GetWidth();
        aPrimaryRect.Bottom() = aPrimaryRect.Top() + aSuggestedRect.GetHeight();
        // and center it horizontally
        m_aTitle->SetStyle( ( m_aTitle->GetStyle() & ~WB_LEFT ) | WB_CENTER );

        Rectangle aInfoRect( m_aInfoImage->GetPosPixel(), m_aInfoImage->GetSizePixel() );
        // also, if it's not as high as the image ...
        if ( aPrimaryRect.GetHeight() < m_aInfoImage->GetSizePixel().Height() )
        {   // ... make it fit the image height
            aPrimaryRect.Bottom() += aInfoRect.GetHeight() - aPrimaryRect.GetHeight();
            // and center it vertically
            m_aTitle->SetStyle( m_aTitle->GetStyle() | WB_VCENTER );
        }
        else
        {   // ... otherwise, center the image vertically, relative to the primary text
            aInfoRect.Move( 0, ( aPrimaryRect.GetHeight() - aInfoRect.GetHeight() ) / 2 );
            m_aInfoImage->SetPosSizePixel( aInfoRect.TopLeft(), aInfoRect.GetSize() );
        }

        m_aTitle->SetPosSizePixel( aPrimaryRect.TopLeft(), aPrimaryRect.GetSize() );
    }

    // adjust dialog size accordingly
    const Rectangle& rBottomTextRect( bHaveSecondaryText ? aSecondaryRect : aPrimaryRect );
    Size aBorderSize = LogicToPixel( Size( OUTER_MARGIN, OUTER_MARGIN ), MAP_APPFONT );
    Size aDialogSize( LogicToPixel( Size( DIALOG_WIDTH, 30 ), MAP_APPFONT ) );
    aDialogSize.Height() = rBottomTextRect.Bottom() + aBorderSize.Height();
    aDialogSize.Width() = aPrimaryRect.Right() + aBorderSize.Width();

    SetSizePixel( aDialogSize );
    SetPageSizePixel( aDialogSize );
}

void OSQLMessageBox::impl_initImage( MessageType _eImage )
{
    switch (_eImage)
    {
        default:
            OSL_FAIL( "OSQLMessageBox::impl_initImage: unsupported image type!" );
            SAL_FALLTHROUGH;
        case Info:
            m_aInfoImage->SetImage(InfoBox::GetStandardImage());
            break;
        case Warning:
            m_aInfoImage->SetImage(WarningBox::GetStandardImage());
            break;
        case Error:
            m_aInfoImage->SetImage(ErrorBox::GetStandardImage());
            break;
        case Query:
            m_aInfoImage->SetImage(QueryBox::GetStandardImage());
            break;
    }
}

void OSQLMessageBox::impl_createStandardButtons( WinBits _nStyle )
{
    if ( _nStyle & WB_YES_NO_CANCEL )
    {
        lcl_addButton( *this, StandardButtonType::Yes,    ( _nStyle & WB_DEF_YES ) != 0 );
        lcl_addButton( *this, StandardButtonType::No,     ( _nStyle & WB_DEF_NO ) != 0 );
        lcl_addButton( *this, StandardButtonType::Cancel, ( _nStyle & WB_DEF_CANCEL ) != 0 );
    }
    else if ( _nStyle & WB_OK_CANCEL )
    {
        lcl_addButton( *this, StandardButtonType::OK,     ( _nStyle & WB_DEF_OK ) != 0 );
        lcl_addButton( *this, StandardButtonType::Cancel, ( _nStyle & WB_DEF_CANCEL ) != 0 );
    }
    else if ( _nStyle & WB_YES_NO )
    {
        lcl_addButton( *this, StandardButtonType::Yes,    ( _nStyle & WB_DEF_YES ) != 0 );
        lcl_addButton( *this, StandardButtonType::No,     ( _nStyle & WB_DEF_NO ) != 0 );
    }
    else if ( _nStyle & WB_RETRY_CANCEL )
    {
        lcl_addButton( *this, StandardButtonType::Retry,  ( _nStyle & WB_DEF_RETRY ) != 0 );
        lcl_addButton( *this, StandardButtonType::Cancel, ( _nStyle & WB_DEF_CANCEL ) != 0 );
    }
    else
    {
        OSL_ENSURE( WB_OK & _nStyle, "OSQLMessageBox::impl_createStandardButtons: unsupported dialog style requested!" );
        AddButton( StandardButtonType::OK, RET_OK, ButtonDialogFlags::Default | ButtonDialogFlags::Focus );
    }

    if ( !m_sHelpURL.isEmpty() )
    {
        lcl_addButton( *this, StandardButtonType::Help, false );

        OUString aTmp;
        INetURLObject aHID( m_sHelpURL );
        if ( aHID.GetProtocol() == INetProtocol::Hid )
              aTmp = aHID.GetURLPath();
        else
            aTmp = m_sHelpURL;

        SetHelpId( OUStringToOString( aTmp, RTL_TEXTENCODING_UTF8 ) );
    }
}

void OSQLMessageBox::impl_addDetailsButton()
{
    size_t nFirstPageVisible = m_aMessage->IsVisible() ? 2 : 1;

    bool bMoreDetailsAvailable = m_pImpl->aDisplayInfo.size() > nFirstPageVisible;
    if ( !bMoreDetailsAvailable )
    {
        // even if the text fits into what we can display, we might need to details button
        // if there is more non-trivial information in the errors than the mere messages
        for (   ExceptionDisplayChain::const_iterator error = m_pImpl->aDisplayInfo.begin();
                error != m_pImpl->aDisplayInfo.end();
                ++error
            )
        {
            if ( lcl_hasDetails( *error ) )
            {
                bMoreDetailsAvailable = true;
                break;
            }
        }
    }

    if ( bMoreDetailsAvailable )
    {
        AddButton( StandardButtonType::More, RET_MORE);
        PushButton* pButton = GetPushButton( RET_MORE );
        OSL_ENSURE( pButton, "OSQLMessageBox::impl_addDetailsButton: just added this button, why isn't it there?" );
        pButton->SetClickHdl( LINK( this, OSQLMessageBox, ButtonClickHdl ) );
    }
}

void OSQLMessageBox::Construct( WinBits _nStyle, MessageType _eImage )
{
    SetText( utl::ConfigManager::getProductName() + " Base" );

    // position and size the controls and the dialog, depending on whether we have one or two texts to display
    impl_positionControls();

    // init the image
    MessageType eType( _eImage );
    if ( eType == AUTO )
    {
        switch ( m_pImpl->aDisplayInfo[0].eType )
        {
        case SQLExceptionInfo::TYPE::SQLException: eType = Error;    break;
        case SQLExceptionInfo::TYPE::SQLWarning:   eType = Warning;  break;
        case SQLExceptionInfo::TYPE::SQLContext:   eType = Info;     break;
        default: OSL_FAIL( "OSQLMessageBox::Construct: invalid type!" );
        }
    }
    impl_initImage( eType );

    // create buttons
    impl_createStandardButtons( _nStyle );
    impl_addDetailsButton();
}

OSQLMessageBox::OSQLMessageBox(vcl::Window* _pParent, const SQLExceptionInfo& _rException, WinBits _nStyle, const OUString& _rHelpURL )
    :ButtonDialog( _pParent, WB_HORZ | WB_STDDIALOG )
    ,m_aInfoImage( VclPtr<FixedImage>::Create(this) )
    ,m_aTitle( VclPtr<FixedText>::Create(this, WB_WORDBREAK | WB_LEFT) )
    ,m_aMessage( VclPtr<FixedText>::Create(this, WB_WORDBREAK | WB_LEFT) )
    ,m_sHelpURL( _rHelpURL )
    ,m_pImpl( new SQLMessageBox_Impl( _rException ) )
{
    Construct( _nStyle, AUTO );
}

OSQLMessageBox::OSQLMessageBox( vcl::Window* _pParent, const OUString& _rTitle, const OUString& _rMessage, WinBits _nStyle, MessageType _eType, const ::dbtools::SQLExceptionInfo* _pAdditionalErrorInfo )
    :ButtonDialog( _pParent, WB_HORZ | WB_STDDIALOG )
    ,m_aInfoImage( VclPtr<FixedImage>::Create(this) )
    ,m_aTitle( VclPtr<FixedText>::Create(this, WB_WORDBREAK | WB_LEFT) )
    ,m_aMessage( VclPtr<FixedText>::Create(this, WB_WORDBREAK | WB_LEFT) )
{
    SQLContext aError;
    aError.Message = _rTitle;
    aError.Details = _rMessage;
    if ( _pAdditionalErrorInfo )
        aError.NextException = _pAdditionalErrorInfo->get();

    m_pImpl.reset( new SQLMessageBox_Impl( SQLExceptionInfo( aError ) ) );

    Construct( _nStyle, _eType );
}

OSQLMessageBox::~OSQLMessageBox()
{
    disposeOnce();
}

void OSQLMessageBox::dispose()
{
    m_aInfoImage.disposeAndClear();
    m_aTitle.disposeAndClear();
    m_aMessage.disposeAndClear();
    ButtonDialog::dispose();
}

IMPL_LINK_NOARG_TYPED( OSQLMessageBox, ButtonClickHdl, Button *, void )
{
    ScopedVclPtrInstance< OExceptionChainDialog > aDlg( this, m_pImpl->aDisplayInfo );
    aDlg->Execute();
}

// OSQLWarningBox
OSQLWarningBox::OSQLWarningBox( vcl::Window* _pParent, const OUString& _rMessage, WinBits _nStyle,
    const ::dbtools::SQLExceptionInfo* _pAdditionalErrorInfo )
    :OSQLMessageBox( _pParent, ModuleRes( STR_EXCEPTION_WARNING ), _rMessage, _nStyle, OSQLMessageBox::Warning, _pAdditionalErrorInfo )
{
}

// OSQLErrorBox
OSQLErrorBox::OSQLErrorBox( vcl::Window* _pParent, const OUString& _rMessage, WinBits _nStyle,
    const ::dbtools::SQLExceptionInfo* _pAdditionalErrorInfo )
    :OSQLMessageBox( _pParent, ModuleRes( STR_EXCEPTION_ERROR ), _rMessage, _nStyle, OSQLMessageBox::Error, _pAdditionalErrorInfo )
{
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
