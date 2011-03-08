/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef _CROSSREFBOOKMRK_HXX
#define _CROSSREFBOOKMRK_HXX

#include <IMark.hxx>
#include <bookmrk.hxx>
#include <rtl/ustring.hxx>


namespace sw { namespace mark
{
    class CrossRefBookmark
        : public Bookmark
    {
        public:
            CrossRefBookmark(const SwPaM& rPaM,
                const KeyCode& rCode,
                const ::rtl::OUString& rName,
                const ::rtl::OUString& rShortName,
                const ::rtl::OUString& rPrefix);

            // getters
            virtual SwPosition& GetOtherMarkPos() const;
            virtual SwPosition& GetMarkStart() const
                { return *m_pPos1; }
            virtual SwPosition& GetMarkEnd() const
                { return *m_pPos1; }
            virtual bool IsExpanded() const
                { return false; }

            // setters
            virtual void SetMarkPos(const SwPosition& rNewPos);
            virtual void SetOtherMarkPos(const SwPosition&)
            {
                OSL_PRECOND(false,
                    "<CrossRefBookmark::SetOtherMarkPos(..)>"
                    " - misusage of CrossRefBookmark: other bookmark position isn't allowed to be set." );
            }
            virtual void ClearOtherMarkPos()
            {
                OSL_PRECOND(false,
                    "<SwCrossRefBookmark::ClearOtherMarkPos(..)>"
                    " - misusage of CrossRefBookmark: other bookmark position isn't allowed to be set or cleared." );
            }
    };

    class CrossRefHeadingBookmark
        : public CrossRefBookmark
    {
        public:
            CrossRefHeadingBookmark(const SwPaM& rPaM,
                const KeyCode& rCode,
                const ::rtl::OUString& rName,
                const ::rtl::OUString& rShortName);
            static ::rtl::OUString GenerateNewName();
            static bool IsLegalName(const ::rtl::OUString& rName);
            static const ::rtl::OUString our_sNamePrefix;
    };

    class CrossRefNumItemBookmark
        : public CrossRefBookmark
    {
        public:
            CrossRefNumItemBookmark(const SwPaM& rPaM,
                const KeyCode& rCode,
                const ::rtl::OUString& rName,
                const ::rtl::OUString& rShortName);
            static ::rtl::OUString GenerateNewName();
            static bool IsLegalName(const ::rtl::OUString& rName);
            static const ::rtl::OUString our_sNamePrefix;
    };

}}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
