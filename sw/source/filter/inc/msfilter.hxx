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

#ifndef INCLUDED_SW_SOURCE_FILTER_INC_MSFILTER_HXX
#define INCLUDED_SW_SOURCE_FILTER_INC_MSFILTER_HXX

#include <sal/config.h>

#include <cstddef>
#include <set>
#include <map>
#include <vector>
#include <memory>
#include <swtypes.hxx>
#include "wwstyles.hxx"
#include <rtl/textenc.h>
#include <tools/gen.hxx>
#include <filter/msfilter/util.hxx>
#include "fltshell.hxx"
#include <redline.hxx>
#include <shellio.hxx>
#include <svl/zforlist.hxx>

class SwDoc;
class SwPaM;
class SwTableNode;
class SwNodeIndex;
class SwNoTextNode;
class SwTextNode;
class WW8TabDesc;

namespace myImplHelpers
{
template<class C> class StyleMapperImpl;
}

class SwTextFormatColl;
class SwCharFormat;

namespace sw
{
    namespace ms
    {
        /** MSOffice appears to set the charset of unicode fonts to MS 932

            But we do "default", whatever that means.

            @param eTextEncoding
                the OOo encoding to convert from

            @return
                a msoffice equivalent charset identifier
        */
        sal_uInt8 rtl_TextEncodingToWinCharset(rtl_TextEncoding eTextEncoding);

        /** MSOffice appears to set the charset of unicode fonts to MS 932

            Arial Unicode MS for example is a unicode font, but word sets
            exported uses of it to the MS 932 charset

        */
        sal_uInt8 rtl_TextEncodingToWinCharsetRTF(OUString const& rFontName,
                OUString const& rAltName, rtl_TextEncoding eTextEncoding);

        /** Convert from DTTM to Writer's DateTime

        */
        sal_uInt32 DateTime2DTTM( const DateTime& rDT );

        /** Convert from Word Date/Time field str to Writer's Date Time str

        */
        sal_uLong MSDateTimeFormatToSwFormat(OUString& rParams, SvNumberFormatter *pFormatter, LanguageType &rLang, bool bHijri, LanguageType nDocLang);

        /*Used to identify if the previous token is AM time field*/
        bool IsPreviousAM(OUString const & rParams, sal_Int32 nPos);

        /*Used to identify if the next token is PM time field*/
        bool IsNextPM(OUString const & rParams, sal_Int32 nPos);

        /** Used by MSDateTimeFormatToSwFormat to identify AM time fields

        */
        bool IsNotAM(OUString const & rParams, sal_Int32 nPos);

        /** Another function used by MSDateTimeFormatToSwFormat

        */
        void SwapQuotesInField(OUString &rFormat);

    }

    namespace util
    {
        /// Redlining Authors, map word author key to writer author value
        typedef std::map<sal_uInt16, std::size_t> AuthorInfos;

        /** Clips a value to MAX/MIN 16bit value to make it safe for use
            as a position value to give to writer. i.e. +-57.8cm. Sometimes
            we see ridiculous values for positioning in rtf and word document,
            this captures such ones and clips them to values which are
            still outside the document, but of a value that doesn't cause
            problems for writer's layout, e.g. see
            https://bz.apache.org/ooo/show_bug.cgi?id=i9245

            @param nIn

            @return nIn clipped to min/max 16bit value
        */
        SwTwips MakeSafePositioningValue(SwTwips nIn);

        /** Knows which writer style a given word style should be imported as

            Mapping a word style to a writer style has to consider mapping
            the word builtin styles like "Normal" as the default root style
            to our default root style which is called "Default" in english,
            and "Normal" in german.

            Additionally it then has to avoid name collisions such as

            a) styles "Normal" and "Default" in a single document, where
            we can use the original distinct names "Normal" and "Default" and..
            b) styles "Normal" and "Default" in a single document, where
            we can not use the original names, and must come up with an
            alternative name for one of them..

            And it needs to report to the importer if the style being mapped to
            was already in existence, for the cut and paste/insert file mode we
            should not modify the returned style if it is already in use as it
            is does not belong to us to change.
        */
        class ParaStyleMapper
        {
        private:
            //I hate these things stupid pImpl things, but its warranted here
             std::unique_ptr<::myImplHelpers::StyleMapperImpl<SwTextFormatColl> > mpImpl;
        public:
            explicit ParaStyleMapper(SwDoc &rDoc);
            ~ParaStyleMapper();

