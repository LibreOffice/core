/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#include "ooxmlimport.hxx"
#include <types.hxx>

#include <oox/mathml/importutils.hxx>
#include <oox/token/namespaces.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>

using namespace oox::formulaimport;

/*
The primary internal data structure for the formula is the text representation
(the SmNode tree is built from it), so read data must be converted into this format.
*/

#define OPENING( token ) XML_STREAM_OPENING( token )
#define CLOSING( token ) XML_STREAM_CLOSING( token )

// TODO create IS_OPENING(), IS_CLOSING() instead of doing 'next == OPENING( next )' ?

SmOoxmlImport::SmOoxmlImport( oox::formulaimport::XmlStream& s )
    : m_rStream( s )
{
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
    return ret;
}

OUString SmOoxmlImport::handleAcc()
{
    m_rStream.ensureOpeningTag( M_TOKEN( acc ));
    sal_Unicode accChr = 0x302;
    if( XmlStream::Tag accPr = m_rStream.checkOpeningTag( M_TOKEN( accPr )))
    {
        if( XmlStream::Tag chr = m_rStream.checkOpeningTag( M_TOKEN( chr )))
        {
            accChr = chr.attribute( M_TOKEN( val ), accChr );
            m_rStream.ensureClosingTag( M_TOKEN( chr ));
        }
        m_rStream.ensureClosingTag( M_TOKEN( accPr ));
    }
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
            SAL_WARN( "starmath.ooxml", "Unknown m:chr in m:acc \'" << accChr << "\'" );
            break;
    }
    OUString e = readOMathArgInElement( M_TOKEN( e ));
    m_rStream.ensureClosingTag( M_TOKEN( acc ));
    return acc + " {" + e + "}";
}

OUString SmOoxmlImport::handleBar()
{
    m_rStream.ensureOpeningTag( M_TOKEN( bar ));
    enum pos_t { top, bot } topbot = bot;
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
        m_rStream.ensureClosingTag( M_TOKEN( barPr ));
    }
    OUString e = readOMathArgInElement( M_TOKEN( e ));
    m_rStream.ensureClosingTag( M_TOKEN( bar ));
    if( topbot == top )
        return "overline {" + e + "}";
    else
        return "underline {" + e + "}";
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
    if( m_rStream.checkOpeningTag( M_TOKEN( borderBoxPr )))
    {
        if( XmlStream::Tag strikeH = m_rStream.checkOpeningTag( M_TOKEN( strikeH )))
        {
            if( strikeH.attribute( M_TOKEN( val ), false ))
                isStrikeH = true;
            m_rStream.ensureClosingTag( M_TOKEN( strikeH ));
        }
        m_rStream.ensureClosingTag( M_TOKEN( borderBoxPr ));
    }
    OUString e = readOMathArgInElement( M_TOKEN( e ));
    m_rStream.ensureClosingTag( M_TOKEN( borderBox ));
    if( isStrikeH )
        return "overstrike {" + e + "}";
    // LO does not seem to implement anything for handling the other cases
    return e;
}

