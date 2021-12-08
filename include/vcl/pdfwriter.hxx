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
#ifndef INCLUDED_VCL_PDFWRITER_HXX
#define INCLUDED_VCL_PDFWRITER_HXX

#include <config_options.h>
#include <sal/types.h>

#include <tools/gen.hxx>
#include <tools/color.hxx>
#include <rtl/strbuf.hxx>

#include <vcl/dllapi.h>
#include <vcl/font.hxx>
#include <vcl/outdev.hxx>
#include <vcl/graph.hxx>

#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/util/DateTime.hpp>

#include <memory>
#include <vector>
#include <set>

namespace com::sun::star::beans { class XMaterialHolder; }
namespace com::sun::star::io { class XOutputStream; }
namespace com::sun::star::security { class XCertificate; }

class GDIMetaFile;
class MapMode;
class LineInfo;
namespace tools {
    class Polygon;
    class PolyPolygon;
}
class Bitmap;
class BitmapEx;
class Gradient;
class Hatch;
class Wallpaper;

namespace vcl
{

class PDFExtOutDevData;
class PDFWriterImpl;

struct PDFNote
{
    OUString          Title;          // optional title for the popup containing the note
    OUString          Contents;       // contents of the note
    css::util::DateTime maModificationDate;
};

class VCL_DLLPUBLIC PDFOutputStream
{
    public:
    virtual ~PDFOutputStream();
    virtual void write( const css::uno::Reference< css::io::XOutputStream >& xStream ) = 0;
};

class VCL_DLLPUBLIC PDFWriter
{
    ScopedVclPtr<PDFWriterImpl> xImplementation;

    PDFWriter(const PDFWriter&) = delete;
    PDFWriter& operator=(const PDFWriter&) = delete;

public:
    // extended line info
    enum CapType { capButt, capRound, capSquare };
    enum JoinType { joinMiter, joinRound, joinBevel };
    struct ExtLineInfo
    {
        double                      m_fLineWidth;
        double                      m_fTransparency;
        CapType                     m_eCap;
        JoinType                    m_eJoin;
        double                      m_fMiterLimit;
        std::vector< double >       m_aDashArray;

        ExtLineInfo() : m_fLineWidth( 0.0 ),
                        m_fTransparency( 0.0 ),
                        m_eCap( capButt ),
                        m_eJoin( joinMiter ),
                        m_fMiterLimit( 10.0 )
        {}
    };

    enum class Orientation { Portrait, Inherit };

    // in case the below enum is added PDF_1_6 PDF_1_7, please add them just after PDF_1_5
    enum class PDFVersion { PDF_1_2, PDF_1_3, PDF_1_4, PDF_1_5, PDF_1_6, PDF_A_1, PDF_A_2, PDF_A_3 };//i59651, PDF/A-1b & -1a, only -1b implemented for now
    // for the meaning of DestAreaType please look at PDF Reference Manual
    // version 1.4 section 8.2.1, page 475
    enum class DestAreaType { XYZ, FitRectangle };

    // for a definition of structural element types please refer to
    // PDF Reference, 3rd ed. section 9.7.4
    enum StructElement
    {
        // special element to place outside the structure hierarchy
        NonStructElement,
        // Grouping elements
        Document, Part, Article, Section, Division, BlockQuote,
        Caption, TOC, TOCI, Index,

        // block level elements
        Paragraph, Heading, H1, H2, H3, H4, H5, H6,
        List, ListItem, LILabel, LIBody,
        Table, TableRow, TableHeader, TableData,

        // inline level elements
        Span, Quote, Note, Reference, BibEntry, Code, Link,

        // illustration elements
        Figure, Formula, Form
    };

    enum StructAttribute
    {
        Placement, WritingMode, SpaceBefore, SpaceAfter, StartIndent, EndIndent,
        TextIndent, TextAlign, Width, Height, BlockAlign, InlineAlign,
        LineHeight, BaselineShift, TextDecorationType, ListNumbering,
        RowSpan, ColSpan,

        // link destination is an artificial attribute that sets
        // the link annotation ID of a Link element
        // further note: since structure attributes can only be
        // set during content creation, but links can be
        // created after the fact, it is possible to set
        // an arbitrary id as structure attribute here. In this
        // case the arbitrary id has to be passed again when the
        // actual link annotation is created via SetLinkPropertyID
        LinkAnnotation,
        // Language currently sets a LanguageType (see i18nlangtag/lang.h)
        // which will be internally changed to a corresponding locale
        Language
    };

    enum StructAttributeValue
    {
        Invalid,
        NONE,
        // Placement
        Block, Inline, Before, After, Start, End,
        // WritingMode
        LrTb, RlTb, TbRl,
        // TextAlign
        Center, Justify,
        // Width, Height,
        Auto,
        // BlockAlign
        Middle,
        // LineHeight
        Normal,
        // TextDecorationType
        Underline, Overline, LineThrough,
        // ListNumbering
        Disc, Circle, Square, Decimal, UpperRoman, LowerRoman, UpperAlpha, LowerAlpha
    };

