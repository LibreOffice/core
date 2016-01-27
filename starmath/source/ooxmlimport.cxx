/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#include "ooxmlimport.hxx"
#include "types.hxx"

#include <oox/token/tokens.hxx>
#include <oox/token/namespaces.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

using namespace oox::formulaimport;

/*
The primary internal data structure for the formula is the text representation
(the SmNode tree is built from it), so read data must be converted into this format.
*/

#define M_TOKEN( token ) OOX_TOKEN( officeMath, token )
#define OPENING( token ) XML_STREAM_OPENING( token )
#define CLOSING( token ) XML_STREAM_CLOSING( token )

// TODO create IS_OPENING(), IS_CLOSING() instead of doing 'next == OPENING( next )' ?

SmOoxmlImport::SmOoxmlImport( oox::formulaimport::XmlStream& s )
: stream( s )
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
    stream.ensureOpeningTag( M_TOKEN( oMath ));
    OUString ret;
    while( !stream.atEnd() && stream.currentToken() != CLOSING( M_TOKEN( oMath )))
    {
        // strictly speaking, it is not OMathArg here, but currently supported
        // functionality is the same like OMathArg, in the future this may need improving
        OUString item = readOMathArg( M_TOKEN( oMath ));
        if( item.isEmpty())
            continue;
        if( !ret.isEmpty())
            ret += " ";
        ret += item;
    }
    stream.ensureClosingTag( M_TOKEN( oMath ));
    // Placeholders are written out as nothing (i.e. nothing inside e.g. the <e> element),
    // which will result in "{}" in the formula text. Fix this up.
    ret = ret.replaceAll( "{}", "<?>" );
    // And as a result, empty parts of the formula that are not placeholders are written out
    // as a single space, so fix that up too.
    ret = ret.replaceAll( "{ }", "{}" );
    SAL_INFO( "starmath.ooxml", "Formula: " << ret );
    return ret;
}

OUString SmOoxmlImport::readOMathArg( int stoptoken )
{
    OUString ret;
    while( !stream.atEnd() && stream.currentToken() != CLOSING( stoptoken ))
    {
        if( !ret.isEmpty())
            ret += " ";
        switch( stream.currentToken())
        {
            case OPENING( M_TOKEN( acc )):
                ret += handleAcc();
                break;
            case OPENING( M_TOKEN( bar )):
                ret += handleBar();
                break;
            case OPENING( M_TOKEN( box )):
                ret += handleBox();
                break;
            case OPENING( M_TOKEN( borderBox )):
                ret += handleBorderBox();
                break;
            case OPENING( M_TOKEN( d )):
                ret += handleD();
                break;
            case OPENING( M_TOKEN( eqArr )):
                ret += handleEqArr();
                break;
            case OPENING( M_TOKEN( f )):
                ret += handleF();
                break;
            case OPENING( M_TOKEN( func )):
                ret += handleFunc();
                break;
            case OPENING( M_TOKEN( limLow )):
                ret += handleLimLowUpp( LimLow );
                break;
            case OPENING( M_TOKEN( limUpp )):
                ret += handleLimLowUpp( LimUpp );
                break;
            case OPENING( M_TOKEN( groupChr )):
                ret += handleGroupChr();
                break;
            case OPENING( M_TOKEN( m )):
                ret += handleM();
                break;
            case OPENING( M_TOKEN( nary )):
                ret += handleNary();
                break;
            case OPENING( M_TOKEN( r )):
                ret += handleR();
                break;
            case OPENING( M_TOKEN( rad )):
                ret += handleRad();
                break;
            case OPENING( M_TOKEN( sPre )):
                ret += handleSpre();
                break;
            case OPENING( M_TOKEN( sSub )):
                ret += handleSsub();
                break;
            case OPENING( M_TOKEN( sSubSup )):
                ret += handleSsubsup();
                break;
            case OPENING( M_TOKEN( sSup )):
                ret += handleSsup();
                break;
            default:
                stream.handleUnexpectedTag();
                break;
        }
    }
    return ret;
}

