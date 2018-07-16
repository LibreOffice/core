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

#include <queue>
#include <tuple>
#include <vector>
#include <boost/optional.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <oox/mathml/importutils.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/color.hxx>

#include <rtftok/RTFDocument.hxx>
#include "rtfreferencetable.hxx"
#include "rtfsprm.hxx"
#include "rtflistener.hxx"

class SvStream;
namespace oox
{
class GraphicHelper;
}

namespace writerfilter
{
namespace rtftok
{
class RTFParserState;
class RTFDocumentImpl;
class RTFTokenizer;
class RTFSdrImport;
struct TableRowBuffer;

enum class RTFBorderState
{
    NONE,
    PARAGRAPH,
    PARAGRAPH_BOX,
    CELL,
    PAGE,
    CHARACTER
};

/// Different kind of buffers for table cell contents.
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
    /// Imports a shape.
    BUFFER_RESOLVESHAPE,
    BUFFER_ENDSHAPE,
    BUFFER_RESOLVESUBSTREAM,
    /// Restores RTFParserState::aPicture.
    BUFFER_PICTURE
};

/// Form field types
enum class RTFFormFieldType
{
    NONE,
    TEXT,
    CHECKBOX,
    LIST
};

enum class RTFBmpStyle
{
    NONE,
    PNG,
    JPEG,
    DIBITMAP
};

enum class RTFFieldStatus
{
    NONE,
    INSTRUCTION,
    RESULT
};

/// A buffer storing dmapper calls.
using Buf_t = std::tuple<RTFBufferTypes, RTFValue::Pointer_t, std::shared_ptr<TableRowBuffer>>;
using RTFBuffer_t = std::deque<Buf_t>;

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

    TableRowBuffer(RTFBuffer_t aBuffer, std::deque<RTFSprms> aSprms,
                   std::deque<RTFSprms> aAttributes, int const i_nCells)
        : buffer(std::move(aBuffer))
        , cellsSprms(std::move(aSprms))
        , cellsAttributes(std::move(aAttributes))
        , nCells(i_nCells)
    {
    }
};

/// An entry in the color table.
class RTFColorTableEntry
{
public:
    void SetRed(sal_uInt8 nRed)
    {
        m_bAuto = false;
        m_nR = nRed;
    }
    void SetGreen(sal_uInt8 nGreen)
    {
        m_bAuto = false;
        m_nG = nGreen;
    }
    void SetBlue(sal_uInt8 nBlue)
    {
        m_bAuto = false;
        m_nB = nBlue;
    }
    Color GetColor() const { return m_bAuto ? COL_AUTO : Color(m_nR, m_nG, m_nB); }

private:
    bool m_bAuto = true;
    sal_uInt8 m_nR = 0;
    sal_uInt8 m_nG = 0;
    sal_uInt8 m_nB = 0;
};

/// Stores the properties of a shape.
class RTFShape
{
public:
    RTFShape();
    std::vector<std::pair<OUString, OUString>> aProperties; ///< Properties of a single shape.
    std::vector<std::pair<OUString, OUString>>
        aGroupProperties; ///< Properties applied on the groupshape.
    sal_Int32 nLeft = 0;
    sal_Int32 nTop = 0;
    sal_Int32 nRight = 0;
    sal_Int32 nBottom = 0;
    boost::optional<sal_Int32> oZ; ///< Z-Order of the shape.
    sal_Int16 nHoriOrientRelation = 0; ///< Horizontal text::RelOrientation for drawinglayer shapes.
    sal_Int16 nVertOrientRelation = 0; ///< Vertical text::RelOrientation for drawinglayer shapes.
    sal_uInt32 nHoriOrientRelationToken = 0; ///< Horizontal dmapper token for Writer pictures.
    sal_uInt32 nVertOrientRelationToken = 0; ///< Vertical dmapper token for Writer pictures.
    css::text::WrapTextMode nWrap = css::text::WrapTextMode::WrapTextMode_MAKE_FIXED_SIZE;
    /// If shape is below text (true) or text is below shape (false).
    bool bInBackground = false;
    /// Wrap polygon, written by RTFSdrImport::resolve(), read by RTFDocumentImpl::resolvePict().
    RTFSprms aWrapPolygonSprms;
    /// Anchor attributes like wrap distance, written by RTFSdrImport::resolve(), read by RTFDocumentImpl::resolvePict().
    RTFSprms aAnchorAttributes;
    /// Wrap type, written by RTFDocumentImpl::popState(), read by RTFDocumentImpl::resolvePict().
    std::pair<Id, RTFValue::Pointer_t> aWrapSprm{ 0, nullptr };
};

