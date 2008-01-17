/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: headerfooterparser.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:06:08 $
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

#include "oox/xls/headerfooterparser.hxx"
#include <vector>
#include <set>
#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sheet/XHeaderFooterContent.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/FilenameDisplayFormat.hpp>
#include "oox/helper/attributelist.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/xls/stylesbuffer.hxx"
#include "oox/xls/themebuffer.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::rtl::OString;
using ::rtl::OStringBuffer;
using ::rtl::OUStringToOString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::sheet::XHeaderFooterContent;
using ::com::sun::star::text::XText;
using ::com::sun::star::text::XTextCursor;
using ::com::sun::star::text::XTextContent;
using ::com::sun::star::text::XTextRange;

namespace oox {
namespace xls {

// ============================================================================

enum HFPortionId
{
    HF_LEFT,
    HF_CENTER,
    HF_RIGHT,
    HF_COUNT
};

// ----------------------------------------------------------------------------

struct HFPortionInfo
{
    Reference< XText >  mxText;                 /// XText interface of this portion.
    Reference< XTextCursor > mxStart;           /// Start position of current text range for formatting.
    Reference< XTextCursor > mxEnd;             /// End position of current text range for formatting.
    double              mfTotalHeight;          /// Sum of heights of previous lines in points.
    double              mfCurrHeight;           /// Height of the current text line in points.

    bool                initialize( const Reference< XText >& rxText );
};

bool HFPortionInfo::initialize( const Reference< XText >& rxText )
{
    mfTotalHeight = mfCurrHeight = 0.0;
    mxText = rxText;
    if( mxText.is() )
    {
        mxStart = mxText->createTextCursor();
        mxEnd = mxText->createTextCursor();
    }
    bool bRet = mxText.is() && mxStart.is() && mxEnd.is();
    OSL_ENSURE( bRet, "HFPortionInfo::initialize - missing interfaces" );
    return bRet;
}

// ----------------------------------------------------------------------------

class HeaderFooterParserImpl : public WorkbookHelper
{
public:
    explicit            HeaderFooterParserImpl( const WorkbookHelper& rHelper );

    /** Parses the passed string and creates the header/footer contents. */
    void                parse(
                            const Reference< XHeaderFooterContent >& rxContext,
                            const OUString& rData );

    /** Returns the total height of the converted header or footer in points. */
    double              getTotalHeight() const;

private:
    /** Returns the current edit engine text object. */
    inline HFPortionInfo& getPortion() { return maPortions[ meCurrPortion ]; }
    /** Returns the start cursor of the current text range. */
    inline const Reference< XTextCursor >& getStartPos() { return getPortion().mxStart; }
    /** Returns the end cursor of the current text range. */
    inline const Reference< XTextCursor >& getEndPos() { return getPortion().mxEnd; }

    /** Returns the current line height of the specified portion. */
    double              getCurrHeight( HFPortionId ePortion ) const;
    /** Returns the current line height. */
    double              getCurrHeight() const;

    /** Updates the current line height of the specified portion, using the current font size. */
    void                updateCurrHeight( HFPortionId ePortion );
    /** Updates the current line height, using the current font size. */
    void                updateCurrHeight();

    /** Sets the font attributes at the current selection. */
    void                setAttributes();
    /** Appends and clears internal string buffer. */
    void                appendText();
    /** Appends a line break and adjusts internal text height data. */
    void                appendLineBreak();

    /** Creates a text field from the passed service name. */
    Reference< XTextContent > createField( const OUString& rServiceName ) const;
    /** Appends the passed text field. */
    void                appendField( const Reference< XTextContent >& rxContent );

    /** Sets the passed font name if it is valid. */
    void                convertFontName( const OUString& rStyle );
    /** Converts a font style given as string. */
    void                convertFontStyle( const OUString& rStyle );
    /** Converts a font color given as string. */
    void                convertFontColor( const OUString& rColor );

    /** Finalizes current portion: sets font attributes and updates text height data. */
    void                finalizePortion();
    /** Changes current header/footer portion. */
    void                setNewPortion( HFPortionId ePortion );

private:
    typedef ::std::vector< HFPortionInfo >  HFPortionInfoVec;
    typedef ::std::set< OString >           OStringSet;

