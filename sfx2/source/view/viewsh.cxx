/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <config_features.h>

#include <boost/property_tree/json_parser.hpp>

#include <sal/log.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/whiter.hxx>
#include <utility>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/weld.hxx>
#include <svl/intitem.hxx>
#include <svtools/langhelp.hxx>
#include <com/sun/star/awt/XPopupMenu.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/EmbedMisc.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/container/XContainerQuery.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardListener.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp>
#include <com/sun/star/view/XRenderable.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/ui/XAcceleratorConfiguration.hpp>

#include <cppuhelper/weakref.hxx>

#include <com/sun/star/accessibility/XAccessibleTextAttributes.hpp>
#include <com/sun/star/accessibility/AccessibleTextType.hpp>
#include <com/sun/star/awt/FontSlant.hpp>

#include <comphelper/diagnose_ex.hxx>
#include <tools/urlobj.hxx>
#include <unotools/tempfile.hxx>
#include <svtools/soerr.hxx>
#include <tools/svborder.hxx>

#include <framework/actiontriggerhelper.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/settings.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include <officecfg/Setup.hxx>
#include <sfx2/app.hxx>
#include <sfx2/flatpak.hxx>
#include <sfx2/viewsh.hxx>
#include "viewimp.hxx"
#include <sfx2/sfxresid.hxx>
#include <sfx2/request.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/strings.hrc>
#include <sfx2/sfxbasecontroller.hxx>
#include <sfx2/mailmodelapi.hxx>
#include <bluthsndapi.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/event.hxx>
#include <sfx2/ipclient.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/objface.hxx>
#include <sfx2/lokhelper.hxx>
#include <sfx2/lokcallback.hxx>
#include <openuriexternally.hxx>
#include <iostream>
#include <vector>
#include <libxml/xmlwriter.h>
#include <unordered_map>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::cppu;

#define ShellClass_SfxViewShell
#include <sfxslots.hxx>


class SfxClipboardChangeListener : public ::cppu::WeakImplHelper<
    datatransfer::clipboard::XClipboardListener >
{
public:
    SfxClipboardChangeListener( SfxViewShell* pView, uno::Reference< datatransfer::clipboard::XClipboardNotifier > xClpbrdNtfr );

    // XEventListener
    virtual void SAL_CALL disposing( const lang::EventObject& rEventObject ) override;

    // XClipboardListener
    virtual void SAL_CALL changedContents( const datatransfer::clipboard::ClipboardEvent& rEventObject ) override;

    void DisconnectViewShell() { m_pViewShell = nullptr; }
    void ChangedContents();

    enum AsyncExecuteCmd
    {
        ASYNCEXECUTE_CMD_DISPOSING,
        ASYNCEXECUTE_CMD_CHANGEDCONTENTS
    };

    struct AsyncExecuteInfo
    {
        AsyncExecuteInfo( AsyncExecuteCmd eCmd, SfxClipboardChangeListener* pListener ) :
            m_eCmd( eCmd ), m_xListener( pListener ) {}

        AsyncExecuteCmd m_eCmd;
        rtl::Reference<SfxClipboardChangeListener> m_xListener;
    };

private:
    SfxViewShell* m_pViewShell;
    uno::Reference< datatransfer::clipboard::XClipboardNotifier > m_xClpbrdNtfr;
    uno::Reference< lang::XComponent > m_xCtrl;

    DECL_STATIC_LINK( SfxClipboardChangeListener, AsyncExecuteHdl_Impl, void*, void );
};

SfxClipboardChangeListener::SfxClipboardChangeListener( SfxViewShell* pView, uno::Reference< datatransfer::clipboard::XClipboardNotifier > xClpbrdNtfr )
  : m_pViewShell( nullptr ), m_xClpbrdNtfr(std::move( xClpbrdNtfr )), m_xCtrl(pView->GetController())
{
    if ( m_xCtrl.is() )
    {
        m_xCtrl->addEventListener( uno::Reference < lang::XEventListener > ( static_cast < lang::XEventListener* >( this ) ) );
        m_pViewShell = pView;
    }
    if ( m_xClpbrdNtfr.is() )
    {
        m_xClpbrdNtfr->addClipboardListener( uno::Reference< datatransfer::clipboard::XClipboardListener >(
            static_cast< datatransfer::clipboard::XClipboardListener* >( this )));
    }
}

void SfxClipboardChangeListener::ChangedContents()
{
    const SolarMutexGuard aGuard;
    if (!m_pViewShell)
        return;

    SfxBindings& rBind = m_pViewShell->GetViewFrame()->GetBindings();
    rBind.Invalidate(SID_PASTE);
    rBind.Invalidate(SID_PASTE_SPECIAL);
    rBind.Invalidate(SID_CLIPBOARD_FORMAT_ITEMS);

    if (comphelper::LibreOfficeKit::isActive())
    {
        // In the future we might send the payload as well.
        SfxLokHelper::notifyAllViews(LOK_CALLBACK_CLIPBOARD_CHANGED, "");
    }
}

IMPL_STATIC_LINK( SfxClipboardChangeListener, AsyncExecuteHdl_Impl, void*, p, void )
{
    AsyncExecuteInfo* pAsyncExecuteInfo = static_cast<AsyncExecuteInfo*>(p);
    if ( pAsyncExecuteInfo )
    {
        if ( pAsyncExecuteInfo->m_xListener.is() )
        {
            if ( pAsyncExecuteInfo->m_eCmd == ASYNCEXECUTE_CMD_DISPOSING )
                pAsyncExecuteInfo->m_xListener->DisconnectViewShell();
            else if ( pAsyncExecuteInfo->m_eCmd == ASYNCEXECUTE_CMD_CHANGEDCONTENTS )
                pAsyncExecuteInfo->m_xListener->ChangedContents();
        }
    }
    delete pAsyncExecuteInfo;
}

void SAL_CALL SfxClipboardChangeListener::disposing( const lang::EventObject& /*rEventObject*/ )
{
    // Either clipboard or ViewShell is going to be destroyed -> no interest in listening anymore
    uno::Reference< lang::XComponent > xCtrl( m_xCtrl );
    uno::Reference< datatransfer::clipboard::XClipboardNotifier > xNotify( m_xClpbrdNtfr );

    uno::Reference< datatransfer::clipboard::XClipboardListener > xThis( static_cast< datatransfer::clipboard::XClipboardListener* >( this ));
    if ( xCtrl.is() )
        xCtrl->removeEventListener( uno::Reference < lang::XEventListener > ( static_cast < lang::XEventListener* >( this )));
    if ( xNotify.is() )
        xNotify->removeClipboardListener( xThis );

    // Make asynchronous call to avoid locking SolarMutex which is the
    // root for many deadlocks, especially in conjunction with the "Windows"
    // based single thread apartment clipboard code!
    AsyncExecuteInfo* pInfo = new AsyncExecuteInfo( ASYNCEXECUTE_CMD_DISPOSING, this );
    if (!Application::PostUserEvent( LINK( nullptr, SfxClipboardChangeListener, AsyncExecuteHdl_Impl ), pInfo ))
        delete pInfo;
}

void SAL_CALL SfxClipboardChangeListener::changedContents( const datatransfer::clipboard::ClipboardEvent& )
{
    // Make asynchronous call to avoid locking SolarMutex which is the
    // root for many deadlocks, especially in conjunction with the "Windows"
    // based single thread apartment clipboard code!
    AsyncExecuteInfo* pInfo = new AsyncExecuteInfo( ASYNCEXECUTE_CMD_CHANGEDCONTENTS, this );
    if (!Application::PostUserEvent( LINK( nullptr, SfxClipboardChangeListener, AsyncExecuteHdl_Impl ), pInfo ))
        delete pInfo;
}

class LOKDocumentFocusListener :
    public ::cppu::WeakImplHelper< accessibility::XAccessibleEventListener >
{
    static constexpr sal_Int64 MAX_ATTACHABLE_CHILDREN = 30;

    const SfxViewShell* m_pViewShell;
    std::set< uno::Reference< uno::XInterface > > m_aRefList;
    OUString m_sFocusedParagraph;
    bool m_bFocusedParagraphNotified;
    sal_Int32 m_nCaretPosition;
    sal_Int32 m_nSelectionStart;
    sal_Int32 m_nSelectionEnd;
    OUString m_sSelectedText;
    bool m_bIsEditingCell;
    OUString m_sSelectedCellAddress;

public:
    LOKDocumentFocusListener(const SfxViewShell* pViewShell);

    /// @throws lang::IndexOutOfBoundsException
    /// @throws uno::RuntimeException
    void attachRecursive(
        const uno::Reference< accessibility::XAccessible >& xAccessible
    );

    /// @throws lang::IndexOutOfBoundsException
    /// @throws uno::RuntimeException
    void attachRecursive(
        const uno::Reference< accessibility::XAccessible >& xAccessible,
        const uno::Reference< accessibility::XAccessibleContext >& xContext
    );

    /// @throws lang::IndexOutOfBoundsException
    /// @throws uno::RuntimeException
    void attachRecursive(
        const uno::Reference< accessibility::XAccessible >& xAccessible,
        const uno::Reference< accessibility::XAccessibleContext >& xContext,
        const sal_Int64 nStateSet
    );

    /// @throws lang::IndexOutOfBoundsException
    /// @throws uno::RuntimeException
    void detachRecursive(
        const uno::Reference< accessibility::XAccessible >& xAccessible
    );

    /// @throws lang::IndexOutOfBoundsException
    /// @throws uno::RuntimeException
    void detachRecursive(
        const uno::Reference< accessibility::XAccessibleContext >& xContext
    );

    /// @throws lang::IndexOutOfBoundsException
    /// @throws uno::RuntimeException
    void detachRecursive(
        const uno::Reference< accessibility::XAccessibleContext >& xContext,
        const sal_Int64 nStateSet
    );

    /// @throws lang::IndexOutOfBoundsException
    /// @throws uno::RuntimeException
    static uno::Reference< accessibility::XAccessible > getAccessible(const lang::EventObject& aEvent );

    // XEventListener
    virtual void SAL_CALL disposing( const lang::EventObject& Source ) override;

    // XAccessibleEventListener
    virtual void SAL_CALL notifyEvent( const accessibility::AccessibleEventObject& aEvent ) override;

    void notifyFocusedParagraphChanged();
    void notifyCaretChanged();
    void notifyTextSelectionChanged();

    OUString getFocusedParagraph() const;
    int getCaretPosition() const;
};

LOKDocumentFocusListener::LOKDocumentFocusListener(const SfxViewShell* pViewShell)
    : m_pViewShell(pViewShell)
    , m_bFocusedParagraphNotified(false)
    , m_nCaretPosition(0)
    , m_nSelectionStart(0)
    , m_nSelectionEnd(0)
    , m_bIsEditingCell(false)
{
}

OUString LOKDocumentFocusListener::getFocusedParagraph() const
{
    SAL_INFO("lok.a11y", "LOKDocumentFocusListener::getFocusedParagraph: " << m_sFocusedParagraph);
    const_cast<LOKDocumentFocusListener*>(this)->m_bFocusedParagraphNotified = true;

    sal_Int32 nSelectionStart = m_nSelectionStart;
    sal_Int32 nSelectionEnd = m_nSelectionEnd;
    if (nSelectionStart < 0 || nSelectionEnd < 0)
        nSelectionStart = nSelectionEnd = m_nCaretPosition;

    boost::property_tree::ptree aPayloadTree;
    aPayloadTree.put("content", m_sFocusedParagraph.toUtf8().getStr());
    aPayloadTree.put("start", nSelectionStart);
    aPayloadTree.put("end", nSelectionEnd);
    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aPayloadTree);
    std::string aPayload = aStream.str();
    OUString sRet = OUString::fromUtf8(aPayload);
    return sRet;
}

int LOKDocumentFocusListener::getCaretPosition() const
{
    SAL_INFO("lok.a11y", "LOKDocumentFocusListener::getCaretPosition: " << m_nCaretPosition);
    return m_nCaretPosition;
}

void LOKDocumentFocusListener::notifyFocusedParagraphChanged()
{
    boost::property_tree::ptree aPayloadTree;
    aPayloadTree.put("content", m_sFocusedParagraph.toUtf8().getStr());
    aPayloadTree.put("position", m_nCaretPosition);
    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aPayloadTree);
    std::string aPayload = aStream.str();
    if (m_pViewShell)
    {
        SAL_INFO("lok.a11y", "LOKDocumentFocusListener::notifyFocusedParagraphChanged: " << m_sFocusedParagraph);
        m_bFocusedParagraphNotified = true;
        m_pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_A11Y_FOCUS_CHANGED, aPayload.c_str());
    }
}