    enum class PageTransition
    {
        Regular,
        SplitHorizontalInward, SplitHorizontalOutward,
        SplitVerticalInward, SplitVerticalOutward,
        BlindsHorizontal, BlindsVertical,
        BoxInward, BoxOutward,
        WipeLeftToRight, WipeBottomToTop, WipeRightToLeft, WipeTopToBottom,
        Dissolve
    };

    enum WidgetType
    {
        PushButton, RadioButton, CheckBox, Edit, ListBox, ComboBox, Hierarchy,
        Signature
    };

    enum ErrorCode
    {
        // transparent object occurred and was draw opaque because
        // PDF/A does not allow transparency
        Warning_Transparency_Omitted_PDFA,

        // transparent object occurred but is only supported since
        // PDF 1.4
        Warning_Transparency_Omitted_PDF13,

        // a form action was exported that is not suitable for PDF/A
        // the action was skipped
        Warning_FormAction_Omitted_PDFA,

        // transparent objects were converted to a bitmap in order
        // to removetransparencies from the output
        Warning_Transparency_Converted,

        // signature generation failed
        Error_Signature_Failed,
    };

    struct UNLESS_MERGELIBS(VCL_DLLPUBLIC) AnyWidget
    {
        WidgetType          Type;       // primitive RTTI
    public:
        OUString            Name;       // a distinct name to identify the control
        OUString            Description;// descriptive text for the control (e.g. for tool tip)
        OUString            Text;       // user text to appear on the control
        DrawTextFlags       TextStyle;  // style flags
        bool                ReadOnly;
        tools::Rectangle           Location;   // describes the area filled by the control
        bool                Border;     // true: widget should have a border, false: no border
        Color               BorderColor;// COL_TRANSPARENT and Border=true means get color from application settings
        bool                Background; // true: widget shall draw its background, false: no background
        Color               BackgroundColor; // COL_TRANSPARENT and Background=true means get color from application settings
        vcl::Font           TextFont;   // an empty font will be replaced by the
                                        // appropriate font from the user settings
        Color               TextColor;  // COL_TRANSPARENT will be replaced by the appropriate color from application settings
        sal_Int32           TabOrder; // lowest number is first in tab order

        /* style flags for text are those for OutputDevice::DrawText
           allowed values are:
           DrawTextFlags::Left, DrawTextFlags::Center, DrawTextFlags::Right, DrawTextFlags::Top,
           DrawTextFlags::VCenter, DrawTextFlags::Bottom,
           DrawTextFlags::MultiLine, DrawTextFlags::WordBreak

           if TextStyle is 0, then each control will fill in default values
         */

         // note: the Name member comprises the field name of the resulting
         // PDF field names need to be globally unique. Therefore if any
         // Widget with an already used name is created, the name will be
         // made unique by adding an underscore ('_') and an ascending number
         // to the name.

        AnyWidget( WidgetType eType ) :
                Type( eType ),
                TextStyle( DrawTextFlags::NONE ),
                ReadOnly( false ),
                Border( false ),
                BorderColor( COL_TRANSPARENT ),
                Background( false ),
                BackgroundColor( COL_TRANSPARENT ),
                TextColor( COL_TRANSPARENT ),
                TabOrder( -1 )
        {}
        virtual ~AnyWidget();

        WidgetType getType() const { return Type; }

        virtual std::shared_ptr<AnyWidget> Clone() const = 0;

    protected:
        // note that this equals the default compiler-generated copy-ctor, but we want to have it
        // protected, to only allow sub classes to access it
        AnyWidget( const AnyWidget& rSource )
            :Type( rSource.Type )
            ,Name( rSource.Name )
            ,Description( rSource.Description )
            ,Text( rSource.Text )
            ,TextStyle( rSource.TextStyle )
            ,ReadOnly( rSource.ReadOnly )
            ,Location( rSource.Location )
            ,Border( rSource.Border )
            ,BorderColor( rSource.BorderColor )
            ,Background( rSource.Background )
            ,BackgroundColor( rSource.BackgroundColor )
            ,TextFont( rSource.TextFont )
            ,TextColor( rSource.TextColor )
            ,TabOrder( rSource.TabOrder )
        {
        }
        AnyWidget& operator=( const AnyWidget& );  // never implemented
    };

    struct PushButtonWidget final : public AnyWidget
    {
        /* If Dest is set to a valid link destination,
           Then pressing the button will act as a goto
           action within the document.

           Else:
           An empty URL means this button will reset the form.

           If URL is not empty and Submit is set, then the URL
           contained will be set as the URL to submit the
           form to. In this case the submit method will be
           either GET if SubmitGet is true or POST if
           SubmitGet is false.

           If URL is not empty and Submit is clear, then
           the URL contained will be interpreted as a
           hyperlink to be executed on pushing the button.

           There will be no error checking or any kind of
           conversion done to the URL parameter execept this:
           it will be output as 7bit Ascii. The URL
           will appear literally in the PDF file produced
        */
        sal_Int32           Dest;
        OUString       URL;
        bool                Submit;
        bool                SubmitGet;

        PushButtonWidget()
                : AnyWidget( vcl::PDFWriter::PushButton ),
                  Dest( -1 ), Submit( false ), SubmitGet( false )
        {}

