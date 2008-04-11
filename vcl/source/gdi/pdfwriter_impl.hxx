/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: pdfwriter_impl.hxx,v $
 * $Revision: 1.55 $
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
#ifndef _VCL_PDFWRITER_IMPL_HXX
#define _VCL_PDFWRITER_IMPL_HXX

#include "vcl/pdfwriter.hxx"
#include "rtl/ustring.hxx"
#include "osl/file.h"
#include "tools/gen.hxx"
#include "tools/stream.hxx"
#include "vcl/outdev.hxx"
#include "vcl/bitmapex.hxx"
#include "vcl/gradient.hxx"
#include "vcl/hatch.hxx"
#include "vcl/wall.hxx"
#include "vcl/outdata.hxx"
#include "rtl/strbuf.hxx"
#include "rtl/cipher.h"
#include "rtl/digest.h"
#include "com/sun/star/util/XURLTransformer.hpp"
#include "com/sun/star/lang/Locale.hpp"

#include <vcl/sallayout.hxx>
#include "pdffontcache.hxx"

#include <vector>
#include <map>
#include <hash_map>
#include <list>

class ImplFontSelectData;
class ImplFontMetricData;
struct FontSubsetInfo;
class ZCodec;

// the maximum password length
#define ENCRYPTED_PWD_SIZE     32
#define MD5_DIGEST_SIZE        16
#define SECUR_40BIT_KEY         5
// security 128 bit
#define SECUR_128BIT_KEY       16
// maximum length of MD5 digest input, in step 2 of algorithm 3.1
// PDF spec ver. 1.4: see there for details
#define MAXIMUM_RC4_KEY_LENGTH (SECUR_128BIT_KEY+3+2)

namespace vcl
{

class PDFSalLayout;
class PDFStreamIf;
class Matrix3;

class PDFWriterImpl
{
    friend class PDFSalLayout;
    friend class PDFStreamIf;
public:
    // definition of structs
    struct BuiltinFont
    {
        const char *                m_pName;                     // Name
        const char *                m_pStyleName;                // StyleName
        const char *                m_pPSName;                   // PSName
        int                         m_nAscent;
        int                         m_nDescent;
        FontFamily                  m_eFamily;                   // Family
        CharSet                     m_eCharSet;                  // CharSet
        FontPitch                   m_ePitch;                    // Pitch
        FontWidth                   m_eWidthType;                // WidthType
        FontWeight                  m_eWeight;                   // Weight
        FontItalic                  m_eItalic;                   // Italic
        int                         m_aWidths[256];              // character metrics

        rtl::OString getNameObject() const;
    };


    enum ResourceKind { ResXObject, ResExtGState, ResShading, ResPattern };
    typedef std::map< rtl::OString, sal_Int32 > ResourceMap;
    struct ResourceDict
    {
        // note: handle fonts globally for performance
        ResourceMap m_aXObjects;
        ResourceMap m_aExtGStates;
        ResourceMap m_aShadings;
        ResourceMap m_aPatterns;

        void append( rtl::OStringBuffer&, sal_Int32 nFontDictObject );
    };

    struct PDFPage
    {
        PDFWriterImpl*              m_pWriter;
        sal_Int32                   m_nPageWidth;           // in inch/72
        sal_Int32                   m_nPageHeight;          // in inch/72
        PDFWriter::Orientation      m_eOrientation;
        sal_Int32                   m_nPageObject;
        sal_Int32                   m_nPageIndex;
        std::vector<sal_Int32>      m_aStreamObjects;
        sal_Int32                   m_nStreamLengthObject;
        sal_uInt64                  m_nBeginStreamPos;
        std::vector<sal_Int32>      m_aAnnotations;
        std::vector<sal_Int32>      m_aMCIDParents;
        PDFWriter::PageTransition   m_eTransition;
        sal_uInt32                  m_nTransTime;
        sal_uInt32                  m_nDuration;
        bool                        m_bHasWidgets;

        PDFPage( PDFWriterImpl* pWriter, sal_Int32 nPageWidth, sal_Int32 nPageHeight, PDFWriter::Orientation eOrientation );
        ~PDFPage();

        void beginStream();
        void endStream();
        bool emit( sal_Int32 nParentPage );

        // converts point from ref device coordinates to
        // page coordinates and appends the point to the buffer
        // if bNeg is true, the coordinates are inverted AFTER transformation
        // to page (useful for transformation matrices
        // if pOutPoint is set it will be updated to the emitted point
        // (in PDF map mode, that is 10th of point)
        void appendPoint( const Point& rPoint, rtl::OStringBuffer& rBuffer, bool bNeg = false, Point* pOutPoint = NULL ) const;
        // appends a rectangle
        void appendRect( const Rectangle& rRect, rtl::OStringBuffer& rBuffer ) const;
        // converts a rectangle to 10th points page space
        void convertRect( Rectangle& rRect ) const;
        // appends a polygon optionally closing it
        void appendPolygon( const Polygon& rPoly, rtl::OStringBuffer& rBuffer, bool bClose = true ) const;
        // appends a polypolygon optionally closing the subpaths
        void appendPolyPolygon( const PolyPolygon& rPolyPoly, rtl::OStringBuffer& rBuffer, bool bClose = true ) const;
        // converts a length (either vertical or horizontal; this
        // can be important if the source MapMode is not
        // symmetrical) to page length and appends it to the buffer
        // if pOutLength is set it will be updated to the emitted length
        // (in PDF map mode, that is 10th of point)
        void appendMappedLength( sal_Int32 nLength, rtl::OStringBuffer& rBuffer, bool bVertical = true, sal_Int32* pOutLength = NULL ) const;
        // the same for double values
        void appendMappedLength( double fLength, rtl::OStringBuffer& rBuffer, bool bVertical = true, sal_Int32* pOutLength = NULL ) const;
        // appends LineInfo
        // returns false if too many dash array entry were created for
        // the implementation limits of some PDF readers
        bool appendLineInfo( const LineInfo& rInfo, rtl::OStringBuffer& rBuffer ) const;
        // appends a horizontal waveline with vertical offset (helper for drawWaveLine)
        void appendWaveLine( sal_Int32 nLength, sal_Int32 nYOffset, sal_Int32 nDelta, rtl::OStringBuffer& rBuffer ) const;

        sal_Int32 getWidth() const { return m_nPageWidth ? m_nPageWidth : m_pWriter->m_nInheritedPageWidth; }
        sal_Int32 getHeight() const { return m_nPageHeight ? m_nPageHeight : m_pWriter->m_nInheritedPageHeight; }
    };

    friend struct PDFPage;

    struct BitmapID
    {
        Size        m_aPixelSize;
        sal_Int32   m_nSize;
        sal_Int32   m_nChecksum;
        sal_Int32   m_nMaskChecksum;

        BitmapID() : m_nSize( 0 ), m_nChecksum( 0 ), m_nMaskChecksum( 0 ) {}