OUString SmOoxmlImport::readOMathArgInElement( int token )
{
    stream.ensureOpeningTag( token );
    OUString ret = readOMathArg( token );
    stream.ensureClosingTag( token );
    return ret;
}

OUString SmOoxmlImport::handleAcc()
{
    stream.ensureOpeningTag( M_TOKEN( acc ));
    sal_Unicode accChr = 0x302;
    if( XmlStream::Tag accPr = stream.checkOpeningTag( M_TOKEN( accPr )))
    {
        if( XmlStream::Tag chr = stream.checkOpeningTag( M_TOKEN( chr )))
        {
            accChr = chr.attribute( M_TOKEN( val ), accChr );
            stream.ensureClosingTag( M_TOKEN( chr ));
        }
        stream.ensureClosingTag( M_TOKEN( accPr ));
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
            acc = "dot";
            break;
        case MS_DDOT:
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
    stream.ensureClosingTag( M_TOKEN( acc ));
    return acc + " {" + e + "}";
}

OUString SmOoxmlImport::handleBar()
{
    stream.ensureOpeningTag( M_TOKEN( bar ));
    enum pos_t { top, bot } topbot = bot;
    if( stream.checkOpeningTag( M_TOKEN( barPr )))
    {
        if( XmlStream::Tag pos = stream.checkOpeningTag( M_TOKEN( pos )))
        {
            if( pos.attribute( M_TOKEN( val )) == "top" )
                topbot = top;
            else if( pos.attribute( M_TOKEN( val )) == "bot" )
                topbot = bot;
            stream.ensureClosingTag( M_TOKEN( pos ));
        }
        stream.ensureClosingTag( M_TOKEN( barPr ));
    }
    OUString e = readOMathArgInElement( M_TOKEN( e ));
    stream.ensureClosingTag( M_TOKEN( bar ));
    if( topbot == top )
        return "overline {" + e + "}";
    else
        return "underline {" + e + "}";
}

OUString SmOoxmlImport::handleBox()
{
    // there does not seem to be functionality in LO to actually implement this
    // (or is there), but at least read in the contents instead of ignoring them
    stream.ensureOpeningTag( M_TOKEN( box ));
    OUString e = readOMathArgInElement( M_TOKEN( e ));
    stream.ensureClosingTag( M_TOKEN( box ));
    return e;
}


OUString SmOoxmlImport::handleBorderBox()
{
    stream.ensureOpeningTag( M_TOKEN( borderBox ));
    bool isStrikeH = false;
    if( stream.checkOpeningTag( M_TOKEN( borderBoxPr )))
    {
        if( XmlStream::Tag strikeH = stream.checkOpeningTag( M_TOKEN( strikeH )))
        {
            if( strikeH.attribute( M_TOKEN( val ), false ))
                isStrikeH = true;
            stream.ensureClosingTag( M_TOKEN( strikeH ));
        }
        stream.ensureClosingTag( M_TOKEN( borderBoxPr ));
    }
    OUString e = readOMathArgInElement( M_TOKEN( e ));
    stream.ensureClosingTag( M_TOKEN( borderBox ));
    if( isStrikeH )
        return "overstrike {" + e + "}";
    // LO does not seem to implement anything for handling the other cases
    return e;
}

OUString SmOoxmlImport::handleD()
{
    stream.ensureOpeningTag( M_TOKEN( d ));
    OUString opening = "(";
    OUString closing = ")";
    OUString separator = "|";
    if( XmlStream::Tag dPr = stream.checkOpeningTag( M_TOKEN( dPr )))
    {
        if( XmlStream::Tag begChr = stream.checkOpeningTag( M_TOKEN( begChr )))
        {
            opening = begChr.attribute( M_TOKEN( val ), opening );
            stream.ensureClosingTag( M_TOKEN( begChr ));
        }
        if( XmlStream::Tag sepChr = stream.checkOpeningTag( M_TOKEN( sepChr )))
        {
            separator = sepChr.attribute( M_TOKEN( val ), separator );
            stream.ensureClosingTag( M_TOKEN( sepChr ));
        }
        if( XmlStream::Tag endChr = stream.checkOpeningTag( M_TOKEN( endChr )))
        {
            closing = endChr.attribute( M_TOKEN( val ), closing );
            stream.ensureClosingTag( M_TOKEN( endChr ));
        }
        stream.ensureClosingTag( M_TOKEN( dPr ));
    }
    if( opening == "{" )
        opening = "left lbrace ";
    if( closing == "}" )
        closing = " right rbrace";
    if( opening == OUString( sal_Unicode( 0x27e6 )))
        opening = "left ldbracket ";
    if( closing == OUString( sal_Unicode( 0x27e7 )))
        closing = " right rdbracket";
    if( opening == "|" )
        opening = "left lline ";
    if( closing == "|" )
        closing = " right rline";
    if (opening == OUString(MS_DLINE) || opening == OUString(MS_DVERTLINE))
        opening = "left ldline ";
    if (closing == OUString(MS_DLINE) || closing == OUString(MS_DVERTLINE))
        closing = " right rdline";
    if (opening == OUString(MS_LANGLE) || opening == OUString(MS_LMATHANGLE))
        opening = "left langle ";
    if (closing == OUString(MS_RANGLE) || closing == OUString(MS_RMATHANGLE))
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
    while( stream.findTag( OPENING( M_TOKEN( e ))))
    {
        if( !first )
            ret.append( separator );
        first = false;
        ret.append( readOMathArgInElement( M_TOKEN( e )));
    }
    ret.append( closing );
    stream.ensureClosingTag( M_TOKEN( d ));
    return ret.makeStringAndClear();
}

OUString SmOoxmlImport::handleEqArr()
{
    stream.ensureOpeningTag( M_TOKEN( eqArr ));
    OUString ret;
    do
    { // there must be at least one m:e
        if( !ret.isEmpty())
            ret += "#";
        ret += " ";
        ret += readOMathArgInElement( M_TOKEN( e ));
        ret += " ";
    } while( !stream.atEnd() && stream.findTag( OPENING( M_TOKEN( e ))));
    stream.ensureClosingTag( M_TOKEN( eqArr ));
    return "stack {" + ret + "}";
}

OUString SmOoxmlImport::handleF()
{
    stream.ensureOpeningTag( M_TOKEN( f ));
    enum operation_t { bar, lin, noBar } operation = bar;
    if( stream.checkOpeningTag( M_TOKEN( fPr )))
    {
        if( XmlStream::Tag type = stream.checkOpeningTag( M_TOKEN( type )))
        {
            if( type.attribute( M_TOKEN( val )) == "bar" )
                operation = bar;
            else if( type.attribute( M_TOKEN( val )) == "lin" )
                operation = lin;
            else if( type.attribute( M_TOKEN( val )) == "noBar" )
                operation = noBar;
            stream.ensureClosingTag( M_TOKEN( type ));
        }
        stream.ensureClosingTag( M_TOKEN( fPr ));
    }
    OUString num = readOMathArgInElement( M_TOKEN( num ));
    OUString den = readOMathArgInElement( M_TOKEN( den ));
    stream.ensureClosingTag( M_TOKEN( f ));
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
    stream.ensureOpeningTag( M_TOKEN( func ));
    OUString fname = readOMathArgInElement( M_TOKEN( fName ));
    // fix the various functions
    if( fname.startsWith( "lim csub {" ))
        fname = "lim from {" + fname.copy( 10 );
    OUString ret = fname + " {" + readOMathArgInElement( M_TOKEN( e )) + "}";
    stream.ensureClosingTag( M_TOKEN( func ));
    return ret;
}

OUString SmOoxmlImport::handleLimLowUpp( LimLowUpp_t limlowupp )
{
    int token = limlowupp == LimLow ? M_TOKEN( limLow ) : M_TOKEN( limUpp );
    stream.ensureOpeningTag( token );
    OUString e = readOMathArgInElement( M_TOKEN( e ));
    OUString lim = readOMathArgInElement( M_TOKEN( lim ));
    stream.ensureClosingTag( token );
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
    stream.ensureOpeningTag( M_TOKEN( groupChr ));
    sal_Unicode chr = 0x23df;
    enum pos_t { top, bot } pos = bot;
    if( stream.checkOpeningTag( M_TOKEN( groupChrPr )))
    {
        if( XmlStream::Tag chrTag = stream.checkOpeningTag( M_TOKEN( chr )))
        {
            chr = chrTag.attribute( M_TOKEN( val ), chr );
            stream.ensureClosingTag( M_TOKEN( chr ));
        }
        if( XmlStream::Tag posTag = stream.checkOpeningTag( M_TOKEN( pos )))
        {
            if( posTag.attribute( M_TOKEN( val ), OUString( "bot" )) == "top" )
                pos = top;
            stream.ensureClosingTag( M_TOKEN( pos ));
        }
        stream.ensureClosingTag( M_TOKEN( groupChrPr ));
    }
    OUString e = readOMathArgInElement( M_TOKEN( e ));
    stream.ensureClosingTag( M_TOKEN( groupChr ));
    if( pos == top && chr == sal_Unicode( 0x23de ))
        return "{" + e + "} overbrace { }";
    if( pos == bot && chr == sal_Unicode( 0x23df ))
        return "{" + e + "} underbrace { }";
    if( pos == top )
        return "{" + e + "} csup {" + OUString( chr ) + "}";
    else
        return "{" + e + "} csub {" + OUString( chr ) + "}";
}

OUString SmOoxmlImport::handleM()
{
    stream.ensureOpeningTag( M_TOKEN( m ));
    OUString allrows;
    do // there must be at least one m:mr
    {
        stream.ensureOpeningTag( M_TOKEN( mr ));
        OUString row;
        do // there must be at least one m:e
        {
            if( !row.isEmpty())
                row += " # ";
            row += readOMathArgInElement( M_TOKEN( e ));
        } while( !stream.atEnd() && stream.findTag( OPENING( M_TOKEN( e ))));
        if( !allrows.isEmpty())
            allrows += " ## ";
        allrows += row;
        stream.ensureClosingTag( M_TOKEN( mr ));
    } while( !stream.atEnd() && stream.findTag( OPENING( M_TOKEN( mr ))));
    stream.ensureClosingTag( M_TOKEN( m ));
    return "matrix {" + allrows + "}";
}

OUString SmOoxmlImport::handleNary()
{
    stream.ensureOpeningTag( M_TOKEN( nary ));
    sal_Unicode chr = 0x222b;
    bool subHide = false;
    bool supHide = false;
    if( stream.checkOpeningTag( M_TOKEN( naryPr )))
    {
        if( XmlStream::Tag chrTag = stream.checkOpeningTag( M_TOKEN( chr )))
        {
            chr = chrTag.attribute( M_TOKEN( val ), chr );
            stream.ensureClosingTag( M_TOKEN( chr ));
        }
        if( XmlStream::Tag subHideTag = stream.checkOpeningTag( M_TOKEN( subHide )))
        {
            subHide = subHideTag.attribute( M_TOKEN( val ), subHide );
            stream.ensureClosingTag( M_TOKEN( subHide ));
        }
        if( XmlStream::Tag supHideTag = stream.checkOpeningTag( M_TOKEN( supHide )))
        {
            supHide = supHideTag.attribute( M_TOKEN( val ), supHide );
            stream.ensureClosingTag( M_TOKEN( supHide ));
        }
        stream.ensureClosingTag( M_TOKEN( naryPr ));
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
    stream.ensureClosingTag( M_TOKEN( nary ));
    return ret;
}

// NOT complete
OUString SmOoxmlImport::handleR()
{
    stream.ensureOpeningTag( M_TOKEN( r ));
    bool normal = false;
    bool literal = false;
    if( XmlStream::Tag rPr = stream.checkOpeningTag( M_TOKEN( rPr )))
    {
        if( XmlStream::Tag litTag = stream.checkOpeningTag( M_TOKEN( lit )))
        {
            literal = litTag.attribute( M_TOKEN( val ), true );
            stream.ensureClosingTag( M_TOKEN( lit ));
        }
        if( XmlStream::Tag norTag = stream.checkOpeningTag( M_TOKEN( nor )))
        {
            normal = norTag.attribute( M_TOKEN( val ), true );
            stream.ensureClosingTag( M_TOKEN( nor ));
        }
        stream.ensureClosingTag( M_TOKEN( rPr ));
    }
    OUString text;
    while( !stream.atEnd() && stream.currentToken() != CLOSING( stream.currentToken()))
    {
        switch( stream.currentToken())
        {
            case OPENING( M_TOKEN( t )):
            {
                XmlStream::Tag rtag = stream.ensureOpeningTag( M_TOKEN( t ));
                if( rtag.attribute( OOX_TOKEN( xml, space )) != "preserve" )
                    text += rtag.text.trim();
                else
                    text += rtag.text;
                stream.ensureClosingTag( M_TOKEN( t ));
                break;
            }
            default:
                stream.handleUnexpectedTag();
                break;
        }
    }
    stream.ensureClosingTag( M_TOKEN( r ));
    if( normal || literal )
        text = "\"" + text + "\"";
    return text.replaceAll("{", "\\{").replaceAll("}", "\\}");
}

OUString SmOoxmlImport::handleRad()
{
    stream.ensureOpeningTag( M_TOKEN( rad ));
    bool degHide = false;
    if( stream.checkOpeningTag( M_TOKEN( radPr )))
    {
        if( XmlStream::Tag degHideTag = stream.checkOpeningTag( M_TOKEN( degHide )))
        {
            degHide = degHideTag.attribute( M_TOKEN( val ), degHide );
            stream.ensureClosingTag( M_TOKEN( degHide ));
        }
        stream.ensureClosingTag( M_TOKEN( radPr ));
    }
    OUString deg = readOMathArgInElement( M_TOKEN( deg ));
    OUString e = readOMathArgInElement( M_TOKEN( e ));
    stream.ensureClosingTag( M_TOKEN( rad ));
    if( degHide )
        return "sqrt {" + e + "}";
    else
        return "nroot {" + deg + "} {" + e + "}";
}

OUString SmOoxmlImport::handleSpre()
{
    stream.ensureOpeningTag( M_TOKEN( sPre ));
    OUString sub = readOMathArgInElement( M_TOKEN( sub ));
    OUString sup = readOMathArgInElement( M_TOKEN( sup ));
    OUString e = readOMathArgInElement( M_TOKEN( e ));
    stream.ensureClosingTag( M_TOKEN( sPre ));
    return "{" + e + "} lsub {" + sub + "} lsup {" + sup + "}";
}

OUString SmOoxmlImport::handleSsub()
{
    stream.ensureOpeningTag( M_TOKEN( sSub ));
    OUString e = readOMathArgInElement( M_TOKEN( e ));
    OUString sub = readOMathArgInElement( M_TOKEN( sub ));
    stream.ensureClosingTag( M_TOKEN( sSub ));
    return "{" + e + "} rsub {" + sub + "}";
}

OUString SmOoxmlImport::handleSsubsup()
{
    stream.ensureOpeningTag( M_TOKEN( sSubSup ));
    OUString e = readOMathArgInElement( M_TOKEN( e ));
    OUString sub = readOMathArgInElement( M_TOKEN( sub ));
    OUString sup = readOMathArgInElement( M_TOKEN( sup ));
    stream.ensureClosingTag( M_TOKEN( sSubSup ));
    return "{" + e + "} rsub {" + sub + "} rsup {" + sup + "}";
}

OUString SmOoxmlImport::handleSsup()
{
    stream.ensureOpeningTag( M_TOKEN( sSup ));
    OUString e = readOMathArgInElement( M_TOKEN( e ));
    OUString sup = readOMathArgInElement( M_TOKEN( sup ));
    stream.ensureClosingTag( M_TOKEN( sSup ));
    return "{" + e + "} ^ {" + sup + "}";
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
