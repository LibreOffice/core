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
#pragma once

#include <com/sun/star/text/XTextContent.hpp>
#include <cppuhelper/weakref.hxx>
#include <osl/diagnose.h>
#include <rtl/ustring.hxx>
#include <sfx2/Metadatable.hxx>
#include <unotools/weakref.hxx>
#include <tools/ref.hxx>
#include <vcl/keycod.hxx>
#include "swrect.hxx"
#include "calbck.hxx"
#include "pam.hxx"
#include "names.hxx"
#include "swdllapi.h"
#include <map>
#include <memory>
#include <optional>
#include <string_view>

class SwDoc;
class SwEditWin;
class SvNumberFormatter;
class SwServerObject;
class SfxViewShell;
class SwXBookmark;
class FormFieldButton;

namespace sw::mark
{
    enum class InsertMode
    {
        New,
        CopyText,
    };

    class SW_DLLPUBLIC MarkBase
        : public ISwContentIndexOwner,
          virtual public sw::BroadcastingModify // inherited as interface
    {
    public:
        virtual SwContentIndexOwnerType GetOwnerType() const override final { return SwContentIndexOwnerType::Mark; }

        //getters
        SwPosition& GetMarkPos() const
            { return const_cast<SwPosition&>(*m_oPos1); }
        const ReferenceMarkerName& GetName() const
            { return m_aName; }
        // GetOtherMarkPos() is only guaranteed to return a valid
        // reference if IsExpanded() returned true
        virtual SwPosition& GetOtherMarkPos() const
        {
            OSL_PRECOND(IsExpanded(), "<SwPosition::GetOtherMarkPos(..)> - I have no other Pos set." );
            return const_cast<SwPosition&>(*m_oPos2);
        }
        virtual SwPosition& GetMarkStart() const
        {
            SwPosition& rPos1 = GetMarkPos();
            if( !IsExpanded() )
                return rPos1;
            SwPosition& rPos2 = GetOtherMarkPos();
            if ( rPos1 < rPos2 )
                return rPos1;
            else
                return rPos2;
        }
        virtual SwPosition& GetMarkEnd() const
        {
            SwPosition& rPos1 = GetMarkPos();
            if( !IsExpanded() )
                return rPos1;
            SwPosition& rPos2 = GetOtherMarkPos();
            if ( rPos1 >= rPos2 )
                return rPos1;
            else
                return rPos2;
        }
        virtual std::pair<SwPosition&,SwPosition&> GetMarkStartEnd() const
        {
            SwPosition& rPos1 = GetMarkPos();
            if( !IsExpanded() )
                return {rPos1, rPos1};
            SwPosition& rPos2 = GetOtherMarkPos();
            if ( rPos1 < rPos2 )
                return {rPos1, rPos2};
            else
                return {rPos2, rPos1};
        }

        bool IsCoveringPosition(const SwPosition& rPos) const;
        virtual bool IsExpanded() const
            { return m_oPos2.has_value(); }

        void SetName(const ReferenceMarkerName& rName)
            { m_aName = rName; }
        virtual void SetMarkPos(const SwPosition& rNewPos);
        virtual void SetOtherMarkPos(const SwPosition& rNewPos);
        virtual void ClearOtherMarkPos()
            { m_oPos2.reset(); }

        virtual auto InvalidateFrames() -> void;

        virtual OUString ToString( ) const;
        virtual void dumpAsXml(xmlTextWriterPtr pWriter) const;

        void Swap()
        {
            if(m_oPos2)
                m_oPos1.swap(m_oPos2);
        }

        virtual void InitDoc(SwDoc&, sw::mark::InsertMode, SwPosition const*)
        {
        }

        ~MarkBase() override;

        const unotools::WeakReference<SwXBookmark> & GetXBookmark() const
                { return m_wXBookmark; }
        void SetXBookmark(rtl::Reference<SwXBookmark> const& xBkmk);

        static ReferenceMarkerName GenerateNewName(std::u16string_view rPrefix);
    protected:
        // SwClient
        void SwClientNotify(const SwModify&, const SfxHint&) override;

        MarkBase(const SwPaM& rPaM, ReferenceMarkerName aName);
        std::optional<SwPosition> m_oPos1;
        std::optional<SwPosition> m_oPos2;
        ReferenceMarkerName m_aName;

        unotools::WeakReference<SwXBookmark> m_wXBookmark;
    };

