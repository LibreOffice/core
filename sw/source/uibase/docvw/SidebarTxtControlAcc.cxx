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

#include <sal/config.h>

#include <memory>

#include <SidebarTxtControlAcc.hxx>

#include <SidebarTxtControl.hxx>

#include <svl/SfxBroadcaster.hxx>
#include <toolkit/awt/vclxaccessiblecomponent.hxx>
#include <com/sun/star/accessibility/XAccessibleEditableText.hpp>
#include <editeng/unoedsrc.hxx>
#include <editeng/unoforou.hxx>
#include <editeng/unoviwou.hxx>
#include <editeng/unoedhlp.hxx>
#include <svx/AccessibleTextHelper.hxx>
#include <editeng/outliner.hxx>
#include <o3tl/make_unique.hxx>

namespace sw { namespace sidebarwindows {

// declaration and implementation of <SvxEditSource>
// for <::accessibility::AccessibleTextHelper> instance
class SidebarTextEditSource : public SvxEditSource,
                              public SfxBroadcaster
{
    public:
        explicit SidebarTextEditSource(SidebarTextControl& rSidebarTextControl);
        virtual ~SidebarTextEditSource();

        virtual SvxEditSource* Clone() const override;

        virtual SvxTextForwarder* GetTextForwarder() override;
        virtual SvxViewForwarder* GetViewForwarder() override;
        virtual SvxEditViewForwarder* GetEditViewForwarder( bool bCreate = false ) override;

        virtual void UpdateData() override;

        virtual SfxBroadcaster& GetBroadcaster() const override;
        DECL_LINK_TYPED( NotifyHdl, EENotify&, void );

    private:
        SidebarTextControl& mrSidebarTextControl;
        SvxOutlinerForwarder mTextForwarder;
        SvxDrawOutlinerViewForwarder mViewForwarder;
};

SidebarTextEditSource::SidebarTextEditSource( SidebarTextControl& rSidebarTextControl )
    : SvxEditSource()
    , mrSidebarTextControl( rSidebarTextControl )
    , mTextForwarder( *(rSidebarTextControl.GetTextView()->GetOutliner()), false )
    , mViewForwarder( *(rSidebarTextControl.GetTextView()) )
{
    if ( mrSidebarTextControl.GetTextView() )
    {
        mrSidebarTextControl.GetTextView()->GetOutliner()->SetNotifyHdl( LINK(this, SidebarTextEditSource, NotifyHdl) );
    }
}

SidebarTextEditSource::~SidebarTextEditSource()
{
    if ( mrSidebarTextControl.GetTextView() )
    {
        mrSidebarTextControl.GetTextView()->GetOutliner()->SetNotifyHdl( Link<EENotify&,void>() );
    }
}

SvxEditSource* SidebarTextEditSource::Clone() const
{
    return new SidebarTextEditSource( mrSidebarTextControl );
}

SvxTextForwarder* SidebarTextEditSource::GetTextForwarder()
{
    return &mTextForwarder;
}

SvxViewForwarder* SidebarTextEditSource::GetViewForwarder()
{
    return &mViewForwarder;
}

SvxEditViewForwarder* SidebarTextEditSource::GetEditViewForwarder( bool /*bCreate*/ )
{
    return &mViewForwarder;
}

void SidebarTextEditSource::UpdateData()
{
    // nothing to do
}

SfxBroadcaster& SidebarTextEditSource::GetBroadcaster() const
{
    return *( const_cast< SidebarTextEditSource* > (this) );
}

IMPL_LINK_TYPED(SidebarTextEditSource, NotifyHdl, EENotify&, rNotify, void)
{
    std::unique_ptr< SfxHint > aHint( SvxEditSourceHelper::EENotification2Hint( &rNotify ) );

    if( aHint.get() )
    {
        Broadcast( *aHint.get() );
    }
}

// declaration and implementation of accessible context for <SidebarTextControl> instance
class SidebarTextControlAccessibleContext : public VCLXAccessibleComponent, public css::accessibility::XAccessibleEditableText
{
    public:
        explicit SidebarTextControlAccessibleContext( SidebarTextControl& rSidebarTextControl );
        virtual ~SidebarTextControlAccessibleContext();

        virtual sal_Int32 SAL_CALL
                getAccessibleChildCount()
                throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
                getAccessibleChild( sal_Int32 i )
                throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;

        virtual void SAL_CALL
                addAccessibleEventListener (
                    const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener)
                throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL
                removeAccessibleEventListener (
                    const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener)
                throw (css::uno::RuntimeException, std::exception) override;


