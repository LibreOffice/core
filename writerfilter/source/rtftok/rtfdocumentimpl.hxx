/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Miklos Vajna <vmiklos@frugalware.org>
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef _RTFDOCUMENTIMPL_HXX_
#define _RTFDOCUMENTIMPL_HXX_

#include <stack>
#include <queue>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <oox/helper/graphichelper.hxx>
#include <oox/mathml/importutils.hxx>
#include <rtl/strbuf.hxx>

#include <rtftok/RTFDocument.hxx>
#include <rtfreferencetable.hxx>
#include <rtfsprm.hxx>

class SvStream;

namespace writerfilter {
    namespace rtftok {
        enum RTFInternalState
        {
            INTERNAL_NORMAL,
            INTERNAL_BIN,
            INTERNAL_HEX
        };

        // Note that this is not a 1:1 mapping between destination control
        // words, e.g. RTF_PICT gets mapped to DESTINATION_PICT or
        // DESTINATION_SHAPEPROPERTYVALUEPICT, etc.
        enum RTFDesitnationState
        {
            DESTINATION_NORMAL,
            DESTINATION_SKIP,
            DESTINATION_FONTTABLE,
            DESTINATION_FONTENTRY,
            DESTINATION_COLORTABLE,
            DESTINATION_STYLESHEET,
            DESTINATION_STYLEENTRY,
            DESTINATION_FIELD,
            DESTINATION_FIELDINSTRUCTION,
            DESTINATION_FIELDRESULT,
            DESTINATION_LISTTABLE,
            DESTINATION_LISTENTRY,
            DESTINATION_LISTOVERRIDETABLE,
            DESTINATION_LISTOVERRIDEENTRY,
            DESTINATION_LISTLEVEL,
            DESTINATION_LEVELTEXT,
            DESTINATION_LEVELNUMBERS,
            DESTINATION_SHPPICT,
            DESTINATION_PICT,
            DESTINATION_PICPROP,
            DESTINATION_SHAPEPROPERTY,
            DESTINATION_SHAPEPROPERTYNAME,
            DESTINATION_SHAPEPROPERTYVALUE,
            DESTINATION_SHAPE,
            DESTINATION_SHAPEINSTRUCTION,
            DESTINATION_SHAPEPROPERTYVALUEPICT,
            DESTINATION_NESTEDTABLEPROPERTIES,
            DESTINATION_FOOTNOTE,
            DESTINATION_BOOKMARKSTART,
            DESTINATION_BOOKMARKEND,
            DESTINATION_REVISIONTABLE,
            DESTINATION_REVISIONENTRY,
            DESTINATION_SHAPETEXT,
            DESTINATION_FORMFIELD,
            DESTINATION_FORMFIELDNAME,
            DESTINATION_FORMFIELDLIST,
            DESTINATION_DATAFIELD,
            DESTINATION_INFO,
            DESTINATION_CREATIONTIME,
            DESTINATION_REVISIONTIME,
            DESTINATION_PRINTTIME,
            DESTINATION_AUTHOR,
            DESTINATION_KEYWORDS,
            DESTINATION_OPERATOR,
            DESTINATION_COMPANY,
            DESTINATION_COMMENT,
            DESTINATION_OBJECT,
            DESTINATION_OBJDATA,
            DESTINATION_RESULT,
            DESTINATION_ANNOTATIONDATE,
            DESTINATION_ANNOTATIONAUTHOR,
            DESTINATION_FALT,
            DESTINATION_FLYMAINCONTENT,
            DESTINATION_DRAWINGOBJECT,
            DESTINATION_PARAGRAPHNUMBERING,
            DESTINATION_PARAGRAPHNUMBERING_TEXTBEFORE,
            DESTINATION_PARAGRAPHNUMBERING_TEXTAFTER,
            DESTINATION_TITLE,
            DESTINATION_SUBJECT,
            DESTINATION_DOCCOMM,
            DESTINATION_ATNID,
            DESTINATION_MOMATH,
            DESTINATION_MR,
            DESTINATION_MF,
            DESTINATION_MFPR,
            DESTINATION_MCTRLPR,
            DESTINATION_MNUM,
            DESTINATION_MDEN,
            DESTINATION_MACC,
            DESTINATION_MACCPR,
            DESTINATION_MCHR,
            DESTINATION_MPOS,
            DESTINATION_MVERTJC,
            DESTINATION_MSTRIKEH,
            DESTINATION_MDEGHIDE,
            DESTINATION_ME,
            DESTINATION_MBAR,
            DESTINATION_MBARPR,
            DESTINATION_MD,
            DESTINATION_MDPR,
            DESTINATION_MBEGCHR,
            DESTINATION_MSEPCHR,
            DESTINATION_MENDCHR,
            DESTINATION_MFUNC,
            DESTINATION_MFUNCPR,
            DESTINATION_MFNAME,
            DESTINATION_MLIMLOW,
            DESTINATION_MLIMLOWPR,
            DESTINATION_MLIM,
            DESTINATION_MM,
            DESTINATION_MMPR,
            DESTINATION_MMR,
            DESTINATION_MNARY,
            DESTINATION_MNARYPR,
            DESTINATION_MSUB,
            DESTINATION_MSUP,
            DESTINATION_MSUBHIDE,
            DESTINATION_MSUPHIDE,
            DESTINATION_MLIMUPP,
            DESTINATION_MLIMUPPPR,
            DESTINATION_MGROUPCHR,
            DESTINATION_MGROUPCHRPR,
            DESTINATION_MBORDERBOX,
            DESTINATION_MBORDERBOXPR,
            DESTINATION_MRAD,
            DESTINATION_MRADPR,
            DESTINATION_MDEG,
            DESTINATION_MSSUB,
            DESTINATION_MSSUBPR,
            DESTINATION_MSSUP,
            DESTINATION_MSSUPPR,
            DESTINATION_MSSUBSUP,
            DESTINATION_MSSUBSUPPR,
            DESTINATION_MSPRE,
            DESTINATION_MSPREPR,
            DESTINATION_MTYPE,
            DESTINATION_MGROW,
            DESTINATION_MBOX,
            DESTINATION_MEQARR,
        };