    class NavigatorReminder final
        : public MarkBase
    {
    public:
        NavigatorReminder(const SwPaM& rPaM);
    };

    class UnoMark final
        : public MarkBase
    {
    public:
        UnoMark(const SwPaM& rPaM);
    };

    class SW_DLLPUBLIC DdeBookmark
        : public MarkBase
    {
    public:
        DdeBookmark(const SwPaM& rPaM);

        const SwServerObject* GetRefObject() const { return m_aRefObj.get(); }
        SwServerObject* GetRefObject() { return m_aRefObj.get(); }

        bool IsServer() const { return m_aRefObj.is(); }

        void SetRefObject( SwServerObject* pObj );

        virtual void DeregisterFromDoc(SwDoc& rDoc);
        ~DdeBookmark() override;

    private:
        tools::SvRef<SwServerObject> m_aRefObj;
    };

    class SW_DLLPUBLIC Bookmark
        : public DdeBookmark
        , public ::sfx2::Metadatable
    {
    public:
        Bookmark(const SwPaM& rPaM,
            const vcl::KeyCode& rCode,
            const ReferenceMarkerName& rName);
        void InitDoc(SwDoc& io_Doc, sw::mark::InsertMode eMode, SwPosition const* pSepPos) override;

        void DeregisterFromDoc(SwDoc& io_rDoc) override;

        auto InvalidateFrames() -> void override;

        const OUString& GetShortName() const
            { return m_sShortName; }
        const vcl::KeyCode& GetKeyCode() const
            { return m_aCode; }
        void SetShortName(const OUString& rShortName)
            { m_sShortName = rShortName; }
        void SetKeyCode(const vcl::KeyCode& rCode)
            { m_aCode = rCode; }
        bool IsHidden() const
            { return m_bHidden; }
        const OUString& GetHideCondition() const
            { return m_sHideCondition; }
        void Hide(bool rHide);
        void SetHideCondition(const OUString& rHideCondition);

        // ::sfx2::Metadatable
        ::sfx2::IXmlIdRegistry& GetRegistry() override;
        bool IsInClipboard() const override;
        bool IsInUndo() const override;
        bool IsInContent() const override;
        void sendLOKDeleteCallback();
        css::uno::Reference< css::rdf::XMetadatable > MakeUnoObject() override;

    private:
        vcl::KeyCode m_aCode;
        OUString m_sShortName;
        bool m_bHidden;
        OUString m_sHideCondition;
    };

    class SW_DLLPUBLIC Fieldmark
        : public MarkBase
    {
    public:
        typedef std::map< OUString, css::uno::Any> parameter_map_t;

        Fieldmark(const SwPaM& rPaM);

        virtual OUString GetContent() const { return OUString(); }
        virtual void ReplaceContent(const OUString& /*sNewContent*/) {}

        const OUString & GetFieldname() const { return m_aFieldname; }
        const OUString & GetFieldHelptext() const { return m_aFieldHelptext; }

        parameter_map_t* GetParameters() { return &m_vParams; }

        const parameter_map_t* GetParameters() const
            { return &m_vParams; }

        void SetFieldname(const OUString& aFieldname)
            { m_aFieldname = aFieldname; }
        void SetFieldHelptext(const OUString& aFieldHelptext)
            { m_aFieldHelptext = aFieldHelptext; }

        virtual void ReleaseDoc(SwDoc&) = 0;

        void SetMarkStartPos( const SwPosition& rNewStartPos );

        void Invalidate();
        OUString ToString() const override;
        void dumpAsXml(xmlTextWriterPtr pWriter) const override;

    private:
        OUString m_aFieldname;
        OUString m_aFieldHelptext;
        parameter_map_t m_vParams;
    };

    OUString ExpandFieldmark(Fieldmark* pBM);

    class TextFieldmark final
        : public Fieldmark
    {
    public:
        TextFieldmark(const SwPaM& rPaM, const ReferenceMarkerName& rName);
        ~TextFieldmark();
        void InitDoc(SwDoc& io_rDoc, sw::mark::InsertMode eMode, SwPosition const* pSepPos) override;
        void ReleaseDoc(SwDoc& rDoc) override;

        OUString GetContent() const override;
        void ReplaceContent(const OUString& sNewContent) override;

        bool HasDefaultContent() const;

    private:
        sw::DocumentContentOperationsManager* m_pDocumentContentOperationsManager;
    };

