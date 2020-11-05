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

#ifdef DBG_UTIL

/*
 * And here's the description:
 *
 * The PROTOCOL macros allow you to log events in frame methods. In places where
 * logging is useful either one of the PROTOCOL(...) or PROTOCOL_ENTER(...) can
 * be used. PROTOCOL_ENTER(...) additionally logs the leaving of a method.
 *
 * The PROTOCOL macros accept the following parameters:
 * 1.   A pointer to an SwFrame (usually "this" or "rThis")
 * 2.   The function group i.e. PROT::MakeAll. This is used to decide (inline)
 *      whether this event shall be logged at the current time.
 * 3.   The action, usually 0. For example DbgAction::Start indents output in the log
 *      file and DbgAction::End stops the indentation. This allows for example
 *      PROTOCOL_ENTER to indent at the beginning of a method and stop indenting
 *      when leaving the method.
 * 4.   The fourth parameter is a void pointer which allows to pass anything
 *      which can be used in the log. A good example is PROT::Grow: this requires
 *      a pointer to the value which defines how much to grow.
 *
 * The log file is called "dbg_lay.out", which is saved in the current (BIN-)
 * directory. The file contains lines with FrameId, function group and additional
 * information.
 *
 * What exactly is going to be logged, can be defined as follows:
 * 1.   The static variable SwProtocol::nRecord contains the function groups
 *      which shall be logged.
 *      A value of i.e. PROT::Grow causes calls to SwFrame::Grow to be
 *      logged; PROT::MakeAll logs the calls to xxx::MakeAll.
 *      The PROT_XY values can be combined using binary OR, the default value
 *      is null - no method calls are logged.
 * 2.   The SwImplProtocol class contains a filter for frame types, only method
 *      call of frame types which are defined there are logged.
 *      The member nTypes can be set to values like SwFrameType::Page or SwFrameType::Section and
 *      may be combined using binary OR. The default values is 0xFFFF - meaning
 *      all frame types.
 * 3.   The SwImplProtocol class contains an ArrayPointer to FrameIds which need to be
 *      tracked. If the pointer is null, all frames will be logged; otherwise
 *      only frames of linked from the array will be logged.
 *
 * Code changes are needed to start logging; either change the default of nRecord
 * in SwProtocol::Init() or change the debugger. There are several possible
 * places in the debugger:
 * 1.   Set a breakpoint in SwProtocol::Init() and manipulate nRecord there, set
 *      FrameIds accordingly then start logging during program start.
 * 2.   Set a breakpoint before any PROTOCOL or PROTOCOL_ENTER macro during
 *      program execution, then set the lowest bit (PROT::Init) of
 *      SwProtocol::nRecord. This activates the function group of the following
 *      macro and causes it to be logged in the future.
 * 3.   There's a special case for 2: If one uses 2. in SwRootFrame::PaintSwFrame(..),
 *      the log settings are taken from the file "dbg_lay.ini"!
 *      In this INI-file you can have comment lines starting with a '#'.
 *      The sections "[frmid]", "[frmtype]" and "[record]" are relevant.
 *      In the [frmid] section, you can put FrameIds of the Frames to be logged.
 *      If there are no entries in this section, all Frames will be logged.
 *      In the [frmtype] section, the frame types which should be logged are
 *      listed; default is USHRT_MAX which means that all types are logged.
 *      It's possible to remove types from the list using '!' in front of a
 *      value. The value !0xC000 would for example exclude SwContentFrames from
 *      logging.
 *      In the [record] section the functions group which should be logged are
 *      listed; default is 0 which means that none are logged. It's also
 *      possible to remove functions using '!'.
 *      An example INI file:
 *           #Functions: all(0x0007ffff), except PrintArea (0x200)
 *           [record] 524287 !512
 *           [frmid]
 *           #the following FrameIds:
 *           1 2 12 13 14 15
 *           #no layout frames, except ColumnFrames
 *           [frmtype] !0x3FFF 0x4
 *
 * As soon as the logging is in process, one can manipulate many things in
 * SwImplProtocol::Record_(...) using a debugger, especially concerning
 * frame types and FrameIds.
 */

#include <dbg_lay.hxx>

#include <txtfrm.hxx>
#include <fntcache.hxx>
#include <tabfrm.hxx>
#include <rowfrm.hxx>
#include <cellfrm.hxx>
#include <layfrm.hxx>
#include <frame.hxx>
#include <swtable.hxx>
#include <ndtxt.hxx>
#include <rtl/strbuf.hxx>
#include <sal/log.hxx>
#include <tools/stream.hxx>