            /** StyleResult
                StyleResult is a std::pair of a pointer to a style and a flag
                which is true if the style existed previously in the document.
            */
            typedef std::pair<SwTextFormatColl*, bool> StyleResult;

            /** Get the writer style which the word style should map to

                @param rName
                The name of the word style

                @param eSti
                The style id of the word style, we are really only interested
                in knowing if the style has either a builtin standard id, or is
                a user defined style.

                @return
                The equivalent writer style packaged as a StyleResult to use
                for this word style.

                It will only return a failure in the pathological case of
                catastropic failure elsewhere of there exist already styles
                rName and WW-rName[0..SAL_MAX_INT32], which is both unlikely
                and impossible.
            */
            StyleResult GetStyle(const OUString& rName, ww::sti eSti);
        };

        /** Knows which writer style a given word style should be imported as

            Mapping a word style to a writer style has to consider mapping
            the word builtin styles like "Normal" as the default root style
            to our default root style which is called "Default" in english,
            and "Normal" in german.

            Additionally it then has to avoid name collisions such as

            a) styles "Normal" and "Default" in a single document, where
            we can use the original distinct names "Normal" and "Default" and..
            b) styles "Normal" and "Default" in a single document, where
            we can not use the original names, and must come up with an
            alternative name for one of them..

            And it needs to report to the importer if the style being mapped to
            was already in existence, for the cut and paste/insert file mode we
            should not modify the returned style if it is already in use as it
            is does not belong to us to change.
        */
        class CharStyleMapper
        {
        private:
            //I hate these things stupid pImpl things, but its warranted here
            std::unique_ptr<::myImplHelpers::StyleMapperImpl<SwCharFormat>> mpImpl;
        public:
            explicit CharStyleMapper(SwDoc &rDoc);
            ~CharStyleMapper();

            /** StyleResult
                StyleResult is a std::pair of a pointer to a style and a flag
                which is true if the style existed previously in the document.
            */
            typedef std::pair<SwCharFormat*, bool> StyleResult;

            /** Get the writer style which the word style should map to

                @param rName
                The name of the word style

                @param eSti
                The style id of the word style, we are really only interested
                in knowing if the style has either a builtin standard id, or is
                a user defined style.

                @return
                The equivalent writer style packaged as a StyleResult to use
                for this word style.

                It will only return a failure in the pathological case of
                catastropic failure elsewhere of there exist already styles
                rName and WW-rName[0..SAL_MAX_INT32], which is both unlikely
                and impossible.
            */
            StyleResult GetStyle(const OUString& rName, ww::sti eSti);
        };

        /** Find suitable names for exporting this font

            Given a fontname description find the best primary and secondary
            fallback font to use from MSWord's persp font

            @see #i10242#/#i19164# for examples
        */
        class FontMapExport
        {
        public:
            OUString msPrimary;
            OUString msSecondary;
            explicit FontMapExport(const OUString &rFontDescription);
        };

        class InsertedTableClient : public SwClient
        {
        public:
            explicit InsertedTableClient(SwTableNode & rNode);
            SwTableNode * GetTableNode();
        };

