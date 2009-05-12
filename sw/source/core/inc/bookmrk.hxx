/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: $
 * $Revision: $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _BOOKMRK_HXX
#define _BOOKMRK_HXX

#include <IMark.hxx>
#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>

struct SwPosition;  // fwd Decl. wg. UI
class SwDoc;

namespace sw { namespace mark
{
    class MarkBase
        : virtual public IMark
        , private ::boost::noncopyable
    {
        public:
            //getters
            virtual const SwPosition& GetMarkPos() const
                { return *m_pPos1; }
            virtual const ::rtl::OUString& GetName() const
                { return m_aName; }
            virtual bool IsCoveringPosition(const SwPosition& rPos) const
                { return GetMarkStart() <= rPos && rPos <= GetMarkEnd(); };
            virtual const SwPosition& GetOtherMarkPos() const
            {
                OSL_PRECOND(IsExpanded(), "<SwPosition::GetOtherMarkPos(..)> - I have no other Pos set." );
                return *m_pPos2;
            }
            virtual const SwPosition& GetMarkStart() const
            {
                if(!m_pPos2 /* !IsExpanded()*/) return *m_pPos1;
                return *m_pPos1 < *m_pPos2 ? *m_pPos1 : *m_pPos2;
            }
            virtual const SwPosition& GetMarkEnd() const
            {
                if(!m_pPos2 /* !IsExpanded()*/ ) return *m_pPos1;
                return *m_pPos1 > *m_pPos2 ? *m_pPos1 : *m_pPos2;
            }
            virtual bool IsExpanded() const
                { return m_pPos2; }

            //setters
            virtual void SetName(const ::rtl::OUString& rName)
                { m_aName = rName; }
            virtual void SetMarkPos(const SwPosition& rNewPos);
            virtual void SetOtherMarkPos(const SwPosition& rNewPos);
            virtual void ClearOtherMarkPos()
                { m_pPos2.reset(); }

            virtual void Swap()
            {
                if(m_pPos2)
                    m_pPos1.swap(m_pPos2);
            }

            virtual void InitDoc(SwDoc* const)
            {}

            virtual ~MarkBase();
        protected:
            MarkBase(const SwPaM& rPaM,
                const ::rtl::OUString& rName);
            ::boost::scoped_ptr<SwPosition> m_pPos1;
            ::boost::scoped_ptr<SwPosition> m_pPos2;
            ::rtl::OUString m_aName;
            static ::rtl::OUString GenerateNewName(const ::rtl::OUString& rPrefix);
    };

    class NavigatorReminder
        : public MarkBase
        , virtual public IMark
    {
        public:
            NavigatorReminder(const SwPaM& rPaM);
        private:
            static const ::rtl::OUString our_sNamePrefix;
    };

    class UnoMark
        : public MarkBase
    {
        public:
            UnoMark(const SwPaM& rPaM);
        private:
            static const ::rtl::OUString our_sNamePrefix;
    };

    class DdeBookmark
        : public MarkBase
    {
        public:
            DdeBookmark(const SwPaM& rPaM);

            //getters
            const SwServerObject* GetRefObject() const
                { return &m_aRefObj; }
            SwServerObject* GetRefObject()
                { return &m_aRefObj; }

            bool IsServer() const
                { return m_aRefObj.Is(); }

            //setters
            void SetRefObject( SwServerObject* pObj );

            void DeregisterFromDoc(SwDoc* const pDoc);
            virtual ~DdeBookmark();
        private:
            SwServerObjectRef m_aRefObj;
            static const ::rtl::OUString our_sNamePrefix;
    };

    class Bookmark
        : virtual public IBookmark
        , public DdeBookmark
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

            // getters
            ::rtl::OUString GetFieldname() const
                { return m_aFieldname; }
            ::rtl::OUString GetFieldHelptext() const
                { return m_aFieldHelptext; }

            // setters
            void SetFieldname(const ::rtl::OUString& aFieldname)
                { m_aFieldname = aFieldname; }
            void SetFieldHelptext(const ::rtl::OUString& aFieldHelptext)
                { m_aFieldHelptext = aFieldHelptext; }
        private:
            //int fftype; // Type: 0 = Text, 1 = Check Box, 2 = List
            //bool ffprot;

            ::rtl::OUString m_aFieldname;
            ::rtl::OUString m_aFieldHelptext;
            static const ::rtl::OUString our_sNamePrefix;

    };

    class TextFieldmark
        : public Fieldmark
    {
        public:
            TextFieldmark(const SwPaM& rPaM);
            virtual void InitDoc(SwDoc* const io_pDoc);
        private:
            //int fftypetxt; // Type of text field: 0 = Regular text, 1 = Number, 2 = Date, 3 = Current date, 4 = Current time, 5 = Calculation
            //int ffmaxlen; // Number of characters for text field. Zero means unlimited.
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
        private:
            bool m_isChecked;
            //bool ffsize; // 0 = Auto, 1=Exact (see ffhps)
            //bool ffrecalc;
            //int ffhps; // Check box size (half-point sizes).
    };

}}
#endif
