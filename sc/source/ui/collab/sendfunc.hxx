/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _SENDFUNC_HXX_
#define _SENDFUNC_HXX_

#include <sal/config.h>

#include "docfunc.hxx"
class ScCollaboration;
class ScBaseCell;

namespace {

OUString formulaCellToString( ScFormulaCell *pCell )
{
    (void)pCell; // FIXME: implement me
    return OUString();
}

OUString editToString( const EditTextObject& /*rEditText*/ )
{
    // FIXME: implement me.
    return OUString();
}

EditTextObject stringToEdit( const OUString& rStr )
{
    // FIXME: implement me.
    return EditTextObject();
}

ScFormulaCell* stringToFormulaCell( const OUString &rString )
{
    (void)rString; // FIXME: implement me
    return NULL;
}

// Ye noddy mangling - needs improvement ...
// method name ';' then arguments ; separated
class ScChangeOpWriter
{
    rtl::OUStringBuffer aMessage;
    void appendSeparator()
    {
        aMessage.append( sal_Unicode( ';' ) );
    }

public:
    ScChangeOpWriter( const char *pName )
    {
        aMessage.appendAscii( pName );
        appendSeparator();
    }

    void appendString( const rtl::OUString &rStr )
    {
        if ( rStr.indexOf( sal_Unicode( '"' ) ) >= 0 ||
             rStr.indexOf( sal_Unicode( ';' ) ) >= 0 )
        {
            rtl::OUString aQuoted( rStr );
            ScGlobal::AddQuotes( aQuoted, sal_Unicode( '"' ) );
            aMessage.append( aQuoted );
        }
        else
            aMessage.append( rStr );
        appendSeparator();
    }

    void appendAddress( const ScAddress &rPos )
    {
        rtl::OUString aStr;
        rPos.Format( aStr, SCA_VALID );
        aMessage.append( aStr );
        appendSeparator();
    }

    void appendInt( sal_Int32 i )
    {
        aMessage.append( i );
        appendSeparator();
    }

    void appendBool( sal_Bool b )
    {
        aMessage.appendAscii( b ? "true" : "false" );
        appendSeparator();
    }

    void appendFormulaCell( ScFormulaCell *pCell )
    {
        appendString( formulaCellToString( pCell ) );
    }

    void appendEditText( const EditTextObject& rStr )
    {
        appendString( editToString(rStr) );
    }

    void appendDouble( double fVal )
    {
        aMessage.append(fVal);
        appendSeparator();
    }

    rtl::OString toString()
    {
        return rtl::OUStringToOString( aMessage.toString(), RTL_TEXTENCODING_UTF8 );
    }
};

struct ProtocolError {
    const char *message;
};

class ScChangeOpReader {
    std::vector< rtl::OUString > maArgs;

public:
    ScChangeOpReader( const rtl::OUString &rString)
    {
        // will need to handle escaping etc.
        // Surely someone else wrote this before ! [!?]
        enum {
            IN_TEXT, CHECK_QUOTE, FIND_LAST_QUOTE, SKIP_SEMI
        } eState = CHECK_QUOTE;

        sal_Int32 nStart = 0;
        for (sal_Int32 n = 0; n < rString.getLength(); n++)
        {
            if (rString[n] == '\\')
            {
                n++; // skip next char
                continue;
            }
            switch (eState) {
            case CHECK_QUOTE:
                if (rString[n] == '"')
                {
                    nStart = n + 1;
                    eState = FIND_LAST_QUOTE;
                    break;
                }
                // else drop through
            case IN_TEXT:
                if (rString[n] == ';')
                {
                    maArgs.push_back( rString.copy( nStart, n - nStart ) );
                    nStart = n + 1;
                    eState = CHECK_QUOTE;
                }
                break;
            case FIND_LAST_QUOTE:
                if (rString[n] == '"')
                {
                    maArgs.push_back( rString.copy( nStart, n - nStart ) );
                    eState = SKIP_SEMI;
                    break;
                }
                break;
            case SKIP_SEMI:
                if (rString[n] == ';')
                {
                    nStart = n + 1;
                    eState = CHECK_QUOTE;
                }
                break;
            }
        }
        if ( nStart < rString.getLength())
            maArgs.push_back( rString.copy( nStart, rString.getLength() - nStart ) );
    }
    ~ScChangeOpReader() {}