        virtual std::shared_ptr<AnyWidget> Clone() const override
        {
            return std::make_shared<PushButtonWidget>( *this );
        }
    };

    struct CheckBoxWidget final : public AnyWidget
    {
        bool                Checked;

        CheckBoxWidget()
                : AnyWidget( vcl::PDFWriter::CheckBox ),
                  Checked( false )
        {}

        virtual std::shared_ptr<AnyWidget> Clone() const override
        {
            return std::make_shared<CheckBoxWidget>( *this );
        }
    };

    struct RadioButtonWidget final : public AnyWidget
    {
        bool                Selected;
        sal_Int32           RadioGroup;
        OUString       OnValue; // the value of the radio button if it is selected

        RadioButtonWidget()
                : AnyWidget( vcl::PDFWriter::RadioButton ),
                  Selected( false ),
                  RadioGroup( 0 )
        {}

        virtual std::shared_ptr<AnyWidget> Clone() const override
        {
            return std::make_shared<RadioButtonWidget>( *this );
        }
        // radio buttons having the same RadioGroup id comprise one
        // logical radio button group, that is at most one of the RadioButtons
        // in a group can be checked at any time
        //
        // note: a PDF radio button field consists of a named field
        // containing unnamed checkbox child fields. The name of the
        // radio button field is taken from the first RadioButtonWidget created
        // in the group
    };

    struct EditWidget final : public AnyWidget
    {
        bool                MultiLine;  // whether multiple lines are allowed
        bool                Password;   // visible echo off
        bool                FileSelect; // field is a file selector
        sal_Int32           MaxLen;     // maximum field length in characters, 0 means unlimited

        EditWidget()
                : AnyWidget( vcl::PDFWriter::Edit ),
                  MultiLine( false ),
                  Password( false ),
                  FileSelect( false ),
                  MaxLen( 0 )
        {}

        virtual std::shared_ptr<AnyWidget> Clone() const override
        {
            return std::make_shared<EditWidget>( *this );
        }
    };

    struct ListBoxWidget final : public AnyWidget
    {
        bool                            DropDown;
        bool                            MultiSelect;
        std::vector<OUString>      Entries;
        std::vector<sal_Int32>          SelectedEntries;
         // if MultiSelect is false only the first entry of SelectedEntries
         // will be taken into account. the same is implicit for PDF < 1.4
         // since multiselect is a 1.4+ feature

        ListBoxWidget()
                : AnyWidget( vcl::PDFWriter::ListBox ),
                  DropDown( false ),
                  MultiSelect( false )
        {}

        virtual std::shared_ptr<AnyWidget> Clone() const override
        {
            return std::make_shared<ListBoxWidget>( *this );
        }
    };

    // note: PDF only supports dropdown comboboxes
    struct ComboBoxWidget final : public AnyWidget
    {
        std::vector<OUString>      Entries;
        // set the current value in AnyWidget::Text

        ComboBoxWidget()
                : AnyWidget( vcl::PDFWriter::ComboBox )
        {}

        virtual std::shared_ptr<AnyWidget> Clone() const override
        {
            return std::make_shared<ComboBoxWidget>( *this );
        }
    };

    struct SignatureWidget final : public AnyWidget
    {
        SignatureWidget()
                : AnyWidget( vcl::PDFWriter::Signature )
        {}

        virtual std::shared_ptr<AnyWidget> Clone() const override
        {
            return std::make_shared<SignatureWidget>( *this );
        }
    };

    enum ExportDataFormat { HTML, XML, FDF, PDF };
// see 3.6.1 of PDF 1.4 ref for details, used for 8.1 PDF v 1.4 ref also
// These emuns are treated as integer while reading/writing to configuration
    enum PDFViewerPageMode
    {
        ModeDefault,
        UseOutlines,
        UseThumbs
    };
// These emuns are treated as integer while reading/writing to configuration
    enum PDFViewerAction
    {
        ActionDefault,
        FitInWindow,
        FitWidth,
        FitVisible,
        ActionZoom
    };
// These enums are treated as integer while reading/writing to configuration
    enum PDFPageLayout
    {
        DefaultLayout,
        SinglePage,
        Continuous,
        ContinuousFacing
    };

    // These emuns are treated as integer while reading/writing to configuration
    //what default action to generate in a PDF hyperlink to external document/site
    enum PDFLinkDefaultAction
    {
        URIAction,
        URIActionDestination,
        LaunchAction
    };

/*
The following structure describes the permissions used in PDF security
 */
    struct PDFEncryptionProperties
    {

        //for both 40 and 128 bit security, see 3.5.2 PDF v 1.4 table 3.15, v 1.5 and v 1.6 table 3.20.
        bool CanPrintTheDocument;
        bool CanModifyTheContent;
        bool CanCopyOrExtract;
        bool CanAddOrModify;
        //for revision 3 (bit 128 security) only
        bool CanFillInteractive;
        bool CanExtractForAccessibility;
        bool CanAssemble;
        bool CanPrintFull;