void LOKDocumentFocusListener::notifyCaretChanged()
{
    boost::property_tree::ptree aPayloadTree;
    aPayloadTree.put("position", m_nCaretPosition);
    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aPayloadTree);
    std::string aPayload = aStream.str();
    if (m_pViewShell)
    {
        SAL_INFO("lok.a11y", "LOKDocumentFocusListener::notifyCaretChanged: " << m_nCaretPosition);
        m_pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_A11Y_CARET_CHANGED, aPayload.c_str());
    }
}

void LOKDocumentFocusListener::notifyTextSelectionChanged()
{
    boost::property_tree::ptree aPayloadTree;
    aPayloadTree.put("start", m_nSelectionStart);
    aPayloadTree.put("end", m_nSelectionEnd);
    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aPayloadTree);
    std::string aPayload = aStream.str();
    if (m_pViewShell)
    {
        SAL_INFO("lok.a11y", "LOKDocumentFocusListener::notifyTextSelectionChanged: "
                << "start: " << m_nSelectionStart << ", end: " << m_nSelectionEnd);
        m_pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_A11Y_TEXT_SELECTION_CHANGED, aPayload.c_str());
    }
}

void LOKDocumentFocusListener::disposing( const lang::EventObject& aEvent )
{
    // Unref the object here, but do not remove as listener since the object
    // might no longer be in a state that safely allows this.
    if( aEvent.Source.is() )
        m_aRefList.erase(aEvent.Source);

}

namespace
{
bool hasState(const accessibility::AccessibleEventObject& aEvent, ::sal_Int64 nState)
{
    bool res = false;
    uno::Reference< accessibility::XAccessibleContext > xContext(aEvent.Source, uno::UNO_QUERY);
    if (xContext.is())
    {
        ::sal_Int64 nStateSet = xContext->getAccessibleStateSet();
        res = (nStateSet & nState) != 0;
    }
    return res;
}

bool isFocused(const accessibility::AccessibleEventObject& aEvent)
{
    return hasState(aEvent, accessibility::AccessibleStateType::FOCUSED);
}
} // anonymous namespace

void LOKDocumentFocusListener::notifyEvent( const accessibility::AccessibleEventObject& aEvent )
{
    try
    {
        switch( aEvent.EventId )
        {
            case accessibility::AccessibleEventId::STATE_CHANGED:
            {
                uno::Reference< accessibility::XAccessible > xAccStateChanged = getAccessible(aEvent);
                sal_Int64 nState = accessibility::AccessibleStateType::INVALID;
                aEvent.NewValue >>= nState;
                SAL_INFO("lok.a11y", "LOKDocumentFocusListener::notifyEvent: "
                        << "STATE_CHANGED: XAccessible: " << xAccStateChanged.get() << ", nState: " << nState);

                if( accessibility::AccessibleStateType::FOCUSED == nState )
                {
                    SAL_INFO("lok.a11y", "LOKDocumentFocusListener::notifyEvent: FOCUSED");

                    if (m_bIsEditingCell)
                    {
                        if (!hasState(aEvent, accessibility::AccessibleStateType::ACTIVE))
                        {
                            SAL_WARN("lok.a11y",
                                    "LOKDocumentFocusListener::notifyEvent: FOCUSED: Cell not ACTIVE for editing yet");
                            return;
                        }
                    }
                    uno::Reference<css::accessibility::XAccessibleText> xAccText(xAccStateChanged, uno::UNO_QUERY);
                    if( xAccText.is() )
                    {
                        OUString sText = xAccText->getText();
                        sal_Int32 nLength = sText.getLength();
                        sal_Int32 nCaretPosition = xAccText->getCaretPosition();
                        SAL_INFO("lok.a11y", "LOKDocumentFocusListener::notifyEvent: xAccText: " << xAccText.get()
                                << ", text: >" << sText << "<, caret pos: " << nCaretPosition);

                        if (nLength)
                        {
                            css::uno::Reference<css::accessibility::XAccessibleTextAttributes>
                                xAccTextAttr(xAccText, uno::UNO_QUERY);
                            css::uno::Sequence< OUString > aRequestedAttributes;

                            sal_Int32 nPos = 0;
                            while (nPos < nLength)
                            {
                                css::accessibility::TextSegment aTextSegment =
                                        xAccText->getTextAtIndex(nPos, css::accessibility::AccessibleTextType::ATTRIBUTE_RUN);
                                SAL_INFO("lok.a11y", "LOKDocumentFocusListener::notifyEvent: "
                                        << "text segment: '" << aTextSegment.SegmentText
                                        << "', start: " << aTextSegment.SegmentStart
                                        << ", end: " << aTextSegment.SegmentEnd);

                                css::uno::Sequence< css::beans::PropertyValue > aRunAttributeList;
                                if (xAccTextAttr.is())
                                {
                                    aRunAttributeList = xAccTextAttr->getRunAttributes(nPos, aRequestedAttributes);
                                }
                                else
                                {
                                    aRunAttributeList = xAccText->getCharacterAttributes(nPos, aRequestedAttributes);
                                }

                                sal_Int32 nSize = aRunAttributeList.getLength();
                                SAL_INFO("lok.a11y",
                                         "LOKDocumentFocusListener::notifyEvent: attribute list size: " << nSize);
                                if (nSize)
                                {
                                    OUString sValue;
                                    OUString sAttributes = "{ ";
                                    for (const auto& attribute: aRunAttributeList)
                                    {
                                        if (attribute.Name.isEmpty())
                                            continue;

                                        if (attribute.Name == "CharHeight" || attribute.Name == "CharWeight")
                                        {
                                            float fValue;
                                            attribute.Value >>= fValue;
                                            sValue = OUString::number(fValue);
                                        }
                                        else if (attribute.Name == "CharPosture")
                                        {
                                            awt::FontSlant nValue;
                                            attribute.Value >>= nValue;
                                            sValue = OUString::number((unsigned int)(nValue));
                                        }
                                        else if (attribute.Name == "CharUnderline")
                                        {
                                            sal_Int16 nValue;
                                            attribute.Value >>= nValue;
                                            sValue = OUString::number(nValue);
                                        }
                                        else if (attribute.Name == "CharFontName")
                                        {
                                            attribute.Value >>= sValue;
                                        }
                                        else if (attribute.Name == "Rsid")
                                        {
                                            sal_uInt32 nValue;
                                            attribute.Value >>= nValue;
                                            sValue = OUString::number(nValue);
                                        }

                                        if (!sValue.isEmpty())
                                        {
                                            if (sAttributes != "{ ")
                                                sAttributes += ", ";
                                            sAttributes += attribute.Name + ": ";
                                            sAttributes += sValue;
                                            sValue = "";
                                        }
                                    }
                                    sAttributes += " }";
                                    SAL_INFO("lok.a11y",
                                             "LOKDocumentFocusListener::notifyEvent: attributes: " << sAttributes);
                                    }
                                nPos = aTextSegment.SegmentEnd + 1;
                            }
                        }
                        if (!m_bFocusedParagraphNotified || m_sFocusedParagraph != sText)
                        {
                            m_sFocusedParagraph = sText;
                            m_nCaretPosition = nCaretPosition;
                            notifyFocusedParagraphChanged();
                        }
                    }
                }

                break;
            }

            case accessibility::AccessibleEventId::CARET_CHANGED:
            {
                if (!isFocused(aEvent))
                {
                    SAL_WARN("lok.a11y",
                             "LOKDocumentFocusListener::notifyEvent: CARET_CHANGED: skip non focused paragraph");
                    return;
                }

                sal_Int32 nNewPos = -1;
                aEvent.NewValue >>= nNewPos;
                sal_Int32 nOldPos = -1;
                aEvent.OldValue >>= nOldPos;

                if (nNewPos >= 0)
                {
                    SAL_INFO("lok.a11y", "LOKDocumentFocusListener::notifyEvent: CARET_CHANGED: " <<
                            "new pos: " << nNewPos << ", nOldPos: " << nOldPos);
                    uno::Reference<css::accessibility::XAccessibleText>
                        xAccText(getAccessible(aEvent), uno::UNO_QUERY);
                    if( xAccText.is() )
                    {
                        OUString sText = xAccText->getText();
                        SAL_INFO("lok.a11y", "LOKDocumentFocusListener::notifyEvent: CARET_CHANGED: "
                                << "xAccText: " << xAccText.get() << ", text: >" << sText << "<");

                        m_nCaretPosition = nNewPos;
                        m_nSelectionStart = m_nSelectionEnd = m_nCaretPosition;
                        notifyCaretChanged();
                    }
                }

                break;
            }

            case accessibility::AccessibleEventId::TEXT_CHANGED:
            {
                if (!isFocused(aEvent))
                {
                    SAL_WARN("lok.a11y",
                             "LOKDocumentFocusListener::notifyEvent: TEXT_CHANGED: skip non focused paragraph");
                    return;
                }

                accessibility::TextSegment aDeletedText;
                accessibility::TextSegment aInsertedText;

                if (aEvent.OldValue >>= aDeletedText)
                {
                    SAL_INFO("lok.a11y", "LOKDocumentFocusListener::notifyEvent: TEXT_CHANGED: "
                            << "deleted text: >" << aDeletedText.SegmentText << "<");
                }
                if (aEvent.NewValue >>= aInsertedText)
                {
                    SAL_INFO("lok.a11y", "LOKDocumentFocusListener::notifyEvent: TEXT_CHANGED: "
                            << "inserted text: >" << aInsertedText.SegmentText << "<");
                }
                uno::Reference<css::accessibility::XAccessibleText> xAccText(getAccessible(aEvent), uno::UNO_QUERY);
                if (xAccText.is())
                {
                    OUString sText = xAccText->getText();
                    SAL_INFO("lok.a11y", "LOKDocumentFocusListener::notifyEvent: TEXT_CHANGED: "
                            << "xAccText: " << xAccText.get() << ", text: >" << sText << "<");
                    m_sFocusedParagraph = sText;
                    m_bFocusedParagraphNotified = false;
                }

                break;
            }
            case accessibility::AccessibleEventId::TEXT_SELECTION_CHANGED:
            {
                if (!isFocused(aEvent))
                {
                    SAL_WARN("lok.a11y",
                          "LOKDocumentFocusListener::notifyEvent: TEXT_SELECTION_CHANGED: skip non focused paragraph");
                    return;
                }

                uno::Reference<css::accessibility::XAccessibleText> xAccText(getAccessible(aEvent), uno::UNO_QUERY);
                if (xAccText.is())
                {
                    OUString sText = xAccText->getText();
                    sal_Int32 nSelectionStart = xAccText->getSelectionStart();
                    sal_Int32 nSelectionEnd = xAccText->getSelectionEnd();
                    m_sSelectedText = xAccText->getSelectedText();

                    SAL_INFO("lok.a11y", "LOKDocumentFocusListener::notifyEvent: TEXT_SELECTION_CHANGED: "
                            << "\n  xAccText: " << xAccText.get() << ", text: >" << sText << "<"
                            << "\n  start: " << nSelectionStart << ", end: " << nSelectionEnd
                            << "\n  selected text: >" << m_sSelectedText << "<");

                    // This should not be risky since selection start/end are set also on CARET_CHANGED event
                    if (nSelectionStart == m_nSelectionStart && nSelectionEnd == m_nSelectionEnd)
                        return;

                    // We send a message to client also when start/end are -1, in this way the client knows
                    // if a text selection object exists or not. That's needed because of the odd behavior
                    // occurring when <backspace>/<delete> are hit and a text selection is empty but it still exists.
                    // Such keys delete the empty selection instead of the previous/next char.
                    m_nSelectionStart = nSelectionStart;
                    m_nSelectionEnd = nSelectionEnd;

                    // Calc: when editing a formula send the update content
                    if (m_bIsEditingCell && !m_sSelectedCellAddress.isEmpty()
                            && !m_sSelectedText.isEmpty() && sText.startsWith("="))
                    {
                        notifyFocusedParagraphChanged();
                    }
                    notifyTextSelectionChanged();
                }

                break;
            }
            case accessibility::AccessibleEventId::SELECTION_CHANGED:
            {
                uno::Reference< accessibility::XAccessible > xNewValue;
                aEvent.NewValue >>= xNewValue;
                if (xNewValue.is())
                {
                    uno::Reference< accessibility::XAccessibleContext > xContext =
                            xNewValue->getAccessibleContext();

                    if (xContext.is())
                    {
                        OUString sName = xContext->getAccessibleName();
                        SAL_INFO("lok.a11y", "LOKDocumentFocusListener::notifyEvent: SELECTION_CHANGED: this: " << this
                                << ", selected cell address: >" << sName << "<"
                                << ", m_bIsEditingCell: " << m_bIsEditingCell);
                        if (m_bIsEditingCell && !sName.isEmpty())
                        {
                            m_sSelectedCellAddress = sName;
                            // Check cell address: "$Sheet1.A10".
                            // On cell editing SELECTION_CHANGED is not emitted when selection is expanded.
                            // So selection can't be a cell range.
                            sal_Int32 nDotIndex = m_sSelectedText.indexOf('.');
                            OUString sCellAddress = m_sSelectedText.copy(nDotIndex + 1);
                            SAL_INFO("lok.a11y", "LOKDocumentFocusListener::notifyEvent: SELECTION_CHANGED: "
                                    << "cell address: >" << sCellAddress << "<");
                            if (m_sSelectedCellAddress == sCellAddress)
                            {
                                notifyFocusedParagraphChanged();
                                notifyTextSelectionChanged();
                            }
                        }
                    }
                }
                break;
            }
            case accessibility::AccessibleEventId::CHILD:
            {
                uno::Reference< accessibility::XAccessible > xChild;
                if( (aEvent.OldValue >>= xChild) && xChild.is() )
                    detachRecursive(xChild);

                if( (aEvent.NewValue >>= xChild) && xChild.is() )
                    attachRecursive(xChild);

                break;
            }

            case accessibility::AccessibleEventId::INVALIDATE_ALL_CHILDREN:
                SAL_INFO("lok.a11y", "Invalidate all children called");
                break;

            default:
                break;
        }
    }
    catch( const lang::IndexOutOfBoundsException& )
    {
        SAL_WARN("lok.a11y", "Focused object has invalid index in parent");
    }
}