PROT SwProtocol::s_nRecord = PROT::FileInit;
SwImplProtocol* SwProtocol::s_pImpl = nullptr;

static sal_uLong lcl_GetFrameId( const SwFrame* pFrame )
{
#if OSL_DEBUG_LEVEL > 1
    static bool bFrameId = false;
    if( bFrameId )
        return pFrame->GetFrameId();
#endif
    if( pFrame )
        return pFrame->GetFrameId();
    return 0;
}

class SwImplProtocol
{
    std::unique_ptr<SvFileStream> m_pStream; // output stream
    std::unique_ptr<std::set<sal_uInt16>>
        m_pFrameIds; // which FrameIds shall be logged ( NULL == all)
    std::vector<tools::Long> m_aVars; // variables
    OStringBuffer m_aLayer; // indentation of output ("  " per start/end)
    SwFrameType m_nTypes; // which types shall be logged
    sal_uInt16 m_nLineCount; // printed lines
    sal_uInt16 m_nMaxLines; // max lines to be printed
    sal_uInt8 m_nInitFile; // range (FrameId,FrameType,Record) during reading of the INI file
    sal_uInt8
        m_nTestMode; // special for test formatting, logging may only be done in test formatting.
    void Record_( const SwFrame* pFrame, PROT nFunction, DbgAction nAct, void* pParam );
    bool NewStream();
    void CheckLine( OString& rLine );
    static void SectFunc( OStringBuffer& rOut, DbgAction nAct, void const * pParam );
public:
    SwImplProtocol();
    ~SwImplProtocol();
    // logging
    void Record( const SwFrame* pFrame, PROT nFunction, DbgAction nAct, void* pParam )
        {
            if (m_pStream)
                Record_(pFrame, nFunction, nAct, pParam);
        }
    void InsertFrame( sal_uInt16 nFrameId );    // take FrameId for logging
    void DeleteFrame( sal_uInt16 nFrameId );    // remove FrameId; don't log him anymore
    void FileInit();                    // read the INI file
    void ChkStream() {
        if (!m_pStream)
            NewStream();
    }
};

/* Through the PROTOCOL_ENTER macro a SwEnterLeave object gets created. If the
 * current function should be logged as SwImplEnterLeace object gets created.
 * The funny thing here is, that the Ctor of the Impl object is automatically
 * called at the beginning of the function and the Dtor is automatically called
 * when leaving the function. In the base implementation the Ctor calls only
 * PROTOCOL(..) with DbgAction::Start and in the Dtor a PROTOCOL(..) with DbgAction::End.
 * It's possible to derive from this class, for example to be able to document
 * frame resize while leaving a function. To do this, one only needs to add the
 * desired SwImplEnterLeave class in SwEnterLeave::Ctor().
 */

class SwImplEnterLeave
{
protected:
    const SwFrame* m_pFrame; // the frame
    PROT m_nFunction; // the function
    DbgAction m_nAction; // the action if needed
    void* m_pParam; // further parameter
public:
    SwImplEnterLeave(const SwFrame* pF, PROT nFunct, DbgAction nAct, void* pPar)
        : m_pFrame(pF)
        , m_nFunction(nFunct)
        , m_nAction(nAct)
        , m_pParam(pPar)
    {
    }
    virtual ~SwImplEnterLeave() {}
    virtual void Enter();           // message when entering
    virtual void Leave();           // message when leaving
};

namespace {

class SwSizeEnterLeave : public SwImplEnterLeave
{
    tools::Long m_nFrameHeight;

public:
    SwSizeEnterLeave(const SwFrame* pF, PROT nFunct, DbgAction nAct, void* pPar)
        : SwImplEnterLeave(pF, nFunct, nAct, pPar)
        , m_nFrameHeight(pF->getFrameArea().Height())
    {
    }

    virtual void Leave() override;           // resize message
};

class SwUpperEnterLeave : public SwImplEnterLeave
{
    sal_uInt16 m_nFrameId;

public:
    SwUpperEnterLeave(const SwFrame* pF, PROT nFunct, DbgAction nAct, void* pPar)
        : SwImplEnterLeave(pF, nFunct, nAct, pPar)
        , m_nFrameId(0)
    {
    }