        BitmapID& operator=( const BitmapID& rCopy )
        {
            m_aPixelSize    = rCopy.m_aPixelSize;
            m_nSize         = rCopy.m_nSize;
            m_nChecksum     = rCopy.m_nChecksum;
            m_nMaskChecksum = rCopy.m_nMaskChecksum;
            return *this;
        }

        bool operator==( const BitmapID& rComp )
        {
            return (m_aPixelSize == rComp.m_aPixelSize &&
                    m_nSize == rComp.m_nSize &&
                    m_nChecksum == rComp.m_nChecksum &&
                    m_nMaskChecksum == rComp.m_nMaskChecksum );
        }
    };

    struct BitmapEmit
    {
        BitmapID    m_aID;
        BitmapEx    m_aBitmap;
        sal_Int32   m_nObject;
        bool        m_bDrawMask;

        BitmapEmit() : m_bDrawMask( false ) {}
    };

    struct JPGEmit
    {
        BitmapID            m_aID;
        SvMemoryStream*     m_pStream;
        Bitmap              m_aMask;
        sal_Int32           m_nObject;
        bool                m_bTrueColor;

        JPGEmit() : m_pStream( NULL ) {}
        ~JPGEmit() { delete m_pStream; }
    };

    struct GradientEmit
    {
        Gradient    m_aGradient;
        Size        m_aSize;
        sal_Int32   m_nObject;
    };

    // for tilings (drawWallpaper, begin/endPattern)
    struct TilingEmit
    {
        sal_Int32                   m_nObject;
        Rectangle                   m_aRectangle;
        Size                        m_aCellSize;
        SvtGraphicFill::Transform   m_aTransform;
        ResourceDict                m_aResources;
        SvMemoryStream*             m_pTilingStream;

        TilingEmit()
                : m_nObject( 0 ),
                  m_pTilingStream( NULL )
        {}
    };

    // for transparency group XObjects
    struct TransparencyEmit
    {
        sal_Int32           m_nObject;
        sal_Int32           m_nExtGStateObject;
        double              m_fAlpha;
        Rectangle           m_aBoundRect;
        SvMemoryStream*     m_pContentStream;
        SvMemoryStream*     m_pSoftMaskStream;

        TransparencyEmit()
                : m_nObject( 0 ),
                  m_nExtGStateObject( -1 ),
                  m_fAlpha( 0.0 ),
                  m_pContentStream( NULL ),
                  m_pSoftMaskStream( NULL )
        {}
        ~TransparencyEmit()
        {
            delete m_pContentStream;
            delete m_pSoftMaskStream;
        }
    };

    // font subsets
    struct GlyphEmit
    {
        sal_Ucs     m_aUnicode;
        sal_uInt8   m_nSubsetGlyphID;
    };
    typedef std::map< sal_GlyphId, GlyphEmit > FontEmitMapping;
    struct FontEmit
    {
        sal_Int32           m_nFontID;
        FontEmitMapping     m_aMapping;

        FontEmit( sal_Int32 nID ) : m_nFontID( nID ) {}
    };
    typedef std::list< FontEmit > FontEmitList;
    struct Glyph
    {
        sal_Int32   m_nFontID;
        sal_uInt8   m_nSubsetGlyphID;
    };
    typedef std::map< long, Glyph > FontMapping;

    struct FontSubset
    {
        FontEmitList        m_aSubsets;
        FontMapping         m_aMapping;
    };
    typedef std::map< const ImplFontData*, FontSubset > FontSubsetData;

    struct EmbedCode
    {
        sal_Ucs             m_aUnicode;
        rtl::OString        m_aName;
    };
    struct EmbedEncoding
    {
        sal_Int32                       m_nFontID;
        std::vector< EmbedCode >        m_aEncVector;
        std::map< sal_Ucs, sal_Int8 >   m_aCMap;
    };
    struct EmbedFont
    {
        sal_Int32                       m_nNormalFontID;
        std::list< EmbedEncoding >      m_aExtendedEncodings;
    };
    typedef std::map< const ImplFontData*, EmbedFont > FontEmbedData;

    struct PDFDest
    {
        sal_Int32                   m_nPage;
        PDFWriter::DestAreaType     m_eType;
        Rectangle                   m_aRect;
    };

//--->i56629
    struct PDFNamedDest
    {
        rtl::OUString               m_aDestName;
        sal_Int32                   m_nPage;
        PDFWriter::DestAreaType     m_eType;
        Rectangle                   m_aRect;
    };
//<---

    struct PDFOutlineEntry
    {
        sal_Int32                   m_nParentID;
        sal_Int32                   m_nObject;
        sal_Int32                   m_nParentObject;
        sal_Int32                   m_nNextObject;
        sal_Int32                   m_nPrevObject;
        std::vector< sal_Int32 >    m_aChildren;
        rtl::OUString               m_aTitle;
        sal_Int32                   m_nDestID;

        PDFOutlineEntry()
                : m_nParentID( -1 ),
                  m_nObject( 0 ),
                  m_nParentObject( 0 ),
                  m_nNextObject( 0 ),
                  m_nPrevObject( 0 ),
                  m_nDestID( -1 )
        {}
    };

    struct PDFAnnotation
    {
        sal_Int32                   m_nObject;
        Rectangle                   m_aRect;
        sal_Int32                   m_nPage;

        PDFAnnotation()
                : m_nObject( -1 ),
                  m_nPage( -1 )
        {}
    };

    struct PDFLink : public PDFAnnotation
    {
        sal_Int32                   m_nDest; // set to -1 for URL, to a dest else
        rtl::OUString               m_aURL;
        sal_Int32                   m_nStructParent; // struct parent entry

        PDFLink()
                : m_nDest( -1 ),
                  m_nStructParent( -1 )
        {}
    };

    struct PDFNoteEntry : public PDFAnnotation
    {
        PDFNote                     m_aContents;

        PDFNoteEntry()
        {}
    };

    typedef std::hash_map< rtl::OString, SvMemoryStream*, rtl::OStringHash > PDFAppearanceStreams;
    typedef std::hash_map< rtl::OString, PDFAppearanceStreams, rtl::OStringHash > PDFAppearanceMap;

