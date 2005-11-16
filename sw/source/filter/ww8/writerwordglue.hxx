/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: writerwordglue.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2005-11-16 13:54:04 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */
/// @HTML

#ifndef SW_WRITERWORDGLUE
#define SW_WRITERWORDGLUE

#ifndef WW_NEEDED_CAST_HXX
#   include "needed_cast.hxx"
#endif
#ifndef WW_TYPES
#   include "types.hxx"
#endif

class SwFrmFmt;
class SfxItemSet;
class SwDoc;
class SwTxtFmtColl;
class String;
class PoolItems;

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


        /** If a page dimension is close to a standard page size, snap to it.

            Commonly a page dimension is slightly different from a standard
            page size, so close that its likely a rounding error after
            creeping in. Use this to snap to standard sizes when within a
            trivial distance from a standard size.

            @param
            nSize the dimension to test against standard dimensions

            @return New dimension to use, equal to nSize unless within a
            trivial amount of a standard page dimension

            @author
            <a href="mailto:cmc@openoffice.org">Caol&aacute;n McNamara</a>
        */
        long SnapPageDimension(long nSize) throw();
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
/* vi:set tabstop=4 shiftwidth=4 expandtab: */
