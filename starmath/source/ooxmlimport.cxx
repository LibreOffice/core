/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string_view>

#include "ooxmlimport.hxx"
#include <types.hxx>

#include <oox/mathml/importutils.hxx>
#include <oox/token/namespaces.hxx>
#include <rtl/character.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <o3tl/string_view.hxx>
#include <parse5.hxx>
#include <starmathdatabase.hxx>
#include <unordered_set>
#include <utility>

using namespace oox::formulaimport;

/*
The primary internal data structure for the formula is the text representation
(the SmNode tree is built from it), so read data must be converted into this format.
*/

#define OPENING( token ) XML_STREAM_OPENING( token )
#define CLOSING( token ) XML_STREAM_CLOSING( token )

// TODO create IS_OPENING(), IS_CLOSING() instead of doing 'next == OPENING( next )' ?

// True for text that is nothing but math operators. Such text is not an expression on
// its own, so the braces of a color command would make the parser fail on it.
static bool lcl_IsAllMathOperators( std::u16string_view sText )
{
    if( sText.empty())
        return false;
    for( char16_t cChar : sText )
        if( !isSingleCharMathOperator( cChar ))
            return false;
    return true;
}

// True when the text stands as an expression of its own, which is what the braces of a
// color command need to hold. Parsing leaves no state behind, so one parser serves every
// run of a formula.
static bool lcl_ParsesAsExpression( SmParser5& rParser, const OUString& rText )
{
    rParser.ParseExpression( rText );
    return rParser.GetError() == nullptr;
}

// Wraps the text in a color command. Nothing is added when there is no color, when the
// color is already in effect, or when the text is empty.
static OUString lcl_ApplyColorCommand( const OUString& rText, std::u16string_view sColorCommand,
    std::u16string_view sInEffect )
{
    if( sColorCommand.empty() || sColorCommand == sInEffect || rText.isEmpty())
        return rText;
    return OUString::Concat( sColorCommand ) + " {" + rText + "}";
}

// A run holds whatever text the document put in it, which need not be an expression. A
// run of operators is written as a literal, because that is an expression and it keeps
// the color. Anything else that does not parse keeps the color off, so that the formula
// reads as it did before the color was carried over at all.
static OUString lcl_ApplyRunColorCommand( SmParser5& rParser, const OUString& rText,
    std::u16string_view sColorCommand, std::u16string_view sInEffect )
{
    if( sColorCommand.empty() || sColorCommand == sInEffect || rText.isEmpty())
        return rText;
    if( lcl_IsAllMathOperators( rText ))
        return OUString::Concat( sColorCommand ) + " {\"" + rText + "\"}";
    if( !lcl_ParsesAsExpression( rParser, rText ))
        return rText;
    return OUString::Concat( sColorCommand ) + " {" + rText + "}";
}

namespace
{
// Keeps the color command that applies while the parts of one construct are read,
// and puts the previous one back when the construct is done with.
class ColorInEffect
{
public:
    ColorInEffect( OUString& rInEffect, OUString sColorCommand )
        : m_rInEffect( rInEffect )
        , m_sPrevious( rInEffect )
        , m_sColorCommand( std::move( sColorCommand ))
    {
        if( !m_sColorCommand.isEmpty())
            m_rInEffect = m_sColorCommand;
    }
    ~ColorInEffect() { m_rInEffect = m_sPrevious; }
    // Wraps the construct in its color command. The command is left off when the color
    // is already in effect, so the formula names the color only once.
    OUString apply( const OUString& rText ) const
    {
        return lcl_ApplyColorCommand( rText, m_sColorCommand, m_sPrevious );
    }

private:
    OUString& m_rInEffect;
    OUString m_sPrevious;
    OUString m_sColorCommand;
};
}

// A w:color value is either six hexadecimal digits or the word "auto". Anything else
// gives an empty string back, which leaves the default color in place. Black does the
// same, because the default color follows a dark document theme and an explicit black
// would not.
static OUString lcl_ColorCommandFromOoxmlValue( std::u16string_view sValue )
{
    if( sValue.size() != 6 )
        return OUString();
    for( char16_t cDigit : sValue )
        if( !rtl::isAsciiHexDigit( cDigit ))
            return OUString();
    const sal_uInt32 nColor = o3tl::toUInt32( sValue, 16 );
    if( nColor == 0 )
        return OUString();
    // A color that StarMath names is written with that name, so "color red" comes back
    // as it was written. The parser takes these two types bare. Other named colors need
    // a keyword in front of the name, so they are written as digits instead. The parser
    // reads only uppercase digits, so the value goes out in uppercase.
    const SmColorTokenTableEntry aEntry = starmathdatabase::Identify_Color_Parser( nColor );
    if( aEntry.eType == THTMLCOL || aEntry.eType == TMATHMLCOL )
        return "color " + aEntry.aIdent;
    return "color hex " + OUString( sValue ).toAsciiUpperCase();
}