        // XInterface
        virtual css::uno::Any SAL_CALL
                queryInterface( const css::uno::Type& aType )
                throw (css::uno::RuntimeException, std::exception) override;

        virtual void SAL_CALL
                acquire(  ) throw () override
                {};

        virtual void SAL_CALL
                release(  ) throw () override
                {};


        // XAccessibleText
        virtual sal_Int32 SAL_CALL
                getCaretPosition()
                throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL
                setCaretPosition( sal_Int32 nIndex )
                throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Unicode SAL_CALL
                getCharacter( sal_Int32 nIndex )
                throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL
                getCharacterAttributes( sal_Int32 nIndex, const css::uno::Sequence< OUString >& aRequestedAttributes )
                throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual css::awt::Rectangle SAL_CALL
                getCharacterBounds( sal_Int32 nIndex )
                throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL
                getCharacterCount()
                throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL
                getIndexAtPoint( const css::awt::Point& aPoint )
                throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL
                getSelectedText()
                throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL
                getSelectionStart()
                throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL
                getSelectionEnd()
                throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL
                setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
                throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL
                getText()
                throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL
                getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
                throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual css::accessibility::TextSegment SAL_CALL
                getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType )
                throw (css::lang::IndexOutOfBoundsException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual css::accessibility::TextSegment SAL_CALL
                getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType )
                throw (css::lang::IndexOutOfBoundsException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual css::accessibility::TextSegment SAL_CALL
                getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType )
                throw (css::lang::IndexOutOfBoundsException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL
                copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
                throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;


        // XAccessibleEditableText
        virtual sal_Bool SAL_CALL
                cutText ( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
                throw (css::lang::IndexOutOfBoundsException) override;
        virtual sal_Bool SAL_CALL
                pasteText ( sal_Int32 nIndex )
                throw (css::lang::IndexOutOfBoundsException) override;
        virtual sal_Bool SAL_CALL
                deleteText ( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
                throw (css::lang::IndexOutOfBoundsException) override;
        virtual sal_Bool SAL_CALL
                insertText ( const OUString& sText, sal_Int32 nIndex )
                throw (css::lang::IndexOutOfBoundsException) override;
        virtual sal_Bool SAL_CALL
                replaceText ( sal_Int32 nStartIndex, sal_Int32 nEndIndex, const OUString& sReplacement )
                throw (css::lang::IndexOutOfBoundsException) override;
        virtual sal_Bool SAL_CALL
                setAttributes ( sal_Int32 nStartIndex, sal_Int32 nEndIndex,
                const css::uno::Sequence<::com::sun::star::beans::PropertyValue>& aAttributeSet )
                throw (css::lang::IndexOutOfBoundsException) override;
        virtual sal_Bool SAL_CALL
                setText ( const OUString& sText )
                throw (css::uno::RuntimeException, std::exception) override;

    protected:
        virtual void ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;

    private:
        SidebarTextControl& mrSidebarTextControl;
        ::accessibility::AccessibleTextHelper* mpAccessibleTextHelper;

        ::osl::Mutex maMutex;

        void defunc();
};

SidebarTextControlAccessibleContext::SidebarTextControlAccessibleContext( SidebarTextControl& rSidebarTextControl )
    : VCLXAccessibleComponent( rSidebarTextControl.GetWindowPeer() )
    , mrSidebarTextControl( rSidebarTextControl )
    , mpAccessibleTextHelper( nullptr )
    , maMutex()
{
    mpAccessibleTextHelper = new ::accessibility::AccessibleTextHelper( o3tl::make_unique<SidebarTextEditSource>(mrSidebarTextControl) );
    if( rSidebarTextControl.HasFocus() )
        mpAccessibleTextHelper->SetFocus();
    mpAccessibleTextHelper->SetEventSource( mrSidebarTextControl.GetWindowPeer() );
}

SidebarTextControlAccessibleContext::~SidebarTextControlAccessibleContext()
{
    defunc();
}

void SidebarTextControlAccessibleContext::defunc()
{
    delete mpAccessibleTextHelper;
    mpAccessibleTextHelper = nullptr;
}

sal_Int32 SAL_CALL SidebarTextControlAccessibleContext::getAccessibleChildCount()
    throw (css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard aGuard( maMutex );

    sal_Int32 nChildCount( 0 );

    if ( mpAccessibleTextHelper )
    {
        nChildCount = mpAccessibleTextHelper->GetChildCount();
    }

    return nChildCount;
}

css::uno::Reference< css::accessibility::XAccessible > SAL_CALL SidebarTextControlAccessibleContext::getAccessibleChild( sal_Int32 i )
    throw ( css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard( maMutex );

    css::uno::Reference< css::accessibility::XAccessible > xChild;

    if ( mpAccessibleTextHelper )
    {
        xChild = mpAccessibleTextHelper->GetChild( i );
    }

    return xChild;
}

void SAL_CALL SidebarTextControlAccessibleContext::addAccessibleEventListener (
    const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener)
    throw (css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard aGuard( maMutex );

    if ( mpAccessibleTextHelper )
    {
        mpAccessibleTextHelper->AddEventListener(xListener);
    }
}

void SAL_CALL SidebarTextControlAccessibleContext::removeAccessibleEventListener (
    const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener)
    throw (css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard aGuard( maMutex );

    if ( mpAccessibleTextHelper )
    {
        mpAccessibleTextHelper->RemoveEventListener(xListener);
    }
}


// XInterface

css::uno::Any SAL_CALL SidebarTextControlAccessibleContext::queryInterface( const css::uno::Type& rType )
    throw (css::uno::RuntimeException, std::exception)
{
    css::uno::Any aRet;
    if ( rType == cppu::UnoType<XAccessibleEditableText>::get())
    {
        css::uno::Reference<XAccessibleEditableText> aAccEditText = this;
        aRet <<= aAccEditText;
    }

    return aRet;
}


// XAccesibleText

sal_Int32 SAL_CALL SidebarTextControlAccessibleContext::getCaretPosition()
    throw (css::uno::RuntimeException, std::exception)
{
    ESelection aSel = mrSidebarTextControl.GetTextView()->GetSelection();
    aSel.Adjust();

    return aSel.nEndPos;
}

sal_Bool SAL_CALL SidebarTextControlAccessibleContext::setCaretPosition( sal_Int32 nIndex )
    throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception)
{
    sal_Bool aRet = sal_False;
    OUString sText = getText();
    if( (nIndex >= 0) && (nIndex <= sText.getLength()) )
    {
        // Get the current paragraph
        ESelection aSel = mrSidebarTextControl.GetTextView()->GetSelection();
        aSel.nStartPos = nIndex;
        aSel.nEndPos = nIndex;
        mrSidebarTextControl.GetTextView()->SetSelection( aSel );
        aRet = sal_True;
    }
    else
        throw css::lang::IndexOutOfBoundsException();

    return aRet;
}

sal_Unicode SAL_CALL SidebarTextControlAccessibleContext::getCharacter( sal_Int32 nIndex )
    throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception)
{
    sal_Unicode aRet = sal_Unicode();
    OUString sText = getText();
    if( (nIndex >= 0) && (nIndex < sText.getLength()) )
    {
        aRet = sText[nIndex];
    }
    else
        throw css::lang::IndexOutOfBoundsException();

    return aRet;
}

css::uno::Sequence<css::beans::PropertyValue> SAL_CALL SidebarTextControlAccessibleContext::getCharacterAttributes(
    sal_Int32 nIndex,
    const css::uno::Sequence< OUString >& /*aRequestedAttributes*/ )
    throw (css::lang::IndexOutOfBoundsException,
           css::uno::RuntimeException,
           std::exception)
{
    css::uno::Sequence< css::beans::PropertyValue > aRet( 0 );
    OUString sText = getText();
    if( (nIndex >= 0) && (nIndex < sText.getLength()) )
    {
        //unimplemented stub
    }
    else
        throw css::lang::IndexOutOfBoundsException();

    return aRet;
}

css::awt::Rectangle SAL_CALL SidebarTextControlAccessibleContext::getCharacterBounds(
    sal_Int32 nIndex )
    throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception)
{
    css::awt::Rectangle aRet = css::awt::Rectangle();
    OUString sText = getText();
    if( (nIndex >= 0) && (nIndex <= sText.getLength()) )
    {
        //unimplemented stub
    }
    else
        throw css::lang::IndexOutOfBoundsException();

    return aRet;
}

sal_Int32 SAL_CALL SidebarTextControlAccessibleContext::getCharacterCount()
    throw (css::uno::RuntimeException, std::exception)
{
    return getText().getLength();
}

sal_Int32 SAL_CALL SidebarTextControlAccessibleContext::getIndexAtPoint( const css::awt::Point& /*rPoint*/ )
    throw (css::uno::RuntimeException, std::exception)
{
    //unimplemented stub
    sal_Int32 nRet = -1;
    return nRet;
}

OUString SAL_CALL SidebarTextControlAccessibleContext::getSelectedText()
    throw (css::uno::RuntimeException, std::exception)
{
    return mrSidebarTextControl.GetTextView()->GetSelected();
}

sal_Int32 SAL_CALL SidebarTextControlAccessibleContext::getSelectionStart()
    throw (css::uno::RuntimeException, std::exception)
{
    return mrSidebarTextControl.GetTextView()->GetSelection().nStartPos;
}

sal_Int32 SAL_CALL SidebarTextControlAccessibleContext::getSelectionEnd()
    throw (css::uno::RuntimeException, std::exception)
{
    return mrSidebarTextControl.GetTextView()->GetSelection().nEndPos;
}

sal_Bool SAL_CALL SidebarTextControlAccessibleContext::setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception)
{
    sal_Bool aRet = sal_False;
    OUString sText = getText();
    if( (nStartIndex >= 0) && (nStartIndex <= sText.getLength()) && (nEndIndex >= 0) && (nEndIndex <= sText.getLength()) )
    {
        // Get the current paragraph number
        ESelection aSel = mrSidebarTextControl.GetTextView()->GetSelection();
        aSel.nStartPos = nStartIndex;
        aSel.nEndPos = nEndIndex;
        mrSidebarTextControl.GetTextView()->SetSelection( aSel );
        aRet = sal_True;
    }
    else
        throw css::lang::IndexOutOfBoundsException();

    return aRet;
}