uno::Reference< accessibility::XAccessible > LOKDocumentFocusListener::getAccessible(const lang::EventObject& aEvent )
{
    uno::Reference< accessibility::XAccessible > xAccessible(aEvent.Source, uno::UNO_QUERY);

    if( xAccessible.is() )
        return xAccessible;

    uno::Reference< accessibility::XAccessibleContext > xContext(aEvent.Source, uno::UNO_QUERY);

    if( xContext.is() )
    {
        uno::Reference< accessibility::XAccessible > xParent( xContext->getAccessibleParent() );
        if( xParent.is() )
        {
            uno::Reference< accessibility::XAccessibleContext > xParentContext( xParent->getAccessibleContext() );
            if( xParentContext.is() )
            {
                return xParentContext->getAccessibleChild( xContext->getAccessibleIndexInParent() );
            }
        }
    }

    return uno::Reference< accessibility::XAccessible >();
}

void LOKDocumentFocusListener::attachRecursive(
    const uno::Reference< accessibility::XAccessible >& xAccessible
)
{
    SAL_INFO("lok.a11y", "LOKDocumentFocusListener::attachRecursive(1): xAccessible: " << xAccessible.get());

    uno::Reference< accessibility::XAccessibleContext > xContext =
        xAccessible->getAccessibleContext();

    if( xContext.is() )
        attachRecursive(xAccessible, xContext);
}

void LOKDocumentFocusListener::attachRecursive(
    const uno::Reference< accessibility::XAccessible >& xAccessible,
    const uno::Reference< accessibility::XAccessibleContext >& xContext
)
{
    SAL_INFO("lok.a11y", "LOKDocumentFocusListener::attachRecursive(2): xAccessible: " << xAccessible.get()
            << ", role: " << xContext->getAccessibleRole()
            << ", name: " << xContext->getAccessibleName()
            << ", parent: " << xContext->getAccessibleParent().get()
            << ", child count: " << xContext->getAccessibleChildCount());

    sal_Int64 nStateSet = xContext->getAccessibleStateSet();

    if (!m_bIsEditingCell)
    {
        ::rtl::OUString sName = xContext->getAccessibleName();
        m_bIsEditingCell = sName.startsWith("Cell");
    }

    attachRecursive(xAccessible, xContext, nStateSet);
}

void LOKDocumentFocusListener::attachRecursive(
    const uno::Reference< accessibility::XAccessible >& xAccessible,
    const uno::Reference< accessibility::XAccessibleContext >& xContext,
    const sal_Int64 nStateSet
)
{
    SAL_INFO("lok.a11y", "LOKDocumentFocusListener::attachRecursive(3) #1: this: " << this
            << ", xAccessible: " << xAccessible.get()
            << ", role: " << xContext->getAccessibleRole()
            << ", name: " << xContext->getAccessibleName()
            << ", parent: " << xContext->getAccessibleParent().get()
            << ", child count: " << xContext->getAccessibleChildCount());

    uno::Reference< accessibility::XAccessibleEventBroadcaster > xBroadcaster =
        uno::Reference< accessibility::XAccessibleEventBroadcaster >(xContext, uno::UNO_QUERY);

    if (!xBroadcaster.is())
        return;
    SAL_INFO("lok.a11y", "LOKDocumentFocusListener::attachRecursive(3) #2: xBroadcaster.is()");
    // If not already done, add the broadcaster to the list and attach as listener.
    const uno::Reference< uno::XInterface >& xInterface = xBroadcaster;
    if( m_aRefList.insert(xInterface).second )
    {
        SAL_INFO("lok.a11y", "LOKDocumentFocusListener::attachRecursive(3) #3: m_aRefList.insert(xInterface).second");
        xBroadcaster->addAccessibleEventListener(static_cast< accessibility::XAccessibleEventListener *>(this));


        if( !(nStateSet & accessibility::AccessibleStateType::MANAGES_DESCENDANTS) )
        {
            sal_Int64 nmax = xContext->getAccessibleChildCount();
            if( nmax > MAX_ATTACHABLE_CHILDREN )
                nmax = MAX_ATTACHABLE_CHILDREN;

            for( sal_Int64 n = 0; n < nmax; n++ )
            {
                uno::Reference< accessibility::XAccessible > xChild( xContext->getAccessibleChild( n ) );

                if( xChild.is() )
                    attachRecursive(xChild);
            }
        }
    }
}

void LOKDocumentFocusListener::detachRecursive(
    const uno::Reference< accessibility::XAccessible >& xAccessible
)
{
    uno::Reference< accessibility::XAccessibleContext > xContext =
        xAccessible->getAccessibleContext();

    if( xContext.is() )
        detachRecursive(xContext);
}

void LOKDocumentFocusListener::detachRecursive(
    const uno::Reference< accessibility::XAccessibleContext >& xContext
)
{
    sal_Int64 nStateSet = xContext->getAccessibleStateSet();

    SAL_INFO("lok.a11y", "LOKDocumentFocusListener::detachRecursive(2): this: " << this
            << ", name: " << xContext->getAccessibleName()
            << ", parent: " << xContext->getAccessibleParent().get()
            << ", child count: " << xContext->getAccessibleChildCount());

    if (m_bIsEditingCell)
    {
        ::rtl::OUString sName = xContext->getAccessibleName();
        m_bIsEditingCell = !sName.startsWith("Cell");
        if (!m_bIsEditingCell)
        {
            m_sFocusedParagraph = "";
            m_nCaretPosition = 0;
            notifyFocusedParagraphChanged();
        }
    }

    detachRecursive(xContext, nStateSet);
}

void LOKDocumentFocusListener::detachRecursive(
    const uno::Reference< accessibility::XAccessibleContext >& xContext,
    const sal_Int64 nStateSet
)
{
    uno::Reference< accessibility::XAccessibleEventBroadcaster > xBroadcaster =
        uno::Reference< accessibility::XAccessibleEventBroadcaster >(xContext, uno::UNO_QUERY);

    if( xBroadcaster.is() && 0 < m_aRefList.erase(xBroadcaster) )
    {
        xBroadcaster->removeAccessibleEventListener(static_cast< accessibility::XAccessibleEventListener *>(this));

        if( !( nStateSet & accessibility::AccessibleStateType::MANAGES_DESCENDANTS ) )
        {
            sal_Int64 nmax = xContext->getAccessibleChildCount();
            if( nmax > MAX_ATTACHABLE_CHILDREN )
                nmax = MAX_ATTACHABLE_CHILDREN;

            for( sal_Int64 n = 0; n < nmax; n++ )
            {
                uno::Reference< accessibility::XAccessible > xChild( xContext->getAccessibleChild( n ) );

                if( xChild.is() )
                    detachRecursive(xChild);
            }
        }
    }
}

sal_uInt32 SfxViewShell_Impl::m_nLastViewShellId = 0;

SfxViewShell_Impl::SfxViewShell_Impl(SfxViewShellFlags const nFlags, ViewShellDocId nDocId)
:   m_bHasPrintOptions(nFlags & SfxViewShellFlags::HAS_PRINTOPTIONS)
,   m_nFamily(0xFFFF)   // undefined, default set by TemplateDialog
,   m_pLibreOfficeKitViewCallback(nullptr)
,   m_bTiledSearching(false)
,   m_nViewShellId(SfxViewShell_Impl::m_nLastViewShellId++)
,   m_nDocId(nDocId)
{
}

SfxViewShell_Impl::~SfxViewShell_Impl()
{
}

std::vector< SfxInPlaceClient* >& SfxViewShell_Impl::GetIPClients_Impl()
{
    return maIPClients;
}

SFX_IMPL_SUPERCLASS_INTERFACE(SfxViewShell,SfxShell)

void SfxViewShell::InitInterface_Impl()
{
}


/** search for a filter name dependent on type and module
 */
static OUString impl_retrieveFilterNameFromTypeAndModule(
    const css::uno::Reference< css::container::XContainerQuery >& rContainerQuery,
    const OUString& rType,
    const OUString& rModuleIdentifier,
    const sal_Int32 nFlags )
{
    // Retrieve filter from type
    css::uno::Sequence< css::beans::NamedValue > aQuery {
        { "Type", css::uno::Any( rType ) },
        { "DocumentService", css::uno::Any( rModuleIdentifier ) }
    };

    css::uno::Reference< css::container::XEnumeration > xEnumeration =
        rContainerQuery->createSubSetEnumerationByProperties( aQuery );

    OUString aFoundFilterName;
    while ( xEnumeration->hasMoreElements() )
    {
        ::comphelper::SequenceAsHashMap aFilterPropsHM( xEnumeration->nextElement() );
        OUString aFilterName = aFilterPropsHM.getUnpackedValueOrDefault(
            "Name",
            OUString() );

        sal_Int32 nFilterFlags = aFilterPropsHM.getUnpackedValueOrDefault(
            "Flags",
            sal_Int32( 0 ) );

        if ( nFilterFlags & nFlags )
        {
            aFoundFilterName = aFilterName;
            break;
        }
    }

    return aFoundFilterName;
}

namespace {

/** search for an internal typename, which map to the current app module
    and map also to a "family" of file formats as e.g. PDF/MS Doc/OOo Doc.
 */
enum ETypeFamily
{
    E_MS_DOC,
    E_OOO_DOC
};

}

static OUString impl_searchFormatTypeForApp(const css::uno::Reference< css::frame::XFrame >& xFrame     ,
                                                  ETypeFamily                                eTypeFamily)
{
    try
    {
        css::uno::Reference< css::uno::XComponentContext >  xContext      (::comphelper::getProcessComponentContext());
        css::uno::Reference< css::frame::XModuleManager2 >  xModuleManager(css::frame::ModuleManager::create(xContext));

        OUString sModule = xModuleManager->identify(xFrame);
        OUString sType   ;

        switch(eTypeFamily)
        {
            case E_MS_DOC:
            {
                if ( sModule == "com.sun.star.text.TextDocument" )
                    sType = "writer_MS_Word_2007";
                else
                if ( sModule == "com.sun.star.sheet.SpreadsheetDocument" )
                    sType = "MS Excel 2007 XML";
                else
                if ( sModule == "com.sun.star.presentation.PresentationDocument" )
                    sType = "MS PowerPoint 2007 XML";
            }
            break;

            case E_OOO_DOC:
            {
                if ( sModule == "com.sun.star.text.TextDocument" )
                    sType = "writer8";
                else
                if ( sModule == "com.sun.star.sheet.SpreadsheetDocument" )
                    sType = "calc8";
                else
                if ( sModule == "com.sun.star.drawing.DrawingDocument" )
                    sType = "draw8";
                else
                if ( sModule == "com.sun.star.presentation.PresentationDocument" )
                    sType = "impress8";
            }
            break;
        }

        return sType;
    }
    catch (const css::uno::RuntimeException&)
    {
        throw;
    }
    catch (const css::uno::Exception&)
    {
    }

    return OUString();
}

