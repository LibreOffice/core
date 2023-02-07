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

#include <cppuhelper/weakref.hxx>
#include <sfx2/Metadatable.hxx>
#include <vcl/keycod.hxx>
#include <unotools/weakref.hxx>
#include <memory>
#include <optional>
#include <string_view>
#include <com/sun/star/text/XTextContent.hpp>

#include <rtl/ustring.hxx>
#include <osl/diagnose.h>
#include <tools/ref.hxx>
#include <IMark.hxx>
#include <swrect.hxx>
#include "FormFieldButton.hxx"

class SwDoc;
class SwEditWin;
class SwServerObject;
class SvNumberFormatter;
class SfxViewShell;
class SwXBookmark;

namespace sw::mark {
        class MarkBase
            : virtual public IMark
        {
        public:
            //getters
            SwPosition& GetMarkPos() const override
                { return const_cast<SwPosition&>(*m_oPos1); }
            const OUString& GetName() const override
                { return m_aName; }
            SwPosition& GetOtherMarkPos() const override
            {
                OSL_PRECOND(IsExpanded(), "<SwPosition::GetOtherMarkPos(..)> - I have no other Pos set." );
                return const_cast<SwPosition&>(*m_oPos2);
            }
            SwPosition& GetMarkStart() const override
            {
                if( !IsExpanded() ) return GetMarkPos( );
                if ( GetMarkPos( ) < GetOtherMarkPos( ) )
                    return GetMarkPos();
                else
                    return GetOtherMarkPos( );
            }
            SwPosition& GetMarkEnd() const override
            {
                if( !IsExpanded() ) return GetMarkPos();
                if ( GetMarkPos( ) >= GetOtherMarkPos( ) )
                    return GetMarkPos( );
                else
                    return GetOtherMarkPos( );
            }

            bool IsCoveringPosition(const SwPosition& rPos) const override;
            bool IsExpanded() const override
                { return m_oPos2.has_value(); }

            void SetName(const OUString& rName)
                { m_aName = rName; }
            virtual void SetMarkPos(const SwPosition& rNewPos);
            virtual void SetOtherMarkPos(const SwPosition& rNewPos);
            virtual void ClearOtherMarkPos()
                { m_oPos2.reset(); }

            virtual auto InvalidateFrames() -> void;

            OUString ToString( ) const override;
            void dumpAsXml(xmlTextWriterPtr pWriter) const override;

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

        protected:
            // SwClient
            void SwClientNotify(const SwModify&, const SfxHint&) override;

            MarkBase(const SwPaM& rPaM, OUString aName);
            std::optional<SwPosition> m_oPos1;
            std::optional<SwPosition> m_oPos2;
            OUString m_aName;
            static OUString GenerateNewName(std::u16string_view rPrefix);

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

        class DdeBookmark
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

        class Bookmark
            : virtual public IBookmark
            , public DdeBookmark
            , public ::sfx2::Metadatable
        {
        public:
            Bookmark(const SwPaM& rPaM,
                const vcl::KeyCode& rCode,
                const OUString& rName);
            void InitDoc(SwDoc& io_Doc, sw::mark::InsertMode eMode, SwPosition const* pSepPos) override;

            void DeregisterFromDoc(SwDoc& io_rDoc) override;

            auto InvalidateFrames() -> void override;

            const OUString& GetShortName() const override
                { return m_sShortName; }
            const vcl::KeyCode& GetKeyCode() const override
                { return m_aCode; }
            void SetShortName(const OUString& rShortName) override
                { m_sShortName = rShortName; }
            void SetKeyCode(const vcl::KeyCode& rCode) override
                { m_aCode = rCode; }
            bool IsHidden() const override
                { return m_bHidden; }
            const OUString& GetHideCondition() const override
                { return m_sHideCondition; }
            void Hide(bool rHide) override;
            void SetHideCondition(const OUString& rHideCondition) override;

            // ::sfx2::Metadatable
            ::sfx2::IXmlIdRegistry& GetRegistry() override;
            bool IsInClipboard() const override;
            bool IsInUndo() const override;
            bool IsInContent() const override;
            css::uno::Reference< css::rdf::XMetadatable > MakeUnoObject() override;

        private:
            vcl::KeyCode m_aCode;
            OUString m_sShortName;
            bool m_bHidden;
            OUString m_sHideCondition;
        };

        class Fieldmark
            : virtual public IFieldmark
            , public MarkBase
        {
        public:
            Fieldmark(const SwPaM& rPaM);

            OUString GetFieldname() const override
                { return m_aFieldname; }
            OUString GetFieldHelptext() const override
                { return m_aFieldHelptext; }

            IFieldmark::parameter_map_t* GetParameters() override
                { return &m_vParams; }

            const IFieldmark::parameter_map_t* GetParameters() const override
                { return &m_vParams; }

            void SetFieldname(const OUString& aFieldname) override
                { m_aFieldname = aFieldname; }
            void SetFieldHelptext(const OUString& aFieldHelptext) override
                { m_aFieldHelptext = aFieldHelptext; }

            virtual void ReleaseDoc(SwDoc&) = 0;

            void SetMarkStartPos( const SwPosition& rNewStartPos );

            void Invalidate() override;
            OUString ToString() const override;
            void dumpAsXml(xmlTextWriterPtr pWriter) const override;

        private:
            OUString m_aFieldname;
            OUString m_aFieldHelptext;
            IFieldmark::parameter_map_t m_vParams;
        };

        class TextFieldmark final
            : public Fieldmark
        {
        public:
            TextFieldmark(const SwPaM& rPaM, const OUString& rName);
            ~TextFieldmark();
            void InitDoc(SwDoc& io_rDoc, sw::mark::InsertMode eMode, SwPosition const* pSepPos) override;
            void ReleaseDoc(SwDoc& rDoc) override;

            OUString GetContent() const override;
            void ReplaceContent(const OUString& sNewContent) override;

        private:
            sw::DocumentContentOperationsManager* m_pDocumentContentOperationsManager;
        };

        // Non text fieldmarks have no content between the start and end marks.
        class NonTextFieldmark
            : public Fieldmark
        {
        public:
            NonTextFieldmark(const SwPaM& rPaM);
            void InitDoc(SwDoc& io_rDoc, sw::mark::InsertMode eMode, SwPosition const* pSepPos) override;
            void ReleaseDoc(SwDoc& rDoc) override;
        };

        /// Fieldmark representing a checkbox form field.
        class CheckboxFieldmark final
            : virtual public ICheckboxFieldmark
            , public NonTextFieldmark
        {
        public:
            CheckboxFieldmark(const SwPaM& rPaM, const OUString& rName);
            bool IsChecked() const override;
            void SetChecked(bool checked) override;
            OUString GetContent() const override;
            void ReplaceContent(const OUString& sNewContent) override;
        };

        /// Fieldmark with a drop down button (e.g. this button opens the date picker for a date field)
        class FieldmarkWithDropDownButton
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
        class DropDownFieldmark final
            : virtual public IDropdownFieldmark
            , public FieldmarkWithDropDownButton
        {
        public:
            DropDownFieldmark(const SwPaM& rPaM, const OUString& rName);
            ~DropDownFieldmark() override;

            void ShowButton(SwEditWin* pEditWin) override;
            void RemoveButton() override;
            OUString GetContent(sal_Int32* pIndex) const override;
            OUString GetContent() const override;
            void AddContent(const OUString& rText, sal_Int32* pIndex = nullptr) override;
            void DelContent(sal_Int32 nDelIndex = -1) override;
            void ReplaceContent(const OUString* pText, sal_Int32* pIndex) override;
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
        class DateFieldmark final
            : virtual public IDateFieldmark
            , public FieldmarkWithDropDownButton
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

            std::pair<bool, double> GetCurrentDate() const override;
            void SetCurrentDate(double fDate) override;
            OUString GetDateInStandardDateFormat(double fDate) const override;

        private:
            OUString GetDateInCurrentDateFormat(double fDate) const;
            std::pair<bool, double> ParseCurrentDateParam() const;
            void InvalidateCurrentDateParam();

            SvNumberFormatter* m_pNumberFormatter;
            sw::DocumentContentOperationsManager* m_pDocumentContentOperationsManager;
            SwRect m_aPaintAreaStart;
            SwRect m_aPaintAreaEnd;
        };

        /// return position of the CH_TXT_ATR_FIELDSEP for rMark
        SwPosition FindFieldSep(IFieldmark const& rMark);

        /// check if rPaM is valid range of new fieldmark
        bool IsFieldmarkOverlap(SwPaM const& rPaM);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