        // encryption will only happen if EncryptionKey is not empty
        // EncryptionKey is actually a construct out of OValue, UValue and DocumentIdentifier
        // if these do not match, behavior is undefined, most likely an invalid PDF will be produced
        // OValue, UValue, EncryptionKey and DocumentIdentifier can be computed from
        // PDFDocInfo, Owner password and User password used the InitEncryption method which
        // implements the algorithms described in the PDF reference chapter 3.5: Encryption
        std::vector<sal_uInt8> OValue;
        std::vector<sal_uInt8> UValue;
        std::vector<sal_uInt8> EncryptionKey;
        std::vector<sal_uInt8> DocumentIdentifier;

        //permission default set for 128 bit, accessibility only
        PDFEncryptionProperties() :
            CanPrintTheDocument         ( false ),
            CanModifyTheContent         ( false ),
            CanCopyOrExtract            ( false ),
            CanAddOrModify              ( false ),
            CanFillInteractive          ( false ),
            CanExtractForAccessibility  ( true ),
            CanAssemble                 ( false ),
            CanPrintFull                ( false )
            {}


        bool Encrypt() const
        { return ! OValue.empty() && ! UValue.empty() && ! DocumentIdentifier.empty(); }
    };

    struct PDFDocInfo
    {
        OUString          Title;          // document title
        OUString          Author;         // document author
        OUString          Subject;        // subject
        OUString          Keywords;       // keywords
        OUString          Creator;        // application that created the original document
        OUString          Producer;       // OpenOffice
    };

    enum ColorMode
    {
        DrawColor, DrawGreyscale
    };

    struct PDFWriterContext
    {
        /* must be a valid file: URL usable by osl */
        OUString                   URL;
        /* the URL of the document being exported, used for relative links*/
        OUString                   BaseURL;
        /*if relative to file system should be formed*/
        bool                            RelFsys;//i56629, i49415?, i64585?
        /*the action to set the PDF hyperlink to*/
        PDFWriter::PDFLinkDefaultAction DefaultLinkAction;
        //convert the .od? target file type in a link to a .pdf type
        //this is examined before doing anything else
        bool                            ConvertOOoTargetToPDFTarget;
        //when the file type is .pdf, force the GoToR action
        bool                            ForcePDFAction;

        /* decides the PDF language level to be produced */
        PDFVersion                      Version;

        /* PDF/UA compliance */
        bool UniversalAccessibilityCompliance;

        /* valid for PDF >= 1.4
           causes the MarkInfo entry in the document catalog to be set
        */
        bool                            Tagged;
        /*  determines in which format a form
            will be submitted.
         */
        PDFWriter::ExportDataFormat     SubmitFormat;
        bool                            AllowDuplicateFieldNames;
        /* the following data members are used to customize the PDF viewer
           preferences
         */
        /* see 3.6.1 PDF v 1.4 ref*/
        PDFWriter::PDFViewerPageMode    PDFDocumentMode;
        PDFWriter::PDFViewerAction      PDFDocumentAction;
        // in percent, valid only if PDFDocumentAction == ActionZoom
        sal_Int32                       Zoom;

        /* see 8.6 PDF v 1.4 ref
           specifies whether to hide the viewer tool
          bars when the document is active.
        */
        bool                            HideViewerToolbar;
        bool                            HideViewerMenubar;
        bool                            HideViewerWindowControls;
        bool                            FitWindow;
        bool                            OpenInFullScreenMode;
        bool                            CenterWindow;
        bool                            DisplayPDFDocumentTitle;
        PDFPageLayout                   PageLayout;
        bool                            FirstPageLeft;
        // initially visible page in viewer (starting with 0 for first page)
        sal_Int32                       InitialPage;
        sal_Int32                       OpenBookmarkLevels; // -1 means all levels

        PDFWriter::PDFEncryptionProperties  Encryption;
        PDFWriter::PDFDocInfo           DocumentInfo;

        bool                            SignPDF;
        OUString                        SignLocation;
        OUString                        SignPassword;
        OUString                        SignReason;
        OUString                        SignContact;
        css::lang::Locale               DocumentLocale; // defines the document default language
        sal_uInt32                      DPIx, DPIy;     // how to handle MapMode( MapUnit::MapPixel )
                                                        // 0 here specifies a default handling
        PDFWriter::ColorMode            ColorMode;
        css::uno::Reference< css::security::XCertificate> SignCertificate;
        OUString                        SignTSA;
        /// Use reference XObject markup for PDF images.
        bool                            UseReferenceXObject;

        PDFWriterContext() :
                RelFsys( false ), //i56629, i49415?, i64585?
                DefaultLinkAction( PDFWriter::URIAction ),
                ConvertOOoTargetToPDFTarget( false ),
                ForcePDFAction( false ),
                Version( PDFWriter::PDFVersion::PDF_1_6 ),
                UniversalAccessibilityCompliance( false ),
                Tagged( false ),
                SubmitFormat( PDFWriter::FDF ),
                AllowDuplicateFieldNames( false ),
                PDFDocumentMode( PDFWriter::ModeDefault ),
                PDFDocumentAction( PDFWriter::ActionDefault ),
                Zoom( 100 ),
                HideViewerToolbar( false ),
                HideViewerMenubar( false ),
                HideViewerWindowControls( false ),
                FitWindow( false ),
                OpenInFullScreenMode( false ),
                CenterWindow( false ),
                DisplayPDFDocumentTitle( true ),
                PageLayout( PDFWriter::DefaultLayout ),
                FirstPageLeft( false ),
                InitialPage( 1 ),
                OpenBookmarkLevels( -1 ),
                SignPDF( false ),
                DPIx( 0 ),
                DPIy( 0 ),
                ColorMode( PDFWriter::DrawColor ),
                UseReferenceXObject( false )
        {}
    };