SmOoxmlImport::SmOoxmlImport( oox::formulaimport::XmlStream& s )
    : m_rStream( s )
{
}

SmOoxmlImport::~SmOoxmlImport()
{
}

SmParser5& SmOoxmlImport::getParser()
{
    if( !m_pParser )
        m_pParser.reset( new SmParser5 );
    return *m_pParser;
}

OUString SmOoxmlImport::ConvertToStarMath()
{
    return handleStream();
}

// "toplevel" of reading, there will be oMath (if there was oMathPara, that was
// up to the parent component to handle)

// NOT complete
OUString SmOoxmlImport::handleStream()
{
    m_rStream.ensureOpeningTag( M_TOKEN( oMath ));
    OUStringBuffer ret;
    while( !m_rStream.atEnd() && m_rStream.currentToken() != CLOSING( M_TOKEN( oMath )))
    {
        // strictly speaking, it is not OMathArg here, but currently supported
        // functionality is the same like OMathArg, in the future this may need improving
        OUString item = readOMathArg( M_TOKEN( oMath ));
        if( item.isEmpty())
            continue;
        if( !ret.isEmpty())
            ret.append(" ");
        ret.append(item);
    }
    m_rStream.ensureClosingTag( M_TOKEN( oMath ));
    // Placeholders are written out as nothing (i.e. nothing inside e.g. the <e> element),
    // which will result in "{}" in the formula text. Fix this up.
    OUString ret2 = ret.makeStringAndClear().replaceAll( "{}", "<?>" );
    // And as a result, empty parts of the formula that are not placeholders are written out
    // as a single space, so fix that up too.
    ret2 = ret2.replaceAll( "{ }", "{}" );
    SAL_INFO( "starmath.ooxml", "Formula: " << ret2 );
    return ret2;
}

OUString SmOoxmlImport::readOMathArg( int stoptoken )
{
    OUStringBuffer ret;
    while( !m_rStream.atEnd() && m_rStream.currentToken() != CLOSING( stoptoken ))
    {
        if( !ret.isEmpty())
            ret.append(" ");
        switch( m_rStream.currentToken())
        {
            case OPENING( M_TOKEN( acc )):
                ret.append(handleAcc());
                break;
            case OPENING( M_TOKEN( bar )):
                ret.append(handleBar());
                break;
            case OPENING( M_TOKEN( box )):
                ret.append(handleBox());
                break;
            case OPENING( M_TOKEN( borderBox )):
                ret.append(handleBorderBox());
                break;
            case OPENING( M_TOKEN( d )):
                ret.append(handleD());
                break;
            case OPENING( M_TOKEN( eqArr )):
                ret.append(handleEqArr());
                break;
            case OPENING( M_TOKEN( f )):
                ret.append(handleF());
                break;
            case OPENING( M_TOKEN( func )):
                ret.append(handleFunc());
                break;
            case OPENING( M_TOKEN( limLow )):
                ret.append(handleLimLowUpp( LimLow ));
                break;
            case OPENING( M_TOKEN( limUpp )):
                ret.append(handleLimLowUpp( LimUpp ));
                break;
            case OPENING( M_TOKEN( groupChr )):
                ret.append(handleGroupChr());
                break;
            case OPENING( M_TOKEN( m )):
                ret.append(handleM());
                break;
            case OPENING( M_TOKEN( nary )):
                ret.append(handleNary());
                break;
            case OPENING( M_TOKEN( r )):
                ret.append(handleR());
                break;
            case OPENING( M_TOKEN( rad )):
                ret.append(handleRad());
                break;
            case OPENING( M_TOKEN( sPre )):
                ret.append(handleSpre());
                break;
            case OPENING( M_TOKEN( sSub )):
                ret.append(handleSsub());
                break;
            case OPENING( M_TOKEN( sSubSup )):
                ret.append(handleSsubsup());
                break;
            case OPENING( M_TOKEN( sSup )):
                ret.append(handleSsup());
                break;
            default:
                m_rStream.handleUnexpectedTag();
                break;
        }
    }
    return ret.makeStringAndClear();
}