    struct PDFWidget : public PDFAnnotation
    {
        PDFWriter::WidgetType       m_eType;
        rtl::OString                m_aName;
        rtl::OUString               m_aDescription;
        rtl::OUString               m_aText;
        USHORT                      m_nTextStyle;
        rtl::OUString               m_aValue;
        rtl::OString                m_aDAString;
        rtl::OString                m_aMKDict;
        rtl::OString                m_aMKDictCAString;  // i12626, added to be able to encrypt the /CA text string
                                                        // since the object number is not known at the moment
                                                        // of filling m_aMKDict, the string will be encrypted when emitted.
                                                        // the /CA string MUST BE the last added to m_aMKDict
                                                        // see code for details
        sal_Int32                   m_nFlags;
        sal_Int32                   m_nParent; // if not 0, parent's object number
        std::vector<sal_Int32>      m_aKids; // widget children, contains object numbers
        std::vector<sal_Int32>      m_aKidsIndex; // widget children, contains index to m_aWidgets
        rtl::OUString               m_aOnValue;
        sal_Int32                   m_nTabOrder; // lowest number gets first in tab order
        sal_Int32                   m_nRadioGroup;
        sal_Int32                   m_nMaxLen;
        bool                        m_bSubmit;
        bool                        m_bSubmitGet;
        sal_Int32                   m_nDest;
        std::vector<rtl::OUString>  m_aListEntries;
        std::vector<sal_Int32>      m_aSelectedEntries;
        PDFAppearanceMap            m_aAppearances;
        PDFWidget()
                : m_eType( PDFWriter::PushButton ),
                  m_nTextStyle( 0 ),
                  m_nFlags( 0 ),
                  m_nParent( 0 ),
                  m_nRadioGroup( -1 ),
                  m_nMaxLen( 0 ),
                  m_bSubmit( false ),
                  m_bSubmitGet( false ),
                  m_nDest( -1 )
        {}
    };

    struct PDFStructureAttribute
    {
        PDFWriter::StructAttributeValue     eValue;
        sal_Int32                           nValue;

        PDFStructureAttribute()
                : eValue( PDFWriter::Invalid ),
                  nValue( 0 )
        {}

        PDFStructureAttribute( PDFWriter::StructAttributeValue eVal )
                : eValue( eVal ),
                  nValue( 0 )
        {}

        PDFStructureAttribute( sal_Int32 nVal )
                : eValue( PDFWriter::Invalid ),
                  nValue( nVal )
        {}
    };

    typedef std::map<PDFWriter::StructAttribute, PDFStructureAttribute > PDFStructAttributes;

    struct PDFStructureElementKid // for Kids entries
    {
        sal_Int32 nObject;  // an object number if nMCID is -1,
                            // else the page object relevant to MCID
        sal_Int32 nMCID;    // an MCID if >= 0

        PDFStructureElementKid( sal_Int32 nObj ) : nObject( nObj ), nMCID( -1 ) {}
        PDFStructureElementKid( sal_Int32 MCID, sal_Int32 nPage ) : nObject( nPage ), nMCID( MCID ) {}
    };

    struct PDFStructureElement
    {
        sal_Int32                                           m_nObject;
        PDFWriter::StructElement                            m_eType;
        rtl::OString                                        m_aAlias;
        sal_Int32                                           m_nOwnElement; // index into structure vector
        sal_Int32                                           m_nParentElement; // index into structure vector
        sal_Int32                                           m_nFirstPageObject;
        bool                                                m_bOpenMCSeq;
        std::list< sal_Int32 >                              m_aChildren; // indices into strucure vector
        std::list< PDFStructureElementKid >                 m_aKids;
        PDFStructAttributes                                 m_aAttributes;
        Rectangle                                           m_aBBox;
        rtl::OUString                                       m_aActualText;
        rtl::OUString                                       m_aAltText;
        com::sun::star::lang::Locale                        m_aLocale;

        // m_aContents contains the element's marked content sequence
        // as pairs of (page nr, MCID)

        PDFStructureElement()
                : m_nObject( 0 ),
                  m_eType( PDFWriter::NonStructElement ),
                  m_nOwnElement( -1 ),
                  m_nParentElement( -1 ),
                  m_nFirstPageObject( 0 ),
                  m_bOpenMCSeq( false )
        {
        }

    };

    struct PDFAddStream
    {
        rtl::OUString           m_aMimeType;
        PDFOutputStream*        m_pStream;
        sal_Int32               m_nStreamObject;
        bool                    m_bCompress;

        PDFAddStream() : m_pStream( NULL ), m_nStreamObject( 0 ), m_bCompress( true ) {}
    };


    // helper structure for drawLayout and friends
    struct PDFGlyph
    {
        Point       m_aPos;
        sal_Int32   m_nNativeWidth;
        sal_Int32   m_nGlyphId;
        sal_Int32   m_nMappedFontId;
        sal_uInt8   m_nMappedGlyphId;

        PDFGlyph( const Point& rPos,
                  sal_Int32 nNativeWidth,
                  sal_Int32 nGlyphId,
                  sal_Int32 nFontId,
                  sal_uInt8 nMappedGlyphId )
        : m_aPos( rPos ), m_nNativeWidth( nNativeWidth ), m_nGlyphId( nGlyphId ),
          m_nMappedFontId( nFontId ), m_nMappedGlyphId( nMappedGlyphId )
        {}
    };


    static const sal_Char* getStructureTag( PDFWriter::StructElement );
    static const sal_Char* getAttributeTag( PDFWriter::StructAttribute eAtr );
    static const sal_Char* getAttributeValueTag( PDFWriter::StructAttributeValue eVal );

    // returns true if compression was done
    // else false
    static bool compressStream( SvMemoryStream* );

    static void convertLineInfoToExtLineInfo( const LineInfo& rIn, PDFWriter::ExtLineInfo& rOut );
private:
    static const BuiltinFont m_aBuiltinFonts[14];

    OutputDevice*                       m_pReferenceDevice;

    MapMode                             m_aMapMode; // PDFWriterImpl scaled units
    std::vector< PDFPage >              m_aPages;
    PDFDocInfo                          m_aDocInfo;
    /* maps object numbers to file offsets (needed for xref) */
    std::vector< sal_uInt64 >           m_aObjects;
    /* contains Bitmaps until they are written to the
     *  file stream as XObjects*/
    std::list< BitmapEmit >             m_aBitmaps;
    /* contains JPG streams until written to file     */
    std::list<JPGEmit>                  m_aJPGs;
    /*--->i56629 contains all named destinations ever set during the PDF creation,
       destination id is always the destination's position in this vector
     */
    std::vector<PDFNamedDest>           m_aNamedDests;
    //<---
    /* contains all dests ever set during the PDF creation,
       dest id is always the dest's position in this vector
     */
    std::vector<PDFDest>                m_aDests;
    /* contains all links ever set during PDF creation,
       link id is always the link's position in this vector
    */
    std::vector<PDFLink>                m_aLinks;
    /* makes correctly encoded for export to PDF URLS
    */
    com::sun::star::uno::Reference< com::sun::star::util::XURLTransformer > m_xTrans;
    /* maps arbitrary link ids for structure attributes to real link ids
       (for setLinkPropertyId)
    */
    std::map<sal_Int32, sal_Int32>      m_aLinkPropertyMap;
    /* contains all outline items,
       object 0 is the outline root
     */
    std::vector<PDFOutlineEntry>        m_aOutline;
    /* contains all notes set during PDF creation
     */
    std::vector<PDFNoteEntry>           m_aNotes;
    /* the root of the structure tree
     */
    std::vector<PDFStructureElement>    m_aStructure;
    /* current object in the structure hierarchy
     */
    sal_Int32                           m_nCurrentStructElement;
    /* structure parent tree */
    std::vector< rtl::OString >         m_aStructParentTree;
    /* emit strucure marks currently (aka. NonStructElement or not)
     */
    bool                                m_bEmitStructure;
    bool                                m_bNewMCID;
    /* role map of struct tree root */
    std::hash_map< rtl::OString, rtl::OString, rtl::OStringHash >
                                        m_aRoleMap;