    PDFWriter( const PDFWriterContext& rContext, const css::uno::Reference< css::beans::XMaterialHolder >& );
    ~PDFWriter();

    /** Returns an OutputDevice for formatting
        This Output device is guaranteed to use the same
        font metrics as the resulting PDF file.

        @returns
        the reference output device
    */
    OutputDevice* GetReferenceDevice();

    /** Creates a new page to fill
        If width and height are not set the page size
        is inherited from the page tree
        other effects:
        resets the graphics state: MapMode, Font
        Colors and other state information MUST
        be set again or are undefined.
    */
    void NewPage( double nPageWidth, double nPageHeight, Orientation eOrientation = Orientation::Inherit );
    /** Play a metafile like an outputdevice would do
    */
    struct PlayMetafileContext
    {
        int     m_nMaxImageResolution;
        bool    m_bOnlyLosslessCompression;
        int     m_nJPEGQuality;
        bool    m_bTransparenciesWereRemoved;

        PlayMetafileContext()
        : m_nMaxImageResolution( 0 )
        , m_bOnlyLosslessCompression( false )
        , m_nJPEGQuality( 90 )
        , m_bTransparenciesWereRemoved( false )
        {}

    };
    void PlayMetafile( const GDIMetaFile&, const PlayMetafileContext&, vcl::PDFExtOutDevData* pDevDat = nullptr );

    /* sets the document locale originally passed with the context to a new value
     * only affects the output if used before calling Emit.
     */
    void SetDocumentLocale( const css::lang::Locale& rDocLocale );

    /* finishes the file */
    bool Emit();

    /*
     * Get a list of errors that occurred during processing
     * this should enable the producer to give feedback about
     * any anomalies that might have occurred
     */
    std::set< ErrorCode > const & GetErrors() const;

    // uses 128bit encryption
    static css::uno::Reference< css::beans::XMaterialHolder >
           InitEncryption( const OUString& i_rOwnerPassword,
                           const OUString& i_rUserPassword
                         );

    /* functions for graphics state */
    /* flag values: see vcl/outdev.hxx */
    void                Push( PushFlags nFlags = PushFlags::ALL );
    void                Pop();

    void               SetClipRegion();
    void               SetClipRegion( const basegfx::B2DPolyPolygon& rRegion );
    void               MoveClipRegion( tools::Long nHorzMove, tools::Long nVertMove );
    void               IntersectClipRegion( const tools::Rectangle& rRect );
    void               IntersectClipRegion( const basegfx::B2DPolyPolygon& rRegion );

    void               SetLayoutMode( vcl::text::ComplexTextLayoutFlags nMode );
    void               SetDigitLanguage( LanguageType eLang );

    void               SetLineColor( const Color& rColor );
    void               SetLineColor() { SetLineColor( COL_TRANSPARENT ); }

    void               SetFillMode( const PolyFillMode& rFillMode );
    void               SetFillMode() { SetFillMode( PolyFillMode::EVEN_ODD_RULE_ALTERNATE ); }

    void               SetFillColor( const Color& rColor );
    void               SetFillColor() { SetFillColor( COL_TRANSPARENT ); }

    void               SetFont( const vcl::Font& rNewFont );
    void               SetTextColor( const Color& rColor );
    void               SetTextFillColor();
    void               SetTextFillColor( const Color& rColor );

    void               SetTextLineColor();
    void               SetTextLineColor( const Color& rColor );
    void               SetOverlineColor();
    void               SetOverlineColor( const Color& rColor );
    void               SetTextAlign( ::TextAlign eAlign );

    void               SetMapMode( const MapMode& rNewMapMode );


    /* actual drawing functions */
    void                DrawText( const Point& rPos, const OUString& rText );

    void                DrawTextLine( const Point& rPos, tools::Long nWidth,
                                      FontStrikeout eStrikeout,
                                      FontLineStyle eUnderline,
                                      FontLineStyle eOverline );
    void                DrawTextArray( const Point& rStartPt, const OUString& rStr,
                                       o3tl::span<const sal_Int32> pDXAry,
                                       sal_Int32 nIndex,
                                       sal_Int32 nLen );
    void                DrawStretchText( const Point& rStartPt, sal_uLong nWidth,
                                         const OUString& rStr,
                                         sal_Int32 nIndex, sal_Int32 nLen );
    void                DrawText( const tools::Rectangle& rRect,
                                  const OUString& rStr, DrawTextFlags nStyle );