        /** Handle requirements for table formatting in insert->file mode.

            When inserting a table into a document which already has been
            formatted and laid out (e.g using insert->file) then tables
            must be handled in a special way, (or so previous comments and
            code in the filters leads me to believe).

            Before the document is finalized the new tables need to have
            their layout frms deleted and recalculated. This TableManager
            detects the necessity to do this, and all tables inserted into
            a document should be registered with this manager with
            InsertTable, and before finalization DelAndMakeTableFrames should
            be called.

            @see #i25782# for examples
        */
        class InsertedTablesManager
        {
        public:
            typedef std::map<InsertedTableClient *, SwNodeIndex *> TableMap;
            void DelAndMakeTableFrames();
            void InsertTable(SwTableNode &rTableNode, SwPaM &rPaM);
            explicit InsertedTablesManager(const SwDoc &rDoc);
        private:
            bool const mbHasRoot;
            TableMap maTables;
        };

        class RedlineStack
        {
        private:
            std::vector<std::unique_ptr<SwFltStackEntry>> maStack;
            SwDoc &mrDoc;

            RedlineStack(RedlineStack const&) = delete;
            RedlineStack& operator=(RedlineStack const&) = delete;

        public:
            explicit RedlineStack(SwDoc &rDoc) : mrDoc(rDoc) {}
            void MoveAttrs(const SwPosition& rPos);
            void open(const SwPosition& rPos, const SfxPoolItem& rAttr);
            bool close(const SwPosition& rPos, RedlineType_t eType);
            void close(const SwPosition& rPos, RedlineType_t eType,
                WW8TabDesc* pTabDesc );
            void closeall(const SwPosition& rPos);
            ~RedlineStack();
        };

        class SetInDocAndDelete
        {
        private:
            SwDoc &mrDoc;
        public:
            explicit SetInDocAndDelete(SwDoc &rDoc) : mrDoc(rDoc) {}
            void operator()(std::unique_ptr<SwFltStackEntry> & pEntry);
        private:
            SetInDocAndDelete& operator=(const SetInDocAndDelete&) = delete;
        };

        class SetEndIfOpen       //Subclass from something ?
        {
        private:
            const SwPosition &mrPos;
        public:
            explicit SetEndIfOpen(const SwPosition &rPos) : mrPos(rPos) {}
                void operator()(const std::unique_ptr<SwFltStackEntry> & pEntry) const
            {
                if (pEntry->bOpen)
                    pEntry->SetEndPos(mrPos);
            }
        private:
            SetEndIfOpen& operator=(const SetEndIfOpen&) = delete;
        };

        class CompareRedlines
        {
        public:
            bool operator()(const std::unique_ptr<SwFltStackEntry> & pOneE, const std::unique_ptr<SwFltStackEntry> & pTwoE)
                const;
        };

        class WrtRedlineAuthor
        {
        protected:
            std::vector<OUString> maAuthors; // Array of Sw - Bookmarknames

        private:
            WrtRedlineAuthor(WrtRedlineAuthor const&) = delete;
            WrtRedlineAuthor& operator=(WrtRedlineAuthor const&) = delete;

        public:
            WrtRedlineAuthor() = default;
            virtual ~WrtRedlineAuthor() {}

            sal_uInt16 AddName( const OUString& rNm );
            virtual void Write(Writer &rWrt) = 0;
        };

        struct CharRunEntry
        {
            sal_Int32 mnEndPos;
            sal_uInt16 mnScript;
            rtl_TextEncoding meCharSet;
            bool mbRTL;
            CharRunEntry(sal_Int32 nEndPos, sal_uInt16 nScript,
                rtl_TextEncoding eCharSet, bool bRTL)
            : mnEndPos(nEndPos), mnScript(nScript), meCharSet(eCharSet),
            mbRTL(bRTL)
            {
            }
        };

        typedef std::vector<CharRunEntry> CharRuns;

        /** Collect the ranges of Text which share

            Word generally requires characters which share the same direction,
            the same script, and occasionally (depending on the format) the
            same charset to be exported in independent chunks.

            So this function finds these ranges and returns a STL container
            of CharRuns

            @param rTextNd
                The TextNode we want to ranges from

            @return STL container of CharRuns which describe the shared
            direction, script and optionally script of the contiguous sequences
            of characters

            @see #i22537# for example
        */
        CharRuns GetPseudoCharRuns(const SwTextNode& rTextNd);
    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