OUString SmOoxmlImport::readOMathArgInElement( int token )
{
    m_rStream.ensureOpeningTag( token );
    OUString ret = readOMathArg( token );
    m_rStream.ensureClosingTag( token );
    // A bare operator alone in braces (e.g. rsup {+}) is a StarMath parse error.
    if (ret.getLength() == 1 && isSingleCharMathOperator(ret[0]))
        ret = "\"" + ret + "\"";
    return ret;
}

OUString SmOoxmlImport::handleAcc()
{
    m_rStream.ensureOpeningTag( M_TOKEN( acc ));
    sal_Unicode accChr = 0x302;
    OUString sColorCommand;
    if( XmlStream::Tag accPr = m_rStream.checkOpeningTag( M_TOKEN( accPr )))
    {
        if( XmlStream::Tag chr = m_rStream.checkOpeningTag( M_TOKEN( chr )))
        {
            accChr = chr.attribute( M_TOKEN( val ), accChr );
            m_rStream.ensureClosingTag( M_TOKEN( chr ));
        }
        sColorCommand = readCtrlPrColorCommand();
        m_rStream.ensureClosingTag( M_TOKEN( accPr ));
    }
    ColorInEffect aColorInEffect( m_sColorCommandInEffect, sColorCommand );
    // see aTokenTable in parse.cxx
    OUString acc;
    switch( accChr )
    {
        case MS_BAR:
        case MS_COMBBAR:
            acc = "bar";
            break;
        case MS_CHECK:
        case MS_COMBCHECK:
            acc = "check";
            break;
        case MS_ACUTE:
        case MS_COMBACUTE:
            acc = "acute";
            break;
        case MS_COMBOVERLINE:
            acc = "overline";
            break;
        case MS_GRAVE:
        case MS_COMBGRAVE:
            acc = "grave";
            break;
        case MS_BREVE:
        case MS_COMBBREVE:
            acc = "breve";
            break;
        case MS_CIRCLE:
        case MS_COMBCIRCLE:
            acc = "circle";
            break;
        case MS_RIGHTARROW:
        case MS_VEC:
            // prefer wide variants for these 3, .docx can't seem to differentiate
            // between e.g. 'vec' and 'widevec', if whatever the accent is above is short, this
            // shouldn't matter, but short above a longer expression doesn't look right
            acc = "widevec";
            break;
        case MS_LVEC:
            acc = "wideleftvec";
            break;
        case MS_HARPOON:
            acc = "wideharpoon";
            break;
        case MS_LHARPOON:
            acc = "wideleftharpoon";
            break;
        case MS_TILDE:
        case MS_COMBTILDE:
            acc = "widetilde";
            break;
        case MS_HAT:
        case MS_COMBHAT:
            acc = "widehat";
            break;
        case MS_DOT:
        case MS_COMBDOT:
            acc = "dot";
            break;
        case MS_DDOT:
        case MS_COMBDDOT:
            acc = "ddot";
            break;
        case MS_DDDOT:
            acc = "dddot";
            break;
        default:
            acc = "acute";
            SAL_WARN( "starmath.ooxml", "Unknown m:chr in m:acc \'" << OUString(accChr) << "\'" );
            break;
    }
    OUString e = readOMathArgInElement( M_TOKEN( e ));
    m_rStream.ensureClosingTag( M_TOKEN( acc ));
    return aColorInEffect.apply( acc + " {" + e + "}" );
}

OUString SmOoxmlImport::handleBar()
{
    m_rStream.ensureOpeningTag( M_TOKEN( bar ));
    enum pos_t { top, bot } topbot = bot;
    OUString sColorCommand;
    if( m_rStream.checkOpeningTag( M_TOKEN( barPr )))
    {
        if( XmlStream::Tag pos = m_rStream.checkOpeningTag( M_TOKEN( pos )))
        {
            if( pos.attribute( M_TOKEN( val )) == "top" )
                topbot = top;
            else if( pos.attribute( M_TOKEN( val )) == "bot" )
                topbot = bot;
            m_rStream.ensureClosingTag( M_TOKEN( pos ));
        }
        sColorCommand = readCtrlPrColorCommand();
        m_rStream.ensureClosingTag( M_TOKEN( barPr ));
    }
    ColorInEffect aColorInEffect( m_sColorCommandInEffect, sColorCommand );
    OUString e = readOMathArgInElement( M_TOKEN( e ));
    m_rStream.ensureClosingTag( M_TOKEN( bar ));
    if( topbot == top )
        return aColorInEffect.apply( "overline {" + e + "}" );
    else
        return aColorInEffect.apply( "underline {" + e + "}" );
}

