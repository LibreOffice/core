/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _RTFDOCUMENTIMPL_HXX_
#define _RTFDOCUMENTIMPL_HXX_

#include <stack>
#include <queue>
#include <boost/optional.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <oox/helper/graphichelper.hxx>
#include <oox/mathml/importutils.hxx>
#include <rtl/strbuf.hxx>

#include <rtftok/RTFDocument.hxx>
#include <rtfreferencetable.hxx>
#include <rtfsprm.hxx>
#include <rtflistener.hxx>

class SvStream;

namespace writerfilter {
    namespace rtftok {
        enum RTFBorderState
        {
            BORDER_NONE,
            BORDER_PARAGRAPH,
            BORDER_PARAGRAPH_BOX,
            BORDER_CELL,
            BORDER_PAGE
        };

        /// Minimalistic buffer of elements for nested cells.
        enum RTFBufferTypes
        {
            BUFFER_PROPS,
            BUFFER_CELLEND,
            BUFFER_STARTRUN,
            BUFFER_TEXT,
            BUFFER_UTEXT,
            BUFFER_ENDRUN,
            BUFFER_PAR,
            BUFFER_STARTSHAPE,
            BUFFER_ENDSHAPE
        };

        /// Form field types
        enum RTFFormFieldTypes
        {
            FORMFIELD_NONE,
            FORMFIELD_TEXT,
            FORMFIELD_CHECKBOX,
            FORMFIELD_LIST
        };

        enum RTFBmpStyles
        {
            BMPSTYLE_NONE,
            BMPSTYLE_PNG,
            BMPSTYLE_JPEG
        };

        enum RTFFieldStatus
        {
            FIELD_NONE,
            FIELD_INSTRUCTION,
            FIELD_RESULT
        };

        /// A buffer storing dmapper calls.
        typedef std::deque< std::pair<RTFBufferTypes, RTFValue::Pointer_t> > RTFBuffer_t;

        /// An entry in the color table.
        class RTFColorTableEntry
        {
            public:
                RTFColorTableEntry();
                sal_uInt8 nRed;
                sal_uInt8 nGreen;
                sal_uInt8 nBlue;
        };

        /// Stores the properties of a shape.
        class RTFShape
        {
            public:
                RTFShape();
                std::vector< std::pair<OUString, OUString> > aProperties;
                sal_Int32 nLeft;
                sal_Int32 nTop;
                sal_Int32 nRight;
                sal_Int32 nBottom;
                boost::optional<sal_Int32> oZ; ///< Z-Order of the shape.
                sal_Int16 nHoriOrientRelation; ///< Horizontal text::RelOrientation for drawinglayer shapes.
                sal_Int16 nVertOrientRelation; ///< Vertical text::RelOrientation for drawinglayer shapes.
                sal_uInt32 nHoriOrientRelationToken; ///< Horizontal dmapper token for Writer pictures.
                sal_uInt32 nVertOrientRelationToken; ///< Vertical dmapper token for Writer pictures.
                int nWrap;
        };

        /// Stores the properties of a drawing object.
        class RTFDrawingObject : public RTFShape
        {
            public:
                RTFDrawingObject();
                uno::Reference<drawing::XShape> xShape;
                uno::Reference<beans::XPropertySet> xPropertySet;
                std::vector<beans::PropertyValue> aPendingProperties;
                sal_uInt8 nLineColorR, nLineColorG, nLineColorB;
                bool bHasLineColor;
                sal_uInt8 nFillColorR, nFillColorG, nFillColorB;
                bool bHasFillColor;
                sal_Int32 nDhgt;
                sal_Int32 nFLine;
                sal_Int32 nPolyLineCount;
                uno::Sequence<awt::Point> aPolyLinePoints;
                bool bHadShapeText;
        };

        /// Stores the properties of a picture.
        class RTFPicture
        {
            public:
                RTFPicture();
                sal_uInt16 nWidth, nHeight;
                sal_uInt16 nGoalWidth, nGoalHeight;
                sal_uInt16 nScaleX, nScaleY;
                short nCropT, nCropB, nCropL, nCropR;
                sal_uInt16 eWMetafile;
                RTFBmpStyles nStyle;
        };

        class RTFParserState;