    virtual void Enter() override;           // message
    virtual void Leave() override;           // message of FrameId from upper
};

class SwFrameChangesLeave : public SwImplEnterLeave
{
    SwRect m_aFrame;

public:
    SwFrameChangesLeave(const SwFrame* pF, PROT nFunct, DbgAction nAct, void* pPar)
        : SwImplEnterLeave(pF, nFunct, nAct, pPar)
        , m_aFrame(pF->getFrameArea())
    {
    }

    virtual void Enter() override;           // no message
    virtual void Leave() override;           // message when resizing the Frame area
};

}

void SwProtocol::Record( const SwFrame* pFrame, PROT nFunction, DbgAction nAct, void* pParam )
{
    if( Start() )
    {   // We reach this point if SwProtocol::nRecord is binary OR'd with PROT::Init(0x1) using the debugger
        bool bFinit = false; // This gives the possibility to stop logging of this action in the debugger
        if( bFinit )
        {
            s_nRecord &= ~nFunction;  // Don't log this function any longer
            s_nRecord &= ~PROT::Init;  // Always reset PROT::Init
            return;
        }
        s_nRecord |= nFunction;       // Activate logging of this function
        s_nRecord &= ~PROT::Init;      // Always reset PROT::Init
        if( s_pImpl )
            s_pImpl->ChkStream();
    }
    if( !s_pImpl )                        // Create Impl object if needed
        s_pImpl = new SwImplProtocol();
    s_pImpl->Record( pFrame, nFunction, nAct, pParam ); // ...and start logging
}

// The following function gets called when pulling in the writer DLL through
// TextInit(..) and gives the possibility to release functions
// and/or FrameIds to the debugger

void SwProtocol::Init()
{
    s_nRecord = PROT::FileInit;
    SvFileStream aStream( "dbg_lay.go", StreamMode::READ );
    if( aStream.IsOpen() )
    {
        s_pImpl = new SwImplProtocol();
        s_pImpl->FileInit();
    }
    aStream.Close();
}

// End of logging

void SwProtocol::Stop()
{
     if( s_pImpl )
     {
        delete s_pImpl;
        s_pImpl = nullptr;
        if( pFntCache )
            pFntCache->Flush();
     }
     s_nRecord = PROT::FileInit;
}

SwImplProtocol::SwImplProtocol()
    : m_nTypes(FRM_ALL)
    , m_nLineCount(0)
    , m_nMaxLines(USHRT_MAX)
    , m_nTestMode(0)
{
    NewStream();
}

bool SwImplProtocol::NewStream()
{
    m_nLineCount = 0;
    m_pStream.reset(new SvFileStream("dbg_lay.out", StreamMode::WRITE | StreamMode::TRUNC));
    if (m_pStream->GetError())
    {
        m_pStream.reset();
    }
    return nullptr != m_pStream;
}

SwImplProtocol::~SwImplProtocol()
{
    if (m_pStream)
    {
        m_pStream->Close();
        m_pStream.reset();
    }
    m_pFrameIds.reset();
    m_aVars.clear();
}