OUString SmOoxmlImport::handleBox()
{
    // there does not seem to be functionality in LO to actually implement this
    // (or is there), but at least read in the contents instead of ignoring them
    m_rStream.ensureOpeningTag( M_TOKEN( box ));
    OUString e = readOMathArgInElement( M_TOKEN( e ));
    m_rStream.ensureClosingTag( M_TOKEN( box ));
    return e;
}


OUString SmOoxmlImport::handleBorderBox()
{
    m_rStream.ensureOpeningTag( M_TOKEN( borderBox ));
    bool isStrikeH = false;
    OUString sColorCommand;
    if( m_rStream.checkOpeningTag( M_TOKEN( borderBoxPr )))
    {
        if( XmlStream::Tag strikeH = m_rStream.checkOpeningTag( M_TOKEN( strikeH )))
        {
            if( strikeH.attribute( M_TOKEN( val ), false ))
                isStrikeH = true;
            m_rStream.ensureClosingTag( M_TOKEN( strikeH ));
        }
        sColorCommand = readCtrlPrColorCommand();
        m_rStream.ensureClosingTag( M_TOKEN( borderBoxPr ));
    }
    ColorInEffect aColorInEffect( m_sColorCommandInEffect, sColorCommand );
    OUString e = readOMathArgInElement( M_TOKEN( e ));
    m_rStream.ensureClosingTag( M_TOKEN( borderBox ));
    if( isStrikeH )
        return aColorInEffect.apply( "overstrike {" + e + "}" );
    // LO does not seem to implement anything for handling the other cases. The color
    // still belongs on the content, which is all that is left of the border box.
    return aColorInEffect.apply( e );
}

OUString SmOoxmlImport::handleD()
{
    m_rStream.ensureOpeningTag( M_TOKEN( d ));
    OUString opening = u"("_ustr;
    OUString closing = u")"_ustr;
    OUString separator = u"|"_ustr;
    OUString sColorCommand;
    if( XmlStream::Tag dPr = m_rStream.checkOpeningTag( M_TOKEN( dPr )))
    {
        if( XmlStream::Tag begChr = m_rStream.checkOpeningTag( M_TOKEN( begChr )))
        {
            opening = begChr.attribute( M_TOKEN( val ), opening );
            m_rStream.ensureClosingTag( M_TOKEN( begChr ));
        }
        if( XmlStream::Tag sepChr = m_rStream.checkOpeningTag( M_TOKEN( sepChr )))
        {
            separator = sepChr.attribute( M_TOKEN( val ), separator );
            m_rStream.ensureClosingTag( M_TOKEN( sepChr ));
        }
        if( XmlStream::Tag endChr = m_rStream.checkOpeningTag( M_TOKEN( endChr )))
        {
            closing = endChr.attribute( M_TOKEN( val ), closing );
            m_rStream.ensureClosingTag( M_TOKEN( endChr ));
        }
        sColorCommand = readCtrlPrColorCommand();
        m_rStream.ensureClosingTag( M_TOKEN( dPr ));
    }
    ColorInEffect aColorInEffect( m_sColorCommandInEffect, sColorCommand );
    if( opening == "{" )
        opening = "left lbrace ";
    if( closing == "}" )
        closing = " right rbrace";
    if( opening == u"\u27e6" )
        opening = "left ldbracket ";
    if( closing == u"\u27e7" )
        closing = " right rdbracket";
    if( opening == "|" )
        opening = "left lline ";
    if( closing == "|" )
        closing = " right rline";
    if (opening == OUStringChar(MS_DLINE)
        || opening == OUStringChar(MS_DVERTLINE))
        opening = "left ldline ";
    if (closing == OUStringChar(MS_DLINE)
        || closing == OUStringChar(MS_DVERTLINE))
        closing = " right rdline";
    if (opening == OUStringChar(MS_LANGLE)
        || opening == OUStringChar(MS_LMATHANGLE))
        opening = "left langle ";
    if (closing == OUStringChar(MS_RANGLE)
        || closing == OUStringChar(MS_RMATHANGLE))
        closing = " right rangle";
    // use scalable brackets (the explicit "left" or "right")
    if( opening == "(" || opening == "[" || opening == ")" || opening == "]" )
        opening = "left " + opening;
    if( closing == ")" || closing == "]" || closing == "(" || closing == "[")
        closing = " right " + closing;
    if( separator == "|" ) // plain "|" would be actually "V" (logical or)
        separator = " mline ";
    if( opening.isEmpty())
        opening = "left none ";
    if( closing.isEmpty())
        closing = " right none";
    OUStringBuffer ret( opening );
    bool first = true;
    while( m_rStream.findTag( OPENING( M_TOKEN( e ))))
    {
        if( !first )
            ret.append( separator );
        first = false;
        ret.append( readOMathArgInElement( M_TOKEN( e )));
    }
    ret.append( closing );
    m_rStream.ensureClosingTag( M_TOKEN( d ));
    return aColorInEffect.apply( ret.makeStringAndClear() );
}