    /* contains all widgets used in the PDF
     */
    std::vector<PDFWidget>              m_aWidgets;
    /* maps radio group id to index of radio group control in m_aWidgets */
    std::map< sal_Int32, sal_Int32 >    m_aRadioGroupWidgets;
    /* used to store control id during beginControlAppearance/endControlAppearance */
    sal_Int32                           m_nCurrentControl;
    /* hash_map for field names, used to ensure unique field names */
    std::hash_map< rtl::OString, sal_Int32, rtl::OStringHash > m_aFieldNameMap;

    /* contains Bitmaps for gradient functions until they are written
     *  to the file stream */
    std::list< GradientEmit >           m_aGradients;
    /* contains bitmap tiling patterns */
    std::vector< TilingEmit >           m_aTilings;
    std::list< TransparencyEmit >       m_aTransparentObjects;
    /*  contains all font subsets in use */
    FontSubsetData                      m_aSubsets;
    bool                                m_bEmbedStandardFonts;
    FontEmbedData                       m_aEmbeddedFonts;
    sal_Int32                           m_nNextFID;
    PDFFontCache                        m_aFontCache;

    sal_Int32                           m_nInheritedPageWidth;  // in inch/72
    sal_Int32                           m_nInheritedPageHeight; // in inch/72
    PDFWriter::Orientation              m_eInheritedOrientation;
    sal_Int32                           m_nCurrentPage;

    sal_Int32                           m_nCatalogObject;
    sal_Int32                           m_nResourceDict;
    ResourceDict                        m_aGlobalResourceDict;
    sal_Int32                           m_nFontDictObject;
    std::map< sal_Int32, sal_Int32 >    m_aBuiltinFontToObjectMap;

    PDFWriter::PDFWriterContext         m_aContext;
    oslFileHandle                       m_aFile;
    bool                                m_bOpen;


    /* output redirection; e.g. to accumulate content streams for
       XObjects
     */
    struct StreamRedirect
    {
        SvStream*       m_pStream;
        MapMode         m_aMapMode;
        Rectangle       m_aTargetRect;
        ResourceDict    m_aResourceDict;
    };
    std::list< StreamRedirect >         m_aOutputStreams;

    // graphics state
    struct GraphicsState
    {
        Font            m_aFont;
        MapMode         m_aMapMode;
        Color           m_aLineColor;
        Color           m_aFillColor;
        Color           m_aTextLineColor;
        Region          m_aClipRegion;
        sal_Int32       m_nAntiAlias;
        sal_Int32       m_nLayoutMode;
        LanguageType    m_aDigitLanguage;
        sal_Int32       m_nTransparentPercent;
        sal_uInt16      m_nFlags;
        sal_uInt16      m_nUpdateFlags;

        static const sal_uInt16 updateFont                  = 0x0001;
        static const sal_uInt16 updateMapMode               = 0x0002;
        static const sal_uInt16 updateLineColor             = 0x0004;
        static const sal_uInt16 updateFillColor             = 0x0008;
        static const sal_uInt16 updateTextLineColor         = 0x0010;
        static const sal_uInt16 updateClipRegion            = 0x0020;
        static const sal_uInt16 updateAntiAlias             = 0x0040;
        static const sal_uInt16 updateLayoutMode            = 0x0080;
        static const sal_uInt16 updateTransparentPercent    = 0x0100;
        static const sal_uInt16 updateDigitLanguage         = 0x0200;

        GraphicsState() :
                m_aLineColor( COL_TRANSPARENT ),
                m_aFillColor( COL_TRANSPARENT ),
                m_aTextLineColor( COL_TRANSPARENT ),
                m_nAntiAlias( 1 ),
                m_nLayoutMode( 0 ),
                m_nTransparentPercent( 0 ),
                m_nFlags( 0xffff ),
                m_nUpdateFlags( 0xffff )
        {}
        GraphicsState( const GraphicsState& rState ) :
                m_aFont( rState.m_aFont ),
                m_aMapMode( rState.m_aMapMode ),
                m_aLineColor( rState.m_aLineColor ),
                m_aFillColor( rState.m_aFillColor ),
                m_aTextLineColor( rState.m_aTextLineColor ),
                m_aClipRegion( rState.m_aClipRegion ),
                m_nAntiAlias( rState.m_nAntiAlias ),
                m_nLayoutMode( rState.m_nLayoutMode ),
                m_nTransparentPercent( rState.m_nTransparentPercent ),
                m_nFlags( rState.m_nFlags ),
                m_nUpdateFlags( rState.m_nUpdateFlags )
        {
        }

        GraphicsState& operator=(const GraphicsState& rState )
        {
            m_aFont                 = rState.m_aFont;
            m_aMapMode              = rState.m_aMapMode;
            m_aLineColor            = rState.m_aLineColor;
            m_aFillColor            = rState.m_aFillColor;
            m_aTextLineColor        = rState.m_aTextLineColor;
            m_aClipRegion           = rState.m_aClipRegion;
            m_nAntiAlias            = rState.m_nAntiAlias;
            m_nLayoutMode           = rState.m_nLayoutMode;
            m_nTransparentPercent   = rState.m_nTransparentPercent;
            m_nFlags                = rState.m_nFlags;
            m_nUpdateFlags          = rState.m_nUpdateFlags;
            return *this;
        }
    };
    std::list< GraphicsState >              m_aGraphicsStack;
    GraphicsState                           m_aCurrentPDFState;

    ZCodec*                                 m_pCodec;
    SvMemoryStream*                         m_pMemStream;

    std::vector< PDFAddStream >             m_aAdditionalStreams;
    std::set< PDFWriter::ErrorCode >        m_aErrors;