OUString SAL_CALL SidebarTextControlAccessibleContext::getText()
    throw (css::uno::RuntimeException, std::exception)
{
     return mrSidebarTextControl.GetTextView()->GetOutliner()->GetEditEngine().GetText();
}

OUString SAL_CALL SidebarTextControlAccessibleContext::getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception)
{
    OUString sRet = OUString();
    OUString sText = getText();
    if( (nStartIndex >= 0) && (nStartIndex <= sText.getLength()) && (nEndIndex >= 0) && (nEndIndex <= sText.getLength()) )
    {
        if( nStartIndex > nEndIndex)
        {
            sal_Int32 nTmp = nStartIndex;
            nStartIndex = nEndIndex;
            nEndIndex = nTmp;
        }
        sRet = sText.copy(nStartIndex, nEndIndex - nStartIndex);
    }
    else
        throw css::lang::IndexOutOfBoundsException();

    return sRet;
}

css::accessibility::TextSegment SAL_CALL SidebarTextControlAccessibleContext::getTextAtIndex( sal_Int32 nIndex, sal_Int16 /*nTextType*/ )
    throw (css::lang::IndexOutOfBoundsException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
{
    css::accessibility::TextSegment aRet = css::accessibility::TextSegment ();
    OUString sText = getText();
    if( (nIndex >= 0) && (nIndex <= sText.getLength()) )
    {
        //unimplemented stub
    }
    else
        throw css::lang::IndexOutOfBoundsException();

    return aRet;
}

css::accessibility::TextSegment SAL_CALL SidebarTextControlAccessibleContext::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 /*nTextType*/ )
    throw (css::lang::IndexOutOfBoundsException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
{
    css::accessibility::TextSegment aRet = css::accessibility::TextSegment ();
    OUString sText = getText();
    if( (nIndex >= 0) && (nIndex <= sText.getLength()) )
    {
        //unimplemented stub
    }
    else
        throw css::lang::IndexOutOfBoundsException();

    return aRet;
}

css::accessibility::TextSegment SAL_CALL SidebarTextControlAccessibleContext::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 /*nTextType*/ )
    throw (css::lang::IndexOutOfBoundsException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
{
    css::accessibility::TextSegment aRet = css::accessibility::TextSegment ();
    OUString sText = getText();
    if( (nIndex >= 0) && (nIndex <= sText.getLength()) )
    {
        //unimplemented stub
    }
    else
        throw css::lang::IndexOutOfBoundsException();

    return aRet;
}

sal_Bool SAL_CALL SidebarTextControlAccessibleContext::copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception)
{
    sal_Bool aRet = sal_False;
    OUString sText = getText();
    if( (nStartIndex >= 0) && (nStartIndex <= sText.getLength()) && (nEndIndex >= 0) && (nEndIndex <= sText.getLength()) )
    {
        //unimplemented stub
    }
    else
        throw css::lang::IndexOutOfBoundsException();

    return aRet;
}