OUString SmOoxmlImport::handleEqArr()
{
    m_rStream.ensureOpeningTag( M_TOKEN( eqArr ));
    OUStringBuffer ret;
    do
    { // there must be at least one m:e
        if( !ret.isEmpty())
            ret.append("#");
        ret.append(" "
            + readOMathArgInElement( M_TOKEN( e ))
            + " ");
    } while( !m_rStream.atEnd() && m_rStream.findTag( OPENING( M_TOKEN( e ))));
    m_rStream.ensureClosingTag( M_TOKEN( eqArr ));
    return "stack {" + ret + "}";
}

OUString SmOoxmlImport::handleF()
{
    m_rStream.ensureOpeningTag( M_TOKEN( f ));
    enum operation_t { bar, lin, noBar } operation = bar;
    OUString sColorCommand;
    if( m_rStream.checkOpeningTag( M_TOKEN( fPr )))
    {
        if( XmlStream::Tag type = m_rStream.checkOpeningTag( M_TOKEN( type )))
        {
            if( type.attribute( M_TOKEN( val )) == "bar" )
                operation = bar;
            else if( type.attribute( M_TOKEN( val )) == "lin" )
                operation = lin;
            else if( type.attribute( M_TOKEN( val )) == "noBar" )
                operation = noBar;
            m_rStream.ensureClosingTag( M_TOKEN( type ));
        }
        sColorCommand = readCtrlPrColorCommand();
        m_rStream.ensureClosingTag( M_TOKEN( fPr ));
    }
    ColorInEffect aColorInEffect( m_sColorCommandInEffect, sColorCommand );
    OUString num = readOMathArgInElement( M_TOKEN( num ));
    OUString den = readOMathArgInElement( M_TOKEN( den ));
    m_rStream.ensureClosingTag( M_TOKEN( f ));
    if( operation == bar )
        return aColorInEffect.apply( "{" + num + "} over {" + den + "}" );
    else if( operation == lin )
        return aColorInEffect.apply( "{" + num + "} / {" + den + "}" );
    else // noBar
    {
        return aColorInEffect.apply( "binom {" + num + "} {" + den + "}" );
    }
}

OUString SmOoxmlImport::handleFunc()
{
//lim from{x rightarrow 1} x
    m_rStream.ensureOpeningTag( M_TOKEN( func ));
    OUString fname = readOMathArgInElement( M_TOKEN( fName ));
    // fix the various functions
    if( fname.startsWith( "lim csub {" ))
        fname = OUString::Concat("lim from {") + fname.subView( 10 );
    OUString ret = fname + " {" + readOMathArgInElement( M_TOKEN( e )) + "}";
    m_rStream.ensureClosingTag( M_TOKEN( func ));
    return ret;
}

OUString SmOoxmlImport::handleLimLowUpp( LimLowUpp_t limlowupp )
{
    int token = limlowupp == LimLow ? M_TOKEN( limLow ) : M_TOKEN( limUpp );
    m_rStream.ensureOpeningTag( token );
    OUString e = readOMathArgInElement( M_TOKEN( e ));
    OUString lim = readOMathArgInElement( M_TOKEN( lim ));
    m_rStream.ensureClosingTag( token );
    // fix up overbrace/underbrace  (use { }, as {} will be converted to a placeholder)
    if( limlowupp == LimUpp && e.endsWith( " overbrace { }" ))
        return e.subView( 0, e.getLength() - 2 ) + lim + "}";
    if( limlowupp == LimLow && e.endsWith( " underbrace { }" ))
        return e.subView( 0, e.getLength() - 2 ) + lim + "}";
    return e
        + ( limlowupp == LimLow
            ? std::u16string_view( u" csub {" ) : std::u16string_view( u" csup {" ))
        + lim + "}";
}

