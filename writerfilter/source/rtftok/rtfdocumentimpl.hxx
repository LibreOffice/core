/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_WRITERFILTER_SOURCE_RTFTOK_RTFDOCUMENTIMPL_HXX
#define INCLUDED_WRITERFILTER_SOURCE_RTFTOK_RTFDOCUMENTIMPL_HXX

#include <stack>
#include <queue>
#include <boost/optional.hpp>
#include <boost/tuple/tuple.hpp>

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

namespace writerfilter
{
namespace rtftok
{
enum RTFBorderState
{
    BORDER_NONE,
    BORDER_PARAGRAPH,
    BORDER_PARAGRAPH_BOX,
    BORDER_CELL,
    BORDER_PAGE,
    BORDER_CHARACTER
};

/// Minimalistic buffer of elements for nested cells.
enum RTFBufferTypes
{
    BUFFER_PROPS,
    BUFFER_NESTROW,
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

struct TableRowBuffer;

/// A buffer storing dmapper calls.
typedef ::boost::tuple<RTFBufferTypes, RTFValue::Pointer_t,
        ::boost::shared_ptr<TableRowBuffer> > Buf_t;
typedef std::deque< Buf_t > RTFBuffer_t;

/// holds one nested table row
struct TableRowBuffer
{
    RTFBuffer_t buffer;
    ::std::deque<RTFSprms> cellsSprms;
    ::std::deque<RTFSprms> cellsAttributes;
    int nCells;
    writerfilter::Reference<Properties>::Pointer_t pParaProperties;
    writerfilter::Reference<Properties>::Pointer_t pFrameProperties;
    writerfilter::Reference<Properties>::Pointer_t pRowProperties;

    TableRowBuffer(RTFBuffer_t const& rBuffer,
                   ::std::deque<RTFSprms> const& rSprms,
                   ::std::deque<RTFSprms> const& rAttributes,
                   int const i_nCells)
        : buffer(rBuffer)
        , cellsSprms(rSprms), cellsAttributes(rAttributes)
        , nCells(i_nCells)
    {}
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

/// Stores the properties of a shape.
class RTFShape
{
public:
    RTFShape();
    std::vector< std::pair<OUString, OUString> > aProperties; ///< Properties of a single shape.
    std::vector< std::pair<OUString, OUString> > aGroupProperties; ///< Properties applied on the groupshape.
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
    css::uno::Reference<css::drawing::XShape> xShape;
    css::uno::Reference<css::beans::XPropertySet> xPropertySet;
    std::vector<css::beans::PropertyValue> aPendingProperties;
    sal_uInt8 nLineColorR, nLineColorG, nLineColorB;
    bool bHasLineColor;
    sal_uInt8 nFillColorR, nFillColorG, nFillColorB;
    bool bHasFillColor;
    sal_Int32 nDhgt;
    sal_Int32 nFLine;
    sal_Int32 nPolyLineCount;
    css::uno::Sequence<css::awt::Point> aPolyLinePoints;
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

    /// CJK or CTL?
    enum { LOCH, HICH, DBCH } eRunType;
    /// ltrch or rtlch
    bool isRightToLeft;

    // Info group.
    int nYear;
    int nMonth;
    int nDay;
    int nHour;
    int nMinute;

    /// Text from special destinations.
    OUStringBuffer aDestinationText;
    /// point to the buffer of the current destination
    OUStringBuffer * pDestinationText;

    /// Index of the current style.
    int nCurrentStyleIndex;