/// analyze a line in the INI file
void SwImplProtocol::CheckLine( OString& rLine )
{
    rLine = rLine.toAsciiLowerCase(); // upper/lower case is the same
    rLine = rLine.replace( '\t', ' ' );
    if( '#' == rLine[0] )   // comments start with '#'
        return;
    if( '[' == rLine[0] )   // section: FrameIds, type or function
    {
        OString aTmp = rLine.getToken(0, ']');
        if (aTmp == "[frmid")      // section FrameIds
        {
            m_nInitFile = 1;
            m_pFrameIds.reset(); // default: log all frames
        }
        else if (aTmp == "[frmtype")// section types
        {
            m_nInitFile = 2;
            m_nTypes = FRM_ALL; // default: log all frame types
        }
        else if (aTmp == "[record")// section functions
        {
            m_nInitFile = 3;
            SwProtocol::SetRecord( PROT::FileInit );// default: don't log any function
        }
        else if (aTmp == "[test")// section functions
        {
            m_nInitFile = 4; // default:
            m_nTestMode = 0; // log outside of test formatting
        }
        else if (aTmp == "[max")// Max number of lines
        {
            m_nInitFile = 5; // default:
            m_nMaxLines = USHRT_MAX;
        }
        else if (aTmp == "[var")// variables
        {
            m_nInitFile = 6;
        }
        else
            m_nInitFile = 0; // oops: unknown section?
        rLine = rLine.copy(aTmp.getLength() + 1);
    }

    // spaces (or tabs) are the delimiter
    sal_Int32 nIndex = 0;
    do
    {
        OString aTok = rLine.getToken( 0, ' ', nIndex );
        bool bNo = false;
        if( !aTok.isEmpty() && '!' == aTok[0] )
        {
            bNo = true;                 // remove this function/type
            aTok = aTok.copy(1);
        }
        if( !aTok.isEmpty() )
        {
            sal_Int64 nVal = aTok.toInt64();
            switch (m_nInitFile)
            {
                case 1: InsertFrame( sal_uInt16( nVal ) );    // add FrameId
                        break;
                case 2: {
                            SwFrameType nNew = static_cast<SwFrameType>(nVal);
                            if( bNo )
                                m_nTypes &= ~nNew; // remove type
                            else
                                m_nTypes |= nNew; // add type
                        }
                        break;
                case 3: {
                            PROT nOld = SwProtocol::Record();
                            if( bNo )
                                nOld &= ~PROT(nVal & o3tl::typed_flags<PROT>::mask); // remove function
                            else
                                nOld |= PROT(nVal & o3tl::typed_flags<PROT>::mask);  // remove function
                            SwProtocol::SetRecord( nOld );
                        }
                        break;
                case 4: {
                            sal_uInt8 nNew = static_cast<sal_uInt8>(nVal);
                            if( bNo )
                                m_nTestMode &= ~nNew; // reset test mode
                            else
                                m_nTestMode |= nNew; // set test mode
                        }
                        break;
                case 5:
                    m_nMaxLines = static_cast<sal_uInt16>(nVal);
                    break;
                case 6:
                    m_aVars.push_back(nVal);
                    break;
            }
        }
    }
    while ( nIndex >= 0 );
}

/// read the file "dbg_lay.ini" in the current directory and evaluate it.
void SwImplProtocol::FileInit()
{
    SvFileStream aStream( "dbg_lay.ini", StreamMode::READ );
    if( aStream.IsOpen() )
    {
        OString aLine;
        m_nInitFile = 0;
        while( aStream.good() )
        {
            char c;
            aStream.ReadChar( c );
            if( '\n' == c || '\r' == c )    // line ending
            {
                aLine = aLine.trim();
                if( !aLine.isEmpty() )
                    CheckLine( aLine );     // evaluate line
                aLine.clear();
            }
            else
                aLine += OString(c);
        }
        if( !aLine.isEmpty() )
            CheckLine( aLine );     // evaluate last line
    }
    aStream.Close();
}

/// enable indentation by two spaces during DbgAction::Start and disable it again at DbgAction::End.
static void lcl_Start(OStringBuffer& rOut, OStringBuffer& rLay, DbgAction nAction)
{
    if( nAction == DbgAction::Start )
    {
        rLay.append("  ");
        rOut.append(" On");
    }
    else if( nAction == DbgAction::End )
    {
        if( rLay.getLength() > 1 )
        {
            rLay.remove(rLay.getLength() - 2, rLay.getLength());
            rOut.remove(0, 2);
        }
        rOut.append(" Off");
    }
}

/// output the ValidSize-, ValidPos- and ValidPrtArea-Flag ("Sz","Ps","PA")
/// of the frame; "+" stands for valid, "-" stands for invalid.
static void lcl_Flags(OStringBuffer& rOut, const SwFrame* pFrame)
{
    rOut.append(" ValidSize");
    rOut.append(pFrame->isFrameAreaSizeValid() ? '+' : '-');
    rOut.append(" ValidPos");
    rOut.append(pFrame->isFrameAreaPositionValid() ? '+' : '-');
    rOut.append(" ValidPrtArea");
    rOut.append(pFrame->isFramePrintAreaValid() ? '+' : '-');
}

static void lcl_Padded(OStringBuffer& rOut, const OString& s, size_t length)
{
    if (sal_Int32(length) < s.getLength())
        length = s.getLength();
    rOut.append(s);
    for (size_t i = 0; i < length - s.getLength(); i++)
    {
        rOut.append(" ");
    }
}

static void lcl_Padded(OStringBuffer& rOut, const tools::Long n, size_t length = 5)
{
    char sz[RTL_STR_MAX_VALUEOFINT64];
    rtl_str_valueOfInt64(sz, n, 10);
    OString s(sz);
    lcl_Padded(rOut, s, length);
}