void SfxViewShell::NewIPClient_Impl( SfxInPlaceClient *pIPClient )
{
    pImpl->GetIPClients_Impl().push_back(pIPClient);
}

void SfxViewShell::IPClientGone_Impl( SfxInPlaceClient const *pIPClient )
{
    std::vector< SfxInPlaceClient* >& pClients = pImpl->GetIPClients_Impl();

    auto it = std::find(pClients.begin(), pClients.end(), pIPClient);
    if (it != pClients.end())
        pClients.erase( it );
}


void SfxViewShell::ExecMisc_Impl( SfxRequest &rReq )
{
    const sal_uInt16 nId = rReq.GetSlot();
    switch( nId )
    {
        case SID_STYLE_FAMILY :
        {
            const SfxUInt16Item* pItem = rReq.GetArg<SfxUInt16Item>(nId);
            if (pItem)
            {
                pImpl->m_nFamily = pItem->GetValue();
            }
            break;
        }
        case SID_ACTIVATE_STYLE_APPLY:
        {
            uno::Reference< frame::XFrame > xFrame =
                GetViewFrame()->GetFrame().GetFrameInterface();

            Reference< beans::XPropertySet > xPropSet( xFrame, UNO_QUERY );
            Reference< frame::XLayoutManager > xLayoutManager;
            if ( xPropSet.is() )
            {
                try
                {
                    Any aValue = xPropSet->getPropertyValue("LayoutManager");
                    aValue >>= xLayoutManager;
                    if ( xLayoutManager.is() )
                    {
                        uno::Reference< ui::XUIElement > xElement = xLayoutManager->getElement( "private:resource/toolbar/textobjectbar" );
                        if(!xElement.is())
                        {
                            xElement = xLayoutManager->getElement( "private:resource/toolbar/frameobjectbar" );
                        }
                        if(!xElement.is())
                        {
                            xElement = xLayoutManager->getElement( "private:resource/toolbar/oleobjectbar" );
                        }
                        if(xElement.is())
                        {
                            uno::Reference< awt::XWindow > xWin( xElement->getRealInterface(), uno::UNO_QUERY_THROW );
                            VclPtr<vcl::Window> pWin = VCLUnoHelper::GetWindow( xWin );
                            ToolBox* pTextToolbox = dynamic_cast< ToolBox* >( pWin.get() );
                            if( pTextToolbox )
                            {
                                ToolBox::ImplToolItems::size_type nItemCount = pTextToolbox->GetItemCount();
                                for( ToolBox::ImplToolItems::size_type nItem = 0; nItem < nItemCount; ++nItem )
                                {
                                    ToolBoxItemId nItemId = pTextToolbox->GetItemId( nItem );
                                    const OUString& rCommand = pTextToolbox->GetItemCommand( nItemId );
                                    if (rCommand == ".uno:StyleApply")
                                    {
                                        vcl::Window* pItemWin = pTextToolbox->GetItemWindow( nItemId );
                                        if( pItemWin )
                                            pItemWin->GrabFocus();
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
                catch (const Exception&)
                {
                }
            }
            rReq.Done();
        }
        break;

        case SID_MAIL_SENDDOCASMS:
        case SID_MAIL_SENDDOCASOOO:
        case SID_MAIL_SENDDOCASPDF:
        case SID_MAIL_SENDDOC:
        case SID_MAIL_SENDDOCASFORMAT:
        {
            SfxObjectShell* pDoc = GetObjectShell();
            if ( pDoc && pDoc->QueryHiddenInformation(
                             HiddenWarningFact::WhenSaving, GetViewFrame()->GetFrameWeld() ) != RET_YES )
                break;


            SfxMailModel  aModel;
            OUString aDocType;

            const SfxStringItem* pMailRecipient = rReq.GetArg<SfxStringItem>(SID_MAIL_RECIPIENT);
            if ( pMailRecipient )
            {
                OUString aRecipient( pMailRecipient->GetValue() );
                OUString aMailToStr("mailto:");

                if ( aRecipient.startsWith( aMailToStr ) )
                    aRecipient = aRecipient.copy( aMailToStr.getLength() );
                aModel.AddToAddress( aRecipient );
            }
            const SfxStringItem* pMailDocType = rReq.GetArg<SfxStringItem>(SID_TYPE_NAME);
            if ( pMailDocType )
                aDocType = pMailDocType->GetValue();

            uno::Reference < frame::XFrame > xFrame( pFrame->GetFrame().GetFrameInterface() );
            SfxMailModel::SendMailResult eResult = SfxMailModel::SEND_MAIL_ERROR;

            if ( nId == SID_MAIL_SENDDOC )
                eResult = aModel.SaveAndSend( xFrame, OUString() );
            else if ( nId == SID_MAIL_SENDDOCASPDF )
                eResult = aModel.SaveAndSend( xFrame, "pdf_Portable_Document_Format");
            else if ( nId == SID_MAIL_SENDDOCASMS )
            {
                aDocType = impl_searchFormatTypeForApp(xFrame, E_MS_DOC);
                if (!aDocType.isEmpty())
                    eResult = aModel.SaveAndSend( xFrame, aDocType );
            }
            else if ( nId == SID_MAIL_SENDDOCASOOO )
            {
                aDocType = impl_searchFormatTypeForApp(xFrame, E_OOO_DOC);
                if (!aDocType.isEmpty())
                    eResult = aModel.SaveAndSend( xFrame, aDocType );
            }

            if ( eResult == SfxMailModel::SEND_MAIL_ERROR )
            {
                weld::Window* pWin = SfxGetpApp()->GetTopWindow();
                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(pWin,
                                                                         VclMessageType::Info, VclButtonsType::Ok,
                                                                         SfxResId(STR_ERROR_SEND_MAIL)));
                xBox->run();
                rReq.Ignore();
            }
            else
                rReq.Done();
        }
        break;

        case SID_BLUETOOTH_SENDDOC:
        {
            SfxBluetoothModel aModel;
            SfxObjectShell* pDoc = GetObjectShell();
            if ( pDoc && pDoc->QueryHiddenInformation(
                            HiddenWarningFact::WhenSaving, GetViewFrame()->GetFrameWeld() ) != RET_YES )
                break;
            uno::Reference < frame::XFrame > xFrame( pFrame->GetFrame().GetFrameInterface() );
            SfxMailModel::SendMailResult eResult = aModel.SaveAndSend( xFrame );
            if( eResult == SfxMailModel::SEND_MAIL_ERROR )
            {
                weld::Window* pWin = SfxGetpApp()->GetTopWindow();
                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(pWin,
                                                                         VclMessageType::Info, VclButtonsType::Ok,
                                                                         SfxResId(STR_ERROR_SEND_MAIL)));
                xBox->run();
                rReq.Ignore();
            }
            else
                rReq.Done();
        }
        break;

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_WEBHTML:
        {
            css::uno::Reference< lang::XMultiServiceFactory > xSMGR(::comphelper::getProcessServiceFactory(), css::uno::UNO_SET_THROW);
            css::uno::Reference< uno::XComponentContext >     xContext(::comphelper::getProcessComponentContext(), css::uno::UNO_SET_THROW);
            css::uno::Reference< css::frame::XFrame >         xFrame( pFrame->GetFrame().GetFrameInterface() );
            css::uno::Reference< css::frame::XModel >         xModel;

            css::uno::Reference< css::frame::XModuleManager2 > xModuleManager( css::frame::ModuleManager::create(xContext) );

            OUString aModule;
            try
            {
                aModule = xModuleManager->identify( xFrame );
            }
            catch (const css::uno::RuntimeException&)
            {
                throw;
            }
            catch (const css::uno::Exception&)
            {
            }

            if ( xFrame.is() )
            {
                css::uno::Reference< css::frame::XController > xController = xFrame->getController();
                if ( xController.is() )
                    xModel = xController->getModel();
            }

            // We need at least a valid module name and model reference
            css::uno::Reference< css::frame::XStorable > xStorable( xModel, css::uno::UNO_QUERY );
            if ( xModel.is() && xStorable.is() )
            {
                OUString aFilterName;
                OUString aTypeName( "generic_HTML" );
                OUString aFileName;

                OUString aLocation = xStorable->getLocation();
                INetURLObject aFileObj( aLocation );

                bool bPrivateProtocol = ( aFileObj.GetProtocol() == INetProtocol::PrivSoffice );
                bool bHasLocation = !aLocation.isEmpty() && !bPrivateProtocol;

                css::uno::Reference< css::container::XContainerQuery > xContainerQuery(
                    xSMGR->createInstance( "com.sun.star.document.FilterFactory" ),
                    css::uno::UNO_QUERY_THROW );

                // Retrieve filter from type

                sal_Int32 nFilterFlags = 0x00000002; // export
                aFilterName = impl_retrieveFilterNameFromTypeAndModule( xContainerQuery, aTypeName, aModule, nFilterFlags );
                if ( aFilterName.isEmpty() )
                {
                    // Draw/Impress uses a different type. 2nd chance try to use alternative type name
                    aFilterName = impl_retrieveFilterNameFromTypeAndModule(
                        xContainerQuery, "graphic_HTML", aModule, nFilterFlags );
                }

                // No filter found => error
                // No type and no location => error
                if ( aFilterName.isEmpty() ||  aTypeName.isEmpty())
                {
                    rReq.Done();
                    return;
                }

                // Use provided save file name. If empty determine file name
                if ( !bHasLocation )
                {
                    // Create a default file name with the correct extension
                    aFileName = "webpreview";
                }
                else
                {
                    // Determine file name from model
                    INetURLObject aFObj( xStorable->getLocation() );
                    aFileName = aFObj.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::NONE );
                }

                OSL_ASSERT( !aFilterName.isEmpty() );
                OSL_ASSERT( !aFileName.isEmpty() );

                // Creates a temporary directory to store our predefined file into it (for the
                // flatpak case, create it in XDG_CACHE_HOME instead of /tmp for technical reasons,
                // so that it can be accessed by the browser running outside the sandbox):
                OUString * parent = nullptr;
                if (flatpak::isFlatpak() && !flatpak::createTemporaryHtmlDirectory(&parent))
                {
                    SAL_WARN("sfx.view", "cannot create Flatpak html temp dir");
                }

                INetURLObject aFilePathObj( ::utl::CreateTempURL(parent, true) );
                aFilePathObj.insertName( aFileName );
                aFilePathObj.setExtension( u"htm" );

                OUString aFileURL = aFilePathObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );

                css::uno::Sequence< css::beans::PropertyValue > aArgs{
                    comphelper::makePropertyValue("FilterName", aFilterName)
                };

                // Store document in the html format
                try
                {
                    xStorable->storeToURL( aFileURL, aArgs );
                }
                catch (const io::IOException&)
                {
                    rReq.Done();
                    return;
                }

                sfx2::openUriExternally(aFileURL, true, rReq.GetFrameWeld());
                rReq.Done(true);
                break;
            }
            else
            {
                rReq.Done();
                return;
            }
        }
    }
}


void SfxViewShell::GetState_Impl( SfxItemSet &rSet )
{

    SfxWhichIter aIter( rSet );
    SfxObjectShell *pSh = GetViewFrame()->GetObjectShell();
    for ( sal_uInt16 nSID = aIter.FirstWhich(); nSID; nSID = aIter.NextWhich() )
    {
        switch ( nSID )
        {

            case SID_BLUETOOTH_SENDDOC:
            case SID_MAIL_SENDDOC:
            case SID_MAIL_SENDDOCASFORMAT:
            case SID_MAIL_SENDDOCASMS:
            case SID_MAIL_SENDDOCASOOO:
            case SID_MAIL_SENDDOCASPDF:
            {
#if HAVE_FEATURE_MACOSX_SANDBOX
                rSet.DisableItem(nSID);
#endif
                if (pSh && pSh->isExportLocked())
                    rSet.DisableItem(nSID);
                break;
            }
            case SID_WEBHTML:
            {
                if (pSh && pSh->isExportLocked())
                    rSet.DisableItem(nSID);
                break;
            }
            // Printer functions
            case SID_PRINTDOC:
            case SID_PRINTDOCDIRECT:
            case SID_SETUPPRINTER:
            case SID_PRINTER_NAME:
            {
                if (Application::GetSettings().GetMiscSettings().GetDisablePrinting()
                    || (pSh && pSh->isPrintLocked()))
                {
                    rSet.DisableItem(nSID);
                    break;
                }

                SfxPrinter *pPrinter = GetPrinter();

                if ( SID_PRINTDOCDIRECT == nSID )
                {
                    OUString aPrinterName;
                    if ( pPrinter != nullptr )
                        aPrinterName = pPrinter->GetName();
                    else
                        aPrinterName = Printer::GetDefaultPrinterName();
                    if ( !aPrinterName.isEmpty() )
                    {
                        uno::Reference < frame::XFrame > xFrame( pFrame->GetFrame().GetFrameInterface() );

                        auto aProperties = vcl::CommandInfoProvider::GetCommandProperties(".uno:PrintDefault",
                            vcl::CommandInfoProvider::GetModuleIdentifier(xFrame));
                        OUString val = vcl::CommandInfoProvider::GetLabelForCommand(aProperties) +
                                        " (" + aPrinterName + ")";

                        rSet.Put( SfxStringItem( SID_PRINTDOCDIRECT, val ) );
                    }
                }
                break;
            }
            case SID_STYLE_FAMILY :
            {
                rSet.Put( SfxUInt16Item( SID_STYLE_FAMILY, pImpl->m_nFamily ) );
                break;
            }
        }
    }
}

void SfxViewShell::SetZoomFactor( const Fraction &rZoomX,
    const Fraction &rZoomY )
{
    DBG_ASSERT( GetWindow(), "no window" );
    MapMode aMap( GetWindow()->GetMapMode() );
    aMap.SetScaleX( rZoomX );
    aMap.SetScaleY( rZoomY );
    GetWindow()->SetMapMode( aMap );
}

ErrCode SfxViewShell::DoVerb(sal_Int32 /*nVerb*/)

/*  [Description]

    Virtual Method used to perform a Verb on a selected Object.
    Since this Object is only known by the derived classes, they must override
    DoVerb.
*/

{
    return ERRCODE_SO_NOVERBS;
}

void SfxViewShell::OutplaceActivated( bool bActive )
{
    if ( !bActive )
        GetFrame()->GetFrame().Appear();
}

void SfxViewShell::UIActivating( SfxInPlaceClient* /*pClient*/ )
{
    uno::Reference < frame::XFrame > xOwnFrame( pFrame->GetFrame().GetFrameInterface() );
    uno::Reference < frame::XFramesSupplier > xParentFrame = xOwnFrame->getCreator();
    if ( xParentFrame.is() )
        xParentFrame->setActiveFrame( xOwnFrame );

    pFrame->GetBindings().HidePopups();
    pFrame->GetDispatcher()->Update_Impl( true );
}


void SfxViewShell::UIDeactivated( SfxInPlaceClient* /*pClient*/ )
{
    if ( !pFrame->GetFrame().IsClosing_Impl() || SfxViewFrame::Current() != pFrame )
        pFrame->GetDispatcher()->Update_Impl( true );
    pFrame->GetBindings().HidePopups(false);

    pFrame->GetBindings().InvalidateAll(true);
}


SfxInPlaceClient* SfxViewShell::FindIPClient
(
    const uno::Reference < embed::XEmbeddedObject >& xObj,
    vcl::Window*             pObjParentWin
)   const
{
    std::vector< SfxInPlaceClient* >& rClients = pImpl->GetIPClients_Impl();
    if ( rClients.empty() )
        return nullptr;

    if( !pObjParentWin )
        pObjParentWin = GetWindow();
    for (SfxInPlaceClient* pIPClient : rClients)
    {
        if ( pIPClient->GetObject() == xObj && pIPClient->GetEditWin() == pObjParentWin )
            return pIPClient;
    }

    return nullptr;
}


SfxInPlaceClient* SfxViewShell::GetIPClient() const
{
    return GetUIActiveClient();
}


SfxInPlaceClient* SfxViewShell::GetUIActiveIPClient_Impl() const
{
    // this method is needed as long as SFX still manages the border space for ChildWindows (see SfxFrame::Resize)
    std::vector< SfxInPlaceClient* >& rClients = pImpl->GetIPClients_Impl();
    if ( rClients.empty() )
        return nullptr;

    for (SfxInPlaceClient* pIPClient : rClients)
    {
        if ( pIPClient->IsUIActive() )
            return pIPClient;
    }

    return nullptr;
}

SfxInPlaceClient* SfxViewShell::GetUIActiveClient() const
{
    std::vector< SfxInPlaceClient* >& rClients = pImpl->GetIPClients_Impl();
    if ( rClients.empty() )
        return nullptr;

    const bool bIsTiledRendering = comphelper::LibreOfficeKit::isActive();

    for (SfxInPlaceClient* pIPClient : rClients)
    {
        if ( pIPClient->IsObjectUIActive() || ( bIsTiledRendering && pIPClient->IsObjectInPlaceActive() ) )
            return pIPClient;
    }

    return nullptr;
}


void SfxViewShell::Activate( bool bMDI )
{
    if ( bMDI )
    {
        SfxObjectShell *pSh = GetViewFrame()->GetObjectShell();
        if (const auto xModel = pSh->GetModel())
            xModel->setCurrentController(GetController());

        SetCurrentDocument();
    }
}


void SfxViewShell::Deactivate(bool /*bMDI*/)
{
}


void SfxViewShell::Move()

/*  [Description]

    This virtual Method is called when the window displayed in the
    SfxViewShell gets a StarView-Move() notification.

    This base implementation does not have to be called.     .

    [Note]

    This Method can be used to cancel a selection, in order to catch the
    mouse movement which is due to moving a window.

    For now the notification does not work In-Place.
*/

{
}


void SfxViewShell::OuterResizePixel
(
    const Point&    /*rToolOffset*/,// Upper left corner Tools in Frame-Window
    const Size&     /*rSize*/       // All available sizes.
)

/*  [Description]

    Override this Method to be able to react to the size-change of
    the View. Thus the View is defined as the Edit window and also the
    attached Tools are defined (for example the ruler).

    The Edit window must not be changed either in size or position.

    The Vis-Area of SfxObjectShell, its scale and position can be changed
    here. The main use is to change the size of the Vis-Area.

    If the Border is changed due to the new calculation then this has to be set
    by <SfxViewShell::SetBorderPixel(const SvBorder&)>. The Positioning of Tools
    is only allowed after the calling of 'SetBorderPixel'.

    [Example]

    void AppViewSh::OuterViewResizePixel( const Point &rOfs, const Size &rSz )
    {
        // Calculate Tool position and size externally, do not set!
        // (due to the following Border calculation)
        Point aHLinPos...; Size aHLinSz...;
        ...

        // Calculate and Set a Border of Tools which matches rSize.
        SvBorder aBorder...
        SetBorderPixel( aBorder ); // Allow Positioning from here on.

        // Arrange Tools
        pHLin->SetPosSizePixel( aHLinPos, aHLinSz );
        ...
    }

    [Cross-reference]

        <SfxViewShell::InnerResizePixel(const Point&,const Size& rSize)>
*/

{
    SetBorderPixel( SvBorder() );
}


void SfxViewShell::InnerResizePixel
(
    const Point&    /*rToolOffset*/,// Upper left corner Tools in Frame-Window
    const Size&     /*rSize*/,      // All available sizes.
    bool
)

/*  [Description]

    Override this Method to be able to react to the size-change of
    the Edit window.

    The Edit window must not be changed either in size or position.
    Neither the Vis-Area of SfxObjectShell nor its scale or position are
    allowed to be changed

    If the Border is changed due to the new calculation then is has to be set
    by <SfxViewShell::SetBorderPixel(const SvBorder&)>.
    The Positioning of Tools is only allowed after the calling of
    'SetBorderPixel'.


    [Note]

    void AppViewSh::InnerViewResizePixel( const Point &rOfs, const Size &rSz )
    {
        // Calculate Tool position and size internally, do not set!
        // (due to the following Border calculation)
        Point aHLinPos...; Size aHLinSz...;
        ...

        // Calculate and Set a Border of Tools which matches rSize.
        SvBorder aBorder...
        SetBorderPixel( aBorder ); // Allow Positioning from here on.

        // Arrange Tools
        pHLin->SetPosSizePixel( aHLinPos, aHLinSz );
        ...
    }

    [Cross-reference]

        <SfxViewShell::OuterResizePixel(const Point&,const Size& rSize)>
*/

{
    SetBorderPixel( SvBorder() );
}


void SfxViewShell::InvalidateBorder()
{
    DBG_ASSERT( GetViewFrame(), "SfxViewShell without SfxViewFrame" );

    GetViewFrame()->InvalidateBorderImpl( this );
    if (pImpl->m_pController.is())
    {
        pImpl->m_pController->BorderWidthsChanged_Impl();
    }
}


void SfxViewShell::SetBorderPixel( const SvBorder &rBorder )
{
    DBG_ASSERT( GetViewFrame(), "SfxViewShell without SfxViewFrame" );

    GetViewFrame()->SetBorderPixelImpl( this, rBorder );

    // notify related controller that border size is changed
    if (pImpl->m_pController.is())
    {
        pImpl->m_pController->BorderWidthsChanged_Impl();
    }
}


const SvBorder& SfxViewShell::GetBorderPixel() const
{
    DBG_ASSERT( GetViewFrame(), "SfxViewShell without SfxViewFrame" );

    return GetViewFrame()->GetBorderPixelImpl();
}


void SfxViewShell::SetWindow
(
    vcl::Window*     pViewPort   // For example Null pointer in the Destructor.
)

/*  [Description]

    With this method the SfxViewShell is set in the data window. This is
    needed for the in-place container and for restoring the proper focus.

    Even in-place-active the conversion of the ViewPort Windows is forbidden.
*/

{
    if( pWindow == pViewPort )
        return;

    // Disconnect existing IP-Clients if possible
    DisconnectAllClients();

    // Switch View-Port
    bool bHadFocus = pWindow && pWindow->HasChildPathFocus( true );
    pWindow = pViewPort;

    if( pWindow )
    {
        // Disable automatic GUI mirroring (right-to-left) for document windows
        pWindow->EnableRTL( false );
    }

    if ( bHadFocus && pWindow )
        pWindow->GrabFocus();
    //TODO/CLEANUP
    //Do we still need this Method?!
    //SfxGetpApp()->GrabFocus( pWindow );
}

ViewShellDocId SfxViewShell::mnCurrentDocId(0);

SfxViewShell::SfxViewShell
(
    SfxViewFrame*     pViewFrame,     /*  <SfxViewFrame>, which will be
                                          displayed in this View */
    SfxViewShellFlags nFlags          /*  See <SfxViewShell-Flags> */
)

:   SfxShell(this)
,   pImpl( new SfxViewShell_Impl(nFlags, SfxViewShell::mnCurrentDocId) )
,   pFrame(pViewFrame)
,   pWindow(nullptr)
,   bNoNewWindow( nFlags & SfxViewShellFlags::NO_NEWWINDOW )
,   mbPrinterSettingsModified(false)
,   maLOKLanguageTag(LANGUAGE_NONE)
,   maLOKLocale(LANGUAGE_NONE)
,   maLOKDeviceFormFactor(LOKDeviceFormFactor::UNKNOWN)
,   mbLOKAccessibilityEnabled(false)
,   mpLOKDocumentFocusListener(nullptr)
{
    SetMargin( pViewFrame->GetMargin_Impl() );

    SetPool( &pViewFrame->GetObjectShell()->GetPool() );
    StartListening(*pViewFrame->GetObjectShell());

    // Insert into list
    std::vector<SfxViewShell*> &rViewArr = SfxGetpApp()->GetViewShells_Impl();
    rViewArr.push_back(this);

    if (comphelper::LibreOfficeKit::isActive())
    {
        maLOKLanguageTag = SfxLokHelper::getDefaultLanguage();
        maLOKLocale = SfxLokHelper::getDefaultLanguage();

        const auto [isTimezoneSet, aTimezone] = SfxLokHelper::getDefaultTimezone();
        maLOKIsTimezoneSet = isTimezoneSet;
        maLOKTimezone = aTimezone;

        maLOKDeviceFormFactor = SfxLokHelper::getDeviceFormFactor();

        vcl::Window* pFrameWin = pViewFrame->GetWindow().GetFrameWindow();
        if (pFrameWin && !pFrameWin->GetLOKNotifier())
            pFrameWin->SetLOKNotifier(this, true);
    }
}


SfxViewShell::~SfxViewShell()
{
    // Remove from list
    const SfxViewShell *pThis = this;
    std::vector<SfxViewShell*> &rViewArr = SfxGetpApp()->GetViewShells_Impl();
    auto it = std::find( rViewArr.begin(), rViewArr.end(), pThis );
    rViewArr.erase( it );

    if ( pImpl->xClipboardListener.is() )
    {
        pImpl->xClipboardListener->DisconnectViewShell();
        pImpl->xClipboardListener = nullptr;
    }

    if (pImpl->m_pController.is())
    {
        pImpl->m_pController->ReleaseShell_Impl();
        pImpl->m_pController.clear();
    }

    vcl::Window* pFrameWin = GetViewFrame()->GetWindow().GetFrameWindow();
    if (pFrameWin && pFrameWin->GetLOKNotifier() == this)
        pFrameWin->ReleaseLOKNotifier();
}

OUString SfxViewShell::getA11yFocusedParagraph() const
{
    const LOKDocumentFocusListener& rDocFocusListener = GetLOKDocumentFocusListener();
    return rDocFocusListener.getFocusedParagraph();
}

int SfxViewShell::getA11yCaretPosition() const
{
    const LOKDocumentFocusListener& rDocFocusListener = GetLOKDocumentFocusListener();
    return rDocFocusListener.getCaretPosition();
}

bool SfxViewShell::PrepareClose
(
    bool bUI     // TRUE: Allow Dialog and so on, FALSE: silent-mode
)
{
    if (GetViewFrame()->GetWindow().GetLOKNotifier() == this)
        GetViewFrame()->GetWindow().ReleaseLOKNotifier();

    SfxPrinter *pPrinter = GetPrinter();
    if ( pPrinter && pPrinter->IsPrinting() )
    {
        if ( bUI )
        {
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetViewFrame()->GetFrameWeld(),
                                                                     VclMessageType::Info, VclButtonsType::Ok,
                                                                     SfxResId(STR_CANT_CLOSE)));
            xBox->run();
        }

        return false;
    }

    if( GetViewFrame()->IsInModalMode() )
        return false;

    if( bUI && GetViewFrame()->GetDispatcher()->IsLocked() )
        return false;

    return true;
}