// XAccesibleEditableText

sal_Bool SAL_CALL SidebarTextControlAccessibleContext::cutText( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    throw (css::lang::IndexOutOfBoundsException)
{
    sal_Bool aRet = sal_False;
    OUString sText = getText();
    if( (nStartIndex >= 0) && (nStartIndex <= sText.getLength()) && (nEndIndex >= 0) && (nEndIndex <= sText.getLength()) )
    {
        //unimplemented stub
    }
    else
        throw css::lang::IndexOutOfBoundsException();

    return aRet;
}

sal_Bool SAL_CALL SidebarTextControlAccessibleContext::pasteText( sal_Int32 nIndex )
    throw (css::lang::IndexOutOfBoundsException)
{
    sal_Bool aRet = sal_False;
    OUString sText = getText();
    if( (nIndex >= 0) && (nIndex <= sText.getLength()) )
    {
        //unimplemented stub
    }
    else
        throw css::lang::IndexOutOfBoundsException();

    return aRet;
}

sal_Bool SAL_CALL SidebarTextControlAccessibleContext::deleteText( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    throw (css::lang::IndexOutOfBoundsException)
{
    return replaceText( nStartIndex, nEndIndex, OUString() );
}

sal_Bool SAL_CALL SidebarTextControlAccessibleContext::insertText( const OUString& sText, sal_Int32 nIndex )
    throw (css::lang::IndexOutOfBoundsException)
{
    return replaceText( nIndex, nIndex, sText );
}

sal_Bool SAL_CALL SidebarTextControlAccessibleContext::replaceText( sal_Int32 nStartIndex, sal_Int32 nEndIndex, const OUString& sReplacement )
    throw (css::lang::IndexOutOfBoundsException)
{
    sal_Bool aRet = sal_False;
    OUString sText = getText();
    if( (nStartIndex >= 0) && (nStartIndex <= sText.getLength()) && (nEndIndex >= 0) && (nEndIndex <= sText.getLength()) )
    {
        ESelection aSel = mrSidebarTextControl.GetTextView()->GetSelection();
        aSel.nStartPos = nStartIndex;
        aSel.nEndPos = nEndIndex;
        mrSidebarTextControl.GetTextView()->GetOutliner()->QuickInsertText(sReplacement, aSel);
        aRet = sal_True;
    }
    else
        throw css::lang::IndexOutOfBoundsException();

    return aRet;
}

sal_Bool SAL_CALL SidebarTextControlAccessibleContext::setAttributes( sal_Int32 nStartIndex, sal_Int32 nEndIndex, const css::uno::Sequence<css::beans::PropertyValue>& /*rAttributeSet*/ )
    throw (css::lang::IndexOutOfBoundsException)
{
    sal_Bool aRet = sal_False;
    OUString sText = getText();
    if( (nStartIndex >= 0) && (nStartIndex <= sText.getLength()) && (nEndIndex >= 0) && (nEndIndex <= sText.getLength()) )
    {
        //unimplemented stub
    }
    else
        throw css::lang::IndexOutOfBoundsException();

    return aRet;
}

sal_Bool SAL_CALL SidebarTextControlAccessibleContext::setText( const OUString& sText )
    throw (css::uno::RuntimeException, std::exception)
{
    OUString sOldText = getText();
    return replaceText( 0, sOldText.getLength(), sText);
}




void SidebarTextControlAccessibleContext::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    if ( mpAccessibleTextHelper )
    {
        switch ( rVclWindowEvent.GetId() )
        {
            case VCLEVENT_OBJECT_DYING:
            {
                defunc();
            }
            break;
            case VCLEVENT_WINDOW_GETFOCUS:
            case VCLEVENT_CONTROL_GETFOCUS:
            {
                mpAccessibleTextHelper->SetFocus();
            }
            break;
            case VCLEVENT_WINDOW_LOSEFOCUS:
            case VCLEVENT_CONTROL_LOSEFOCUS:
            {
                mpAccessibleTextHelper->SetFocus( false );
            }
            break;
        }
    }

    VCLXAccessibleComponent::ProcessWindowEvent( rVclWindowEvent );
}

// implementation of accessible for <SidebarTextControl> instance
SidebarTextControlAccessible::SidebarTextControlAccessible( SidebarTextControl& rSidebarTextControl )
    : VCLXWindow()
    , mrSidebarTextControl( rSidebarTextControl )
{
    SetWindow( &mrSidebarTextControl );
}

SidebarTextControlAccessible::~SidebarTextControlAccessible()
{
}

css::uno::Reference< css::accessibility::XAccessibleContext > SidebarTextControlAccessible::CreateAccessibleContext()
{
    SidebarTextControlAccessibleContext* pAccContext(
                        new SidebarTextControlAccessibleContext( mrSidebarTextControl ) );
    css::uno::Reference< css::accessibility::XAccessibleContext > xAcc( pAccContext );
    return xAcc;
}

} } // end of namespace sw::sidebarwindows

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