/// Stores the properties of a drawing object.
class RTFDrawingObject : public RTFShape
{
public:
    RTFDrawingObject();
    css::uno::Reference<css::drawing::XShape> xShape;
    css::uno::Reference<css::beans::XPropertySet> xPropertySet;
    std::vector<css::beans::PropertyValue> aPendingProperties;
    sal_uInt8 nLineColorR = 0;
    sal_uInt8 nLineColorG = 0;
    sal_uInt8 nLineColorB = 0;
    bool bHasLineColor = false;
    sal_uInt8 nFillColorR = 0;
    sal_uInt8 nFillColorG = 0;
    sal_uInt8 nFillColorB = 0;
    bool bHasFillColor = false;
    sal_Int32 nDhgt = 0;
    sal_Int32 nFLine = -1;
    sal_Int32 nPolyLineCount = 0;
    std::vector<css::awt::Point> aPolyLinePoints;
    bool bHadShapeText = false;
};

/// Stores the properties of a picture.
class RTFPicture
{
public:
    RTFPicture();
    sal_uInt16 nWidth = 0;
    sal_uInt16 nHeight = 0;
    sal_uInt16 nGoalWidth = 0;
    sal_uInt16 nGoalHeight = 0;
    sal_uInt16 nScaleX = 100;
    sal_uInt16 nScaleY = 100;
    short nCropT = 0;
    short nCropB = 0;
    short nCropL = 0;
    short nCropR = 0;
    sal_uInt16 eWMetafile = 0;
    RTFBmpStyle eStyle = RTFBmpStyle::NONE;
};

/// Stores the properties of a frame
class RTFFrame
{
private:
    RTFDocumentImpl* m_pDocumentImpl;
    sal_Int32 m_nX, m_nY, m_nW, m_nH;
    sal_Int32 m_nHoriPadding, m_nVertPadding;
    sal_Int32 m_nHoriAlign, m_nHoriAnchor, m_nVertAlign, m_nVertAnchor;
    Id m_nHRule;
    boost::optional<Id> m_oWrap;

public:
    explicit RTFFrame(RTFParserState* pParserState);
    sal_Int16 m_nAnchorType;

    /// Convert the stored properties to Sprms
    RTFSprms getSprms();
    /// Store a property
    void setSprm(Id nId, Id nValue);
    bool hasProperties();
    /// If we got tokens indicating we're in a frame.
    bool inFrame();
};

/// State of the parser, which gets saved / restored when changing groups.
class RTFParserState
{
public:
    explicit RTFParserState(RTFDocumentImpl* pDocumentImpl);

    RTFDocumentImpl* m_pDocumentImpl;
    RTFInternalState nInternalState;
    Destination eDestination;
    RTFFieldStatus eFieldStatus;
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
    /// If aLevelNumbers should be read at all.
    bool bLevelNumbersValid;

    RTFPicture aPicture;
    RTFShape aShape;
    RTFDrawingObject aDrawingObject;
    RTFFrame aFrame;

    /// Maps to OOXML's ascii, cs or eastAsia.
    enum class RunType
    {
        LOCH,
        HICH,
        DBCH
    };
    RunType eRunType;
    /// ltrch or rtlch
    bool isRightToLeft;

    // Info group.
    sal_Int16 nYear;
    sal_uInt16 nMonth;
    sal_uInt16 nDay;
    sal_uInt16 nHour;
    sal_uInt16 nMinute;

    /// Text from special destinations.
    OUStringBuffer aDestinationText;
    /// point to the buffer of the current destination
    OUStringBuffer* pDestinationText;

    void appendDestinationText(const OUString& rString)
    {
        if (pDestinationText)
            pDestinationText->append(rString);
    }