SfxViewShell* SfxViewShell::Current()
{
    SfxViewFrame *pCurrent = SfxViewFrame::Current();
    return pCurrent ? pCurrent->GetViewShell() : nullptr;
}


SfxViewShell* SfxViewShell::Get( const Reference< XController>& i_rController )
{
    if ( !i_rController.is() )
        return nullptr;

    for (   SfxViewShell* pViewShell = SfxViewShell::GetFirst( false );
            pViewShell;
            pViewShell = SfxViewShell::GetNext( *pViewShell, false )
        )
    {
        if ( pViewShell->GetController() == i_rController )
            return pViewShell;
    }
    return nullptr;
}


SdrView* SfxViewShell::GetDrawView() const

/*  [Description]

    This virtual Method has to be overloaded by the sub classes, to be able
    make the Property-Editor available.

    The default implementation does always return zero.
*/

{
    return nullptr;
}


OUString SfxViewShell::GetSelectionText
(
    bool /*bCompleteWords*/, /*  FALSE (default)
                                Only the actual selected text is returned.

                                TRUE
                                The selected text is expanded so that only
                                whole words are returned. As word separators
                                these are used: white spaces and punctuation
                                ".,;" and single and double quotes.
                            */
    bool /*bOnlyASample*/ /* used by some dialogs to avoid constructing monster strings e.g. in calc */
)