        /// Stores the properties of a frame
        class RTFFrame
        {
            private:
                RTFParserState* m_pParserState;
                sal_Int32 nX, nY, nW, nH;
                sal_Int32 nHoriPadding, nVertPadding;
                sal_Int32 nHoriAlign, nHoriAnchor, nVertAlign, nVertAnchor;
                Id nHRule;
                boost::optional<Id> oWrap;
            public:
                RTFFrame(RTFParserState* pParserState);
                sal_Int16 nAnchorType;

                /// Convert the stored properties to Sprms
                RTFSprms getSprms();
                /// Store a property
                void setSprm(Id nId, Id nValue);
                bool hasProperties();
                /// If we got tokens indicating we're in a frame.
                bool inFrame();
        };

        class RTFDocumentImpl;

        /// State of the parser, which gets saved / restored when changing groups.
        class RTFParserState
        {
            public:
                RTFParserState(RTFDocumentImpl* pDocumentImpl);
                /// Resets aFrame.
                void resetFrame();

                RTFDocumentImpl* m_pDocumentImpl;
                RTFInternalState nInternalState;
                RTFDestinationState nDestinationState;
                RTFFieldStatus nFieldStatus;
                RTFBorderState nBorderState;
                // font table, stylesheet table
                RTFSprms aTableSprms;
                RTFSprms aTableAttributes;
                // reset by plain
                RTFSprms aCharacterSprms;
                RTFSprms aCharacterAttributes;
                // reset by pard
                RTFSprms aParagraphSprms;
                RTFSprms aParagraphAttributes;
                // reset by sectd
                RTFSprms aSectionSprms;
                RTFSprms aSectionAttributes;
                // reset by trowd
                RTFSprms aTableRowSprms;
                RTFSprms aTableRowAttributes;
                // reset by cellx
                RTFSprms aTableCellSprms;
                RTFSprms aTableCellAttributes;
                // reset by row/nestrow
                std::deque<RTFSprms> aTableCellsSprms;
                std::deque<RTFSprms> aTableCellsAttributes;
                // backup of the above two, to support inheriting cell props
                std::deque<RTFSprms> aTableInheritingCellsSprms;
                std::deque<RTFSprms> aTableInheritingCellsAttributes;
                // reset by tx
                RTFSprms aTabAttributes;

                RTFColorTableEntry aCurrentColor;

                rtl_TextEncoding nCurrentEncoding;

                /// Current \uc value.
                int nUc;
                /// Characters to skip, set to nUc by \u.
                int nCharsToSkip;
                /// Characters to read, once in binary mode.
                int nBinaryToRead;

                /// Next list level index to use when parsing list table.
                int nListLevelNum;
                /// List level entries, which will form a list entry later.
                RTFSprms aListLevelEntries;

                /// List of character positions in leveltext to replace.
                std::vector<sal_Int32> aLevelNumbers;

                RTFPicture aPicture;
                RTFShape aShape;
                RTFDrawingObject aDrawingObject;
                RTFFrame aFrame;

                /// Current cellx value.
                int nCellX;
                int nCells;
                int nInheritingCells;

                /// CJK or CTL?
                bool bIsCjk;

                // Info group.
                int nYear;
                int nMonth;
                int nDay;
                int nHour;
                int nMinute;

                /// Text from special destinations.
                OUStringBuffer aDestinationText;

                /// Same as the int value of NS_rtf::LN_ISTD in aParagraphAttributes, for performance reasons.
                int nCurrentStyleIndex;

                /// Points to the active buffer, if there is one.
                RTFBuffer_t* pCurrentBuffer;

                /// If a table style was requested to be used.
                bool bHasTableStyle;

                /// If we're inside a \listpicture group.
                bool bInListpicture;

                /// If we're inside a \background group.
                bool bInBackground;

                bool bHadShapeText;
                bool bInShapeGroup; ///< If we're inside a \shpgrp group.
                bool bInShape; ///< If we're inside a \shp group.
                bool bCreatedShapeGroup; ///< A GroupShape was created and pushed to the parent stack.
                bool bStartedTrackchange; ///< Track change is started, need to end it before popping.
        };

        class RTFTokenizer;
        class RTFSdrImport;