OUString SmOoxmlImport::handleD()
{
    m_rStream.ensureOpeningTag( M_TOKEN( d ));
    OUString opening = "(";
    OUString closing = ")";
    OUString separator = "|";
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
        m_rStream.ensureClosingTag( M_TOKEN( dPr ));
    }
    if( opening == "{" )
        opening = "left lbrace ";
    if( closing == "}" )
        closing = " right rbrace";
    if( opening == OUStringLiteral1(0x27e6) )
        opening = "left ldbracket ";
    if( closing == OUStringLiteral1(0x27e7) )
        closing = " right rdbracket";
    if( opening == "|" )
        opening = "left lline ";
    if( closing == "|" )
        closing = " right rline";
    if (opening == OUStringLiteral1(MS_DLINE)
        || opening == OUStringLiteral1(MS_DVERTLINE))
        opening = "left ldline ";
    if (closing == OUStringLiteral1(MS_DLINE)
        || closing == OUStringLiteral1(MS_DVERTLINE))
        closing = " right rdline";
    if (opening == OUStringLiteral1(MS_LANGLE)
        || opening == OUStringLiteral1(MS_LMATHANGLE))
        opening = "left langle ";
    if (closing == OUStringLiteral1(MS_RANGLE)
        || closing == OUStringLiteral1(MS_RMATHANGLE))
        closing = " right rangle";
    // use scalable brackets (the explicit "left" or "right")
    if( opening == "(" || opening == "[" )
        opening = "left " + opening;
    if( closing == ")" || closing == "]" )
        closing = " right " + closing;
    if( separator == "|" ) // plain "|" would be actually "V" (logical or)
        separator = " mline ";
    if( opening.isEmpty())
        opening = "left none ";
    if( closing.isEmpty())
        closing = " right none";
    OUStringBuffer ret;
    ret.append( opening );
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
    return ret.makeStringAndClear();
}

OUString SmOoxmlImport::handleEqArr()
{
    m_rStream.ensureOpeningTag( M_TOKEN( eqArr ));
    OUStringBuffer ret;
    do
    { // there must be at least one m:e
        if( !ret.isEmpty())
            ret.append("#");
        ret.append(" ");
        ret.append(readOMathArgInElement( M_TOKEN( e )));
        ret.append(" ");
    } while( !m_rStream.atEnd() && m_rStream.findTag( OPENING( M_TOKEN( e ))));
    m_rStream.ensureClosingTag( M_TOKEN( eqArr ));
    return "stack {" + ret.makeStringAndClear() + "}";
}

OUString SmOoxmlImport::handleF()
{
    m_rStream.ensureOpeningTag( M_TOKEN( f ));
    enum operation_t { bar, lin, noBar } operation = bar;
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
        m_rStream.ensureClosingTag( M_TOKEN( fPr ));
    }
    OUString num = readOMathArgInElement( M_TOKEN( num ));
    OUString den = readOMathArgInElement( M_TOKEN( den ));
    m_rStream.ensureClosingTag( M_TOKEN( f ));
    if( operation == bar )
        return "{" + num + "} over {" + den + "}";
    else if( operation == lin )
        return "{" + num + "} / {" + den + "}";
    else // noBar
    {
        return "binom {" + num + "} {" + den + "}";
    }
}

OUString SmOoxmlImport::handleFunc()
{
//lim from{x rightarrow 1} x
    m_rStream.ensureOpeningTag( M_TOKEN( func ));
    OUString fname = readOMathArgInElement( M_TOKEN( fName ));
    // fix the various functions
    if( fname.startsWith( "lim csub {" ))
        fname = "lim from {" + fname.copy( 10 );
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
        return e.copy( 0, e.getLength() - 2 ) + lim + "}";
    if( limlowupp == LimLow && e.endsWith( " underbrace { }" ))
        return e.copy( 0, e.getLength() - 2 ) + lim + "}";
    return e
        + ( limlowupp == LimLow ? OUString( " csub {" ) : OUString( " csup {" ))
        + lim + "}";
}