    rtlDigest                               m_aDocDigest;

/*
variables for PDF security
i12626
*/
/* used to cipher the stream data and for password management */
    rtlCipher                               m_aCipher;
    rtlDigest                               m_aDigest;
/* pad string used for password in Standard security handler */
    sal_uInt8                               m_nPadString[ENCRYPTED_PWD_SIZE];
/* the owner password, in clear text */
    rtl::OUString                           m_aOwnerPassword;
/* the padded owner password */
    sal_uInt8                               m_nPaddedOwnerPassword[ENCRYPTED_PWD_SIZE];
/* the encryption dictionary owner password, according to algorithm 3.3 */
    sal_uInt8                               m_nEncryptedOwnerPassword[ENCRYPTED_PWD_SIZE];
/* the user password, in clear text */
    rtl::OUString                           m_aUserPassword;
/* the padded user password */
    sal_uInt8                               m_nPaddedUserPassword[ENCRYPTED_PWD_SIZE];
/* the encryption dictionary user password, according to algorithm 3.4 or 3.5 depending on the
   security handler revision */
    sal_uInt8                               m_nEncryptedUserPassword[ENCRYPTED_PWD_SIZE];

/* the encryption key, formed with the user password according to algorithm 3.2, maximum length is 16 bytes + 3 + 2
   for 128 bit security   */
    sal_uInt8                               m_nEncryptionKey[MAXIMUM_RC4_KEY_LENGTH];
    sal_Int32                               m_nKeyLength; // key length, 16 or 5
    sal_Int32                               m_nRC4KeyLength; // key length, 16 or 10, to be input to the algorith 3.1

/* set to true if the following stream must be encrypted, used inside writeBuffer() */
    sal_Bool                                m_bEncryptThisStream;

/* the numerical value of the access permissions, according to PDF spec, must be signed */
    sal_Int32                               m_nAccessPermissions;
/* the document ID, the raw MD5 hash */
    sal_uInt8                               m_nDocID[MD5_DIGEST_SIZE];
/* string buffer to hold document ID, this is the output string */
    rtl::OStringBuffer                      m_aDocID;
/* string to hold the PDF creation date */
    rtl::OStringBuffer                      m_aCreationDateString;
/* string to hold the PDF creation date, for PDF/A metadata */
    rtl::OStringBuffer                      m_aCreationMetaDateString;
/* the buffer where the data are encrypted, dynamically allocated */
    sal_uInt8                               *m_pEncryptionBuffer;
/* size of the buffer */
    sal_Int32                               m_nEncryptionBufferSize;

/* check and reallocate the buffer for encryption */
    sal_Bool checkEncryptionBufferSize( register sal_Int32 newSize )
        {
            if( m_nEncryptionBufferSize < newSize )
            {
/* reallocate the buffer, the used function allocate as rtl_allocateMemory
   if the pointer parameter is NULL */
                m_pEncryptionBuffer = (sal_uInt8*)rtl_reallocateMemory( m_pEncryptionBuffer, newSize );
                if( m_pEncryptionBuffer )
                    m_nEncryptionBufferSize = newSize;
                else
                    m_nEncryptionBufferSize = 0;
            }
            return ( m_nEncryptionBufferSize != 0 );
        }
/* init the internal pad string */
    void initPadString()
        {
            static const sal_uInt8 nPadString[32] =
                {
                    0x28, 0xBF, 0x4E, 0x5E, 0x4E, 0x75, 0x8A, 0x41, 0x64, 0x00, 0x4E, 0x56, 0xFF, 0xFA, 0x01, 0x08,
                    0x2E, 0x2E, 0x00, 0xB6, 0xD0, 0x68, 0x3E, 0x80, 0x2F, 0x0C, 0xA9, 0xFE, 0x64, 0x53, 0x69, 0x7A
                };

            for(sal_uInt32 i = 0; i < sizeof( nPadString ); i++ )
                m_nPadString[i] = nPadString[i];

        };
/* initialize the encryption engine */
    void initEncryption();

/* this function implements part of the PDF spec algorithm 3.1 in encryption, the rest (the actual encryption) is in PDFWriterImpl::writeBuffer */
    void checkAndEnableStreamEncryption( register sal_Int32 nObject )
        {
            if( m_aContext.Encrypt )
            {
                m_bEncryptThisStream = true;
                register sal_Int32 i = m_nKeyLength;
                m_nEncryptionKey[i++] = (sal_uInt8)nObject;
                m_nEncryptionKey[i++] = (sal_uInt8)( nObject >> 8 );
                m_nEncryptionKey[i++] = (sal_uInt8)( nObject >> 16 );
//the other location of m_nEncryptionKey are already set to 0, our fixed generation number
// do the MD5 hash
                sal_uInt8 nMD5Sum[ RTL_DIGEST_LENGTH_MD5 ];
                // the i+2 to take into account the generation number, always zero
                rtl_digest_MD5( &m_nEncryptionKey, i+2, nMD5Sum, sizeof(nMD5Sum) );
// initialize the RC4 with the key
// key legth: see algoritm 3.1, step 4: (N+5) max 16
                rtl_cipher_initARCFOUR( m_aCipher, rtl_Cipher_DirectionEncode, nMD5Sum, m_nRC4KeyLength, NULL, 0 );
            }
        };

    void disableStreamEncryption() { m_bEncryptThisStream = false; };

/* */
    void enableStringEncryption( register sal_Int32 nObject )
        {
            register sal_Int32 i = m_nKeyLength;
            m_nEncryptionKey[i++] = (sal_uInt8)nObject;
            m_nEncryptionKey[i++] = (sal_uInt8)( nObject >> 8 );
            m_nEncryptionKey[i++] = (sal_uInt8)( nObject >> 16 );
//the other location of m_nEncryptionKey are already set to 0, our fixed generation number
// do the MD5 hash
            sal_uInt8 nMD5Sum[ RTL_DIGEST_LENGTH_MD5 ];
            // the i+2 to take into account the generation number, always zero
            rtl_digest_MD5( &m_nEncryptionKey, i+2, nMD5Sum, sizeof(nMD5Sum) );
// initialize the RC4 with the key
// key legth: see algoritm 3.1, step 4: (N+5) max 16
            rtl_cipher_initARCFOUR( m_aCipher, rtl_Cipher_DirectionEncode, nMD5Sum, m_nRC4KeyLength, NULL, 0 );
        };

// test if the encryption is active, if yes than encrypt the unicode string  and add to the OStringBuffer parameter
    void appendUnicodeTextStringEncrypt( const rtl::OUString& rInString, const sal_Int32 nInObjectNumber, rtl::OStringBuffer& rOutBuffer );

    void appendLiteralStringEncrypt( const rtl::OUString& rInString, const sal_Int32 nInObjectNumber, rtl::OStringBuffer& rOutBuffer );
    void appendLiteralStringEncrypt( const rtl::OString& rInString, const sal_Int32 nInObjectNumber, rtl::OStringBuffer& rOutBuffer );
    void appendLiteralStringEncrypt( rtl::OStringBuffer& rInString, const sal_Int32 nInObjectNumber, rtl::OStringBuffer& rOutBuffer );

    /* creates fonts and subsets that will be emitted later */
    void registerGlyphs( int nGlyphs, sal_GlyphId* pGlyphs, sal_Int32* pGlpyhWidths, sal_Ucs* pUnicodes, sal_uInt8* pMappedGlyphs, sal_Int32* pMappedFontObjects, const ImplFontData* pFallbackFonts[] );