/// output the frame as plain text.
static void lcl_FrameRect(OStringBuffer& rOut, const char* hint, const SwRect& rect)
{
    rOut.append("[");
    rOut.append(hint);
    rOut.append(":X:");
    lcl_Padded(rOut, rect.Pos().X());
    rOut.append(", Y:");
    lcl_Padded(rOut, rect.Pos().Y());
    rOut.append(", Width:");
    lcl_Padded(rOut, rect.SSize().Width());
    rOut.append(", Height:");
    lcl_Padded(rOut, rect.SSize().Height());
    rOut.append("] ");
}

static OString lcl_TableInfo(const SwTabFrame* pTabFrame)
{
    const SwTable* pTable = pTabFrame->GetTable();
    const SwFormat* pFormat = static_cast<const SwFormat*>(pTable->GetRegisteredIn());
    const OUString& text = pFormat->GetName();
    return OUStringToOString(text, RTL_TEXTENCODING_ASCII_US);
}

static OString lcl_RowInfo(const SwRowFrame* pFrame)
{
    // dummy, needs actual functionality...
    if (pFrame == nullptr)
        return "";
    const SwTableLine* pTabLine = pFrame->GetTabLine();
    if (pTabLine == nullptr)
        return "";

    return "RowInfo";
}

static OUString lcl_CellText(const SwCellFrame* pFrame)
{
    OUString result;
    int n = 0;

    const SwStartNode* pStartNode = pFrame->GetTabBox()->GetSttNd();
    const SwEndNode* pEndNode = pStartNode->EndOfSectionNode();
    const SwNodes& nodes = pStartNode->GetNodes();

    for (sal_uLong i = pStartNode->GetIndex(); i < nodes.Count(); i++)
    {
        SwNode* pNode = nodes[i];

        if (pNode->IsEndNode())
        {
            if (pNode->EndOfSectionNode() == pEndNode)
                break;
        }
        else if (pNode->IsTextNode())
        {
            n++;
            result += "Para:" + OUString::number(10) + " " +
                pNode->GetTextNode()->GetText();
        }
    }

    return OUString::number(n) + " para(s):" + result;
}

static OString lcl_CellInfo(const SwCellFrame* pFrame)
{
    const OUString text = "CellInfo: " + pFrame->GetTabBox()->GetName() + " Text: " + lcl_CellText(pFrame);
    return OUStringToOString(text, RTL_TEXTENCODING_ASCII_US);
}

/// output the type of the frame as plain text.
static void lcl_FrameType( OStringBuffer& rOut, const SwFrame* pFrame )
{
    if( pFrame->IsTextFrame() )
        rOut.append("SwTextFrame ");
    else if( pFrame->IsLayoutFrame() )
    {
        if( pFrame->IsPageFrame() )
            rOut.append("SwPageFrame ");
        else if( pFrame->IsColumnFrame() )
            rOut.append("SwColumnFrame ");
        else if( pFrame->IsBodyFrame() )
        {
            if( pFrame->GetUpper() && pFrame->IsColBodyFrame() )
                rOut.append("(Col)");
            rOut.append("SwBodyFrame ");
        }
        else if( pFrame->IsRootFrame() )
            rOut.append("SwRootFrame ");
        else if( pFrame->IsCellFrame() )
            rOut.append("SwCellFrame ");
        else if( pFrame->IsTabFrame() )
            rOut.append("SwTabFrame ");
        else if( pFrame->IsRowFrame() )
            rOut.append("SwRowFrame ");
        else if( pFrame->IsSctFrame() )
            rOut.append("SwSectionFrame ");
        else if( pFrame->IsHeaderFrame() )
            rOut.append("SwHeaderFrame ");
        else if( pFrame->IsFooterFrame() )
            rOut.append("SwFooterFrame ");
        else if( pFrame->IsFootnoteFrame() )
            rOut.append("SwFootnoteFrame ");
        else if( pFrame->IsFootnoteContFrame() )
            rOut.append("SwFootnoteContFrame ");
        else if( pFrame->IsFlyFrame() )
            rOut.append("SwFlyFrame ");
        else
            rOut.append("SwLayoutFrame ");
    }
    else if( pFrame->IsNoTextFrame() )
        rOut.append("SwNoTextFrame");
    else
        rOut.append("Not impl. ");
}

/**
 * Is only called if the PROTOCOL macro finds out,
 * that this function should be recorded ( @see{SwProtocol::nRecord} ).
 *
 * In this method we also check if FrameId and frame type should be logged.
 */