/*  [Description]

    Override this Method to return a text that
    is included in the current selection. This is for example used when
    sending emails.

    When called with "CompleteWords == TRUE", it is for example sufficient
    with having the Cursor positioned somewhere within a URL in-order
    to have the entire URL returned.
*/

{
    return OUString();
}


bool SfxViewShell::HasSelection( bool ) const

/*  [Description]

    With this virtual Method can a for example a Dialog be queried, to
    check if something is selected in the current view. If the Parameter
    is <BOOL> TRUE then it is checked whether some text is selected.
*/

{
    return false;
}

void SfxViewShell::AddSubShell( SfxShell& rShell )
{
    pImpl->aArr.push_back(&rShell);
    SfxDispatcher *pDisp = pFrame->GetDispatcher();
    if ( pDisp->IsActive(*this) )
    {
        pDisp->Push(rShell);
        pDisp->Flush();
    }
}

void SfxViewShell::RemoveSubShell( SfxShell* pShell )
{
    SfxDispatcher *pDisp = pFrame->GetDispatcher();
    if ( !pShell )
    {
        size_t nCount = pImpl->aArr.size();
        if ( pDisp->IsActive(*this) )
        {
            for(size_t n = nCount; n > 0; --n)
                pDisp->Pop(*pImpl->aArr[n - 1]);
            pDisp->Flush();
        }
        pImpl->aArr.clear();
    }
    else
    {
        SfxShellArr_Impl::iterator i = std::find(pImpl->aArr.begin(), pImpl->aArr.end(), pShell);
        if(i != pImpl->aArr.end())
        {
            pImpl->aArr.erase(i);
            if(pDisp->IsActive(*this))
            {
                pDisp->RemoveShell_Impl(*pShell);
                pDisp->Flush();
            }
        }
    }
}

SfxShell* SfxViewShell::GetSubShell( sal_uInt16 nNo )
{
    sal_uInt16 nCount = pImpl->aArr.size();
    if(nNo < nCount)
        return pImpl->aArr[nCount - nNo - 1];
    return nullptr;
}

void SfxViewShell::PushSubShells_Impl( bool bPush )
{
    SfxDispatcher *pDisp = pFrame->GetDispatcher();
    if ( bPush )
    {
        for (auto const& elem : pImpl->aArr)
            pDisp->Push(*elem);
    }
    else if(!pImpl->aArr.empty())
    {
        SfxShell& rPopUntil = *pImpl->aArr[0];
        if ( pDisp->GetShellLevel( rPopUntil ) != USHRT_MAX )
            pDisp->Pop( rPopUntil, SfxDispatcherPopFlags::POP_UNTIL );
    }

    pDisp->Flush();
}


void SfxViewShell::WriteUserData( OUString&, bool )
{
}


void SfxViewShell::ReadUserData(const OUString&, bool )
{
}

void SfxViewShell::ReadUserDataSequence ( const uno::Sequence < beans::PropertyValue >& )
{
}

void SfxViewShell::WriteUserDataSequence ( uno::Sequence < beans::PropertyValue >& )
{
}


// returns the first shell of spec. type viewing the specified doc.
SfxViewShell* SfxViewShell::GetFirst
(
    bool          bOnlyVisible,
    const std::function< bool ( const SfxViewShell* ) >& isViewShell
)
{
    // search for a SfxViewShell of the specified type
    std::vector<SfxViewShell*> &rShells = SfxGetpApp()->GetViewShells_Impl();
    for (SfxViewShell* pShell : rShells)
    {
        if ( pShell )
        {
            // This code used to check that the frame exists in the other list,
            // because of https://bz.apache.org/ooo/show_bug.cgi?id=62084, with the explanation:
            // sometimes dangling SfxViewShells exist that point to a dead SfxViewFrame
            // these ViewShells shouldn't be accessible anymore
            // a destroyed ViewFrame is not in the ViewFrame array anymore, so checking this array helps
            // That doesn't seem to be needed anymore, but keep an assert, just in case.
            assert(std::find(SfxGetpApp()->GetViewFrames_Impl().begin(), SfxGetpApp()->GetViewFrames_Impl().end(),
                pShell->GetViewFrame()) != SfxGetpApp()->GetViewFrames_Impl().end());
            if ( ( !bOnlyVisible || pShell->GetViewFrame()->IsVisible() ) && (!isViewShell || isViewShell(pShell)))
                return pShell;
        }
    }

    return nullptr;
}


// returns the next shell of spec. type viewing the specified doc.

SfxViewShell* SfxViewShell::GetNext
(
    const SfxViewShell& rPrev,
    bool                bOnlyVisible,
    const std::function<bool ( const SfxViewShell* )>& isViewShell
)
{
    std::vector<SfxViewShell*> &rShells = SfxGetpApp()->GetViewShells_Impl();
    size_t nPos;
    for ( nPos = 0; nPos < rShells.size(); ++nPos )
        if ( rShells[nPos] == &rPrev )
            break;

    for ( ++nPos; nPos < rShells.size(); ++nPos )
    {
        SfxViewShell *pShell = rShells[nPos];
        if ( pShell )
        {
            assert(std::find(SfxGetpApp()->GetViewFrames_Impl().begin(), SfxGetpApp()->GetViewFrames_Impl().end(),
                pShell->GetViewFrame()) != SfxGetpApp()->GetViewFrames_Impl().end());
            if ( ( !bOnlyVisible || pShell->GetViewFrame()->IsVisible() ) && (!isViewShell || isViewShell(pShell)) )
                return pShell;
        }
    }

    return nullptr;
}


void SfxViewShell::Notify( SfxBroadcaster& rBC,
                            const SfxHint& rHint )
{
    const SfxEventHint* pEventHint = dynamic_cast<const SfxEventHint*>(&rHint);
    if ( !(pEventHint && pEventHint->GetEventId() == SfxEventHintId::LoadFinished) )
        return;

    if ( !GetController().is() )
        return;

    // avoid access to dangling ViewShells
    auto &rFrames = SfxGetpApp()->GetViewFrames_Impl();
    for (SfxViewFrame* frame : rFrames)
    {
        if ( frame == GetViewFrame() && &rBC == GetObjectShell() )
        {
            SfxItemSet* pSet = GetObjectShell()->GetMedium()->GetItemSet();
            const SfxUnoAnyItem* pItem = SfxItemSet::GetItem<SfxUnoAnyItem>(pSet, SID_VIEW_DATA, false);
            if ( pItem )
            {
                pImpl->m_pController->restoreViewData( pItem->GetValue() );
                pSet->ClearItem( SID_VIEW_DATA );
            }
            break;
        }
    }
}

bool SfxViewShell::ExecKey_Impl(const KeyEvent& aKey)
{
    bool setModuleConfig = false; // In case libreofficekit is active, we will re-set the module config class.
    if (!pImpl->m_xAccExec)
    {
        pImpl->m_xAccExec = ::svt::AcceleratorExecute::createAcceleratorHelper();
        pImpl->m_xAccExec->init(::comphelper::getProcessComponentContext(),
            pFrame->GetFrame().GetFrameInterface());
        setModuleConfig = true;
    }

    if (comphelper::LibreOfficeKit::isActive())
    {
        // Get the module name.
        css::uno::Reference< css::uno::XComponentContext >  xContext      (::comphelper::getProcessComponentContext());
        css::uno::Reference< css::frame::XModuleManager2 >  xModuleManager(css::frame::ModuleManager::create(xContext));
        OUString sModule = xModuleManager->identify(pFrame->GetFrame().GetFrameInterface());

        // Get the language name.
        OUString viewLang = GetLOKLanguageTag().getBcp47();

        // Merge them & have a key.
        OUString key = sModule + viewLang;

        // Check it in configurations map. Create a configuration manager if there isn't one for the key.
        std::unordered_map<OUString, css::uno::Reference<com::sun::star::ui::XAcceleratorConfiguration>>& acceleratorConfs = SfxApplication::Get()->GetAcceleratorConfs_Impl();
        if (acceleratorConfs.find(key) == acceleratorConfs.end())
        {
            // Create a new configuration manager for the module.

            OUString actualLang = officecfg::Setup::L10N::ooLocale::get();

            std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
            officecfg::Setup::L10N::ooLocale::set(viewLang, batch);
            batch->commit();

            // We have set the language. Time to create the config manager.
            acceleratorConfs[key] = svt::AcceleratorExecute::lok_createNewAcceleratorConfiguration(::comphelper::getProcessComponentContext(), sModule);

            std::shared_ptr<comphelper::ConfigurationChanges> batch2(comphelper::ConfigurationChanges::create());
            officecfg::Setup::L10N::ooLocale::set(actualLang, batch2);
            batch2->commit();
        }

        if (setModuleConfig)
            pImpl->m_xAccExec->lok_setModuleConfig(acceleratorConfs[key]);
    }

    return pImpl->m_xAccExec->execute(aKey.GetKeyCode());
}