OUString SmOoxmlImport::handleGroupChr()
{
    m_rStream.ensureOpeningTag( M_TOKEN( groupChr ));
    sal_Unicode chr = 0x23df;
    enum pos_t { top, bot } pos = bot;
    OUString sColorCommand;
    if( m_rStream.checkOpeningTag( M_TOKEN( groupChrPr )))
    {
        if( XmlStream::Tag chrTag = m_rStream.checkOpeningTag( M_TOKEN( chr )))
        {
            chr = chrTag.attribute( M_TOKEN( val ), chr );
            m_rStream.ensureClosingTag( M_TOKEN( chr ));
        }
        if( XmlStream::Tag posTag = m_rStream.checkOpeningTag( M_TOKEN( pos )))
        {
            if( posTag.attribute( M_TOKEN( val ), u"bot"_ustr) == "top" )
                pos = top;
            m_rStream.ensureClosingTag( M_TOKEN( pos ));
        }
        sColorCommand = readCtrlPrColorCommand();
        m_rStream.ensureClosingTag( M_TOKEN( groupChrPr ));
    }
    ColorInEffect aColorInEffect( m_sColorCommandInEffect, sColorCommand );
    OUString e = readOMathArgInElement( M_TOKEN( e ));
    m_rStream.ensureClosingTag( M_TOKEN( groupChr ));
    if( pos == top && chr == u'\x23de')
        return aColorInEffect.apply( "{" + e + "} overbrace { }" );
    if( pos == bot && chr == u'\x23df')
        return aColorInEffect.apply( "{" + e + "} underbrace { }" );
    if( pos == top )
        return aColorInEffect.apply( "{" + e + "} csup {" + OUStringChar( chr ) + "}" );
    else
        return aColorInEffect.apply( "{" + e + "} csub {" + OUStringChar( chr ) + "}" );
}

OUString SmOoxmlImport::handleM()
{
    m_rStream.ensureOpeningTag( M_TOKEN( m ));
    OUStringBuffer allrows;
    do // there must be at least one m:mr
    {
        m_rStream.ensureOpeningTag( M_TOKEN( mr ));
        OUStringBuffer row;
        do // there must be at least one m:e
        {
            if( !row.isEmpty())
                row.append(" # ");
            row.append(readOMathArgInElement( M_TOKEN( e )));
        } while( !m_rStream.atEnd() && m_rStream.findTag( OPENING( M_TOKEN( e ))));
        if( !allrows.isEmpty())
            allrows.append(" ## ");
        allrows.append(row);
        m_rStream.ensureClosingTag( M_TOKEN( mr ));
    } while( !m_rStream.atEnd() && m_rStream.findTag( OPENING( M_TOKEN( mr ))));
    m_rStream.ensureClosingTag( M_TOKEN( m ));
    return "matrix {" + allrows + "}";
}

OUString SmOoxmlImport::handleNary()
{
    m_rStream.ensureOpeningTag( M_TOKEN( nary ));
    sal_Unicode chr = 0x222b;
    bool subHide = false;
    bool supHide = false;
    OUString sColorCommand;
    if( m_rStream.checkOpeningTag( M_TOKEN( naryPr )))
    {
        if( XmlStream::Tag chrTag = m_rStream.checkOpeningTag( M_TOKEN( chr )))
        {
            chr = chrTag.attribute( M_TOKEN( val ), chr );
            m_rStream.ensureClosingTag( M_TOKEN( chr ));
        }
        if( XmlStream::Tag subHideTag = m_rStream.checkOpeningTag( M_TOKEN( subHide )))
        {
            subHide = subHideTag.attribute( M_TOKEN( val ), subHide );
            m_rStream.ensureClosingTag( M_TOKEN( subHide ));
        }
        if( XmlStream::Tag supHideTag = m_rStream.checkOpeningTag( M_TOKEN( supHide )))
        {
            supHide = supHideTag.attribute( M_TOKEN( val ), supHide );
            m_rStream.ensureClosingTag( M_TOKEN( supHide ));
        }
        sColorCommand = readCtrlPrColorCommand();
        m_rStream.ensureClosingTag( M_TOKEN( naryPr ));
    }
    ColorInEffect aColorInEffect( m_sColorCommandInEffect, sColorCommand );
    OUString sub = readOMathArgInElement( M_TOKEN( sub ));
    OUString sup = readOMathArgInElement( M_TOKEN( sup ));
    OUString e = readOMathArgInElement( M_TOKEN( e ));
    OUString ret;
    switch( chr )
    {
        case MS_INT:
            ret = "int";
            break;
        case MS_IINT:
            ret = "iint";
            break;
        case MS_IIINT:
            ret = "iiint";
            break;
        case MS_LINT:
            ret = "lint";
            break;
        case MS_LLINT:
            ret = "llint";
            break;
        case MS_LLLINT:
            ret = "lllint";
            break;
        case MS_PROD:
            ret = "prod";
            break;
        case MS_COPROD:
            ret = "coprod";
            break;
        case MS_SUM:
            ret = "sum";
            break;
        default:
            SAL_WARN( "starmath.ooxml", "Unknown m:nary chr \'" << OUString(chr) << "\'" );
            break;
    }
    if( !subHide )
        ret += " from {" + sub + "}";
    if( !supHide )
        ret += " to {" + sup + "}";
    ret += " {" + e + "}";
    m_rStream.ensureClosingTag( M_TOKEN( nary ));
    return aColorInEffect.apply( ret );
}