void SwImplProtocol::Record_( const SwFrame* pFrame, PROT nFunction, DbgAction nAct, void* pParam )
{
    sal_uInt16 nSpecial = 0;
    if( nSpecial )  // the possible debugger manipulations
    {
        sal_uInt16 nId = sal_uInt16(lcl_GetFrameId( pFrame ));
        switch ( nSpecial )
        {
            case 1: InsertFrame( nId ); break;
            case 2: DeleteFrame( nId ); break;
            case 3:
                m_pFrameIds.reset();
                break;
            case 4:
                m_pStream.reset();
                break;
        }
        return;
    }
    if (!m_pStream && !NewStream())
        return; // still no stream

    if (m_pFrameIds && !m_pFrameIds->count(sal_uInt16(lcl_GetFrameId(pFrame))))
        return; // doesn't belong to the wished FrameIds

    if (!(pFrame->GetType() & m_nTypes))
        return; // the type is unwanted

    if (1 == m_nTestMode && nFunction != PROT::TestFormat)
        return; // we may only log inside a test formatting
    bool bTmp = false;
    OStringBuffer aOut(m_aLayer);
    aOut.append(static_cast<sal_Int64>(lcl_GetFrameId(pFrame)));
    aOut.append(' ');
    lcl_FrameType( aOut, pFrame );    // then the frame type
    switch ( nFunction )            // and the function
    {
        case PROT::MakeAll:  aOut.append("SwFrame::MakeAll");
            lcl_Start(aOut, m_aLayer, nAct);
            if (nAct == DbgAction::Start)
                lcl_Flags(aOut, pFrame);
            break;
        case PROT::MoveFwd: bTmp = true;
                            [[fallthrough]];
        case PROT::MoveBack:
                            if (nFunction == (bTmp ? PROT::Init : PROT::FileInit))
                                aOut.append("SwFlowFrame::MoveFwd");
                            else
                                aOut.append("SwFlowFrame::MoveBwd");
                            lcl_Start(aOut, m_aLayer, nAct);
                            if( pParam )
                            {
                                aOut.append(' ');
                                aOut.append(static_cast<sal_Int32>(*static_cast<sal_uInt16*>(pParam)));
                            }
                            break;
        case PROT::GrowTest:
                            aOut.append("SwFrame::Grow (test)");
                            lcl_Start(aOut, m_aLayer, nAct);
                            break;
        case PROT::ShrinkTest:
                            aOut.append("SwFrame::Shrink (test)");
                            lcl_Start(aOut, m_aLayer, nAct);
                            break;
        case PROT::AdjustN :
        case PROT::Shrink:   bTmp = true;
                            [[fallthrough]];
        case PROT::Grow:
                            if (!bTmp)
                                aOut.append("SwFrame::Grow");
                            else
                            {
                                if (nFunction == PROT::Shrink)
                                    aOut.append("SwFrame::Shrink");
                                else
                                    aOut.append("SwFrame::AdjustNeighbourhood");
                            }
                            lcl_Start(aOut, m_aLayer, nAct);
                            if( pParam )
                            {
                                aOut.append(' ');
                                aOut.append(static_cast<sal_Int64>(*static_cast<tools::Long*>(pParam)));
                            }
                            break;
        case PROT::PrintArea:  aOut.append("PROT::PrintArea");
            lcl_Start(aOut, m_aLayer, nAct);
            break;
        case PROT::Size:     aOut.append("PROT::Size");
            lcl_Start(aOut, m_aLayer, nAct);
            aOut.append(' ');
            aOut.append(static_cast<sal_Int64>(pFrame->getFrameArea().Height()));
            break;
        case PROT::Leaf:     aOut.append("SwFrame::GetPrev/NextSctLeaf");
            lcl_Start(aOut, m_aLayer, nAct);
            aOut.append(' ');
            if (pParam)
            {
                aOut.append(' ');
                aOut.append(static_cast<sal_Int64>(lcl_GetFrameId(static_cast<SwFrame*>(pParam))));
                            }
                            break;
        case PROT::FileInit: FileInit();
                            aOut.append("Initialize");
                            break;
        case PROT::Section:  SectFunc(aOut, nAct, pParam);
                            break;
        case PROT::Cut:      bTmp = true;
                            [[fallthrough]];
        case PROT::Paste:
                            if (bTmp)
                                aOut.append("PROT::Cut from ");
                            else
                                aOut.append("PROT::Paste to ");
                            aOut.append(static_cast<sal_Int64>(lcl_GetFrameId(static_cast<SwFrame*>(pParam))));
                            break;
        case PROT::TestFormat:
                            aOut.append("SwTextFrame::TestFormat");
                            lcl_Start(aOut, m_aLayer, nAct);
                            if( DbgAction::Start == nAct )
                                m_nTestMode |= 2;
                            else
                                m_nTestMode &= ~2;
                            break;
        case PROT::FrmChanges:
                            {
                                SwRect& rFrame = *static_cast<SwRect*>(pParam);
                                if( pFrame->getFrameArea().Pos() != rFrame.Pos() )
                                {
                                    aOut.append("PosChg: (");
                                    aOut.append(static_cast<sal_Int64>(rFrame.Left()));
                                    aOut.append(", ");
                                    aOut.append(static_cast<sal_Int64>(rFrame.Top()));
                                    aOut.append(") -> (");
                                    aOut.append(static_cast<sal_Int64>(pFrame->getFrameArea().Left()));
                                    aOut.append(", ");
                                    aOut.append(static_cast<sal_Int64>(pFrame->getFrameArea().Top()));
                                    aOut.append(") ");
                                }
                                if( pFrame->getFrameArea().Height() != rFrame.Height() )
                                {
                                    aOut.append("Height: ");
                                    aOut.append(static_cast<sal_Int64>(rFrame.Height()));
                                    aOut.append(" -> ");
                                    aOut.append(static_cast<sal_Int64>(pFrame->getFrameArea().Height()));
                                    aOut.append(" ");
                                }
                                if( pFrame->getFrameArea().Width() != rFrame.Width() )
                                {
                                    aOut.append("Width: ");
                                    aOut.append(static_cast<sal_Int64>(rFrame.Width()));
                                    aOut.append(" -> ");
                                    aOut.append(static_cast<sal_Int64>(pFrame->getFrameArea().Width()));
                                    aOut.append(' ');
                                }
                                break;
                            }
        default: break;
    }

    aOut.append("  ");
    while (aOut.getLength() < 40) aOut.append(" ");
    lcl_FrameRect(aOut, "SwFrame", pFrame->getFrameArea());

    aOut.append(" ");
    while (aOut.getLength() < 90) aOut.append(" ");
    lcl_FrameRect(aOut, "SwPrint", pFrame->getFramePrintArea());

    if (pFrame->IsTextFrame())
    {
        aOut.append(" ");
        while (aOut.getLength() < 140) aOut.append(" ");
        const OUString& text = static_cast<const SwTextFrame*>(pFrame)->GetText();
        OString o = OUStringToOString(text, RTL_TEXTENCODING_ASCII_US);
        aOut.append(o);
    }
    else if (pFrame->IsTabFrame())
    {
        const SwTabFrame* pTabFrame = static_cast<const SwTabFrame*>(pFrame);
        aOut.append(lcl_TableInfo(pTabFrame));
    }
    else if (pFrame->IsRowFrame())
    {
        const SwRowFrame* pRowFrame = static_cast<const SwRowFrame*>(pFrame);
        aOut.append(lcl_RowInfo(pRowFrame));

    }
    else if (pFrame->IsCellFrame())
    {
        const SwCellFrame* pCellFrame = static_cast<const SwCellFrame*>(pFrame);
        aOut.append(lcl_CellInfo(pCellFrame));
    }

    SAL_INFO("sw.layout.debug", aOut.getStr());
    m_pStream->WriteOString(aOut.makeStringAndClear());
    (*m_pStream) << endl; // output
    m_pStream->Flush(); // to the disk, so we can read it immediately
    if (++m_nLineCount >= m_nMaxLines) // max number of lines reached?
    {
        SAL_WARN("sw.layout.debug", "max number of lines reached");
        SwProtocol::SetRecord( PROT::FileInit );        // => end f logging
    }
}

