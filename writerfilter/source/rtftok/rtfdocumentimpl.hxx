#ifndef _RTFDOCUMENTIMPL_HXX_
#define _RTFDOCUMENTIMPL_HXX_

#include <memory>
#include <stack>
#include <map>
#include <rtl/strbuf.hxx>
#include <rtftok/RTFDocument.hxx>
#include <rtfcontrolwords.hxx>
#include <rtfreferencetable.hxx>
#include <rtfvalue.hxx>

class SvStream;

namespace writerfilter {
    namespace rtftok {
        enum RTFInternalState
        {
            INTERNAL_NORMAL,
            INTERNAL_BIN,
            INTERNAL_HEX
        };

        enum RTFDesitnationState
        {
            DESTINATION_NORMAL,
            DESTINATION_SKIP,
            DESTINATION_FONTTABLE,
            DESTINATION_FONTENTRY,
            DESTINATION_COLORTABLE,
            DESTINATION_STYLESHEET,
            DESTINATION_STYLEENTRY,
            DESTINATION_FIELDINSTRUCTION,
            DESTINATION_FIELDRESULT
        };

        enum RTFErrors
        {
            ERROR_OK,
            ERROR_GROUP_UNDER,
            ERROR_GROUP_OVER,
            ERROR_EOF,
            ERROR_HEX_INVALID
        };

        enum RTFControlTypes
        {
            CONTROL_FLAG, // eg \sbknone takes no parameter
            CONTROL_DESTINATION, // eg \fonttbl, if ignored, the whole group should be skipped
            CONTROL_SYMBOL, // eg \tab
            CONTROL_TOGGLE, // eg \b (between on and off)
            CONTROL_VALUE // eg \fs (requires parameter)
        };

        /// An entry in the color table.
        class RTFColorTableEntry
        {
            public:
                RTFColorTableEntry();
                sal_uInt8 nRed;
                sal_uInt8 nGreen;
                sal_uInt8 nBlue;
        };

        /// State of the parser, which gets saved / restored when changing groups.
        class RTFParserState
        {
            public:
                RTFParserState();
                RTFInternalState nInternalState;
                RTFDesitnationState nDestinationState;
                std::map<Id, RTFValue::Pointer_t> aSprms;
                // HACK: dmapper requires to store some of the properties here, should not be necessary
                std::map<Id, RTFValue::Pointer_t> aAttributes;

                RTFReferenceTable::Entries_t aFontTableEntries;
                int nCurrentFontIndex;

                RTFColorTableEntry aCurrentColor;

                RTFReferenceTable::Entries_t aStyleTableEntries;
                int nCurrentStyleIndex;

                rtl_TextEncoding nCurrentEncoding;

                rtl::OStringBuffer aFieldInstruction;
        };

        /// Implementation of the RTFDocument interface.
        class RTFDocumentImpl
            : public RTFDocument
        {
            public:
                RTFDocumentImpl(com::sun::star::uno::Reference<com::sun::star::io::XInputStream> const& xInputStream);
                virtual ~RTFDocumentImpl();
                virtual void resolve(Stream & rHandler);
                virtual std::string getType() const;

                SvStream& Strm();
                Stream& Mapper();
                sal_uInt32 getColorTable(sal_uInt32 nIndex);
                sal_uInt32 getEncodingTable(sal_uInt32 nFontIndex);
                Id getBorderTable(sal_uInt32 nIndex);
                void skipDestination(bool bParsed);
            private:
                int resolveParse();
                int resolveKeyword();

                int dispatchKeyword(rtl::OString& rKeyword, bool bParam, int nParam);
                int dispatchFlag(RTFKeyword nKeyword);
                int dispatchDestination(RTFKeyword nKeyword);
                int dispatchSymbol(RTFKeyword nKeyword);
                int dispatchToggle(RTFKeyword nKeyword, bool bParam, int nParam);
                int dispatchValue(RTFKeyword nKeyword, int nParam);

                int resolveChars(char ch);
                int pushState();
                int popState();
                void text(rtl::OUString& rString);

                SvStream* m_pInStream;
                Stream* m_pMapperStream;
                /// Same as m_aStates.size(), except that this can be negative for invalid input.
                int m_nGroup;
                std::stack<RTFParserState> m_aStates;
                /// Read by RTF_PARD.
                RTFParserState m_aDefaultState;
                bool m_bSkipUnknown;
                /// For debugging purposes, where int value would not be enough
                rtl::OString* m_pCurrentKeyword;
                /// Font index <-> encoding map, *not* part of the parser state
                std::map<int, rtl_TextEncoding> m_aFontEncodings;
                /// Color index <-> RGB color value map
                std::vector<sal_uInt32> m_aColorTable;
                bool m_bFirstRun;
                bool m_bNeedPap;
        };
    } // namespace rtftok
} // namespace writerfilter

#endif // _RTFDOCUMENTIMPL_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