    /// Index of the current style.
    int nCurrentStyleIndex;
    /// Index of the current character style.
    int nCurrentCharacterStyleIndex;
    /// Current listid, points to a listtable entry.
    int nCurrentListIndex = -1;
    /// Current ls, points to a listoverridetable entry.
    int nCurrentListOverrideIndex = -1;

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

    /// User-defined property: key name.
    OUString aPropName;
    /// User-defined property: value type.
    css::uno::Type aPropType;

    /// Width of invisible cell at the end of the row.
    int nTableRowWidthAfter;
};

/// An RTF stack is similar to std::stack, except that it has an operator[].
struct RTFStack
{
private:
    std::deque<RTFParserState> m_Impl;

public:
    RTFParserState& top()
    {
        if (m_Impl.empty())
            throw std::out_of_range("empty rtf state stack");
        return m_Impl.back();
    }
    void pop()
    {
        if (m_Impl.empty())
            throw std::out_of_range("empty rtf state stack");
        return m_Impl.pop_back();
    }
    void push(RTFParserState const& rState) { return m_Impl.push_back(rState); }
    bool empty() const { return m_Impl.empty(); }
    size_t size() const { return m_Impl.size(); }
    const RTFParserState& operator[](size_t nIndex) const { return m_Impl[nIndex]; }
    RTFParserState& operator[](size_t nIndex) { return m_Impl[nIndex]; }
};

void putBorderProperty(RTFStack& aStates, Id nId, const RTFValue::Pointer_t& pValue);
void putNestedSprm(RTFSprms& rSprms, Id nParent, Id nId, const RTFValue::Pointer_t& pValue,
                   RTFOverwrite eOverwrite = RTFOverwrite::NO_APPEND);
Id getParagraphBorder(sal_uInt32 nIndex);
void putNestedAttribute(RTFSprms& rSprms, Id nParent, Id nId, const RTFValue::Pointer_t& pValue,
                        RTFOverwrite eOverwrite = RTFOverwrite::YES, bool bAttribute = true);
bool eraseNestedAttribute(RTFSprms& rSprms, Id nParent, Id nId);

/// Looks up the nParent then the nested nId attribute in rSprms.
RTFValue::Pointer_t getNestedAttribute(RTFSprms& rSprms, Id nParent, Id nId);

/// Looks up the nParent then the nested nId sprm in rSprms.
RTFValue::Pointer_t getNestedSprm(RTFSprms& rSprms, Id nParent, Id nId);

/// Checks if rName is contained at least once in rProperties as a key.
bool findPropertyName(const std::vector<css::beans::PropertyValue>& rProperties,
                      const OUString& rName);
RTFSprms& getLastAttributes(RTFSprms& rSprms, Id nId);
OString DTTM22OString(long nDTTM);

/// Implementation of the RTFDocument interface.
class RTFDocumentImpl : public RTFDocument, public RTFListener
{
public:
    using Pointer_t = std::shared_ptr<RTFDocumentImpl>;
    RTFDocumentImpl(css::uno::Reference<css::uno::XComponentContext> const& xContext,
                    css::uno::Reference<css::io::XInputStream> const& xInputStream,
                    css::uno::Reference<css::lang::XComponent> const& xDstDoc,
                    css::uno::Reference<css::frame::XFrame> const& xFrame,
                    css::uno::Reference<css::task::XStatusIndicator> const& xStatusIndicator,
                    const utl::MediaDescriptor& rMediaDescriptor);
    ~RTFDocumentImpl() override;

    // RTFDocument
    void resolve(Stream& rMapper) override;

    // RTFListener
    RTFError dispatchDestination(RTFKeyword nKeyword) override;
    RTFError dispatchFlag(RTFKeyword nKeyword) override;
    RTFError dispatchSymbol(RTFKeyword nKeyword) override;
    RTFError dispatchToggle(RTFKeyword nKeyword, bool bParam, int nParam) override;
    RTFError dispatchValue(RTFKeyword nKeyword, int nParam) override;
    RTFError resolveChars(char ch) override;
    RTFError pushState() override;
    RTFError popState() override;
    Destination getDestination() override;
    void setDestination(Destination eDestination) override;
    RTFInternalState getInternalState() override;
    void setInternalState(RTFInternalState nInternalState) override;
    bool getSkipUnknown() override;
    void setSkipUnknown(bool bSkipUnknown) override;
    void finishSubstream() override;
    bool isSubstream() const override;