    void                DrawPixel( const Point& rPt, const Color& rColor );
    void                DrawPixel( const Point& rPt )
    { DrawPixel( rPt, COL_TRANSPARENT ); }

    void                DrawLine( const Point& rStartPt, const Point& rEndPt );
    void                DrawLine( const Point& rStartPt, const Point& rEndPt,
                                  const LineInfo& rLineInfo );
    void                DrawPolyLine( const tools::Polygon& rPoly );
    void                DrawPolyLine( const tools::Polygon& rPoly,
                                      const LineInfo& rLineInfo );
    void                DrawPolyLine( const tools::Polygon& rPoly, const ExtLineInfo& rInfo );
    void                DrawPolygon( const tools::Polygon& rPoly );
    void                DrawPolyPolygon( const tools::PolyPolygon& rPolyPoly );
    void                DrawRect( const tools::Rectangle& rRect );
    void                DrawRect( const tools::Rectangle& rRect,
                                  sal_uLong nHorzRount, sal_uLong nVertRound );
    void                DrawEllipse( const tools::Rectangle& rRect );
    void                DrawArc( const tools::Rectangle& rRect,
                                 const Point& rStartPt, const Point& rEndPt );
    void                DrawPie( const tools::Rectangle& rRect,
                                 const Point& rStartPt, const Point& rEndPt );
    void                DrawChord( const tools::Rectangle& rRect,
                                   const Point& rStartPt, const Point& rEndPt );

    void                DrawBitmap( const Point& rDestPt, const Size& rDestSize,
                                    const Bitmap& rBitmap, const Graphic& rGraphic );

    void                DrawBitmapEx( const Point& rDestPt, const Size& rDestSize,
                                      const BitmapEx& rBitmapEx );

    void                DrawGradient( const tools::Rectangle& rRect, const Gradient& rGradient );
    void                DrawGradient( const tools::PolyPolygon& rPolyPoly, const Gradient& rGradient );

    void                DrawHatch( const tools::PolyPolygon& rPolyPoly, const Hatch& rHatch );

    void                DrawWallpaper( const tools::Rectangle& rRect, const Wallpaper& rWallpaper );
    void                DrawTransparent( const tools::PolyPolygon& rPolyPoly,
                                         sal_uInt16 nTransparencePercent );

    /** Start a transparency group

    Drawing operations can be grouped together to acquire a common transparency
    behaviour; after calling BeginTransparencyGroup all drawing
    operations will be grouped together into a transparent object.

    The transparency behaviour is set with one of the EndTransparencyGroup
    calls and can be either a constant transparency factor or a transparent
    soft mask in form of an 8 bit gray scale bitmap.

    It is permissible to nest transparency group.

    Transparency groups MUST NOT span multiple pages

    Transparency is a feature introduced in PDF1.4, so transparency group
    will be ignored if the produced PDF has a lower version. The drawing
    operations will be emitted normally.
    */
    void                BeginTransparencyGroup();

    /** End a transparency group with constant transparency factor

    This ends a transparency group and inserts it on the current page. The
    coordinates of the group result out of the grouped drawing operations.

    @param rBoundRect
    The bounding rectangle of the group

    @param nTransparencePercent
    The transparency factor
    */
    void                EndTransparencyGroup( const tools::Rectangle& rBoundRect, sal_uInt16 nTransparencePercent );

    /** Insert a JPG encoded image (optionally with mask)

    @param rJPGData
    a Stream containing the encoded image

    @param bIsTrueColor
    true: jpeg is 24 bit true color, false: jpeg is 8 bit greyscale

    @param rSrcSizePixel
    size in pixel of the image

    @param rTargetArea
    where to put the image

    @param rMask
    optional mask; if not empty it must have
    the same pixel size as the image and
    be either 1 bit black&white or 8 bit grey
    */
    void                DrawJPGBitmap( SvStream& rJPGData, bool bIsTrueColor, const Size& rSrcSizePixel, const tools::Rectangle& rTargetArea, const AlphaMask& rAlphaMask, const Graphic& rGraphic );

    /** Create a new named destination to be used in a link from another PDF document

    @param sDestName
    the name (label) of the bookmark, to be used to jump to

    @param rRect
    target rectangle on page to be displayed if dest is jumped to

    @param nPageNr
    number of page the dest is on (as returned by NewPage)
    or -1 in which case the current page is used

    @param eType
    what dest type to use

    @returns
    the destination id (to be used in SetLinkDest) or
    -1 if page id does not exist
    */
    sal_Int32           CreateNamedDest( const OUString& sDestName, const tools::Rectangle& rRect, sal_Int32 nPageNr, DestAreaType eType );
    /** Create a new destination to be used in a link

    @param rRect
    target rectangle on page to be displayed if dest is jumped to

    @param nPageNr
    number of page the dest is on (as returned by NewPage)
    or -1 in which case the current page is used

    @param eType
    what dest type to use

    @returns
    the destination id (to be used in SetLinkDest) or
    -1 if page id does not exist
    */
    sal_Int32           CreateDest( const tools::Rectangle& rRect, sal_Int32 nPageNr, DestAreaType eType );
    /** Create a new link on a page

    @param rRect
    active rectangle of the link (that is the area that has to be
    hit to activate the link)

    @param nPageNr
    number of page the link is on (as returned by NewPage)
    or -1 in which case the current page is used

    @returns
    the link id (to be used in SetLinkDest, SetLinkURL) or
    -1 if page id does not exist
    */
    sal_Int32           CreateLink( const tools::Rectangle& rRect, sal_Int32 nPageNr );