// An m:ctrlPr is the last child of a construct's property element. It carries the
// formatting of the parts the construct draws itself, such as a fraction bar or a
// pair of brackets, and this reads the color out of it.
OUString SmOoxmlImport::readCtrlPrColorCommand()
{
    OUString sRet;
    if( m_rStream.checkOpeningTag( M_TOKEN( ctrlPr )))
    {
        if( m_rStream.checkOpeningTag( W_TOKEN( rPr )))
        {
            if( XmlStream::Tag aColorTag = m_rStream.checkOpeningTag( W_TOKEN( color )))
            {
                sRet = lcl_ColorCommandFromOoxmlValue( aColorTag.attribute( W_TOKEN( val )));
                m_rStream.ensureClosingTag( W_TOKEN( color ));
            }
            m_rStream.ensureClosingTag( W_TOKEN( rPr ));
        }
        m_rStream.ensureClosingTag( M_TOKEN( ctrlPr ));
    }
    return sRet;
}

// NOT complete
OUString SmOoxmlImport::handleR()
{
    m_rStream.ensureOpeningTag( M_TOKEN( r ));
    bool normal = false;
    bool literal = false;
    OUString scrString;
    OUString sColorCommand;
    if( XmlStream::Tag rPr = m_rStream.checkOpeningTag( M_TOKEN( rPr )))
    {
        if( XmlStream::Tag litTag = m_rStream.checkOpeningTag( M_TOKEN( lit )))
        {
            literal = litTag.attribute( M_TOKEN( val ), true );
            m_rStream.ensureClosingTag( M_TOKEN( lit ));
        }
        if( XmlStream::Tag norTag = m_rStream.checkOpeningTag( M_TOKEN( nor )))
        {
            normal = norTag.attribute( M_TOKEN( val ), true );
            m_rStream.ensureClosingTag( M_TOKEN( nor ));
        }
        if (XmlStream::Tag srcTag = m_rStream.checkOpeningTag( M_TOKEN( scr )))
        {
            scrString = srcTag.attribute( M_TOKEN( val ), scrString );
            m_rStream.ensureClosingTag( M_TOKEN( scr ));
        }
        m_rStream.ensureClosingTag( M_TOKEN( rPr ));
    }
    OUStringBuffer text;
    bool isTagT = false;
    while( !m_rStream.atEnd() && m_rStream.currentToken() != CLOSING( m_rStream.currentToken()))
    {
        switch( m_rStream.currentToken())
        {
            // A run has two property elements. The math one m:rPr is read above.
            // This is the text one w:rPr and it holds the font color.
            case OPENING( W_TOKEN( rPr )):
            {
                m_rStream.ensureOpeningTag( W_TOKEN( rPr ));
                if( XmlStream::Tag aColorTag = m_rStream.checkOpeningTag( W_TOKEN( color )))
                {
                    sColorCommand
                        = lcl_ColorCommandFromOoxmlValue( aColorTag.attribute( W_TOKEN( val )));
                    m_rStream.ensureClosingTag( W_TOKEN( color ));
                }
                m_rStream.ensureClosingTag( W_TOKEN( rPr ));
                break;
            }
            case OPENING( M_TOKEN( t )):
            {
                isTagT = true;
                XmlStream::Tag rtag = m_rStream.ensureOpeningTag( M_TOKEN( t ));
                OUString sTagText = rtag.text;
                if (scrString == "double-struck")
                {
                    sTagText = SmOoxmlImport::handleSetString(sTagText);
                }
                if( rtag.attribute( OOX_TOKEN( xml, space )) != "preserve" )
                    sTagText = o3tl::trim(sTagText);
                text.append(sTagText);
                m_rStream.ensureClosingTag( M_TOKEN( t ));
                break;
            }
            default:
                m_rStream.handleUnexpectedTag();
                break;
        }
    }
    m_rStream.ensureClosingTag( M_TOKEN( r ));
    OUString sRet;
    if (scrString.isEmpty() && (normal || literal || isTagT))
        sRet = encloseOrEscapeLiteral(text.makeStringAndClear(), normal || literal);
    else
        sRet = text.makeStringAndClear();
    return lcl_ApplyRunColorCommand( getParser(), sRet, sColorCommand, m_sColorCommandInEffect );
}