void SfxViewShell::setLibreOfficeKitViewCallback(SfxLokCallbackInterface* pCallback)
{
    pImpl->m_pLibreOfficeKitViewCallback = pCallback;

    afterCallbackRegistered();

    if (!pImpl->m_pLibreOfficeKitViewCallback)
        return;

    // Ask other views to tell us about their cursors.
    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        if (pViewShell->GetDocId() == GetDocId())
            pViewShell->NotifyCursor(this);
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

SfxLokCallbackInterface* SfxViewShell::getLibreOfficeKitViewCallback() const
{
    return pImpl->m_pLibreOfficeKitViewCallback;
}

void SfxViewShell::dumpLibreOfficeKitViewState(rtl::OStringBuffer &rState)
{
    if (pImpl->m_pLibreOfficeKitViewCallback)
        pImpl->m_pLibreOfficeKitViewCallback->dumpState(rState);
}

static bool ignoreLibreOfficeKitViewCallback(int nType, const SfxViewShell_Impl* pImpl)
{
    if (!comphelper::LibreOfficeKit::isActive())
        return true;

    if (comphelper::LibreOfficeKit::isTiledPainting())
    {
        switch (nType)
        {
        case LOK_CALLBACK_FORM_FIELD_BUTTON:
        case LOK_CALLBACK_TEXT_SELECTION:
        case LOK_CALLBACK_COMMENT:
            break;
        default:
            // Reject e.g. invalidate during paint.
            return true;
        }
    }

    if (pImpl->m_bTiledSearching)
    {
        switch (nType)
        {
        case LOK_CALLBACK_TEXT_SELECTION:
        case LOK_CALLBACK_TEXT_VIEW_SELECTION:
        case LOK_CALLBACK_TEXT_SELECTION_START:
        case LOK_CALLBACK_TEXT_SELECTION_END:
        case LOK_CALLBACK_GRAPHIC_SELECTION:
        case LOK_CALLBACK_GRAPHIC_VIEW_SELECTION:
            return true;
        }
    }

    return false;
}

void SfxViewShell::libreOfficeKitViewInvalidateTilesCallback(const tools::Rectangle* pRect, int nPart, int nMode) const
{
    if (ignoreLibreOfficeKitViewCallback(LOK_CALLBACK_INVALIDATE_TILES, pImpl.get()))
        return;
    if (pImpl->m_pLibreOfficeKitViewCallback)
        pImpl->m_pLibreOfficeKitViewCallback->libreOfficeKitViewInvalidateTilesCallback(pRect, nPart, nMode);
    else
        SAL_INFO(
            "sfx.view",
            "SfxViewShell::libreOfficeKitViewInvalidateTilesCallback no callback set!");
}

void SfxViewShell::libreOfficeKitViewCallbackWithViewId(int nType, const char* pPayload, int nViewId) const
{
    if (ignoreLibreOfficeKitViewCallback(nType, pImpl.get()))
        return;
    if (pImpl->m_pLibreOfficeKitViewCallback)
        pImpl->m_pLibreOfficeKitViewCallback->libreOfficeKitViewCallbackWithViewId(nType, pPayload, nViewId);
    else
        SAL_INFO(
            "sfx.view",
            "SfxViewShell::libreOfficeKitViewCallbackWithViewId no callback set! Dropped payload of type "
            << lokCallbackTypeToString(nType) << ": [" << pPayload << ']');
}

void SfxViewShell::libreOfficeKitViewCallback(int nType, const char* pPayload) const
{
    if (ignoreLibreOfficeKitViewCallback(nType, pImpl.get()))
        return;
    if (pImpl->m_pLibreOfficeKitViewCallback)
        pImpl->m_pLibreOfficeKitViewCallback->libreOfficeKitViewCallback(nType, pPayload);
    else
        SAL_INFO(
            "sfx.view",
            "SfxViewShell::libreOfficeKitViewCallback no callback set! Dropped payload of type "
            << lokCallbackTypeToString(nType) << ": [" << pPayload << ']');
}

void SfxViewShell::libreOfficeKitViewUpdatedCallback(int nType) const
{
    if (ignoreLibreOfficeKitViewCallback(nType, pImpl.get()))
        return;
    if (pImpl->m_pLibreOfficeKitViewCallback)
        pImpl->m_pLibreOfficeKitViewCallback->libreOfficeKitViewUpdatedCallback(nType);
    else
        SAL_INFO(
            "sfx.view",
            "SfxViewShell::libreOfficeKitViewUpdatedCallback no callback set! Dropped payload of type "
            << lokCallbackTypeToString(nType));
}

void SfxViewShell::libreOfficeKitViewUpdatedCallbackPerViewId(int nType, int nViewId, int nSourceViewId) const
{
    if (ignoreLibreOfficeKitViewCallback(nType, pImpl.get()))
        return;
    if (pImpl->m_pLibreOfficeKitViewCallback)
        pImpl->m_pLibreOfficeKitViewCallback->libreOfficeKitViewUpdatedCallbackPerViewId(nType, nViewId, nSourceViewId);
    else
        SAL_INFO(
            "sfx.view",
            "SfxViewShell::libreOfficeKitViewUpdatedCallbackPerViewId no callback set! Dropped payload of type "
            << lokCallbackTypeToString(nType));
}

void SfxViewShell::libreOfficeKitViewAddPendingInvalidateTiles()
{
    if (pImpl->m_pLibreOfficeKitViewCallback)
        pImpl->m_pLibreOfficeKitViewCallback->libreOfficeKitViewAddPendingInvalidateTiles();
    else
        SAL_INFO(
            "sfx.view",
            "SfxViewShell::libreOfficeKitViewAddPendingInvalidateTiles no callback set!");
}

void SfxViewShell::afterCallbackRegistered()
{
}

void SfxViewShell::flushPendingLOKInvalidateTiles()
{
    // SfxViewShell itself does not delay any tile invalidations.
}

std::optional<OString> SfxViewShell::getLOKPayload(int nType, int /*nViewId*/) const
{
    // SfxViewShell itself currently doesn't handle any updated-payload types.
    SAL_WARN("sfx.view", "SfxViewShell::getLOKPayload unhandled type " << lokCallbackTypeToString(nType));
    abort();
}

vcl::Window* SfxViewShell::GetEditWindowForActiveOLEObj() const
{
    vcl::Window* pEditWin = nullptr;
    SfxInPlaceClient* pIPClient = GetIPClient();
    if (pIPClient)
    {
        pEditWin = pIPClient->GetEditWin();
    }
    return pEditWin;
}

void SfxViewShell::SetLOKLanguageTag(const OUString& rBcp47LanguageTag)
{
    LanguageTag aTag(rBcp47LanguageTag, true);

    css::uno::Sequence<OUString> inst(officecfg::Setup::Office::InstalledLocales::get()->getElementNames());
    LanguageTag aFallbackTag = LanguageTag(getInstalledLocaleForSystemUILanguage(inst, /* bRequestInstallIfMissing */ false, rBcp47LanguageTag), true).makeFallback();

    // If we want de-CH, and the de localisation is available, we don't want to use de-DE as then
    // the magic in Translate::get() won't turn ess-zet into double s. Possibly other similar cases?
    if (comphelper::LibreOfficeKit::isActive() && aTag.getLanguage() == aFallbackTag.getLanguage())
        maLOKLanguageTag = aTag;
    else
        maLOKLanguageTag = aFallbackTag;
}

LOKDocumentFocusListener& SfxViewShell::GetLOKDocumentFocusListener()
{
    if (mpLOKDocumentFocusListener)
        return *mpLOKDocumentFocusListener;

    mpLOKDocumentFocusListener = new LOKDocumentFocusListener(this);
    return *mpLOKDocumentFocusListener;
}

const LOKDocumentFocusListener& SfxViewShell::GetLOKDocumentFocusListener() const
{
    return const_cast<SfxViewShell*>(this)->GetLOKDocumentFocusListener();
}

void SfxViewShell::SetLOKAccessibilityState(bool bEnabled)
{
    if (bEnabled == mbLOKAccessibilityEnabled)
        return;
    mbLOKAccessibilityEnabled = bEnabled;

    LOKDocumentFocusListener& rDocumentFocusListener = GetLOKDocumentFocusListener();

    if (!pWindow)
        return;

    uno::Reference< accessibility::XAccessible > xAccessible =
        pWindow->GetAccessible();

    if (!xAccessible.is())
        return;

    if (mbLOKAccessibilityEnabled)
    {
        try
        {
            rDocumentFocusListener.attachRecursive(xAccessible);
        }
        catch (const uno::Exception&)
        {
            SAL_WARN("SetLOKAccessibilityState", "Exception caught processing LOKDocumentFocusListener::attachRecursive");
        }
    }
    else
    {
        try
        {
            rDocumentFocusListener.detachRecursive(xAccessible);
        }
        catch (const uno::Exception&)
        {
            SAL_WARN("SetLOKAccessibilityState", "Exception caught processing LOKDocumentFocusListener::detachRecursive");
        }
    }
}

void SfxViewShell::SetLOKLocale(const OUString& rBcp47LanguageTag)
{
    maLOKLocale = LanguageTag(rBcp47LanguageTag, true).makeFallback();
}

void SfxViewShell::NotifyCursor(SfxViewShell* /*pViewShell*/) const
{
}

void SfxViewShell::setTiledSearching(bool bTiledSearching)
{
    pImpl->m_bTiledSearching = bTiledSearching;
}

int SfxViewShell::getPart() const
{
    return 0;
}

int SfxViewShell::getEditMode() const
{
    return 0;
}

ViewShellId SfxViewShell::GetViewShellId() const
{
    return pImpl->m_nViewShellId;
}

void SfxViewShell::SetCurrentDocId(ViewShellDocId nId)
{
    mnCurrentDocId = nId;
}

ViewShellDocId SfxViewShell::GetDocId() const
{
    assert(pImpl->m_nDocId >= ViewShellDocId(0) && "m_nDocId should have been initialized, but it is invalid.");
    return pImpl->m_nDocId;
}

void SfxViewShell::notifyInvalidation(tools::Rectangle const* pRect) const
{
    SfxLokHelper::notifyInvalidation(this, pRect);
}

void SfxViewShell::NotifyOtherViews(int nType, const OString& rKey, const OString& rPayload)
{
    SfxLokHelper::notifyOtherViews(this, nType, rKey, rPayload);
}

void SfxViewShell::NotifyOtherView(OutlinerViewShell* pOther, int nType, const OString& rKey, const OString& rPayload)
{
    auto pOtherShell = dynamic_cast<SfxViewShell*>(pOther);
    if (!pOtherShell)
        return;

    SfxLokHelper::notifyOtherView(this, pOtherShell, nType, rKey, rPayload);
}

void SfxViewShell::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SfxViewShell"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("id"), BAD_CAST(OString::number(static_cast<sal_Int32>(GetViewShellId())).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
}

bool SfxViewShell::KeyInput( const KeyEvent &rKeyEvent )

/*  [Description]

    This Method executes the KeyEvent 'rKeyEvent' of the Keys (Accelerator)
    configured either direct or indirect (for example by the Application)
    in the SfxViewShell.

    [Return value]

    bool                    TRUE
                            The Key (Accelerator) is configured and the
                            associated Handler was called

                            FALSE
                            The Key (Accelerator) is not configured and
                            subsequently no Handler was called

    [Cross-reference]

    <SfxApplication::KeyInput(const KeyEvent&)>
*/
{
    return ExecKey_Impl(rKeyEvent);
}

bool SfxViewShell::GlobalKeyInput_Impl( const KeyEvent &rKeyEvent )
{
    return ExecKey_Impl(rKeyEvent);
}


void SfxViewShell::ShowCursor( bool /*bOn*/ )

/*  [Description]

    Subclasses must override this Method so that SFx can switch the
    Cursor on and off, for example while a <SfxProgress> is running.
*/

{
}


void SfxViewShell::ResetAllClients_Impl( SfxInPlaceClient const *pIP )
{

    std::vector< SfxInPlaceClient* >& rClients = pImpl->GetIPClients_Impl();
    if ( rClients.empty() )
        return;

    for (SfxInPlaceClient* pIPClient : rClients)
    {
        if( pIPClient != pIP )
            pIPClient->ResetObject();
    }
}


void SfxViewShell::DisconnectAllClients()
{
    std::vector< SfxInPlaceClient* >& rClients = pImpl->GetIPClients_Impl();
    if ( rClients.empty() )
        return;

    for ( size_t n = 0; n < rClients.size(); )
        // clients will remove themselves from the list
        delete rClients.at( n );
}


void SfxViewShell::QueryObjAreaPixel( tools::Rectangle& ) const
{
}


void SfxViewShell::VisAreaChanged()
{
    std::vector< SfxInPlaceClient* >& rClients = pImpl->GetIPClients_Impl();
    if ( rClients.empty() )
        return;

    for (SfxInPlaceClient* pIPClient : rClients)
    {
        if ( pIPClient->IsObjectInPlaceActive() )
            // client is active, notify client that the VisArea might have changed
            pIPClient->VisAreaChanged();
    }
}


