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

#include <core_resource.hxx>
#include <sqlmessage.hxx>
#include <dbu_dlg.hxx>
#include <strings.hrc>
#include <bitmaps.hlst>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/svapp.hxx>
#include <osl/diagnose.h>
#include <svtools/treelistbox.hxx>
#include <svtools/treelistentry.hxx>
#include <svtools/svmedit.hxx>
#include <connectivity/dbexception.hxx>
#include <connectivity/sqlerror.hxx>
#include <unotools/configmgr.hxx>
#include <sfx2/sfxuno.hxx>
#include <UITools.hxx>

#include <tools/urlobj.hxx>

#define RET_MORE   RET_RETRY + 1

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
        OUString m_defaultImageID;

        mutable Image   m_defaultImage;

    public:
        explicit ImageProvider(const OUString& _defaultImageID)
            :m_defaultImageID( _defaultImageID )
        {
        }

        Image const & getImage() const
        {
            if ( !m_defaultImage )
                m_defaultImage = Image(BitmapEx(m_defaultImageID));
            return m_defaultImage;
        }
    };

    class LabelProvider
    {
    private:
        OUString  m_label;
    public:
        explicit LabelProvider(const char* labelResourceID)
            : m_label(DBA_RES(labelResourceID))
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
            OUString sNormalImageID(BMP_EXCEPTION_ERROR);

            switch ( _eType )
            {
            case SQLExceptionInfo::TYPE::SQLWarning:
                ppProvider = &m_pWarningsImage;
                sNormalImageID = BMP_EXCEPTION_WARNING;
                break;

            case SQLExceptionInfo::TYPE::SQLContext:
                ppProvider = &m_pInfoImage;
                sNormalImageID = BMP_EXCEPTION_INFO;
                break;

            default:
                break;
            }

            if ( !ppProvider->get() )
                ppProvider->reset(new ImageProvider(sNormalImageID));
            return *ppProvider;
        }

        std::shared_ptr< LabelProvider > const & getLabelProvider( SQLExceptionInfo::TYPE _eType, bool _bSubLabel ) const
        {
            std::shared_ptr< LabelProvider >* ppProvider( &m_pErrorLabel );
            const char* pLabelID( STR_EXCEPTION_ERROR );

            switch ( _eType )
            {
            case SQLExceptionInfo::TYPE::SQLWarning:
                ppProvider = &m_pWarningsLabel;
                pLabelID = STR_EXCEPTION_WARNING;
                break;

            case SQLExceptionInfo::TYPE::SQLContext:
                ppProvider = &m_pInfoLabel;
                pLabelID = _bSubLabel ? STR_EXCEPTION_DETAILS : STR_EXCEPTION_INFO;
                break;
            default:
                break;
            }

            if ( !ppProvider->get() )
                ppProvider->reset( new LabelProvider( pLabelID ) );
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

    typedef std::vector< ExceptionDisplayInfo >   ExceptionDisplayChain;

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
    virtual ~OExceptionChainDialog() override { disposeOnce(); }
    virtual void dispose() override
    {
        m_pExceptionList.clear();
        m_pExceptionText.clear();
        ModalDialog::dispose();
    }

protected:
    DECL_LINK(OnExceptionSelected, SvTreeListBox*, void);
};

OExceptionChainDialog::OExceptionChainDialog(vcl::Window* pParent, const ExceptionDisplayChain& _rExceptions)
    : ModalDialog(pParent, "SQLExceptionDialog", "dbaccess/ui/sqlexception.ui")
    , m_aExceptions(_rExceptions)
{
    get(m_pExceptionList, "list");
    Size aListSize(LogicToPixel(Size(85, 93), MapMode(MapUnit::MapAppFont)));
    m_pExceptionList->set_width_request(aListSize.Width());
    m_pExceptionList->set_height_request(aListSize.Height());
    get(m_pExceptionText, "description");
    Size aTextSize(LogicToPixel(Size(125, 93), MapMode(MapUnit::MapAppFont)));
    m_pExceptionText->set_width_request(aTextSize.Width());
    m_pExceptionText->set_height_request(aTextSize.Height());

    m_sStatusLabel = DBA_RES( STR_EXCEPTION_STATUS );
    m_sErrorCodeLabel = DBA_RES( STR_EXCEPTION_ERRORCODE );

    m_pExceptionList->SetSelectionMode(SelectionMode::Single);
    m_pExceptionList->SetDragDropMode(DragDropMode::NONE);
    m_pExceptionList->EnableInplaceEditing(false);
    m_pExceptionList->SetStyle(m_pExceptionList->GetStyle() | WB_HASLINES | WB_HASBUTTONS | WB_HASBUTTONSATROOT | WB_HSCROLL);

    m_pExceptionList->SetSelectHdl(LINK(this, OExceptionChainDialog, OnExceptionSelected));
    m_pExceptionList->SetNodeDefaultImages( );

    bool bHave22018 = false;
    size_t elementPos = 0;

    for (auto const& elem : m_aExceptions)
    {
        lcl_insertExceptionEntry(*m_pExceptionList, elementPos, elem);
        bHave22018 = elem.sSQLState == "22018";
        ++elementPos;
    }

    // if the error has the code 22018, then add an additional explanation
    // #i24021#
    if ( bHave22018 )
    {
        ProviderFactory aProviderFactory;

        ExceptionDisplayInfo aInfo22018;
        aInfo22018.sMessage = DBA_RES( STR_EXPLAN_STRINGCONVERSION_ERROR );
        aInfo22018.pLabelProvider = aProviderFactory.getLabelProvider( SQLExceptionInfo::TYPE::SQLContext, false );
        aInfo22018.pImageProvider = aProviderFactory.getImageProvider( SQLExceptionInfo::TYPE::SQLContext );
        m_aExceptions.push_back( aInfo22018 );

        lcl_insertExceptionEntry( *m_pExceptionList, m_aExceptions.size() - 1, aInfo22018 );
    }
}