    OUString            maPageNumberService;
    OUString            maPageCountService;
    OUString            maSheetNameService;
    OUString            maFileNameService;
    OUString            maDateTimeService;
    OUString            maIsDateProp;
    OUString            maFileFormatProp;
    OStringSet          maBoldNames;            /// All names for bold font style in lowercase UTF-8.
    OStringSet          maItalicNames;          /// All names for italic font style in lowercase UTF-8.
    HFPortionInfoVec    maPortions;
    HFPortionId         meCurrPortion;          /// Identifier of current H/F portion.
    OUStringBuffer      maBuffer;               /// Text data to append to current text range.
    OoxFontData         maFontData;             /// Font attributes of current text range.
};


HeaderFooterParserImpl::HeaderFooterParserImpl( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    maPageNumberService( CREATE_OUSTRING( "com.sun.star.text.TextField.PageNumber" ) ),
    maPageCountService( CREATE_OUSTRING( "com.sun.star.text.TextField.PageCount" ) ),
    maSheetNameService( CREATE_OUSTRING( "com.sun.star.text.TextField.SheetName" ) ),
    maFileNameService( CREATE_OUSTRING( "com.sun.star.text.TextField.FileName" ) ),
    maDateTimeService( CREATE_OUSTRING( "com.sun.star.text.TextField.DateTime" ) ),
    maIsDateProp( CREATE_OUSTRING( "IsDate" ) ),
    maFileFormatProp( CREATE_OUSTRING( "FileFormat" ) ),
    maPortions( static_cast< size_t >( HF_COUNT ) ),
    meCurrPortion( HF_CENTER )
{
    // different names for bold font style (lowercase)
    maBoldNames.insert( CREATE_OSTRING( "bold" ) );
    maBoldNames.insert( CREATE_OSTRING( "fett" ) );
    maBoldNames.insert( CREATE_OSTRING( "demibold" ) );
    maBoldNames.insert( CREATE_OSTRING( "halbfett" ) );
    maBoldNames.insert( CREATE_OSTRING( "black" ) );
    maBoldNames.insert( CREATE_OSTRING( "heavy" ) );

    // different names for italic font style (lowercase)
    maItalicNames.insert( CREATE_OSTRING( "italic" ) );
    maItalicNames.insert( CREATE_OSTRING( "kursiv" ) );
    maItalicNames.insert( CREATE_OSTRING( "oblique" ) );
    maItalicNames.insert( CREATE_OSTRING( "schr\303\204g" ) );  // with uppercase A umlaut
    maItalicNames.insert( CREATE_OSTRING( "schr\303\244g" ) );  // with lowercase A umlaut
}

void HeaderFooterParserImpl::parse( const Reference< XHeaderFooterContent >& rxContext, const OUString& rData )
{
    if( !rxContext.is() || (rData.getLength() == 0) ||
            !maPortions[ HF_LEFT ].initialize( rxContext->getLeftText() ) ||
            !maPortions[ HF_CENTER ].initialize( rxContext->getCenterText() ) ||
            !maPortions[ HF_RIGHT ].initialize( rxContext->getRightText() ) )
        return;

    meCurrPortion = HF_CENTER;
    maBuffer.setLength( 0 );
    maFontData = getStyles().getDefaultFontData();
    OUStringBuffer aFontName;           // current font name
    OUStringBuffer aFontStyle;          // current font style
    sal_Int32 nFontHeight = 0;          // current font height

    /** State of the parser. */
    enum
    {
        STATE_TEXT,         /// Literal text data.
        STATE_TOKEN,        /// Control token following a '&' character.
        STATE_FONTNAME,     /// Font name ('&' is followed by '"', reads until next '"' or ',').
        STATE_FONTSTYLE,    /// Font style name (font part after ',', reads until next '"').
        STATE_FONTHEIGHT    /// Font height ('&' is followed by num. digits, reads until non-digit).
    }
    eState = STATE_TEXT;

    const sal_Unicode* pcChar = rData.getStr();
    const sal_Unicode* pcEnd = pcChar + rData.getLength();
    for( ; (pcChar != pcEnd) && (*pcChar != 0); ++pcChar )
    {
        sal_Unicode cChar = *pcChar;
        switch( eState )
        {
            case STATE_TEXT:
            {
                switch( cChar )
                {
                    case '&':           // new token
                        appendText();
                        eState = STATE_TOKEN;
                    break;
                    case '\n':          // line break
                        appendText();
                        appendLineBreak();
                    break;
                    default:
                        maBuffer.append( cChar );
                }
            }
            break;

            case STATE_TOKEN:
            {
                // default: back to text mode, may be changed in specific cases
                eState = STATE_TEXT;
                // ignore case of token codes
                if( ('a' <= cChar) && (cChar <= 'z') )
                    (cChar -= 'a') += 'A';
                switch( cChar )
                {
                    case '&':   maBuffer.append( cChar );   break;  // the '&' character

                    case 'L':   setNewPortion( HF_LEFT );   break;  // left portion
                    case 'C':   setNewPortion( HF_CENTER ); break;  // center portion
                    case 'R':   setNewPortion( HF_RIGHT );  break;  // right portion

                    case 'P':   // page number
                        appendField( createField( maPageNumberService ) );
                    break;
                    case 'N':   // total page count
                        appendField( createField( maPageCountService ) );
                    break;
                    case 'A':   // current sheet name
                        appendField( createField( maSheetNameService ) );
                    break;

                    case 'F':   // file name
                    {
                        Reference< XTextContent > xContent = createField( maFileNameService );
                        PropertySet aPropSet( xContent );
                        aPropSet.setProperty( maFileFormatProp, ::com::sun::star::text::FilenameDisplayFormat::NAME_AND_EXT );
                        appendField( xContent );
                    }
                    break;
                    case 'Z':   // file path (without file name), BIFF8 and OOX only
                        if( (getFilterType() == FILTER_OOX) || ((getFilterType() == FILTER_BIFF) && (getBiff() == BIFF8)) )
                        {
                            Reference< XTextContent > xContent = createField( maFileNameService );
                            PropertySet aPropSet( xContent );
                            // FilenameDisplayFormat::PATH not supported by Calc
                            aPropSet.setProperty( maFileFormatProp, ::com::sun::star::text::FilenameDisplayFormat::FULL );
                            appendField( xContent );
                            /*  path only is not supported -- if we find a '&Z&F'
                                combination for path/name, skip the '&F' part */
                            if( (pcChar + 2 < pcEnd) && (pcChar[ 1 ] == '&') && ((pcChar[ 2 ] == 'f') || (pcChar[ 2 ] == 'F')) )
                                pcChar += 2;
                        }
                    break;
                    case 'D':   // date
                    {
                        Reference< XTextContent > xContent = createField( maDateTimeService );
                        PropertySet aPropSet( xContent );
                        aPropSet.setProperty( maIsDateProp, true );
                        appendField( xContent );
                    }
                    break;
                    case 'T':   // time
                    {
                        Reference< XTextContent > xContent = createField( maDateTimeService );
                        PropertySet aPropSet( xContent );
                        aPropSet.setProperty( maIsDateProp, false );
                        appendField( xContent );
                    }
                    break;

                    case 'B':   // bold
                        setAttributes();
                        maFontData.mbBold = !maFontData.mbBold;
                    break;
                    case 'I':   // italic
                        setAttributes();
                        maFontData.mbItalic = !maFontData.mbItalic;
                    break;
                    case 'U':   // underline
                        setAttributes();
                        maFontData.mnUnderline = (maFontData.mnUnderline == XML_single) ? XML_none : XML_single;
                    break;
                    case 'E':   // double underline
                        setAttributes();
                        maFontData.mnUnderline = (maFontData.mnUnderline == XML_double) ? XML_none : XML_double;
                    break;
                    case 'S':   // strikeout
                        setAttributes();
                        maFontData.mbStrikeout = !maFontData.mbStrikeout;
                    break;
                    case 'X':   // superscript
                        setAttributes();
                        maFontData.mnEscapement = (maFontData.mnEscapement == XML_superscript) ? XML_baseline : XML_superscript;
                    break;
                    case 'Y':   // subsrcipt
                        setAttributes();
                        maFontData.mnEscapement = (maFontData.mnEscapement == XML_subscript) ? XML_baseline : XML_subscript;
                    break;
                    case 'O':   // outlined
                        setAttributes();
                        maFontData.mbOutline = !maFontData.mbOutline;
                    break;
                    case 'H':   // shadow
                        setAttributes();
                        maFontData.mbShadow = !maFontData.mbShadow;
                    break;

                    case 'K':   // text color (not in BIFF)
                        if( (getFilterType() == FILTER_OOX) && (pcChar + 6 < pcEnd) )
                        {
                            setAttributes();
                            // eat the following 6 characters
                            convertFontColor( OUString( pcChar + 1, 6 ) );
                            pcChar += 6;
                        }
                    break;

                    case '\"':  // font name
                        aFontName.setLength( 0 );
                        aFontStyle.setLength( 0 );
                        eState = STATE_FONTNAME;
                    break;
                    default:
                        if( ('0' <= cChar) && (cChar <= '9') )    // font size
                        {
                            nFontHeight = cChar - '0';
                            eState = STATE_FONTHEIGHT;
                        }
                }
            }
            break;

            case STATE_FONTNAME:
            {
                switch( cChar )
                {
                    case '\"':
                        setAttributes();
                        convertFontName( aFontName.makeStringAndClear() );
                        eState = STATE_TEXT;
                    break;
                    case ',':
                        eState = STATE_FONTSTYLE;
                    break;
                    default:
                        aFontName.append( cChar );
                }
            }
            break;

            case STATE_FONTSTYLE:
            {
                switch( cChar )
                {
                    case '\"':
                        setAttributes();
                        convertFontName( aFontName.makeStringAndClear() );
                        convertFontStyle( aFontStyle.makeStringAndClear() );
                        eState = STATE_TEXT;
                    break;
                    default:
                        aFontStyle.append( cChar );
                }
            }
            break;

            case STATE_FONTHEIGHT:
            {
                if( ('0' <= cChar) && (cChar <= '9') )
                {
                    if( nFontHeight >= 0 )
                    {
                        nFontHeight *= 10;
                        nFontHeight += (cChar - '0');
                        if( nFontHeight > 1000 )
                            nFontHeight = -1;
                    }
                }
                else
                {
                    if( nFontHeight > 0 )
                    {
                        setAttributes();
                        maFontData.mfHeight = nFontHeight;
                    }
                    --pcChar;
                    eState = STATE_TEXT;
                }
            }
            break;
        }
    }

    // finalize
    finalizePortion();
    maPortions[ HF_LEFT   ].mfTotalHeight += getCurrHeight( HF_LEFT );
    maPortions[ HF_CENTER ].mfTotalHeight += getCurrHeight( HF_CENTER );
    maPortions[ HF_RIGHT  ].mfTotalHeight += getCurrHeight( HF_RIGHT );
}

double HeaderFooterParserImpl::getTotalHeight() const
{
    return ::std::max( maPortions[ HF_LEFT ].mfTotalHeight,
        ::std::max( maPortions[ HF_CENTER ].mfTotalHeight, maPortions[ HF_RIGHT ].mfTotalHeight ) );
}

// private --------------------------------------------------------------------

double HeaderFooterParserImpl::getCurrHeight( HFPortionId ePortion ) const
{
    double fMaxHt = maPortions[ ePortion ].mfCurrHeight;
    return (fMaxHt == 0.0) ? maFontData.mfHeight : fMaxHt;
}

double HeaderFooterParserImpl::getCurrHeight() const
{
    return getCurrHeight( meCurrPortion );
}

void HeaderFooterParserImpl::updateCurrHeight( HFPortionId ePortion )
{
    double& rfMaxHt = maPortions[ ePortion ].mfCurrHeight;
    rfMaxHt = ::std::max( rfMaxHt, maFontData.mfHeight );
}

void HeaderFooterParserImpl::updateCurrHeight()
{
    updateCurrHeight( meCurrPortion );
}

void HeaderFooterParserImpl::setAttributes()
{
    Reference< XTextRange > xRange( getStartPos(), UNO_QUERY );
    getEndPos()->gotoRange( xRange, sal_False );
    getEndPos()->gotoEnd( sal_True );
    if( !getEndPos()->isCollapsed() )
    {
        Font aFont( *this, maFontData );
        aFont.finalizeImport();
        PropertySet aPropSet( getEndPos() );
        aFont.writeToPropertySet( aPropSet, FONT_PROPTYPE_RICHTEXT );
        getStartPos()->gotoEnd( sal_False );
        getEndPos()->gotoEnd( sal_False );
    }
}

void HeaderFooterParserImpl::appendText()
{
    if( maBuffer.getLength() > 0 )
    {
        getEndPos()->gotoEnd( sal_False );
        getEndPos()->setString( maBuffer.makeStringAndClear() );
        updateCurrHeight();
    }
}

void HeaderFooterParserImpl::appendLineBreak()
{
    getEndPos()->gotoEnd( sal_False );
    getEndPos()->setString( OUString( sal_Unicode( '\n' ) ) );
    getPortion().mfTotalHeight += getCurrHeight();
    getPortion().mfCurrHeight = 0;
}

Reference< XTextContent > HeaderFooterParserImpl::createField( const OUString& rServiceName ) const
{
    Reference< XTextContent > xContent;
    try
    {
        Reference< XMultiServiceFactory > xFactory( getDocument(), UNO_QUERY_THROW );
        xContent.set( xFactory->createInstance( rServiceName ), UNO_QUERY_THROW );
    }
    catch( Exception& )
    {
        OSL_ENSURE( false,
            OStringBuffer( "HeaderFooterParserImpl::createField - error while creating text field \"" ).
            append( OUStringToOString( rServiceName, RTL_TEXTENCODING_ASCII_US ) ).
            append( '"' ).getStr() );
    }
    return xContent;
}

void HeaderFooterParserImpl::appendField( const Reference< XTextContent >& rxContent )
{
    getEndPos()->gotoEnd( sal_False );
    try
    {
        Reference< XTextRange > xRange( getEndPos(), UNO_QUERY_THROW );
        getPortion().mxText->insertTextContent( xRange, rxContent, sal_False );
        updateCurrHeight();
    }
    catch( Exception& )
    {
    }
}

void HeaderFooterParserImpl::convertFontName( const OUString& rName )
{
    if( rName.getLength() > 0 )
    {
        // single dash is document default font
        if( (rName.getLength() == 1) && (rName[ 0 ] == '-') )
            maFontData.maName = getStyles().getDefaultFontData().maName;
        else
            maFontData.maName = rName;
    }
}

void HeaderFooterParserImpl::convertFontStyle( const OUString& rStyle )
{
    maFontData.mbBold = maFontData.mbItalic = false;
    sal_Int32 nPos = 0;
    sal_Int32 nLen = rStyle.getLength();
    while( (0 <= nPos) && (nPos < nLen) )
    {
        OString aToken = OUStringToOString( rStyle.getToken( 0, ' ', nPos ), RTL_TEXTENCODING_UTF8 ).toAsciiLowerCase();
        if( aToken.getLength() > 0 )
        {
            if( maBoldNames.count( aToken ) > 0 )
                maFontData.mbBold = true;
            else if( maItalicNames.count( aToken ) > 0 )
                maFontData.mbItalic = true;
        }
    }
}

void HeaderFooterParserImpl::convertFontColor( const OUString& rColor )
{
    OSL_ENSURE( rColor.getLength() == 6, "HeaderFooterParserImpl::convertFontColor - invalid font color code" );
    if( (rColor[ 2 ] == '+') || (rColor[ 2 ] == '-') )
        // theme color: TTSNNN (TT = decimal theme index, S = +/-, NNN = decimal tint/shade in percent)
        maFontData.maColor.set(
            XML_theme, rColor.copy( 0, 2 ).toInt32(),
            static_cast< double >( rColor.copy( 2 ).toInt32() ) / 100.0 );
    else
        // RGB color: RRGGBB
        maFontData.maColor.set( XML_rgb, rColor.toInt32( 16 ) );
}

void HeaderFooterParserImpl::finalizePortion()
{
    appendText();
    setAttributes();
}

void HeaderFooterParserImpl::setNewPortion( HFPortionId ePortion )
{
    if( ePortion != meCurrPortion )
    {
        finalizePortion();
        meCurrPortion = ePortion;
        maFontData = getStyles().getDefaultFontData();
    }
}

// ============================================================================

HeaderFooterParser::HeaderFooterParser( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    mxImpl( new HeaderFooterParserImpl( rHelper ) )
{
}

HeaderFooterParser::~HeaderFooterParser()
{
}

void HeaderFooterParser::parse( const Reference< XHeaderFooterContent >& rxContext, const OUString& rData )
{
    mxImpl->parse( rxContext, rData );
}

double HeaderFooterParser::getTotalHeight() const
{
    return mxImpl->getTotalHeight();
}

// ============================================================================

} // namespace xls
} // namespace oox