void SfxViewShell::CheckIPClient_Impl(
        SfxInPlaceClient const *const pIPClient, const tools::Rectangle& rVisArea)
{
    if ( GetObjectShell()->IsInClose() )
        return;

    bool bAlwaysActive =
        ( ( pIPClient->GetObjectMiscStatus() & embed::EmbedMisc::EMBED_ACTIVATEIMMEDIATELY ) != 0 );
    bool bActiveWhenVisible =
        ( pIPClient->GetObjectMiscStatus() & embed::EmbedMisc::MS_EMBED_ACTIVATEWHENVISIBLE ) != 0;

    // this method is called when a client is created
    if (pIPClient->IsObjectInPlaceActive())
        return;

    // object in client is currently not active
    // check if the object wants to be activated always or when it becomes at least partially visible
    // TODO/LATER: maybe we should use the scaled area instead of the ObjArea?!
    if (bAlwaysActive || (bActiveWhenVisible && rVisArea.Overlaps(pIPClient->GetObjArea())))
    {
        try
        {
            pIPClient->GetObject()->changeState( embed::EmbedStates::INPLACE_ACTIVE );
        }
        catch (const uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION("sfx.view", "SfxViewShell::CheckIPClient_Impl");
        }
    }
}


SfxObjectShell* SfxViewShell::GetObjectShell()
{
    return pFrame ? pFrame->GetObjectShell() : nullptr;
}


Reference< XModel > SfxViewShell::GetCurrentDocument() const
{
    Reference< XModel > xDocument;

    const SfxObjectShell* pDocShell( const_cast< SfxViewShell* >( this )->GetObjectShell() );
    OSL_ENSURE( pDocShell, "SfxViewFrame::GetCurrentDocument: no DocShell!?" );
    if ( pDocShell )
        xDocument = pDocShell->GetModel();
    return xDocument;
}


void SfxViewShell::SetCurrentDocument() const
{
    uno::Reference< frame::XModel > xDocument( GetCurrentDocument() );
    if ( xDocument.is() )
        SfxObjectShell::SetCurrentComponent( xDocument );
}


const Size& SfxViewShell::GetMargin() const
{
    return pImpl->aMargin;
}


void SfxViewShell::SetMargin( const Size& rSize )
{
    // the default margin was verified using www.apple.com !!
    Size aMargin = rSize;
    if ( aMargin.Width() == -1 )
        aMargin.setWidth( DEFAULT_MARGIN_WIDTH );
    if ( aMargin.Height() == -1 )
        aMargin.setHeight( DEFAULT_MARGIN_HEIGHT );

    if ( aMargin != pImpl->aMargin )
    {
        pImpl->aMargin = aMargin;
        MarginChanged();
    }
}

void SfxViewShell::MarginChanged()
{
}

void SfxViewShell::JumpToMark( const OUString& rMark )
{
    SfxStringItem aMarkItem( SID_JUMPTOMARK, rMark );
    GetViewFrame()->GetDispatcher()->ExecuteList(
        SID_JUMPTOMARK,
        SfxCallMode::SYNCHRON|SfxCallMode::RECORD,
        { &aMarkItem });
}

void SfxViewShell::SetController( SfxBaseController* pController )
{
    pImpl->m_pController = pController;

    // there should be no old listener, but if there is one, it should be disconnected
    if (  pImpl->xClipboardListener.is() )
        pImpl->xClipboardListener->DisconnectViewShell();

    pImpl->xClipboardListener = new SfxClipboardChangeListener( this, GetClipboardNotifier() );
}

Reference < XController > SfxViewShell::GetController() const
{
    return pImpl->m_pController;
}

SfxBaseController* SfxViewShell::GetBaseController_Impl() const
{
    return pImpl->m_pController.get();
}

void SfxViewShell::AddContextMenuInterceptor_Impl( const uno::Reference< ui::XContextMenuInterceptor >& xInterceptor )
{
    std::unique_lock g(pImpl->aMutex);
    pImpl->aInterceptorContainer.addInterface( g, xInterceptor );
}

void SfxViewShell::RemoveContextMenuInterceptor_Impl( const uno::Reference< ui::XContextMenuInterceptor >& xInterceptor )
{
    std::unique_lock g(pImpl->aMutex);
    pImpl->aInterceptorContainer.removeInterface( g, xInterceptor );
}

bool SfxViewShell::TryContextMenuInterception(const css::uno::Reference<css::awt::XPopupMenu>& rIn,
                                              const OUString& rMenuIdentifier,
                                              css::uno::Reference<css::awt::XPopupMenu>& rOut,
                                              ui::ContextMenuExecuteEvent aEvent)
{
    rOut.clear();
    bool bModified = false;

    // create container from menu
    aEvent.ActionTriggerContainer = ::framework::ActionTriggerHelper::CreateActionTriggerContainerFromMenu(
        rIn, &rMenuIdentifier);

    // get selection from controller
    aEvent.Selection.set( GetController(), uno::UNO_QUERY );

    // call interceptors
    std::unique_lock g(pImpl->aMutex);
    std::vector<uno::Reference< ui::XContextMenuInterceptor>> aInterceptors =
        pImpl->aInterceptorContainer.getElements(g);
    g.unlock();
    for (const auto & rListener : aInterceptors )
    {
        try
        {
            ui::ContextMenuInterceptorAction eAction;
            {
                SolarMutexReleaser rel;
                eAction = rListener->notifyContextMenuExecute( aEvent );
            }
            switch ( eAction )
            {
                case ui::ContextMenuInterceptorAction_CANCELLED :
                    // interceptor does not want execution
                    return false;
                case ui::ContextMenuInterceptorAction_EXECUTE_MODIFIED :
                    // interceptor wants his modified menu to be executed
                    bModified = true;
                    break;
                case ui::ContextMenuInterceptorAction_CONTINUE_MODIFIED :
                    // interceptor has modified menu, but allows for calling other interceptors
                    bModified = true;
                    continue;
                case ui::ContextMenuInterceptorAction_IGNORED :
                    // interceptor is indifferent
                    continue;
                default:
                    OSL_FAIL("Wrong return value of ContextMenuInterceptor!");
                    continue;
            }
        }
        catch (...)
        {
            g.lock();
            pImpl->aInterceptorContainer.removeInterface(g, rListener);
            g.unlock();
        }

        break;
    }

    if (bModified)
    {
        // container was modified, create a new menu out of it
        css::uno::Reference<uno::XComponentContext> xContext(::comphelper::getProcessComponentContext(), css::uno::UNO_SET_THROW);
        rOut.set(xContext->getServiceManager()->createInstanceWithContext("com.sun.star.awt.PopupMenu", xContext), css::uno::UNO_QUERY_THROW);
        ::framework::ActionTriggerHelper::CreateMenuFromActionTriggerContainer(rOut, aEvent.ActionTriggerContainer);
    }

    return true;
}

bool SfxViewShell::TryContextMenuInterception(const css::uno::Reference<css::awt::XPopupMenu>& rPopupMenu,
                                              const OUString& rMenuIdentifier, css::ui::ContextMenuExecuteEvent aEvent)
{
    bool bModified = false;

    // create container from menu
    aEvent.ActionTriggerContainer = ::framework::ActionTriggerHelper::CreateActionTriggerContainerFromMenu(
        rPopupMenu, &rMenuIdentifier);

    // get selection from controller
    aEvent.Selection = css::uno::Reference< css::view::XSelectionSupplier >( GetController(), css::uno::UNO_QUERY );

    // call interceptors
    std::unique_lock g(pImpl->aMutex);
    std::vector<uno::Reference< ui::XContextMenuInterceptor>> aInterceptors =
        pImpl->aInterceptorContainer.getElements(g);
    g.unlock();
    for (const auto & rListener : aInterceptors )
    {
        try
        {
            css::ui::ContextMenuInterceptorAction eAction;
            {
                SolarMutexReleaser rel;
                eAction = rListener->notifyContextMenuExecute( aEvent );
            }
            switch ( eAction )
            {
                case css::ui::ContextMenuInterceptorAction_CANCELLED:
                    // interceptor does not want execution
                    return false;
                case css::ui::ContextMenuInterceptorAction_EXECUTE_MODIFIED:
                    // interceptor wants his modified menu to be executed
                    bModified = true;
                    break;
                case css::ui::ContextMenuInterceptorAction_CONTINUE_MODIFIED:
                    // interceptor has modified menu, but allows for calling other interceptors
                    bModified = true;
                    continue;
                case css::ui::ContextMenuInterceptorAction_IGNORED:
                    // interceptor is indifferent
                    continue;
                default:
                    SAL_WARN( "sfx.view", "Wrong return value of ContextMenuInterceptor!" );
                    continue;
            }
        }
        catch (...)
        {
            g.lock();
            pImpl->aInterceptorContainer.removeInterface(g, rListener);
            g.unlock();
        }

        break;
    }

    if ( bModified )
    {
        rPopupMenu->clear();
        ::framework::ActionTriggerHelper::CreateMenuFromActionTriggerContainer(rPopupMenu, aEvent.ActionTriggerContainer);
    }

    return true;
}

bool SfxViewShell::HandleNotifyEvent_Impl( NotifyEvent const & rEvent )
{
    if (pImpl->m_pController.is())
        return pImpl->m_pController->HandleEvent_Impl( rEvent );
    return false;
}

bool SfxViewShell::HasKeyListeners_Impl() const
{
    return (pImpl->m_pController.is())
        && pImpl->m_pController->HasKeyListeners_Impl();
}

bool SfxViewShell::HasMouseClickListeners_Impl() const
{
    return (pImpl->m_pController.is())
        && pImpl->m_pController->HasMouseClickListeners_Impl();
}

bool SfxViewShell::Escape()
{
    return GetViewFrame()->GetBindings().Execute( SID_TERMINATE_INPLACEACTIVATION );
}

Reference< view::XRenderable > SfxViewShell::GetRenderable()
{
    Reference< view::XRenderable >xRender;
    SfxObjectShell* pObj = GetObjectShell();
    if( pObj )
    {
        Reference< frame::XModel > xModel( pObj->GetModel() );
        if( xModel.is() )
            xRender.set( xModel, UNO_QUERY );
    }
    return xRender;
}

void SfxViewShell::notifyWindow(vcl::LOKWindowId nDialogId, const OUString& rAction, const std::vector<vcl::LOKPayloadItem>& rPayload) const
{
    SfxLokHelper::notifyWindow(this, nDialogId, rAction, rPayload);
}

uno::Reference< datatransfer::clipboard::XClipboardNotifier > SfxViewShell::GetClipboardNotifier() const
{
    uno::Reference< datatransfer::clipboard::XClipboardNotifier > xClipboardNotifier;
    if ( GetViewFrame() )
        xClipboardNotifier.set( GetViewFrame()->GetWindow().GetClipboard(), uno::UNO_QUERY );

    return xClipboardNotifier;
}

void SfxViewShell::AddRemoveClipboardListener( const uno::Reference < datatransfer::clipboard::XClipboardListener >& rClp, bool bAdd )
{
    try
    {
        if ( GetViewFrame() )
        {
            uno::Reference< datatransfer::clipboard::XClipboard > xClipboard( GetViewFrame()->GetWindow().GetClipboard() );
            if( xClipboard.is() )
            {
                uno::Reference< datatransfer::clipboard::XClipboardNotifier > xClpbrdNtfr( xClipboard, uno::UNO_QUERY );
                if( xClpbrdNtfr.is() )
                {
                    if( bAdd )
                        xClpbrdNtfr->addClipboardListener( rClp );
                    else
                        xClpbrdNtfr->removeClipboardListener( rClp );
                }
            }
        }
    }
    catch (const uno::Exception&)
    {
    }
}

weld::Window* SfxViewShell::GetFrameWeld() const
{
    return pWindow ? pWindow->GetFrameWeld() : nullptr;
}

void SfxViewShell::setBlockedCommandList(const char* blockedCommandList)
{
    if(!mvLOKBlockedCommandList.empty())
        return;

    OUString BlockedListString(blockedCommandList, strlen(blockedCommandList), RTL_TEXTENCODING_UTF8);
    OUString command = BlockedListString.getToken(0, ' ');
    for (size_t i = 1; !command.isEmpty(); i++)
    {
        mvLOKBlockedCommandList.emplace(command);
        command = BlockedListString.getToken(i, ' ');
    }
}

bool SfxViewShell::isBlockedCommand(OUString command)
{
    return mvLOKBlockedCommandList.find(command) != mvLOKBlockedCommandList.end();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