    /*  emits a text object according to the passed layout */
    /* TODO: remove rText as soon as SalLayout will change so that rText is not necessary anymore */
    void drawVerticalGlyphs( const std::vector<PDFGlyph>& rGlyphs, rtl::OStringBuffer& rLine, const Point& rAlignOffset, const Matrix3& rRotScale, double fAngle, double fXScale, double fSkew, sal_Int32 nFontHeight );
    void drawHorizontalGlyphs( const std::vector<PDFGlyph>& rGlyphs, rtl::OStringBuffer& rLine, const Point& rAlignOffset, double fAngle, double fXScale, double fSkew, sal_Int32 nFontHeight, sal_Int32 nPixelFontHeight );
    void drawLayout( SalLayout& rLayout, const String& rText, bool bTextLines );
    void drawRelief( SalLayout& rLayout, const String& rText, bool bTextLines );
    void drawShadow( SalLayout& rLayout, const String& rText, bool bTextLines );

    /*  writes differences between graphics stack and current real PDF
     *   state to the file
     */
    void updateGraphicsState();

    /* writes a transparency group object */
    bool writeTransparentObject( TransparencyEmit& rObject );

    /* writes an XObject of type image, may create
       a second for the mask
     */
    bool writeBitmapObject( BitmapEmit& rObject, bool bMask = false );

    bool writeJPG( JPGEmit& rEmit );

    /* tries to find the bitmap by its id and returns its emit data if exists,
       else creates a new emit data block */
    const BitmapEmit& createBitmapEmit( const BitmapEx& rBitmapEx, bool bDrawMask = false );

    /* writes the Do operation inside the content stream */
    void drawBitmap( const Point& rDestPt, const Size& rDestSize, const BitmapEmit& rBitmap, const Color& rFillColor );
    /* write the function object for a Gradient */
    bool writeGradientFunction( GradientEmit& rObject );
    /* creates a GradientEmit and returns its object number */
    sal_Int32 createGradient(  const Gradient& rGradient, const Size& rSize );

    /* writes all tilings */
    bool emitTilings();
    /* writes all gradient patterns */
    bool emitGradients();
    /* writes a builtin font object and returns its objectid (or 0 in case of failure ) */
    sal_Int32 emitBuiltinFont( const ImplFontData*, sal_Int32 nObject = -1 );
    /* writes a type1 embedded font object and returns its mapping from font ids to object ids (or 0 in case of failure ) */
    std::map< sal_Int32, sal_Int32 > emitEmbeddedFont( const ImplFontData*, EmbedFont& );
    /* writes a font descriptor and returns its object id (or 0) */
    sal_Int32 emitFontDescriptor( const ImplFontData*, FontSubsetInfo&, sal_Int32 nSubsetID, sal_Int32 nStream );
    /* writes a ToUnicode cmap, returns the corresponding stream object */
    sal_Int32 createToUnicodeCMap( sal_uInt8* pEncoding, sal_Ucs* pUnicodes, int nGlyphs );

    /* get resource dict object number */
    sal_Int32 getResourceDictObj()
    {
        if( m_nResourceDict <= 0 )
            m_nResourceDict = createObject();
        return m_nResourceDict;
    }
    /* get the font dict object */
    sal_Int32 getFontDictObject()
    {
        if( m_nFontDictObject <= 0 )
            m_nFontDictObject = createObject();
        return m_nFontDictObject;
    }
    /* push resource into current (redirected) resource dict */
    void pushResource( ResourceKind eKind, const rtl::OString& rResource, sal_Int32 nObject );

    void appendBuiltinFontsToDict( rtl::OStringBuffer& rDict ) const;
    /* writes a the font dictionary and emits all font objects
     * returns object id of font directory (or 0 on error)
     */
    bool emitFonts();
    /* writes the Resource dictionary;
     * returns dict object id (or 0 on error)
     */
    sal_Int32 emitResources();
    // appends a dest
    bool appendDest( sal_Int32 nDestID, rtl::OStringBuffer& rBuffer );
    // write all links
    bool emitLinkAnnotations();
    // write all notes
    bool emitNoteAnnotations();
    // write the appearance streams of a widget
    bool emitAppearances( PDFWidget& rWidget, rtl::OStringBuffer& rAnnotDict );
    // clean up radio button "On" values
    void ensureUniqueRadioOnValues();
    // write all widgets
    bool emitWidgetAnnotations();
    // writes all annotation objects
    bool emitAnnotations();
    // writes the dest dict for the catalog
    sal_Int32 emitDestDict();
    //write the named destination stuff
    sal_Int32 emitNamedDestinations();//i56629
    // writes outline dict and tree
    sal_Int32 emitOutline();
    // puts the attribute objects of a structure element into the returned string,
    // helper for emitStructure
    rtl::OString emitStructureAttributes( PDFStructureElement& rEle );
    // writes document structure
    sal_Int32 emitStructure( PDFStructureElement& rEle );
    // writes structure parent tree
    sal_Int32 emitStructParentTree( sal_Int32 nTreeObject );
    // writes page tree and catalog
    bool emitCatalog();
    // writes xref and trailer
    bool emitTrailer();
    // emit additional streams collected; also create there object numbers
    bool emitAdditionalStreams();
    // emits info dict (if applicable)
    sal_Int32 emitInfoDict( );

    // acrobat reader 5 and 6 use the order of the annotations
    // as their tab order; since PDF1.5 one can make the
    // tab order explicit by using the structure tree
    void sortWidgets();

    // updates the count numbers of outline items
    sal_Int32 updateOutlineItemCount( std::vector< sal_Int32 >& rCounts,
                                      sal_Int32 nItemLevel,
                                      sal_Int32 nCurrentItemId );
    // default appearences for widgets
    sal_Int32 findRadioGroupWidget( const PDFWriter::RadioButtonWidget& rRadio );
    Font replaceFont( const Font& rControlFont, const Font& rAppSetFont );
    sal_Int32 getBestBuiltinFont( const Font& rFont );

    // used for edit and listbox
    Font drawFieldBorder( PDFWidget&, const PDFWriter::AnyWidget&, const StyleSettings& );

    void createDefaultPushButtonAppearance( PDFWidget&, const PDFWriter::PushButtonWidget& rWidget );
    void createDefaultCheckBoxAppearance( PDFWidget&, const PDFWriter::CheckBoxWidget& rWidget );
    void createDefaultRadioButtonAppearance( PDFWidget&, const PDFWriter::RadioButtonWidget& rWidget );
    void createDefaultEditAppearance( PDFWidget&, const PDFWriter::EditWidget& rWidget );
    void createDefaultListBoxAppearance( PDFWidget&, const PDFWriter::ListBoxWidget& rWidget );

    /* ensure proper escapement and uniqueness of field names */
    rtl::OString convertWidgetFieldName( const rtl::OUString& rString );
    /* adds an entry to m_aObjects and returns its index+1,
     * sets the offset to ~0
     */
    sal_Int32 createObject();
    /* sets the offset of object n to the current position of output file+1
     */
    bool updateObject( sal_Int32 n );