    /// Creates a screen annotation.
    sal_Int32 CreateScreen(const tools::Rectangle& rRect, sal_Int32 nPageNr);

    /** creates a destination which is not intended to be referred to by a link, but by a public destination Id.

        Form widgets, for instance, might refer to a destination, without ever actually creating a source link to
        point to this destination. In such cases, a public destination Id will be assigned to the form widget,
        and later on, the concrete destination data for this public Id will be registered using RegisterDestReference.

        @param nDestId
            destination ID

        @param rRect
            target rectangle on page to be displayed if dest is jumped to

        @param nPageNr
            number of page the dest is on (as returned by NewPage)
            or -1 in which case the current page is used

        @param eType
            what dest type to use

        @returns
            the internal destination Id.
    */
    sal_Int32           RegisterDestReference( sal_Int32 nDestId, const tools::Rectangle& rRect, sal_Int32 nPageNr, DestAreaType eType );


    /** Set the destination for a link
        will change a URL type link to a dest link if necessary

        @param nLinkId
        the link to be changed

        @param nDestId
        the dest the link shall point to
    */
    void           SetLinkDest( sal_Int32 nLinkId, sal_Int32 nDestId );
    /** Set the URL for a link
        will change a dest type link to a URL type link if necessary
        @param nLinkId
        the link to be changed

        @param rURL
        the URL the link shall point to.
        The URL will be parsed (and corrected) by the com.sun.star.util.URLTransformer
        service; the result will then appear literally in the PDF file produced
    */
    void           SetLinkURL( sal_Int32 nLinkId, const OUString& rURL );

    /// Sets the URL of a linked screen annotation.
    void SetScreenURL(sal_Int32 nScreenId, const OUString& rURL);
    /// Sets the URL of an embedded screen annotation.
    void SetScreenStream(sal_Int32 nScreenId, const OUString& rURL);

    /** Resolve link in logical structure

        If a link is created after the corresponding visual appearance was drawn
        it is not possible to set the link id as a property attribute to the
        link structure item that should be created in tagged PDF around the
        visual appearance of a link.

        For this reason an arbitrary id can be given to
        SetStructureAttributeNumerical at the time the text for
        the link is drawn. To resolve this arbitrary id again when the actual
        link annotation is created use SetLinkPropertyID. When Emit
        finally gets called all LinkAnnotation type structure attributes
        will be replaced with the correct link id.

        CAUTION: this technique must be used either for all or none of the links
        in a document since the link id space and arbitrary property id space
        could overlap and it would be impossible to resolve whether a Link
        structure attribute value was arbitrary or already a real id.

        @param nLinkId
        the link to be mapped

        @param nPropertyID
        the arbitrary id set in a Link structure element to address
        the link with real id nLinkId
     */
    void                SetLinkPropertyID( sal_Int32 nLinkId, sal_Int32 nPropertyID );
    /** Create a new outline item

        @param nParent
        declares the parent of the new item in the outline hierarchy.
        An invalid value will result in a new toplevel item.

        @param rText
        sets the title text of the item

        @param nDestID
        declares which Dest (created with CreateDest) the outline item
        will point to

        @returns
        the outline item id of the new item
    */
    sal_Int32 CreateOutlineItem( sal_Int32 nParent, const OUString& rText, sal_Int32 nDestID );

    /** Create a new note on a page

    @param rRect
    active rectangle of the note (that is the area that has to be
    hit to popup the annotation)

    @param rNote
    specifies the contents of the note

    @param nPageNr
    number of page the note is on (as returned by NewPage)
    or -1 in which case the current page is used
    */
    void CreateNote( const tools::Rectangle& rRect, const PDFNote& rNote, sal_Int32 nPageNr );