/// Handle the output of the SectionFrames.
void SwImplProtocol::SectFunc(OStringBuffer &rOut, DbgAction nAct, void const * pParam)
{
    bool bTmp = false;
    switch( nAct )
    {
        case DbgAction::Merge:  rOut.append("Merge Section ");
                                rOut.append(static_cast<sal_Int64>(lcl_GetFrameId(static_cast<SwFrame const *>(pParam))));
                                break;
        case DbgAction::CreateMaster: bTmp = true;
                                [[fallthrough]];
        case DbgAction::CreateFollow: rOut.append("Create Section ");
                                if (bTmp)
                                    rOut.append("Master to ");
                                else
                                    rOut.append("Follow from ");
                                rOut.append(static_cast<sal_Int64>(lcl_GetFrameId(static_cast<SwFrame const *>(pParam))));
                                break;
        case DbgAction::DelMaster:    bTmp = true;
                                [[fallthrough]];
        case DbgAction::DelFollow:    rOut.append("Delete Section ");
                                if (bTmp)
                                    rOut.append("Master to ");
                                else
                                    rOut.append("Follow from ");
                                rOut.append(static_cast<sal_Int64>(lcl_GetFrameId(static_cast<SwFrame const *>(pParam))));
                                break;
        default: break;
    }
}

