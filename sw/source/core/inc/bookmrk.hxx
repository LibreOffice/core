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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_BOOKMRK_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_BOOKMRK_HXX

#include <cppuhelper/weakref.hxx>
#include <sfx2/Metadatable.hxx>
#include <vcl/keycod.hxx>
#include <memory>
#include <rtl/ustring.hxx>
#include <osl/diagnose.h>
#include <tools/ref.hxx>
#include <IMark.hxx>
#include <swrect.hxx>
#include "DropDownFormFieldButton.hxx"

namespace com {
    namespace sun {
        namespace star {
            namespace text {
                class XTextContent;
            }
        }
    }
}

class SwDoc;
class SwEditWin;
class SwServerObject;

namespace sw {
    namespace mark {
        class MarkBase
            : virtual public IMark
        {
        public:
            //getters
            virtual SwPosition& GetMarkPos() const override
                { return *m_pPos1; }
            virtual const OUString& GetName() const override
                { return m_aName; }
            virtual SwPosition& GetOtherMarkPos() const override
            {
                OSL_PRECOND(IsExpanded(), "<SwPosition::GetOtherMarkPos(..)> - I have no other Pos set." );
                return *m_pPos2;
            }
            virtual SwPosition& GetMarkStart() const override
            {
                if( !IsExpanded() ) return GetMarkPos( );
                if ( GetMarkPos( ) < GetOtherMarkPos( ) )
                    return GetMarkPos();
                else
                    return GetOtherMarkPos( );
            }
            virtual SwPosition& GetMarkEnd() const override
            {
                if( !IsExpanded() ) return GetMarkPos();
                if ( GetMarkPos( ) >= GetOtherMarkPos( ) )
                    return GetMarkPos( );
                else
                    return GetOtherMarkPos( );
            }

            virtual bool IsCoveringPosition(const SwPosition& rPos) const override;
            virtual bool IsExpanded() const override
                { return static_cast< bool >(m_pPos2); }

            void SetName(const OUString& rName)
                { m_aName = rName; }
            virtual void SetMarkPos(const SwPosition& rNewPos);
            virtual void SetOtherMarkPos(const SwPosition& rNewPos);
            virtual void ClearOtherMarkPos()
                { m_pPos2.reset(); }

            virtual OUString ToString( ) const override;
            virtual void dumpAsXml(xmlTextWriterPtr pWriter) const override;

            void Swap()
            {
                if(m_pPos2)
                    m_pPos1.swap(m_pPos2);
            }

            virtual void InitDoc(SwDoc* const, sw::mark::InsertMode)
            {
            }

            virtual ~MarkBase() override;

            const css::uno::WeakReference< css::text::XTextContent> & GetXBookmark() const
                    { return m_wXBookmark; }
            void SetXBookmark(css::uno::Reference< css::text::XTextContent> const& xBkmk)
                    { m_wXBookmark = xBkmk; }

        protected:
            // SwClient
            virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew ) override;

            MarkBase(const SwPaM& rPaM, const OUString& rName);
            std::unique_ptr<SwPosition> m_pPos1;
            std::unique_ptr<SwPosition> m_pPos2;
            OUString m_aName;
            static OUString GenerateNewName(const OUString& rPrefix);

            css::uno::WeakReference< css::text::XTextContent> m_wXBookmark;
        };

        class NavigatorReminder
            : public MarkBase
        {
        public:
            NavigatorReminder(const SwPaM& rPaM);
        };

        class UnoMark
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

            virtual void DeregisterFromDoc(SwDoc* const pDoc);
            virtual ~DdeBookmark() override;

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
            virtual void InitDoc(SwDoc* const io_Doc, sw::mark::InsertMode eMode) override;

            virtual void DeregisterFromDoc(SwDoc* const io_pDoc) override;

            virtual const OUString& GetShortName() const override
                { return m_sShortName; }
            virtual const vcl::KeyCode& GetKeyCode() const override
                { return m_aCode; }
            virtual void SetShortName(const OUString& rShortName) override
                { m_sShortName = rShortName; }
            virtual void SetKeyCode(const vcl::KeyCode& rCode) override
                { m_aCode = rCode; }
            virtual bool IsHidden() const override
                { return m_bHidden; }
            virtual const OUString& GetHideCondition() const override
                { return m_sHideCondition; }
            virtual void Hide(bool rHide) override
                { m_bHidden = rHide; }
            virtual void SetHideCondition(const OUString& rHideCondition) override
                { m_sHideCondition = rHideCondition; }

            // ::sfx2::Metadatable
            virtual ::sfx2::IXmlIdRegistry& GetRegistry() override;
            virtual bool IsInClipboard() const override;
            virtual bool IsInUndo() const override;
            virtual bool IsInContent() const override;
            virtual css::uno::Reference< css::rdf::XMetadatable > MakeUnoObject() override;

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

            virtual OUString GetFieldname() const override
                { return m_aFieldname; }
            virtual OUString GetFieldHelptext() const override
                { return m_aFieldHelptext; }

            virtual IFieldmark::parameter_map_t* GetParameters() override
                { return &m_vParams; }

            virtual const IFieldmark::parameter_map_t* GetParameters() const override
                { return &m_vParams; }

            virtual void SetFieldname(const OUString& aFieldname) override
                { m_aFieldname = aFieldname; }
            virtual void SetFieldHelptext(const OUString& aFieldHelptext) override
                { m_aFieldHelptext = aFieldHelptext; }

            virtual void ReleaseDoc(SwDoc* const) = 0;

            void SetMarkStartPos( const SwPosition& rNewStartPos );
            void SetMarkEndPos( const SwPosition& rNewEndPos );

            virtual void Invalidate() override;
            virtual OUString ToString() const override;
            virtual void dumpAsXml(xmlTextWriterPtr pWriter) const override;

        private:
            OUString m_aFieldname;
            OUString m_aFieldHelptext;
            IFieldmark::parameter_map_t m_vParams;
        };

        class TextFieldmark
            : public Fieldmark
        {
        public:
            TextFieldmark(const SwPaM& rPaM, const OUString& rName);
            virtual void InitDoc(SwDoc* const io_pDoc, sw::mark::InsertMode eMode) override;
            virtual void ReleaseDoc(SwDoc* const pDoc) override;
        };

        /// Fieldmark representing a checkbox form field.
        class CheckboxFieldmark
            : virtual public ICheckboxFieldmark
            , public Fieldmark
        {
        public:
            CheckboxFieldmark(const SwPaM& rPaM);
            virtual void InitDoc(SwDoc* const io_pDoc, sw::mark::InsertMode eMode) override;
            virtual void ReleaseDoc(SwDoc* const pDoc) override;
            bool IsChecked() const override;
            void SetChecked(bool checked) override;
        };

        /// Fieldmark representing a drop-down form field.
        class DropDownFieldmark
            : public Fieldmark
        {
        public:
            DropDownFieldmark(const SwPaM& rPaM);
            virtual ~DropDownFieldmark() override;
            virtual void InitDoc(SwDoc* const io_pDoc, sw::mark::InsertMode eMode) override;
            virtual void ReleaseDoc(SwDoc* const pDoc) override;

            // This method should be called only by the portion so we can now the portion's painting area
            void SetPortionPaintArea(const SwRect& rPortionPaintArea);

            void ShowButton(SwEditWin* pEditWin);
            void HideButton();
            void RemoveButton();

        private:
            SwRect m_aPortionPaintArea;
            VclPtr<DropDownFormFieldButton> m_pButton;
        };
    }
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