OUString SmOoxmlImport::handleGroupChr()
{
    m_rStream.ensureOpeningTag( M_TOKEN( groupChr ));
    sal_Unicode chr = 0x23df;
    enum pos_t { top, bot } pos = bot;
    if( m_rStream.checkOpeningTag( M_TOKEN( groupChrPr )))
    {
        if( XmlStream::Tag chrTag = m_rStream.checkOpeningTag( M_TOKEN( chr )))
        {
            chr = chrTag.attribute( M_TOKEN( val ), chr );
            m_rStream.ensureClosingTag( M_TOKEN( chr ));
        }
        if( XmlStream::Tag posTag = m_rStream.checkOpeningTag( M_TOKEN( pos )))
        {
            if( posTag.attribute( M_TOKEN( val ), OUString( "bot" )) == "top" )
                pos = top;
            m_rStream.ensureClosingTag( M_TOKEN( pos ));
        }
        m_rStream.ensureClosingTag( M_TOKEN( groupChrPr ));
    }
    OUString e = readOMathArgInElement( M_TOKEN( e ));
    m_rStream.ensureClosingTag( M_TOKEN( groupChr ));
    if( pos == top && chr == u'\x23de')
        return "{" + e + "} overbrace { }";
    if( pos == bot && chr == u'\x23df')
        return "{" + e + "} underbrace { }";
    if( pos == top )
        return "{" + e + "} csup {" + OUStringLiteral1( chr ) + "}";
    else
        return "{" + e + "} csub {" + OUStringLiteral1( chr ) + "}";
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
    return "matrix {" + allrows.makeStringAndClear() + "}";
}

OUString SmOoxmlImport::handleNary()
{
    m_rStream.ensureOpeningTag( M_TOKEN( nary ));
    sal_Unicode chr = 0x222b;
    bool subHide = false;
    bool supHide = false;
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
        m_rStream.ensureClosingTag( M_TOKEN( naryPr ));
    }
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
            SAL_WARN( "starmath.ooxml", "Unknown m:nary chr \'" << chr << "\'" );
            break;
    }
    if( !subHide )
        ret += " from {" + sub + "}";
    if( !supHide )
        ret += " to {" + sup + "}";
    ret += " {" + e + "}";
    m_rStream.ensureClosingTag( M_TOKEN( nary ));
    return ret;
}

// NOT complete
OUString SmOoxmlImport::handleR()
{
    m_rStream.ensureOpeningTag( M_TOKEN( r ));
    bool normal = false;
    bool literal = false;
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
        m_rStream.ensureClosingTag( M_TOKEN( rPr ));
    }
    OUStringBuffer text;
    while( !m_rStream.atEnd() && m_rStream.currentToken() != CLOSING( m_rStream.currentToken()))
    {
        switch( m_rStream.currentToken())
        {
            case OPENING( M_TOKEN( t )):
            {
                XmlStream::Tag rtag = m_rStream.ensureOpeningTag( M_TOKEN( t ));
                if( rtag.attribute( OOX_TOKEN( xml, space )) != "preserve" )
                    text.append(rtag.text.trim());
                else
                    text.append(rtag.text);
                m_rStream.ensureClosingTag( M_TOKEN( t ));
                break;
            }
            default:
                m_rStream.handleUnexpectedTag();
                break;
        }
    }
    m_rStream.ensureClosingTag( M_TOKEN( r ));
    if( normal || literal )
    {
        text.insert(0, "\"");
        text.append("\"");
    }
    return text.makeStringAndClear().replaceAll("{", "\\{").replaceAll("}", "\\}");
}

OUString SmOoxmlImport::handleRad()
{
    m_rStream.ensureOpeningTag( M_TOKEN( rad ));
    bool degHide = false;
    if( m_rStream.checkOpeningTag( M_TOKEN( radPr )))
    {
        if( XmlStream::Tag degHideTag = m_rStream.checkOpeningTag( M_TOKEN( degHide )))
        {
            degHide = degHideTag.attribute( M_TOKEN( val ), degHide );
            m_rStream.ensureClosingTag( M_TOKEN( degHide ));
        }
        m_rStream.ensureClosingTag( M_TOKEN( radPr ));
    }
    OUString deg = readOMathArgInElement( M_TOKEN( deg ));
    OUString e = readOMathArgInElement( M_TOKEN( e ));
    m_rStream.ensureClosingTag( M_TOKEN( rad ));
    if( degHide )
        return "sqrt {" + e + "}";
    else
        return "nroot {" + deg + "} {" + e + "}";
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
    return "{" + e + "} ^ {" + sup + "}";
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