/**
 * if pFrameIds==NULL all Frames will be logged. But as soon as pFrameIds are
 * set, only the added FrameIds are being logged.
 *
 * @param nId new FrameId for logging
 * @return TRUE if newly added, FALSE if FrameId is already under control
 */
void SwImplProtocol::InsertFrame( sal_uInt16 nId )
{
    if (!m_pFrameIds)
        m_pFrameIds.reset(new std::set<sal_uInt16>);
    if (m_pFrameIds->count(nId))
        return;
    m_pFrameIds->insert(nId);
}

/// Removes a FrameId from the pFrameIds array, so that it won't be logged anymore.
void SwImplProtocol::DeleteFrame( sal_uInt16 nId )
{
    if (!m_pFrameIds)
        return;
    m_pFrameIds->erase(nId);
}

/*
 * The task here is to find the right SwImplEnterLeave object based on the
 * function; everything else is then done in his Ctor/Dtor.
 */
SwEnterLeave::SwEnterLeave( const SwFrame* pFrame, PROT nFunc, DbgAction nAct, void* pPar )
{
    if( !SwProtocol::Record( nFunc ) )
        return;
    switch( nFunc )
    {
        case PROT::AdjustN :
        case PROT::Grow:
        case PROT::Shrink : pImpl.reset( new SwSizeEnterLeave( pFrame, nFunc, nAct, pPar ) ); break;
        case PROT::MoveFwd:
        case PROT::MoveBack : pImpl.reset( new SwUpperEnterLeave( pFrame, nFunc, nAct, pPar ) ); break;
        case PROT::FrmChanges : pImpl.reset( new SwFrameChangesLeave( pFrame, nFunc, nAct, pPar ) ); break;
        default: pImpl.reset( new SwImplEnterLeave( pFrame, nFunc, nAct, pPar ) ); break;
    }
    pImpl->Enter();
}

/* This is not inline because we don't want the SwImplEnterLeave definition inside
 * dbg_lay.hxx.
 */
SwEnterLeave::~SwEnterLeave()
{
    if (pImpl)
        pImpl->Leave();
}

void SwImplEnterLeave::Enter()
{
    SwProtocol::Record(m_pFrame, m_nFunction, DbgAction::Start, m_pParam);
}

void SwImplEnterLeave::Leave() {
    SwProtocol::Record(m_pFrame, m_nFunction, DbgAction::End, m_pParam);
}

void SwSizeEnterLeave::Leave()
{
    m_nFrameHeight = m_pFrame->getFrameArea().Height() - m_nFrameHeight;
    SwProtocol::Record(m_pFrame, m_nFunction, DbgAction::End, &m_nFrameHeight);
}

void SwUpperEnterLeave::Enter()
{
    m_nFrameId = m_pFrame->GetUpper() ? sal_uInt16(lcl_GetFrameId(m_pFrame->GetUpper())) : 0;
    SwProtocol::Record(m_pFrame, m_nFunction, DbgAction::Start, &m_nFrameId);
}

void SwUpperEnterLeave::Leave()
{
    m_nFrameId = m_pFrame->GetUpper() ? sal_uInt16(lcl_GetFrameId(m_pFrame->GetUpper())) : 0;
    SwProtocol::Record(m_pFrame, m_nFunction, DbgAction::End, &m_nFrameId);
}

void SwFrameChangesLeave::Enter()
{
}

void SwFrameChangesLeave::Leave()
{
    if (m_pFrame->getFrameArea() != m_aFrame)
        SwProtocol::Record(m_pFrame, PROT::FrmChanges, DbgAction::NONE, &m_aFrame);
}

#endif // DBG_UTIL

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