    /** begin a new logical structure element

    BeginStructureElement/EndStructureElement calls build the logical structure
    of the PDF - the basis for tagged PDF. Structural elements are implemented
    using marked content tags. Each structural element can contain sub elements
    (e.g. a section can contain a heading and a paragraph). The structure hierarchy
    is build automatically from the Begin/EndStructureElement calls.

    A structural element need not be contained on one page; e.g. paragraphs often
    run from one page to the next. In this case the corresponding EndStructureElement
    must be called while drawing the next page.

    BeginStructureElement and EndStructureElement must be called only after
    PDFWriter::NewPage has been called and before PDFWriter::Emit gets called. The
    current page number is an implicit context parameter for Begin/EndStructureElement.

    For pagination artifacts that are not part of the logical structure
    of the document (like header, footer or page number) the special
    StructElement NonStructElement exists. To place content
    outside of the structure tree simply call
    BeginStructureElement( NonStructElement ) then draw your
    content and then call EndStructureElement(). All children
    of a NonStructElement will not be part of the structure.
    Nonetheless if you add a child structural element to a
    NonStructElement you will still have to call
    EndStructureElement for it. Best think of the structure
    tree as a stack.

    Note: there is always one structural element in existence without having
    called BeginStructureElement; this is the root of the structure
    tree (called StructTreeRoot). The StructTreeRoot has always the id 0.

    @param eType
    denotes what kind of element to begin (e.g. a heading or paragraph)

    @param rAlias
    the specified alias will be used as structure tag. Also an entry in the PDF's
    role map will be created mapping alias to regular structure type.

    @returns
    the new structure element's id for use in SetCurrentStructureElement
     */
     sal_Int32 BeginStructureElement( enum StructElement eType, const OUString& rAlias );
    /** end the current logical structure element

    Close the current structure element. The current element's
    parent becomes the current structure element again.

    @see BeginStructureElement
     */
    void EndStructureElement();
    /** set the current structure element

    For different purposes it may be useful to paint a structure element's
    content discontinuously. In that case an already existing structure element
    can be appended to by using SetCurrentStructureElement. The
    referenced structure element becomes the current structure element with
    all consequences: all following structure elements are appended as children
    of the current element.

    @param nElement
    the id of the new current structure element
     */
    void SetCurrentStructureElement( sal_Int32 nElement );

    /** set a structure attribute on the current structural element

    SetStructureAttribute sets an attribute of the current structural element to a
    new value. A consistency check is performed before actually setting the value;
    if the check fails, the function returns False and the attribute remains
    unchanged.

    @param eAttr
    denotes what attribute to change

    @param eVal
    the value to set the attribute to
     */
    void SetStructureAttribute( enum StructAttribute eAttr, enum StructAttributeValue eVal );
    /** set a structure attribute on the current structural element

    SetStructureAttributeNumerical sets an attribute of the current structural element
    to a new numerical value. A consistency check is performed before actually setting
    the value; if the check fails, the function returns False and the attribute
    remains unchanged.

    @param eAttr
    denotes what attribute to change

    @param nValue
    the value to set the attribute to
     */
    void SetStructureAttributeNumerical( enum StructAttribute eAttr, sal_Int32 nValue );
    /** set the bounding box of a structural element

    SetStructureBoundingBox sets the BBox attribute to a new value. Since the BBox
    attribute can only be applied to Table, Figure,
    Form and Formula elements, a call of this function
    for other element types will be ignored and the BBox attribute not be set.

    @param rRect
    the new bounding box for the structural element
     */
    void SetStructureBoundingBox( const tools::Rectangle& rRect );

    /** set the ActualText attribute of a structural element

    ActualText contains the Unicode text without layout artifacts that is shown by
    a structural element. For example if a line is ended prematurely with a break in
    a word and continued on the next line (e.g. "happen-<newline>stance") the
    corresponding ActualText would contain the unbroken line (e.g. "happenstance").

    @param rText
    contains the complete logical text the structural element displays.
     */
    void SetActualText( const OUString& rText );

    /** set the Alt attribute of a strutural element

    Alt is s replacement text describing the contents of a structural element. This
    is mainly used by accessibility applications; e.g. a screen reader would read
    the Alt replacement text for an image to a visually impaired user.

    @param rText
    contains the replacement text for the structural element
    */
    void SetAlternateText( const OUString& rText );

    /** Sets the transitional effect to be applied when the current page gets shown.

    @param eType
    the kind of effect to be used; use Regular to disable transitional effects
    for this page

    @param nMilliSec
    the duration of the transitional effect in milliseconds;
    set 0 to disable transitional effects

    @param nPageNr
    the page number to apply the effect to; -1 denotes the current page
    */
    void SetPageTransition( PageTransition eType, sal_uInt32 nMilliSec, sal_Int32 nPageNr );

    /** create a new form control

    This function creates a new form control in the PDF and sets its various
    properties. Do not pass an actual AnyWidget as rControlType
    will be cast to the type described by the type member.

    @param rControlType
    a descendant of AnyWidget determining the control's properties

    @returns
    the new control's id for reference purposes
     */
    sal_Int32 CreateControl( const AnyWidget& rControlType );

    /** Inserts an additional stream to the PDF file

    This function adds an arbitrary stream to the produced PDF file. May be called
    any time before Emit(). The stream will be written during
    Emit by calling the PDFOutputStream Object's write
    method. After the call the PDFOutputStream will be deleted.

    All additional streams and their mimetypes will be entered into an array
    in the trailer dictionary.

    @param rMimeType
    the mimetype of the stream

    @param pStream
    the interface to the additional stream

    */
    void AddStream( const OUString& rMimeType, PDFOutputStream* pStream );

    /// Write rString as a PDF hex string into rBuffer.
    static void AppendUnicodeTextString(const OUString& rString, OStringBuffer& rBuffer);

    /// Get current date/time in PDF D:YYYYMMDDHHMMSS form.
    static OString GetDateTime();
};

VCL_DLLPUBLIC void escapeStringXML( const OUString& rStr, OUString &rValue);

}

#endif // INCLUDED_VCL_PDFWRITER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