    Stream& Mapper() { return *m_pMapperStream; }
    void setSuperstream(RTFDocumentImpl* pSuperstream);
    const css::uno::Reference<css::lang::XMultiServiceFactory>& getModelFactory()
    {
        return m_xModelFactory;
    }
    bool isInBackground();
    void setDestinationText(OUString const& rString);
    /// Resolve a picture: If not inline, then anchored.
    void resolvePict(bool bInline, css::uno::Reference<css::drawing::XShape> const& rShape);

    /// If this is the first run of the document, starts the initial paragraph.
    void checkFirstRun();
    /// Send NS_ooxml::LN_settings_settings to dmapper.
    void outputSettingsTable();
    /// If the initial paragraph is started.
    bool getFirstRun() { return m_bFirstRun; }
    /// If we need to add a dummy paragraph before a section break.
    void setNeedPar(bool bNeedPar);
    /// Return the dmapper index of an RTF index for fonts.
    int getFontIndex(int nIndex);
    /// Return the name of the font, based on a dmapper index.
    OUString getFontName(int nIndex);
    /// Return the style name of an RTF style index.
    OUString getStyleName(int nIndex);
    /// Return the style type of an RTF style index.
    Id getStyleType(int nIndex);
    /// Return the encoding associated with a font index.
    rtl_TextEncoding getEncoding(int nFontIndex);
    /// Get the default parser state.
    RTFParserState& getDefaultState();
    oox::GraphicHelper& getGraphicHelper();
    /// Are we inside the stylesheet table?
    bool isStyleSheetImport();
    /// Resets m_aStates.top().aFrame.
    void resetFrame();

private:
    SvStream& Strm();
    Color getColorTable(sal_uInt32 nIndex);
    writerfilter::Reference<Properties>::Pointer_t createStyleProperties();
    void resetSprms();
    void resetAttributes();
    void resolveSubstream(std::size_t nPos, Id nId);
    void resolveSubstream(std::size_t nPos, Id nId, OUString const& rIgnoreFirst);

    void text(OUString& rString);
    // Sends a single character to dmapper, taking care of buffering.
    void singleChar(sal_uInt8 nValue, bool bRunProps = false);
    // Sends run properties to dmapper, taking care of buffering.
    void runProps();
    void runBreak();
    void parBreak();
    void tableBreak();
    writerfilter::Reference<Properties>::Pointer_t
    getProperties(RTFSprms& rAttributes, RTFSprms const& rSprms, Id nStyleType);
    void checkNeedPap();
    void sectBreak(bool bFinal = false);
    void prepareProperties(RTFParserState& rState,
                           writerfilter::Reference<Properties>::Pointer_t& o_rpParagraphProperties,
                           writerfilter::Reference<Properties>::Pointer_t& o_rpFrameProperties,
                           writerfilter::Reference<Properties>::Pointer_t& o_rpTableRowProperties,
                           int nCells, int nCurrentCellX);
    /// Send the passed properties to dmapper.
    void sendProperties(writerfilter::Reference<Properties>::Pointer_t const& pParagraphProperties,
                        writerfilter::Reference<Properties>::Pointer_t const& pFrameProperties,
                        writerfilter::Reference<Properties>::Pointer_t const& pTableRowProperties);
    void replayRowBuffer(RTFBuffer_t& rBuffer, ::std::deque<RTFSprms>& rCellsSrpms,
                         ::std::deque<RTFSprms>& rCellsAttributes, int nCells);
    void replayBuffer(RTFBuffer_t& rBuffer, RTFSprms* pSprms, RTFSprms const* pAttributes);
    /// If we have some unicode or hex characters to send.
    void checkUnicode(bool bUnicode, bool bHex);
    /// If we need a final section break at the end of the document.
    void setNeedSect(bool bNeedSect);
    void resetTableRowProperties();
    void backupTableRowProperties();
    void restoreTableRowProperties();
    /// Turns the destination text into an input stream of the current OLE attributes.
    RTFError handleEmbeddedObject();