    // Non text fieldmarks have no content between the start and end marks.
    class SW_DLLPUBLIC NonTextFieldmark
        : public Fieldmark
    {
    public:
        NonTextFieldmark(const SwPaM& rPaM);
        void InitDoc(SwDoc& io_rDoc, sw::mark::InsertMode eMode, SwPosition const* pSepPos) override;
        void ReleaseDoc(SwDoc& rDoc) override;
    };

    /// Fieldmark representing a checkbox form field.
    class SW_DLLPUBLIC CheckboxFieldmark final
        : public NonTextFieldmark
    {
    public:
        CheckboxFieldmark(const SwPaM& rPaM, const ReferenceMarkerName& rName);
        bool IsChecked() const;
        void SetChecked(bool checked);
        OUString GetContent() const override;
        void ReplaceContent(const OUString& sNewContent) override;
    };

    /// Fieldmark with a drop down button (e.g. this button opens the date picker for a date field)
    class SW_DLLPUBLIC FieldmarkWithDropDownButton
        : public NonTextFieldmark
    {
    public:
        FieldmarkWithDropDownButton(const SwPaM& rPaM);
        ~FieldmarkWithDropDownButton() override;

        virtual void ShowButton(SwEditWin* pEditWin) = 0;
        virtual void RemoveButton();
        void LaunchPopup();

    protected:
        VclPtr<FormFieldButton> m_pButton;
    };

    /// Fieldmark representing a drop-down form field.
    class SW_DLLPUBLIC DropDownFieldmark final
        : public FieldmarkWithDropDownButton
    {
    public:
        DropDownFieldmark(const SwPaM& rPaM, const ReferenceMarkerName& rName);
        ~DropDownFieldmark() override;

        void ShowButton(SwEditWin* pEditWin) override;
        void RemoveButton() override;
        OUString GetContent(sal_Int32* pIndex) const;
        OUString GetContent() const override;
        void AddContent(const OUString& rText, sal_Int32* pIndex = nullptr);
        void DelContent(sal_Int32 nDelIndex = -1);
        void ReplaceContent(const OUString* pText, sal_Int32* pIndex);
        void ReplaceContent(const OUString& sNewContent) override;

        // This method should be called only by the portion so we can now the portion's painting area
        void SetPortionPaintArea(const SwRect& rPortionPaintArea);

        void SendLOKShowMessage(const SfxViewShell* pViewShell);
        static void SendLOKHideMessage(const SfxViewShell* pViewShell);

    private:
        SwRect m_aPortionPaintArea;
    };

    /// Fieldmark representing a date form field.
    /// TODO: this was an SDT in DOCX, which is modelled suboptimally here
    /// as a fieldmark; as it cannot contain paragraph breaks, must be
    /// well-formed XML element, and does not have field separator, it
    /// should be a nesting text attribute similar to SwTextMeta.
    class SW_DLLPUBLIC DateFieldmark final
        : public FieldmarkWithDropDownButton
    {
    public:
        DateFieldmark(const SwPaM& rPaM);
        ~DateFieldmark() override;

        void InitDoc(SwDoc& io_rDoc, sw::mark::InsertMode eMode, SwPosition const* pSepPos) override;
        void ReleaseDoc(SwDoc& rDoc) override;

        void ShowButton(SwEditWin* pEditWin) override;

        void SetPortionPaintAreaStart(const SwRect& rPortionPaintArea);
        void SetPortionPaintAreaEnd(const SwRect& rPortionPaintArea);

        OUString GetContent() const override;
        void ReplaceContent(const OUString& sNewContent) override;

        std::pair<bool, double> GetCurrentDate() const;
        void SetCurrentDate(double fDate);
        OUString GetDateInStandardDateFormat(double fDate) const;

    private:
        OUString GetDateInCurrentDateFormat(double fDate) const;
        std::pair<bool, double> ParseCurrentDateParam() const;
        void InvalidateCurrentDateParam();

        SvNumberFormatter* m_pNumberFormatter;
        sw::DocumentContentOperationsManager* m_pDocumentContentOperationsManager;
        SwRect m_aPaintAreaStart;
        SwRect m_aPaintAreaEnd;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
