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

#ifndef _BOOKMRK_HXX
#define _BOOKMRK_HXX

#include <cppuhelper/weakref.hxx>
#include <sfx2/Metadatable.hxx>
#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <map>
#include <rtl/ustring.hxx>
#include <IMark.hxx>

namespace com {
    namespace sun {
        namespace star {
            namespace text {
                class XTextContent;
            }
        }
    }
}

struct SwPosition;  // fwd Decl. wg. UI
class SwDoc;

namespace sw {
    namespace mark {
        class MarkBase
            : virtual public IMark
        {
        public:
            virtual SwPosition& GetMarkPos() const
                { return *m_pPos1; }
            virtual const ::rtl::OUString& GetName() const
                { return m_aName; }
            virtual SwPosition& GetOtherMarkPos() const
            {
                OSL_PRECOND(IsExpanded(), "<SwPosition::GetOtherMarkPos(..)> - I have no other Pos set." );
                return *m_pPos2;
            }
            virtual SwPosition& GetMarkStart() const
            {
                if( !IsExpanded() ) return GetMarkPos( );
                if ( GetMarkPos( ) < GetOtherMarkPos( ) )
                    return GetMarkPos();
                else
                    return GetOtherMarkPos( );
            }
            virtual SwPosition& GetMarkEnd() const
            {
                if( !IsExpanded() ) return GetMarkPos();
                if ( GetMarkPos( ) >= GetOtherMarkPos( ) )
                    return GetMarkPos( );
                else
                    return GetOtherMarkPos( );
            }

            virtual bool IsCoveringPosition(const SwPosition& rPos) const;
            virtual bool IsExpanded() const
                { return static_cast< bool >(m_pPos2); }

            virtual void SetName(const ::rtl::OUString& rName)
                { m_aName = rName; }
            virtual void SetMarkPos(const SwPosition& rNewPos);
            virtual void SetOtherMarkPos(const SwPosition& rNewPos);
            virtual void ClearOtherMarkPos()
                { m_pPos2.reset(); }

            virtual rtl::OUString ToString( ) const;

            virtual void Swap()
            {
                if(m_pPos2)
                    m_pPos1.swap(m_pPos2);
            }

            virtual void InitDoc(SwDoc* const)
            {
            }

            virtual ~MarkBase();

            const ::com::sun::star::uno::WeakReference<
                ::com::sun::star::text::XTextContent> & GetXBookmark() const
                    { return m_wXBookmark; }
            void SetXBookmark(::com::sun::star::uno::Reference<
                        ::com::sun::star::text::XTextContent> const& xBkmk)
                    { m_wXBookmark = xBkmk; }

        protected:
            // SwClient
            virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew );

            MarkBase(const SwPaM& rPaM, const ::rtl::OUString& rName);
            ::boost::scoped_ptr<SwPosition> m_pPos1;
            ::boost::scoped_ptr<SwPosition> m_pPos2;
            ::rtl::OUString m_aName;
            static ::rtl::OUString GenerateNewName(const ::rtl::OUString& rPrefix);

            ::com::sun::star::uno::WeakReference<
                ::com::sun::star::text::XTextContent> m_wXBookmark;
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

            const SwServerObject* GetRefObject() const
                { return &m_aRefObj; }
            SwServerObject* GetRefObject()
                { return &m_aRefObj; }

            bool IsServer() const
                { return m_aRefObj.Is(); }

            void SetRefObject( SwServerObject* pObj );

            void DeregisterFromDoc(SwDoc* const pDoc);
            virtual ~DdeBookmark();

        private:
            SwServerObjectRef m_aRefObj;
        };

        class Bookmark
            : virtual public IBookmark
            , public DdeBookmark
            , public ::sfx2::Metadatable
        {
        public:
            Bookmark(const SwPaM& rPaM,
                const KeyCode& rCode,
                const ::rtl::OUString& rName,
                const ::rtl::OUString& rShortName);
            virtual void InitDoc(SwDoc* const io_Doc);

            virtual const ::rtl::OUString& GetShortName() const
                { return m_sShortName; }
            virtual const KeyCode& GetKeyCode() const
                { return m_aCode; }
            virtual void SetShortName(const ::rtl::OUString& rShortName)
                { m_sShortName = rShortName; }
            virtual void SetKeyCode(const KeyCode& rCode)
                { m_aCode = rCode; }

            // ::sfx2::Metadatable
            virtual ::sfx2::IXmlIdRegistry& GetRegistry();
            virtual bool IsInClipboard() const;
            virtual bool IsInUndo() const;
            virtual bool IsInContent() const;
            virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::rdf::XMetadatable > MakeUnoObject();

        private:
            KeyCode m_aCode;
            ::rtl::OUString m_sShortName;
        };

        class Fieldmark
            : virtual public IFieldmark
            , public MarkBase
        {
        public:
            Fieldmark(const SwPaM& rPaM);

            virtual ::rtl::OUString GetFieldname() const
                { return m_aFieldname; }
            virtual ::rtl::OUString GetFieldHelptext() const
                { return m_aFieldHelptext; }

            virtual IFieldmark::parameter_map_t* GetParameters()
                { return &m_vParams; }

            virtual const IFieldmark::parameter_map_t* GetParameters() const
                { return &m_vParams; }

            virtual void SetFieldname(const ::rtl::OUString& aFieldname)
                { m_aFieldname = aFieldname; }
            virtual void SetFieldHelptext(const ::rtl::OUString& aFieldHelptext)
                { m_aFieldHelptext = aFieldHelptext; }

            virtual void Invalidate();
            virtual rtl::OUString ToString() const;

        private:
            ::rtl::OUString m_aFieldname;
            ::rtl::OUString m_aFieldHelptext;
            IFieldmark::parameter_map_t m_vParams;
        };

        class TextFieldmark
            : public Fieldmark
        {
        public:
            TextFieldmark(const SwPaM& rPaM);
            virtual void InitDoc(SwDoc* const io_pDoc);
            void ReleaseDoc(SwDoc* const pDoc);
        };

        class CheckboxFieldmark
            : virtual public ICheckboxFieldmark
            , public Fieldmark
        {
        public:
            CheckboxFieldmark(const SwPaM& rPaM);
            virtual void InitDoc(SwDoc* const io_pDoc);
            bool IsChecked() const;
            void SetChecked(bool checked);

            virtual rtl::OUString toString( ) const;
        };
    }
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