        /// Implementation of the RTFDocument interface.
        class RTFDocumentImpl
            : public RTFDocument, public RTFListener
        {
            public:
                typedef ::boost::shared_ptr<RTFDocumentImpl> Pointer_t;
                RTFDocumentImpl(uno::Reference<uno::XComponentContext> const& xContext,
                                uno::Reference<io::XInputStream> const& xInputStream,
                                uno::Reference<lang::XComponent> const& xDstDoc,
                                uno::Reference<frame::XFrame> const& xFrame,
                                uno::Reference<task::XStatusIndicator> const& xStatusIndicator);
                virtual ~RTFDocumentImpl();

                // RTFDocument
                virtual void resolve(Stream & rHandler);
                virtual std::string getType() const;

                // RTFListener
                virtual int dispatchDestination(RTFKeyword nKeyword);
                virtual int dispatchFlag(RTFKeyword nKeyword);
                virtual int dispatchSymbol(RTFKeyword nKeyword);
                virtual int dispatchToggle(RTFKeyword nKeyword, bool bParam, int nParam);
                virtual int dispatchValue(RTFKeyword nKeyword, int nParam);
                virtual int resolveChars(char ch);
                virtual int pushState();
                virtual int popState();
                virtual RTFDestinationState getDestinationState();
                virtual void setDestinationState(RTFDestinationState nDestinationState);
                virtual RTFInternalState getInternalState();
                virtual void setInternalState(RTFInternalState nInternalState);
                virtual bool getSkipUnknown();
                virtual void setSkipUnknown(bool bSkipUnknown);
                virtual void finishSubstream();
                virtual bool isSubstream() const;

                Stream& Mapper();
                void setSubstream(bool bIsSubtream);
                void setSuperstream(RTFDocumentImpl *pSuperstream);
                void setAuthor(OUString& rAuthor);
                void setAuthorInitials(OUString& rAuthorInitials);
                void setIgnoreFirst(OUString& rIgnoreFirst);
                void seek(sal_uInt32 nPos);
                uno::Reference<lang::XMultiServiceFactory> getModelFactory();
                bool isInBackground();
                void setDestinationText(OUString& rString);
                /// Resolve a picture: If not inline, then anchored.
                int resolvePict(bool bInline);

                /// If this is the first run of the document, starts the initial paragraph.
                void checkFirstRun();
                /// If the initial paragraph is started.
                bool getFirstRun();
                /// If we need to add a dummy paragraph before a section break.
                void setNeedPar(bool bNeedPar);
                /// Return the dmapper index of an RTF index for fonts.
                int getFontIndex(int nIndex);
                /// Return the encoding associated with a dmapper font index.
                rtl_TextEncoding getEncoding(sal_uInt32 nFontIndex);
                /// Get the default parser state.
                RTFParserState& getDefaultState();
                oox::GraphicHelper& getGraphicHelper();

            private:
                SvStream& Strm();
                sal_uInt32 getColorTable(sal_uInt32 nIndex);
                RTFSprms mergeSprms();
                RTFSprms mergeAttributes();
                void resetSprms();
                void resetAttributes();
                void resolveSubstream(sal_uInt32 nPos, Id nId);
                void resolveSubstream(sal_uInt32 nPos, Id nId, OUString& rIgnoreFirst);

                void text(OUString& rString);
                // Sends a single character to dmapper, taking care of buffering.
                void singleChar(sal_uInt8 nValue, bool bRunProps = false);
                // Sends run properties to dmapper, taking care of buffering.
                void runProps();
                void runBreak();
                void parBreak();
                void tableBreak();
                writerfilter::Reference<Properties>::Pointer_t getProperties(RTFSprms& rAttributes, RTFSprms& rSprms);
                void checkNeedPap();
                void sectBreak(bool bFinal);
                void replayBuffer(RTFBuffer_t& rBuffer);
                /// If we have some unicode or hex characters to send.
                void checkUnicode(bool bUnicode, bool bHex);
                /// If we need a final section break at the end of the document.
                void setNeedSect(bool bNeedSect = true);

                uno::Reference<uno::XComponentContext> const& m_xContext;
                uno::Reference<io::XInputStream> const& m_xInputStream;
                uno::Reference<lang::XComponent> const& m_xDstDoc;
                uno::Reference<frame::XFrame> const& m_xFrame;
                uno::Reference<task::XStatusIndicator> const& m_xStatusIndicator;
                uno::Reference<lang::XMultiServiceFactory> m_xModelFactory;
                uno::Reference<document::XDocumentProperties> m_xDocumentProperties;
                boost::shared_ptr<SvStream> m_pInStream;
                Stream* m_pMapperStream;
                boost::shared_ptr<RTFSdrImport> m_pSdrImport;
                boost::shared_ptr<RTFTokenizer> m_pTokenizer;
                std::stack<RTFParserState> m_aStates;
                /// Read by RTF_PARD.
                RTFParserState m_aDefaultState;
                bool m_bSkipUnknown;
                /// Font index <-> encoding map, *not* part of the parser state
                std::map<int, rtl_TextEncoding> m_aFontEncodings;
                /// Maps the non-continious font indexes to the continous dmapper indexes.
                std::vector<int> m_aFontIndexes;
                /// Color index <-> RGB color value map
                std::vector<sal_uInt32> m_aColorTable;
                bool m_bFirstRun;
                /// If paragraph properties should be emitted on next run.
                bool m_bNeedPap;
                /// If we need to emit a CR at the end of substream.
                bool m_bNeedCr;
                /// Original value of m_bNeedCr -- saved/restored before/after textframes.
                bool m_bNeedCrOrig;
                bool m_bNeedPar;
                /// If set, an empty paragraph will be added at the end of the document.
                bool m_bNeedFinalPar;
                /// The list table and list override table combined.
                RTFSprms m_aListTableSprms;
                /// The settings table attributes.
                RTFSprms m_aSettingsTableAttributes;
                /// The settings table sprms.
                RTFSprms m_aSettingsTableSprms;

                oox::StorageRef m_xStorage;
                boost::shared_ptr<oox::GraphicHelper> m_pGraphicHelper;

                /// Buffered table cells, till cell definitions are not reached.
                RTFBuffer_t m_aTableBuffer;
                /// Buffered superscript, till footnote is reached (or not).
                RTFBuffer_t m_aSuperBuffer;

                bool m_bHasFootnote;
                /// Superstream of this substream.
                RTFDocumentImpl *m_pSuperstream;
                std::queue< std::pair<Id, sal_uInt32> > m_nHeaderFooterPositions;
                sal_uInt32 m_nGroupStartPos;
                /// Ignore the first occurrence of this text.
                OUString m_aIgnoreFirst;
                /// Bookmark name <-> index map.
                std::map<OUString, int> m_aBookmarks;
                /// Revision index <-> author map.
                std::map<int, OUString> m_aAuthors;
                /// Annotation author of the next annotation.
                OUString m_aAuthor;
                /// Initials of author of the next annotation.
                OUString m_aAuthorInitials;

                RTFSprms m_aFormfieldSprms;
                RTFSprms m_aFormfieldAttributes;
                RTFFormFieldTypes m_nFormFieldType;

                RTFSprms m_aObjectSprms;
                RTFSprms m_aObjectAttributes;
                /** If we are in an object group and if the we use its
                 *  \objdata element.
                 *  (if we don't use the \objdata we use the \result element)*/
                bool m_bObject;
                /// Contents of the objdata group.
                boost::shared_ptr<SvStream> m_pObjectData;
                /// If the data for a picture is a binary one, it's stored here.
                boost::shared_ptr<SvStream> m_pBinaryData;

                RTFReferenceTable::Entries_t m_aFontTableEntries;
                int m_nCurrentFontIndex;

                RTFReferenceTable::Entries_t m_aStyleTableEntries;
                int m_nCurrentStyleIndex;
                bool m_bFormField;
                /// If a frame start token is already sent to dmapper (nesting them is not OK).
                bool m_bIsInFrame;
                // Unicode characters are collected here so we don't have to send them one by one.
                OUStringBuffer m_aUnicodeBuffer;
                /// Same for hex characters.
                OStringBuffer m_aHexBuffer;
                /// Formula import.
                oox::formulaimport::XmlStreamBuilder m_aMathBuffer;
                /// Normal text property, that is math italic and math spacing are not applied to the current run.
                bool m_bMathNor;
                /// If the next continous section break should be ignored.
                bool m_bIgnoreNextContSectBreak;
                /// If a section break is needed before the end of the doc (false right after a section break).
                bool m_bNeedSect;
                /// If aFrame.inFrame() was true in the previous state.
                bool m_bWasInFrame;
                /// A picture was seen in the current paragraph.
                bool m_bHadPicture;
                /// The document has multiple sections.
                bool m_bHadSect;
                /// Max width of the rows in the current table.
                int m_nCellxMax;
                /// ID of the next \listlevel picture.
                int m_nListPictureId;
        };
    } // namespace rtftok
} // namespace writerfilter

#endif // _RTFDOCUMENTIMPL_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