    bool writeBuffer( const void* pBuffer, sal_uInt64 nBytes );
    void beginCompression();
    void endCompression();
    void beginRedirect( SvStream* pStream, const Rectangle& );
    // returns an empty rect if no redirection is happending
    Rectangle getRedirectTargetRect() const;
    SvStream* endRedirect();

    void endPage();

    void beginStructureElementMCSeq();
    void endStructureElementMCSeq();
    /** checks whether a non struct element lies in the ancestor hierarchy
        of the current structure element

        @returns
        <true/> if no NonStructElement was found in ancestor path and tagged
        PDF output is enabled
        <false/> else
     */
    bool checkEmitStructure();

    /* draws an emphasis mark */
    void drawEmphasisMark(  long nX, long nY, const PolyPolygon& rPolyPoly, BOOL bPolyLine, const Rectangle& rRect1, const Rectangle& rRect2 );

    /* true if PDF/A-1a or PDF/A-1b is output */
    sal_Bool        m_bIsPDF_A1;

/*
i12626
methods for PDF security

 pad a password according  algorithm 3.2, step 1 */
    void padPassword( const rtl::OUString aPassword, sal_uInt8 *paPasswordTarget );
/* algorithm 3.2: compute an encryption key */
    void computeEncryptionKey( sal_uInt8 *paThePaddedPassword, sal_uInt8 *paEncryptionKey );
/* algorithm 3.3: computing the encryption dictionary'ss owner password value ( /O ) */
    void computeODictionaryValue();
/* algorithm 3.4 or 3.5: computing the encryption dictionary's user password value ( /U ) revision 2 or 3 of the standard security handler */
    void computeUDictionaryValue();

public:
    PDFWriterImpl( const PDFWriter::PDFWriterContext& rContext );
    ~PDFWriterImpl();

    /*  for OutputDevice so the reference device can have a list
     *  that contains only suitable fonts (subsettable or builtin)
     *  produces a new font list
     */
    ImplDevFontList* filterDevFontList( ImplDevFontList* pFontList );
    /*  for OutputDevice: get layout for builtin fonts
     */
    bool isBuiltinFont( const ImplFontData* ) const;
    SalLayout* GetTextLayout( ImplLayoutArgs& rArgs, ImplFontSelectData* pFont );
    void getFontMetric( ImplFontSelectData* pFont, ImplFontMetricData* pMetric ) const;


    /* for documentation of public functions please see pdfwriter.hxx */

    OutputDevice* getReferenceDevice();

    /* document structure */
    sal_Int32 newPage( sal_Int32 nPageWidth , sal_Int32 nPageHeight, PDFWriter::Orientation eOrientation );
    bool emit();
    std::set< PDFWriter::ErrorCode > getErrors();

    PDFWriter::PDFVersion getVersion() const { return m_aContext.Version; }
    void setDocInfo( const PDFDocInfo& rInfo );
    const PDFDocInfo& getDocInfo() const { return m_aDocInfo; }

    void setDocumentLocale( const com::sun::star::lang::Locale& rLoc )
    { m_aContext.DocumentLocale = rLoc; }


    /* graphics state */
    void push( sal_uInt16 nFlags );
    void pop();

    void setFont( const Font& rFont );

    void setMapMode( const MapMode& rMapMode );
    void setMapMode() { setMapMode( m_aMapMode ); }


    const MapMode& getMapMode() { return m_aGraphicsStack.front().m_aMapMode; }

    void setLineColor( const Color& rColor )
    {
        m_aGraphicsStack.front().m_aLineColor = ImplIsColorTransparent(rColor) ? Color( COL_TRANSPARENT ) : rColor;
        m_aGraphicsStack.front().m_nUpdateFlags |= GraphicsState::updateLineColor;
    }

    void setFillColor( const Color& rColor )
    {
        m_aGraphicsStack.front().m_aFillColor = ImplIsColorTransparent(rColor) ? Color( COL_TRANSPARENT ) : rColor;
        m_aGraphicsStack.front().m_nUpdateFlags |= GraphicsState::updateFillColor;
    }

    void setTextLineColor()
    {
        m_aGraphicsStack.front().m_aTextLineColor = Color( COL_TRANSPARENT );
        m_aGraphicsStack.front().m_nUpdateFlags |= GraphicsState::updateTextLineColor;
    }

    void setTextLineColor( const Color& rColor )
    {
        m_aGraphicsStack.front().m_aTextLineColor = rColor;
        m_aGraphicsStack.front().m_nUpdateFlags |= GraphicsState::updateTextLineColor;
    }

    void setTextFillColor( const Color& rColor )
    {
        m_aGraphicsStack.front().m_aFont.SetFillColor( rColor );
        m_aGraphicsStack.front().m_aFont.SetTransparent( ImplIsColorTransparent( rColor ) ? TRUE : FALSE );
        m_aGraphicsStack.front().m_nUpdateFlags |= GraphicsState::updateFont;
    }
    void setTextFillColor()
    {
        m_aGraphicsStack.front().m_aFont.SetFillColor( Color( COL_TRANSPARENT ) );
        m_aGraphicsStack.front().m_aFont.SetTransparent( TRUE );
        m_aGraphicsStack.front().m_nUpdateFlags |= GraphicsState::updateFont;
    }
    void setTextColor( const Color& rColor )
    {
        m_aGraphicsStack.front().m_aFont.SetColor( rColor );
        m_aGraphicsStack.front().m_nUpdateFlags |= GraphicsState::updateFont;
    }

    void clearClipRegion()
    {
        m_aGraphicsStack.front().m_aClipRegion.SetNull();
        m_aGraphicsStack.front().m_nUpdateFlags |= GraphicsState::updateClipRegion;
    }

    void setClipRegion( const Region& rRegion );

    void moveClipRegion( sal_Int32 nX, sal_Int32 nY );

    bool intersectClipRegion( const Rectangle& rRect );

    bool intersectClipRegion( const Region& rRegion );

    void setLayoutMode( sal_Int32 nLayoutMode )
    {
        m_aGraphicsStack.front().m_nLayoutMode = nLayoutMode;
        m_aGraphicsStack.front().m_nUpdateFlags |= GraphicsState::updateLayoutMode;
    }

    void setDigitLanguage( LanguageType eLang )
    {
        m_aGraphicsStack.front().m_aDigitLanguage = eLang;
        m_aGraphicsStack.front().m_nUpdateFlags |= GraphicsState::updateDigitLanguage;
    }

    void setTextAlign( TextAlign eAlign )
    {
        m_aGraphicsStack.front().m_aFont.SetAlign( eAlign );
        m_aGraphicsStack.front().m_nUpdateFlags |= GraphicsState::updateFont;
    }

    void setAntiAlias( sal_Int32 nAntiAlias )
    {
        m_aGraphicsStack.front().m_nAntiAlias = nAntiAlias;
        m_aGraphicsStack.front().m_nUpdateFlags |= GraphicsState::updateAntiAlias;
    }