IMPL_LINK_NOARG(OExceptionChainDialog, OnExceptionSelected, SvTreeListBox*, void)
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
            sText += m_sStatusLabel + ": " + aExceptionInfo.sSQLState + "\n";
        }

        if ( !aExceptionInfo.sErrorCode.isEmpty() )
        {
            sText += m_sErrorCodeLabel + ": " + aExceptionInfo.sErrorCode + "\n";
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
    void lcl_addButton(weld::MessageDialog* pDialog, StandardButtonType eType, bool bDefault)
    {
        sal_uInt16 nButtonID = 0;
        switch (eType)
        {
            case StandardButtonType::Yes:
                nButtonID = RET_YES;
                pDialog->add_button(Button::GetStandardText(StandardButtonType::Yes), nButtonID);
                break;
            case StandardButtonType::No:
                nButtonID = RET_NO;
                pDialog->add_button(Button::GetStandardText(StandardButtonType::No), nButtonID);
                break;
            case StandardButtonType::OK:
                nButtonID = RET_OK;
                pDialog->add_button(Button::GetStandardText(StandardButtonType::OK), nButtonID);
                break;
            case StandardButtonType::Cancel:
                nButtonID = RET_CANCEL;
                pDialog->add_button(Button::GetStandardText(StandardButtonType::Cancel), nButtonID);
                break;
            case StandardButtonType::Retry:
                nButtonID = RET_RETRY;
                pDialog->add_button(Button::GetStandardText(StandardButtonType::Retry), nButtonID);
                break;
            case StandardButtonType::Help:
                nButtonID = RET_HELP;
                pDialog->add_button(Button::GetStandardText(StandardButtonType::Help), nButtonID);
                break;
            default:
                OSL_FAIL( "lcl_addButton: invalid button id!" );
                break;
        }
        if (bDefault)
           pDialog->set_default_response(nButtonID);
    }
}

void OSQLMessageBox::impl_fillMessages()
{
    OSL_PRECOND( !m_pImpl->aDisplayInfo.empty(), "OSQLMessageBox::impl_fillMessages: nothing to display at all?" );

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

    // primary text
    m_xDialog->set_primary_text(lcl_stripOOoBaseVendor(sPrimary));

    // secondary text (if applicable)
    m_xDialog->set_secondary_text(lcl_stripOOoBaseVendor(sSecondary));
}

void OSQLMessageBox::impl_createStandardButtons( MessBoxStyle _nStyle )
{
    if ( _nStyle & MessBoxStyle::YesNoCancel )
    {
        lcl_addButton(m_xDialog.get(), StandardButtonType::Yes,    bool(_nStyle & MessBoxStyle::DefaultYes));
        lcl_addButton(m_xDialog.get(), StandardButtonType::No,     bool(_nStyle & MessBoxStyle::DefaultNo));
        lcl_addButton(m_xDialog.get(), StandardButtonType::Cancel, bool(_nStyle & MessBoxStyle::DefaultCancel));
    }
    else if ( _nStyle & MessBoxStyle::OkCancel )
    {
        lcl_addButton(m_xDialog.get(), StandardButtonType::OK,     bool(_nStyle & MessBoxStyle::DefaultOk));
        lcl_addButton(m_xDialog.get(), StandardButtonType::Cancel, bool(_nStyle & MessBoxStyle::DefaultCancel));
    }
    else if ( _nStyle & MessBoxStyle::YesNo )
    {
        lcl_addButton(m_xDialog.get(), StandardButtonType::Yes,    bool(_nStyle & MessBoxStyle::DefaultYes));
        lcl_addButton(m_xDialog.get(), StandardButtonType::No,     bool(_nStyle & MessBoxStyle::DefaultNo));
    }
    else if ( _nStyle & MessBoxStyle::RetryCancel )
    {
        lcl_addButton(m_xDialog.get(), StandardButtonType::Retry,  bool(_nStyle & MessBoxStyle::DefaultRetry));
        lcl_addButton(m_xDialog.get(), StandardButtonType::Cancel, bool(_nStyle & MessBoxStyle::DefaultCancel));
    }
    else if ( _nStyle & MessBoxStyle::Ok )
    {
        lcl_addButton(m_xDialog.get(), StandardButtonType::OK,     true);
    }

    if ( !m_sHelpURL.isEmpty() )
    {
        lcl_addButton(m_xDialog.get(), StandardButtonType::Help, false);

        OUString aTmp;
        INetURLObject aHID( m_sHelpURL );
        if ( aHID.GetProtocol() == INetProtocol::Hid )
              aTmp = aHID.GetURLPath();
        else
            aTmp = m_sHelpURL;

        m_xDialog->set_help_id(OUStringToOString(aTmp, RTL_TEXTENCODING_UTF8));
    }
}