        enum RTFBorderState
        {
            BORDER_NONE,
            BORDER_PARAGRAPH,
            BORDER_PARAGRAPH_BOX,
            BORDER_CELL,
            BORDER_PAGE
        };

        enum RTFErrors
        {
            ERROR_OK,
            ERROR_GROUP_UNDER,
            ERROR_GROUP_OVER,
            ERROR_EOF,
            ERROR_HEX_INVALID,
            ERROR_CHAR_OVER
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
            BUFFER_PAR
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
            BMPSTYLE_PNG
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
                std::vector< std::pair<rtl::OUString, rtl::OUString> > aProperties;
                int nLeft;
                int nTop;
                int nRight;
                int nBottom;
                sal_Int16 nHoriOrientRelation;
                sal_Int16 nVertOrientRelation;
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
                RTFDesitnationState nDestinationState;
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
                rtl::OUStringBuffer aDestinationText;
        };

        class RTFTokenizer;
        class RTFSdrImport;

        /// Implementation of the RTFDocument interface.
        class RTFDocumentImpl
            : public RTFDocument
        {
            public:
                typedef ::boost::shared_ptr<RTFDocumentImpl> Pointer_t;
                RTFDocumentImpl(uno::Reference<uno::XComponentContext> const& xContext,
                                uno::Reference<io::XInputStream> const& xInputStream,
                                uno::Reference<lang::XComponent> const& xDstDoc,
                                uno::Reference<frame::XFrame> const& xFrame,
                                uno::Reference<task::XStatusIndicator> const& xStatusIndicator);
                virtual ~RTFDocumentImpl();
                virtual void resolve(Stream & rHandler);
                virtual std::string getType() const;

                Stream& Mapper();
                void setSubstream(bool bIsSubtream);
                void setSuperstream(RTFDocumentImpl *pSuperstream);
                void setAuthor(rtl::OUString& rAuthor);
                void setAuthorInitials(rtl::OUString& rAuthorInitials);
                bool isSubstream() const;
                void finishSubstream();
                void setIgnoreFirst(rtl::OUString& rIgnoreFirst);
                void seek(sal_uInt32 nPos);
                uno::Reference<lang::XMultiServiceFactory> getModelFactory();
                RTFParserState& getState();
                void setDestinationText(rtl::OUString& rString);
                /// Resolve a picture: If not inline, then anchored.
                int resolvePict(bool bInline);
                void replayShapetext();
                bool getSkipUnknown();
                void setSkipUnknown(bool bSkipUnknown);

                // These callbacks are invoked by the tokenizer.
                int resolveChars(char ch);
                int pushState();
                int popState();
                int dispatchFlag(RTFKeyword nKeyword);
                int dispatchDestination(RTFKeyword nKeyword);
                int dispatchSymbol(RTFKeyword nKeyword);
                int dispatchToggle(RTFKeyword nKeyword, bool bParam, int nParam);
                int dispatchValue(RTFKeyword nKeyword, int nParam);

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

            private:
                SvStream& Strm();
                sal_uInt32 getColorTable(sal_uInt32 nIndex);
                RTFSprms mergeSprms();
                RTFSprms mergeAttributes();
                void resetSprms();
                void resetAttributes();
                void resolveSubstream(sal_uInt32 nPos, Id nId);
                void resolveSubstream(sal_uInt32 nPos, Id nId, rtl::OUString& rIgnoreFirst);

                void text(rtl::OUString& rString);
                // Sends a single character to dmapper, taking care of buffering.
                void singleChar(sal_uInt8 nValue, bool bRunProps = false);
                // Sends run properties to dmapper, taking care of buffering.
                void runProps();
                void runBreak();
                void parBreak();
                void tableBreak();
                void checkNeedPap();
                void sectBreak(bool bFinal);
                void replayBuffer(RTFBuffer_t& rBuffer);
                /// If we have some unicode or hex characters to send.
                void checkUnicode(bool bUnicode = true, bool bHex = true);
                /// If we have a pending continous section break.
                void checkDeferredContSectBreak();

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
                /// Buffered shape text.
                RTFBuffer_t m_aShapetextBuffer;
                /// Points to the active buffer, if there is one.
                RTFBuffer_t* m_pCurrentBuffer;

                bool m_bHasFootnote;
                /// Superstream of this substream.
                RTFDocumentImpl *m_pSuperstream;
                std::queue< std::pair<Id, sal_uInt32> > m_nHeaderFooterPositions;
                sal_uInt32 m_nGroupStartPos;
                /// Ignore the first occurrence of this text.
                rtl::OUString m_aIgnoreFirst;
                /// Bookmark name <-> index map.
                std::map<rtl::OUString, int> m_aBookmarks;
                /// Revision index <-> author map.
                std::map<int, rtl::OUString> m_aAuthors;
                /// Annotation author of the next annotation.
                rtl::OUString m_aAuthor;
                /// Initials of author of the next annotation.
                rtl::OUString m_aAuthorInitials;

                RTFSprms m_aFormfieldSprms;
                RTFSprms m_aFormfieldAttributes;
                RTFFormFieldTypes m_nFormFieldType;

                RTFSprms m_aObjectSprms;
                RTFSprms m_aObjectAttributes;
                /// If we are in an object group.
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
                rtl::OUStringBuffer m_aUnicodeBuffer;
                /// Same for hex characters.
                rtl::OStringBuffer m_aHexBuffer;
                /// Formula import.
                oox::formulaimport::XmlStreamBuilder m_aMathBuffer;
                bool m_bDeferredContSectBreak;
        };
    } // namespace rtftok
} // namespace writerfilter

#endif // _RTFDOCUMENTIMPL_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
