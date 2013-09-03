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

#ifndef SW_WRITERWORDGLUE
#define SW_WRITERWORDGLUE

#include "needed_cast.hxx"
#include "types.hxx"

class SwFrmFmt;
class SfxItemSet;

namespace sw
{
    namespace types
    {
        /** A static_cast style cast for conversion of word types to writer's

            There are a number of places where the winword types are larger
            than the writer equivalents requiring a cast to silence warnings.
            To avoid throwing away this useful information writer_cast is used
            to identify where writer's types are smaller than word's.

            Based on needed_cast it will compile time assert if the cast
            becomes unnecessary at any time in the future.

            @tplparam
            Ret the desired return type

            @tplparam
            Param the type of the in param

            @param
            in the value to cast from Param to Ret

            @return in casted to type Ret
        */
        template<typename Ret, typename Param> Ret writer_cast(Param in)
        {
            return ww::needed_cast<Ret, Param>(in);
        }

        /** A static_cast style cast for conversion of writer types to word's

            There are a number of places where the writer types are larger than
            the winword equivalents requiring a cast to silence warnings.  To
            avoid throwing away this useful information writer_cast is used to
            identify where word's types are smaller than writers's.

            Based on needed_cast it will compile time assert if the cast
            becomes unnecessary at any time in the future.

            @tplparam
            Ret the desired return type

            @tplparam
            Param the type of the in param

            @param
            in the value to cast from Param to Ret

            @return in casted to type Ret
        */
        template<typename Ret, typename Param> Ret msword_cast(Param in)
        {
            return ww::needed_cast<Ret, Param>(in);
        }
    }

    namespace util
    {
        /** See if two page formats can be expressed as a single word section

            Word doesn't have the idea of page descriptors and follow styles
            like writer does, the only thing it has is a section with a
            different title page. The only difference of the title page from
            the rest of the section is different headers/footers, everything
            else is the same.

            So this function compares two writer page fmts and sees if the
            follow frame and the title frame are the same from word persecptive
            except for the content of their headers.

            @return true if the rTitleFmt followed by rFollowFmt could be
            expressed in word as a single word Section with different title
            page enabled.

            @author
            <a href="mailto:cmc@openoffice.org">Caol&aacute;n McNamara</a>

            @see #i4320#/#i14509#/#i11717# for examples
        */
        bool IsPlausableSingleWordSection(const SwFrmFmt &rTitleFmt,
            const SwFrmFmt &rFollowFmt);

        /** Make export a word section top/bottom values easy

            The top and bottom margins in word and writer are expressed in very
            different ways. This class provides the equivalent word values for
            header/footer distances from a given writer attrset of a page

            @author
            <a href="mailto:cmc@openoffice.org">Caol&aacute;n McNamara</a>
        */
        class HdFtDistanceGlue
        {
        private:
            bool mbHasHeader;
            bool mbHasFooter;
        public:
            sal_uInt16 dyaHdrTop;
            sal_uInt16 dyaHdrBottom;
            sal_uInt16 dyaTop;
            sal_uInt16 dyaBottom;
            HdFtDistanceGlue(const SfxItemSet &rPage);
            bool HasHeader() const { return mbHasHeader; }
            bool HasFooter() const { return mbHasFooter; }

            /** Is the top of the page the same in both objects
                when there are headers\footers present or non-present in both objects

                This test is important, because we would like to ignore cases
                when there is a header in one object and no header in the second
                object - because it is wrong to compare between them.

                @param
                rOther the other HdFtDistanceGlue to compare against

                @return true if the main text areas top and bottom is at the
                same location, false otherwise (assuming both objects have\don't have
                a header\footer)
            */
            bool StrictEqualTopBottom(const HdFtDistanceGlue &rOther) const;

        };
    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