void OSQLMessageBox::impl_addDetailsButton()
{
    size_t nFirstPageVisible = m_xDialog->get_secondary_text().isEmpty() ? 1 : 2;

    bool bMoreDetailsAvailable = m_pImpl->aDisplayInfo.size() > nFirstPageVisible;
    if ( !bMoreDetailsAvailable )
    {
        // even if the text fits into what we can display, we might need to details button
        // if there is more non-trivial information in the errors than the mere messages
        for (auto const& error : m_pImpl->aDisplayInfo)
        {
            if ( lcl_hasDetails(error) )
            {
                bMoreDetailsAvailable = true;
                break;
            }
        }
    }

    if ( bMoreDetailsAvailable )
    {
        m_xDialog->add_button(Button::GetStandardText(StandardButtonType::More), RET_MORE);
        //TO-DO
        //PushButton* pButton = GetPushButton( RET_MORE );
        //OSL_ENSURE( pButton, "OSQLMessageBox::impl_addDetailsButton: just added this button, why isn't it there?" );
        //pButton->SetClickHdl( LINK( this, OSQLMessageBox, ButtonClickHdl ) );
    }
}

void OSQLMessageBox::Construct(weld::Window* pParent, MessBoxStyle _nStyle, MessageType _eImage)
{
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
    VclMessageType eMessageType;
    switch (eType)
    {
        default:
            OSL_FAIL( "OSQLMessageBox::impl_initImage: unsupported image type!" );
            SAL_FALLTHROUGH;
        case Info:
            eMessageType = VclMessageType::Info;
            break;
        case Warning:
            eMessageType = VclMessageType::Warning;
            break;
        case Error:
            eMessageType = VclMessageType::Error;
            break;
        case Query:
            eMessageType = VclMessageType::Question;
            break;
    }

    m_xDialog.reset(Application::CreateMessageDialog(pParent, eMessageType, VclButtonsType::NONE, ""));
    m_xDialog->set_title(utl::ConfigManager::getProductName() + " Base");

    impl_fillMessages();

    // create buttons
    impl_createStandardButtons( _nStyle );
    impl_addDetailsButton();
}

OSQLMessageBox::OSQLMessageBox(weld::Window* pParent, const SQLExceptionInfo& rException, MessBoxStyle nStyle, const OUString& rHelpURL)
    : m_pImpl(new SQLMessageBox_Impl(rException))
    , m_sHelpURL(rHelpURL)
{
    Construct(pParent, nStyle, AUTO);
}

OSQLMessageBox::OSQLMessageBox(weld::Window* pParent, const OUString& rTitle, const OUString& rMessage, MessBoxStyle nStyle, MessageType eType, const ::dbtools::SQLExceptionInfo* pAdditionalErrorInfo )
{
    SQLContext aError;
    aError.Message = rTitle;
    aError.Details = rMessage;
    if (pAdditionalErrorInfo)
        aError.NextException = pAdditionalErrorInfo->get();

    m_pImpl.reset(new SQLMessageBox_Impl(SQLExceptionInfo(aError)));

    Construct(pParent, nStyle, eType);
}

OSQLMessageBox::~OSQLMessageBox()
{
}

IMPL_LINK_NOARG(OSQLMessageBox, ButtonClickHdl, weld::Button&, void)
{
    ScopedVclPtrInstance<OExceptionChainDialog> aDlg(nullptr /*TODO*/, m_pImpl->aDisplayInfo);
    aDlg->Execute();
}

// OSQLWarningBox
OSQLWarningBox::OSQLWarningBox(weld::Window* pParent, const OUString& rMessage, MessBoxStyle nStyle,
                               const ::dbtools::SQLExceptionInfo* pAdditionalErrorInfo )
    : OSQLMessageBox(pParent, DBA_RES(STR_EXCEPTION_WARNING), rMessage, nStyle, MessageType::Warning, pAdditionalErrorInfo)
{
}

// OSQLErrorBox
OSQLErrorBox::OSQLErrorBox(weld::Window* pParent, const OUString& rMessage)
    : OSQLMessageBox(pParent, DBA_RES(STR_EXCEPTION_ERROR), rMessage, MessBoxStyle::Ok | MessBoxStyle::DefaultOk,
                     MessageType::Error, nullptr)
{
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