OUString SmOoxmlImport::handleSetString(const OUString& setOUstring)
{
    std::unordered_set<sal_Unicode> setList= {'C', 'N', 'Q', 'R', 'Z', 'c', 'n', 'q', 'r', 'z'};
    OUString result;
    for (sal_Int32 i = 0; i < setOUstring.getLength(); i++)
    {
        if (setList.contains(setOUstring[i]))
        {
            result += OUString::Concat(" set" ) + OUStringChar(setOUstring[i]) + OUString::Concat(" ");
        }
        else
            result += encloseOrEscapeLiteral(setOUstring.copy(i, 1), /*bForce=*/false);
    }
    return result;
}

OUString SmOoxmlImport::handleRad()
{
    m_rStream.ensureOpeningTag( M_TOKEN( rad ));
    bool degHide = false;
    OUString sColorCommand;
    if( m_rStream.checkOpeningTag( M_TOKEN( radPr )))
    {
        if( XmlStream::Tag degHideTag = m_rStream.checkOpeningTag( M_TOKEN( degHide )))
        {
            degHide = degHideTag.attribute( M_TOKEN( val ), degHide );
            m_rStream.ensureClosingTag( M_TOKEN( degHide ));
        }
        sColorCommand = readCtrlPrColorCommand();
        m_rStream.ensureClosingTag( M_TOKEN( radPr ));
    }
    ColorInEffect aColorInEffect( m_sColorCommandInEffect, sColorCommand );
    OUString deg = readOMathArgInElement( M_TOKEN( deg ));
    OUString e = readOMathArgInElement( M_TOKEN( e ));
    m_rStream.ensureClosingTag( M_TOKEN( rad ));
    if( degHide )
        return aColorInEffect.apply( "sqrt {" + e + "}" );
    else
        return aColorInEffect.apply( "nroot {" + deg + "} {" + e + "}" );
}

OUString SmOoxmlImport::handleSpre()
{
    m_rStream.ensureOpeningTag( M_TOKEN( sPre ));
    OUString sub = readOMathArgInElement( M_TOKEN( sub ));
    OUString sup = readOMathArgInElement( M_TOKEN( sup ));
    OUString e = readOMathArgInElement( M_TOKEN( e ));
    m_rStream.ensureClosingTag( M_TOKEN( sPre ));
    return "{" + e + "} lsub {" + sub + "} lsup {" + sup + "}";
}

OUString SmOoxmlImport::handleSsub()
{
    m_rStream.ensureOpeningTag( M_TOKEN( sSub ));
    OUString e = readOMathArgInElement( M_TOKEN( e ));
    OUString sub = readOMathArgInElement( M_TOKEN( sub ));
    m_rStream.ensureClosingTag( M_TOKEN( sSub ));
    return "{" + e + "} rsub {" + sub + "}";
}

OUString SmOoxmlImport::handleSsubsup()
{
    m_rStream.ensureOpeningTag( M_TOKEN( sSubSup ));
    OUString e = readOMathArgInElement( M_TOKEN( e ));
    OUString sub = readOMathArgInElement( M_TOKEN( sub ));
    OUString sup = readOMathArgInElement( M_TOKEN( sup ));
    m_rStream.ensureClosingTag( M_TOKEN( sSubSup ));
    return "{" + e + "} rsub {" + sub + "} rsup {" + sup + "}";
}

OUString SmOoxmlImport::handleSsup()
{
    m_rStream.ensureOpeningTag( M_TOKEN( sSup ));
    OUString e = readOMathArgInElement( M_TOKEN( e ));
    OUString sup = readOMathArgInElement( M_TOKEN( sup ));
    m_rStream.ensureClosingTag( M_TOKEN( sSup ));
    if (sup == "'")
        return "{" + e + "} {" + sup + "}";
    return "{" + e + "} ^ {" + sup + "}";
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
