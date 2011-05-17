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

                Ignoring the difference in header and footers, will the main
                document text have the same top/bottom bounds in word between
                both these objects.

                @param
                rOther the other HdFtDistanceGlue to compare against

                @return true if the main text areas top and bottom is at the
                same location, false otherwise.
            */
            bool EqualTopBottom(const HdFtDistanceGlue &rOther) const;

        };
    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