    /// Points to the active buffer, if there is one.
    RTFBuffer_t* pCurrentBuffer;

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

/// An RTF stack is similar to std::stack, except that it has an operator[].
struct RTFStack : public std::deque<RTFParserState>
{
    RTFParserState& top()
    {
        return back();
    }
    void pop()
    {
        return pop_back();
    }
    void push(RTFParserState const& rState)
    {
        return push_back(rState);
    }
};

class RTFTokenizer;
class RTFSdrImport;

/// Implementation of the RTFDocument interface.
class RTFDocumentImpl
    : public RTFDocument, public RTFListener
{
public:
    typedef ::boost::shared_ptr<RTFDocumentImpl> Pointer_t;
    RTFDocumentImpl(css::uno::Reference<css::uno::XComponentContext> const& xContext,
                    css::uno::Reference<css::io::XInputStream> const& xInputStream,
                    css::uno::Reference<css::lang::XComponent> const& xDstDoc,
                    css::uno::Reference<css::frame::XFrame> const& xFrame,
                    css::uno::Reference<css::task::XStatusIndicator> const& xStatusIndicator);
    virtual ~RTFDocumentImpl();

    // RTFDocument
    virtual void resolve(Stream& rHandler) SAL_OVERRIDE;
    virtual std::string getType() const SAL_OVERRIDE;

    // RTFListener
    virtual int dispatchDestination(RTFKeyword nKeyword) SAL_OVERRIDE;
    virtual int dispatchFlag(RTFKeyword nKeyword) SAL_OVERRIDE;
    virtual int dispatchSymbol(RTFKeyword nKeyword) SAL_OVERRIDE;
    virtual int dispatchToggle(RTFKeyword nKeyword, bool bParam, int nParam) SAL_OVERRIDE;
    virtual int dispatchValue(RTFKeyword nKeyword, int nParam) SAL_OVERRIDE;
    virtual int resolveChars(char ch) SAL_OVERRIDE;
    virtual int pushState() SAL_OVERRIDE;
    virtual int popState() SAL_OVERRIDE;
    virtual RTFDestinationState getDestinationState() SAL_OVERRIDE;
    virtual void setDestinationState(RTFDestinationState nDestinationState) SAL_OVERRIDE;
    virtual RTFInternalState getInternalState() SAL_OVERRIDE;
    virtual void setInternalState(RTFInternalState nInternalState) SAL_OVERRIDE;
    virtual bool getSkipUnknown() SAL_OVERRIDE;
    virtual void setSkipUnknown(bool bSkipUnknown) SAL_OVERRIDE;
    virtual void finishSubstream() SAL_OVERRIDE;
    virtual bool isSubstream() const SAL_OVERRIDE;

    Stream& Mapper();
    void setSubstream(bool bIsSubtream);
    void setSuperstream(RTFDocumentImpl* pSuperstream);
    void setStreamType(Id nId);
    void setAuthor(OUString& rAuthor);
    void setAuthorInitials(OUString& rAuthorInitials);
    void setIgnoreFirst(OUString& rIgnoreFirst);
    void seek(sal_Size nPos);
    css::uno::Reference<css::lang::XMultiServiceFactory> getModelFactory();
    bool isInBackground();
    void setDestinationText(OUString& rString);
    /// Resolve a picture: If not inline, then anchored.
    int resolvePict(bool bInline,
            css::uno::Reference<css::drawing::XShape> const& xShape);

    /// If this is the first run of the document, starts the initial paragraph.
    void checkFirstRun();
    /// If the initial paragraph is started.
    bool getFirstRun();
    /// If we need to add a dummy paragraph before a section break.
    void setNeedPar(bool bNeedPar);
    /// Return the dmapper index of an RTF index for fonts.
    int getFontIndex(int nIndex);
    /// Return the name of the font, based on a dmapper index.
    OUString getFontName(int nIndex);
    /// Return the style name of an RTF style index.
    OUString getStyleName(int nIndex);
    /// Return the encoding associated with a font index.
    rtl_TextEncoding getEncoding(int nFontIndex);
    /// Get the default parser state.
    RTFParserState& getDefaultState();
    oox::GraphicHelper& getGraphicHelper();

private:
    SvStream& Strm();
    sal_uInt32 getColorTable(sal_uInt32 nIndex);
    writerfilter::Reference<Properties>::Pointer_t createStyleProperties();
    void resetSprms();
    void resetAttributes();
    void resolveSubstream(sal_Size nPos, Id nId);
    void resolveSubstream(sal_Size nPos, Id nId, OUString& rIgnoreFirst);

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
    void prepareProperties(
        RTFParserState& rState,
        writerfilter::Reference<Properties>::Pointer_t&,
        writerfilter::Reference<Properties>::Pointer_t&,
        writerfilter::Reference<Properties>::Pointer_t&,
        int const nCells, int const nCurrentCellX);
    /// Send the passed properties to dmapper.
    void sendProperties(
        writerfilter::Reference<Properties>::Pointer_t const&,
        writerfilter::Reference<Properties>::Pointer_t const&,
        writerfilter::Reference<Properties>::Pointer_t const&);
    void replayRowBuffer(RTFBuffer_t& rBuffer,
                         ::std::deque<RTFSprms>& rCellsSrpms,
                         ::std::deque<RTFSprms>& rCellsAttributes,
                         int const nCells);
    void replayBuffer(RTFBuffer_t& rBuffer,
                      RTFSprms*      const pSprms,
                      RTFSprms const* const pAttributes);
    /// If we have some unicode or hex characters to send.
    void checkUnicode(bool bUnicode, bool bHex);
    /// If we need a final section break at the end of the document.
    void setNeedSect(bool bNeedSect = true);
    void resetTableRowProperties();
    void backupTableRowProperties();
    void restoreTableRowProperties();

    css::uno::Reference<css::uno::XComponentContext> const& m_xContext;
    css::uno::Reference<css::io::XInputStream> const& m_xInputStream;
    css::uno::Reference<css::lang::XComponent> const& m_xDstDoc;
    css::uno::Reference<css::frame::XFrame> const& m_xFrame;
    css::uno::Reference<css::task::XStatusIndicator> const& m_xStatusIndicator;
    css::uno::Reference<css::lang::XMultiServiceFactory> m_xModelFactory;
    css::uno::Reference<css::document::XDocumentProperties> m_xDocumentProperties;
    boost::shared_ptr<SvStream> m_pInStream;
    Stream* m_pMapperStream;
    boost::shared_ptr<RTFSdrImport> m_pSdrImport;
    boost::shared_ptr<RTFTokenizer> m_pTokenizer;
    RTFStack m_aStates;
    /// Read by RTF_PARD.
    RTFParserState m_aDefaultState;
    bool m_bSkipUnknown;
    /// Font index <-> encoding map, *not* part of the parser state
    std::map<int, rtl_TextEncoding> m_aFontEncodings;
    /// Font index <-> name map.
    std::map<int, OUString> m_aFontNames;
    /// Maps the non-continuous font indexes to the continuous dmapper indexes.
    std::vector<int> m_aFontIndexes;
    /// Maps style indexes to style names.
    std::map<int, OUString> m_aStyleNames;
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

    /// cell props buffer for nested tables, reset by \nestrow
    /// the \nesttableprops is a destination and must follow the
    /// nested cells, so it should be sufficient to store the
    /// currently active one, no need for a stack of them
    int m_nNestedCells;
    std::deque<RTFSprms> m_aNestedTableCellsSprms;
    std::deque<RTFSprms> m_aNestedTableCellsAttributes;
    /// cell props buffer for top-level table, reset by \row
    int m_nTopLevelCells;
    std::deque<RTFSprms> m_aTopLevelTableCellsSprms;
    std::deque<RTFSprms> m_aTopLevelTableCellsAttributes;
    /// backup of top-level props, to support inheriting cell props
    int m_nInheritingCells;
    std::deque<RTFSprms> m_aTableInheritingCellsSprms;
    std::deque<RTFSprms> m_aTableInheritingCellsAttributes;

    /// Current cellx value (nested table)
    int m_nNestedCurrentCellX;
    /// Current cellx value (top-level table)
    int m_nTopLevelCurrentCellX;

    // Backup of what \trowd clears, to work around invalid input.
    RTFSprms m_aBackupTableRowSprms;
    RTFSprms m_aBackupTableRowAttributes;
    int m_nBackupTopLevelCurrentCellX;

    /// Buffered table cells, till cell definitions are not reached.
    /// for nested table, one buffer per table level
    std::deque< RTFBuffer_t > m_aTableBufferStack;
    /// Buffered superscript, till footnote is reached (or not).
    RTFBuffer_t m_aSuperBuffer;

    bool m_bHasFootnote;
    /// Superstream of this substream.
    RTFDocumentImpl* m_pSuperstream;
    /// Type of the stream: header, footer, footnote, etc.
    Id m_nStreamType;
    std::queue< std::pair<Id, sal_Size> > m_nHeaderFooterPositions;
    sal_Size m_nGroupStartPos;
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
    /// Used only during font table parsing till we don't know the font name.
    int m_nCurrentEncoding;
    /// Raw default font index, use getFont() on it to get a real one.
    int m_nDefaultFontIndex;

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
    /// If the next continuous section break should be ignored.
    bool m_bIgnoreNextContSectBreak;
    /// clean up a synthetic page break, see RTF_PAGE
    /// if inactive value is -1, otherwise the RTF_SKB* to restore
    RTFKeyword m_nResetBreakOnSectBreak;
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

#endif // INCLUDED_WRITERFILTER_SOURCE_RTFTOK_RTFDOCUMENTIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