    /* actual drawing functions */
    void drawText( const Point& rPos, const String& rText, xub_StrLen nIndex = 0, xub_StrLen nLen = STRING_LEN, bool bTextLines = true );
    void drawTextArray( const Point& rPos, const String& rText, const sal_Int32* pDXArray = NULL, xub_StrLen nIndex = 0, xub_StrLen nLen = STRING_LEN, bool bTextLines = true );
    void drawStretchText( const Point& rPos, ULONG nWidth, const String& rText,
                          xub_StrLen nIndex = 0, xub_StrLen nLen = STRING_LEN,
                          bool bTextLines = true  );
    void drawText( const Rectangle& rRect, const String& rOrigStr, USHORT nStyle, bool bTextLines = true  );
    void drawTextLine( const Point& rPos, long nWidth, FontStrikeout eStrikeout, FontUnderline eUnderline, bool bUnderlineAbove );

    void drawLine( const Point& rStart, const Point& rStop );
    void drawLine( const Point& rStart, const Point& rStop, const LineInfo& rInfo );
    void drawPolygon( const Polygon& rPoly );
    void drawPolyPolygon( const PolyPolygon& rPolyPoly );
    void drawPolyLine( const Polygon& rPoly );
    void drawPolyLine( const Polygon& rPoly, const LineInfo& rInfo );
    void drawPolyLine( const Polygon& rPoly, const PDFWriter::ExtLineInfo& rInfo );
    void drawWaveLine( const Point& rStart, const Point& rStop, sal_Int32 nDelta, sal_Int32 nLineWidth );

    void drawPixel( const Point& rPt, const Color& rColor );
    void drawPixel( const Polygon& rPts, const Color* pColors = NULL );

    void drawRectangle( const Rectangle& rRect );
    void drawRectangle( const Rectangle& rRect, sal_uInt32 nHorzRound, sal_uInt32 nVertRound );
    void drawEllipse( const Rectangle& rRect );
    void drawArc( const Rectangle& rRect, const Point& rStart, const Point& rStop, bool bWithPie, bool bWidthChord );

    void drawBitmap( const Point& rDestPoint, const Size& rDestSize, const Bitmap& rBitmap );
    void drawBitmap( const Point& rDestPoint, const Size& rDestSize, const BitmapEx& rBitmap );
    void drawMask( const Point& rDestPoint, const Size& rDestSize, const Bitmap& rBitmap, const Color& rFillColor );
    void drawJPGBitmap( SvStream& rDCTData, bool bIsTrueColor, const Size& rSizePixel, const Rectangle& rTargetArea, const Bitmap& rMask );

    void drawGradient( const Rectangle& rRect, const Gradient& rGradient );
    void drawGradient( const PolyPolygon& rPolyPoly, const Gradient& rGradient );
    void drawHatch( const PolyPolygon& rPolyPoly, const Hatch& rHatch );
    void drawWallpaper( const Rectangle& rRect, const Wallpaper& rWall );
    void drawTransparent( const PolyPolygon& rPolyPoly, sal_uInt32 nTransparentPercent );
    void beginTransparencyGroup();
    void endTransparencyGroup( const Rectangle& rBoundingBox, sal_uInt32 nTransparentPercent );
    void endTransparencyGroup( const Rectangle& rBoundingBox, const Bitmap& rAlphaMask );
    void beginPattern( const Rectangle& rCell );
    sal_Int32 endPattern( const SvtGraphicFill::Transform& rTransform );
    void drawPolyPolygon( const PolyPolygon& rPolyPoly, sal_Int32 nPattern, bool bEOFill );

    void emitComment( const char* pComment );

    //--->i56629 named destinations
    sal_Int32 createNamedDest( const rtl::OUString& sDestName, const Rectangle& rRect, sal_Int32 nPageNr = -1, PDFWriter::DestAreaType eType = PDFWriter::XYZ );

    //--->i59651
    //emits output intent
    sal_Int32   emitOutputIntent();

    //emits the document metadata
    sal_Int32   emitDocumentMetadata();

    // links
    sal_Int32 createLink( const Rectangle& rRect, sal_Int32 nPageNr = -1 );
    sal_Int32 createDest( const Rectangle& rRect, sal_Int32 nPageNr = -1, PDFWriter::DestAreaType eType = PDFWriter::XYZ );
    sal_Int32 setLinkDest( sal_Int32 nLinkId, sal_Int32 nDestId );
    sal_Int32 setLinkURL( sal_Int32 nLinkId, const rtl::OUString& rURL );
    void setLinkPropertyId( sal_Int32 nLinkId, sal_Int32 nPropertyId );

    // outline
    sal_Int32 createOutlineItem( sal_Int32 nParent = 0, const rtl::OUString& rText = rtl::OUString(), sal_Int32 nDestID = -1 );
    sal_Int32 setOutlineItemParent( sal_Int32 nItem, sal_Int32 nNewParent );
    sal_Int32 setOutlineItemText( sal_Int32 nItem, const rtl::OUString& rText );
    sal_Int32 setOutlineItemDest( sal_Int32 nItem, sal_Int32 nDestID );

    // notes
    void createNote( const Rectangle& rRect, const PDFNote& rNote, sal_Int32 nPageNr = -1 );
    // structure elements
    sal_Int32 beginStructureElement( PDFWriter::StructElement eType, const rtl::OUString& rAlias );
    void endStructureElement();
    bool setCurrentStructureElement( sal_Int32 nElement );
    sal_Int32 getCurrentStructureElement();
    bool setStructureAttribute( enum PDFWriter::StructAttribute eAttr, enum PDFWriter::StructAttributeValue eVal );
    bool setStructureAttributeNumerical( enum PDFWriter::StructAttribute eAttr, sal_Int32 nValue );
    void setStructureBoundingBox( const Rectangle& rRect );
    void setActualText( const String& rText );
    void setAlternateText( const String& rText );

    // transitional effects
    void setAutoAdvanceTime( sal_uInt32 nSeconds, sal_Int32 nPageNr = -1 );
    void setPageTransition( PDFWriter::PageTransition eType, sal_uInt32 nMilliSec, sal_Int32 nPageNr = -1 );

    // controls
    sal_Int32 createControl( const PDFWriter::AnyWidget& rControl, sal_Int32 nPageNr = -1 );
    void beginControlAppearance( sal_Int32 nControl );
    bool endControlAppearance( PDFWriter::WidgetState eState );

    // additional streams
    void addStream( const String& rMimeType, PDFOutputStream* pStream, bool bCompress );

    // helper: eventually begin marked content sequence and
    // emit a comment in debug case
    void MARK( const char*
#if OSL_DEBUG_LEVEL > 1
        pString
#endif
        )
    {
        beginStructureElementMCSeq();
#if OSL_DEBUG_LEVEL > 1
        emitComment( pString );
#endif
    }
};

}

#endif //_VCL_PDFEXPORT_HXX