    rtl::OUString getMethod()
    {
        return maArgs[0];
    }

    size_t getArgCount() { return maArgs.size(); }

    rtl::OUString getString( sal_Int32 n )
    {
        if (n > 0 && (size_t)n < getArgCount() )
        {
            rtl::OUString aUStr( maArgs[ n ] );
            ScGlobal::EraseQuotes( aUStr );
            return aUStr;
        } else
            return rtl::OUString();
    }

    ScAddress getAddress( sal_Int32 n )
    {
        ScAddress aAddr;
        rtl::OUString aToken( getString( n ) );
        aAddr.Parse( aToken );
        return aAddr;
    }

    sal_Int32 getInt( sal_Int32 n )
    {
        return getString( n ).toInt32();
    }

    bool getBool( sal_Int32 n )
    {
        return getString( n ).equalsIgnoreAsciiCase( "true" );
    }

    ScFormulaCell* getFormulaCell( sal_Int32 n )
    {
        return stringToFormulaCell( getString( n ) );
    }

    double getDouble( sal_Int32 n )
    {
        return getString(n).toDouble();
    }

    EditTextObject getEdit( sal_Int32 n )
    {
        return stringToEdit(getString(n));
    }

};

} // anonymous namespace

class ScDocFuncSend : public ScDocFunc
{
    ScDocFuncDirect*    mpDirect;
    ScCollaboration*    mpCollaboration;

    friend class ScCollaboration;
    void                RecvMessage( const rtl::OString &rString );
    void                SendMessage( ScChangeOpWriter &rOp );
public:
    // FIXME: really ScDocFunc should be an abstract base, so
    // we don't need the rDocSh hack/pointer
    ScDocFuncSend( ScDocShell& rDocSh, ScDocFuncDirect* pDirect, ScCollaboration* pCollaboration );
    virtual ~ScDocFuncSend();

    virtual void        EnterListAction( sal_uInt16 nNameResId );
    virtual void        EndListAction();

    virtual sal_Bool    SetNormalString( bool& o_rbNumFmtSet, const ScAddress& rPos, const String& rText, sal_Bool bApi );
    virtual bool SetValueCell( const ScAddress& rPos, double fVal, bool bInteraction );
    virtual bool SetStringCell( const ScAddress& rPos, const OUString& rStr, bool bInteraction );
    virtual bool SetEditCell( const ScAddress& rPos, const EditTextObject& rStr, bool bInteraction );
    virtual bool SetFormulaCell( const ScAddress& rPos, ScFormulaCell* pCell, bool bInteraction );
    virtual bool PutData( const ScAddress& rPos, ScEditEngineDefaulter& rEngine, bool bApi );
    virtual bool SetCellText(
        const ScAddress& rPos, const OUString& rText, bool bInterpret, bool bEnglish,
        bool bApi, const formula::FormulaGrammar::Grammar eGrammar );
    virtual bool        ShowNote( const ScAddress& rPos, bool bShow = true );
    virtual bool        SetNoteText( const ScAddress& rPos, const String& rNoteText, sal_Bool bApi );
    virtual sal_Bool    RenameTable( SCTAB nTab, const String& rName, sal_Bool bRecord, sal_Bool bApi );
    virtual sal_Bool    ApplyAttributes( const ScMarkData& rMark, const ScPatternAttr& rPattern,
                                sal_Bool bRecord, sal_Bool bApi );
    virtual sal_Bool    ApplyStyle( const ScMarkData& rMark, const String& rStyleName,
                                sal_Bool bRecord, sal_Bool bApi );
    virtual sal_Bool    MergeCells( const ScCellMergeOption& rOption, sal_Bool bContents,
                                sal_Bool bRecord, sal_Bool bApi );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