    css::uno::Reference<css::uno::XComponentContext> const& m_xContext;
    css::uno::Reference<css::io::XInputStream> const& m_xInputStream;
    css::uno::Reference<css::lang::XComponent> const& m_xDstDoc;
    css::uno::Reference<css::frame::XFrame> const& m_xFrame;
    css::uno::Reference<css::task::XStatusIndicator> const& m_xStatusIndicator;
    css::uno::Reference<css::lang::XMultiServiceFactory> m_xModelFactory;
    css::uno::Reference<css::document::XDocumentProperties> m_xDocumentProperties;
    std::shared_ptr<SvStream> m_pInStream;
    Stream* m_pMapperStream;
    std::shared_ptr<RTFSdrImport> m_pSdrImport;
    std::shared_ptr<RTFTokenizer> m_pTokenizer;
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
    /// Maps style indexes to style types.
    std::map<int, Id> m_aStyleTypes;
    /// Color index <-> RGB color value map
    std::vector<Color> m_aColorTable;
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
    /// Maps between listoverridetable and listtable indexes.
    std::map<int, int> m_aListOverrideTable;
    /// Maps listtable indexes to listtable entries.
    std::map<int, RTFValue::Pointer_t> m_aListTable;
    /// Index of the current list level in a list table entry.
    int m_nListLevel = -1;
    /// Maps List level indexes to removed values in the current list entry.
    std::map<int, int> m_aInvalidListLevelFirstIndents;
    /// Maps list table indexes to levels (and their removed values).
    std::map<int, std::map<int, int>> m_aInvalidListTableFirstIndents;
    /// The settings table attributes.
    RTFSprms m_aSettingsTableAttributes;
    /// The settings table sprms.
    RTFSprms m_aSettingsTableSprms;

    std::shared_ptr<oox::GraphicHelper> m_pGraphicHelper;

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

    // Left row margin (for nested and top-level rows)
    int m_nNestedTRLeft;
    int m_nTopLevelTRLeft;

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
    std::deque<RTFBuffer_t> m_aTableBufferStack;
    /// Buffered superscript, till footnote is reached (or not).
    RTFBuffer_t m_aSuperBuffer;

    /// Superstream of this substream.
    RTFDocumentImpl* m_pSuperstream;
    /// Type of the stream: header, footer, footnote, etc.
    Id m_nStreamType;
    std::queue<std::pair<Id, std::size_t>> m_nHeaderFooterPositions;
    std::size_t m_nGroupStartPos;
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
    RTFFormFieldType m_nFormFieldType;

    /// OLE attributes are attributes of the ooxml:OLEObject_OLEObject sprm.
    RTFSprms m_aOLEAttributes;
    RTFSprms m_aObjectAttributes;
    /** If we are in an object group and if the we use its
     *  \objdata element.
     *  (if we don't use the \objdata we use the \result element)*/
    bool m_bObject;
    /// If the data for a picture is a binary one, it's stored here.
    std::shared_ptr<SvStream> m_pBinaryData;

    RTFReferenceTable::Entries_t m_aFontTableEntries;
    int m_nCurrentFontIndex;
    /// Used only during font table parsing till we don't know the font name.
    int m_nCurrentEncoding;
    /// Raw default font index, use getFont() on it to get a real one.
    int m_nDefaultFontIndex;

    RTFReferenceTable::Entries_t m_aStyleTableEntries;
    int m_nCurrentStyleIndex;
    bool m_bFormField;
    /// For the INCLUDEPICTURE field's argument.
    OUString m_aPicturePath;
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

    /// New document means not pasting into an existing one.
    bool m_bIsNewDoc;
    /// The media descriptor contains e.g. the base URL of the document.
    const utl::MediaDescriptor& m_rMediaDescriptor;

    /// Flags for ensuring that only one header and footer is added per section
    bool m_hasRHeader;
    bool m_hasFHeader;
    bool m_hasRFooter;
    bool m_hasFFooter;

    /// Are we after a \cell, but before a \row?
    bool m_bAfterCellBeforeRow;
};
} // namespace rtftok
} // namespace writerfilter

#endif // INCLUDED_WRITERFILTER_SOURCE_RTFTOK_RTFDOCUMENTIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
