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
 * 2.   The function group i.e. PROT_MAKEALL. This is used to decide (inline)
 *      whether this event shall be logged at the current time.
 * 3.   The action, usually 0. For example ACT_START indents output in the log
 *      file and ACT_END stops the indentation. This allows for example
 *      PROTOCOL_ENTER to indent at the beginning of a method and stop indenting
 *      when leaving the method.
 * 4.   The fourth parameter is a void pointer which allows to pass anything
 *      which can be used in the log. A good example is PROT_GROW: this requires
 *      a pointer to the value which defines how much to grow.
 *
 * The log file is called "dbg_lay.out", which is saved in the current (BIN-)
 * directory. The file contains lines with FrameId, function group and additional
 * information.
 *
 * What exactly is going to be logged, can be defined as follows:
 * 1.   The static variable SwProtocol::nRecord contains the function groups
 *      which shall be logged.
 *      A value of i.e. PROT_GROW causes calls to SwFrame::Grow to be
 *      logged; PROT_MAKEALL logs the calls to xxx::MakeAll.
 *      The PROT_XY values can be combined using binary OR, the default value
 *      is null - no method calls are logged.
 * 2.   The SwImplProtocol class contains a filter for frame types, only method
 *      call of frame types which are defined there are logged.
 *      The member nTypes can be set to values like FRM_PAGE or FRM_SECTION and
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
 *      program execution, then set the lowest bit (PROT_INIT) of
 *      SwProtocol::nRecord. This activates the function group of the following
 *      macro and causes it to be logged in the future.
 * 3.   There's a special case for 2: If one uses 2. in SwRootFrame::Paint(..),
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
 *           #Functions: all, except PRTARE
 *           [record] 0xFFFFFFFE !0x200
 *           [frmid]
 *           #the following FrameIds:
 *           1 2 12 13 14 15
 *           #no layout frames, except ColumnFrames
 *           [frmtype] !0x3FFF 0x4
 *
 * As soon as the logging is in process, one can manipulate many things in
 * SwImplProtocol::_Record(...) using a debugger, especially concerning
 * frame types and FrameIds.
 */

#include "dbg_lay.hxx"

#include "flyfrm.hxx"
#include "txtfrm.hxx"
#include <fntcache.hxx>

#include <comphelper/string.hxx>

sal_uLong SwProtocol::nRecord = 0;
SwImplProtocol* SwProtocol::pImpl = nullptr;

static sal_uLong lcl_GetFrameId( const SwFrame* pFrame )
{
#if OSL_DEBUG_LEVEL > 1
    static bool bFrameId = false;
    if( bFrameId )
        return pFrame->GetFrameId();
#endif
    if( pFrame && pFrame->IsTextFrame() )
        return static_cast<const SwTextFrame*>(pFrame)->GetTextNode()->GetIndex();
    return 0;
}

class SwImplProtocol
{
    SvFileStream *pStream;          // output stream
    std::set<sal_uInt16> *pFrameIds;  // which FrameIds shall be logged ( NULL == all)
    std::vector<long> aVars;        // variables
    OStringBuffer aLayer;      // indentation of output ("  " per start/end)
    sal_uInt16 nTypes;              // which types shall be logged
    sal_uInt16 nLineCount;          // printed lines
    sal_uInt16 nMaxLines;           // max lines to be printed
    sal_uInt8 nInitFile;            // range (FrameId,FrameType,Record) during reading of the INI file
    sal_uInt8 nTestMode;            // special for test formatting, logging may only be done in test formatting.
    void _Record( const SwFrame* pFrame, sal_uLong nFunction, sal_uLong nAct, void* pParam );
    bool NewStream();
    void CheckLine( OString& rLine );
    static void SectFunc( OStringBuffer& rOut, const SwFrame* pFrame, sal_uLong nAct, void* pParam );
public:
    SwImplProtocol();
    ~SwImplProtocol();
    // logging
    void Record( const SwFrame* pFrame, sal_uLong nFunction, sal_uLong nAct, void* pParam )
        { if( pStream ) _Record( pFrame, nFunction, nAct, pParam ); }
    bool InsertFrame( sal_uInt16 nFrameId );    // take FrameId for logging
    bool DeleteFrame( sal_uInt16 nFrameId );    // remove FrameId; don't log him anymore
    void FileInit();                    // read the INI file
    void ChkStream() { if( !pStream ) NewStream(); }
};

/* Through the PROTOCOL_ENTER macro a SwEnterLeave object gets created. If the
 * current function should be logged a SwImplEnterLeace object gets created.
 * The funny thing here is, that the Ctor of the Impl object is automatically
 * called at the beginning of the function and the Dtor is automatically called
 * when leaving the function. In the base implementation the Ctor calls only
 * PROTOCOL(..) with ACT_START and in the Dtor a PROTOCOL(..) with ACT_END.
 * It's possible to derive from this class, for example to be able to document
 * frame resize while leaving a function. To do this, one only needs to add the
 * desired SwImplEnterLeave class in SwEnterLeave::Ctor().
 */

class SwImplEnterLeave
{
protected:
    const SwFrame* pFrame;              // the frame
    sal_uLong nFunction, nAction;   // the function, the action if needed
    void* pParam;                   // further parameter
public:
    SwImplEnterLeave( const SwFrame* pF, sal_uLong nFunct, sal_uLong nAct, void* pPar )
        : pFrame( pF ), nFunction( nFunct ), nAction( nAct ), pParam( pPar ) {}
    virtual ~SwImplEnterLeave() {}
    virtual void Enter();           // message when entering
    virtual void Leave();           // message when leaving
};

class SwSizeEnterLeave : public SwImplEnterLeave
{
    long nFrameHeight;
public:
    SwSizeEnterLeave( const SwFrame* pF, sal_uLong nFunct, sal_uLong nAct, void* pPar )
        : SwImplEnterLeave( pF, nFunct, nAct, pPar ), nFrameHeight( pF->Frame().Height() ) {}
    virtual ~SwSizeEnterLeave() {}
    virtual void Leave() override;           // resize message
};

class SwUpperEnterLeave : public SwImplEnterLeave
{
    sal_uInt16 nFrameId;
public:
    SwUpperEnterLeave( const SwFrame* pF, sal_uLong nFunct, sal_uLong nAct, void* pPar )
        : SwImplEnterLeave( pF, nFunct, nAct, pPar ), nFrameId( 0 ) {}
    virtual ~SwUpperEnterLeave() {}
    virtual void Enter() override;           // message
    virtual void Leave() override;           // message of FrameId from upper
};

class SwFrameChangesLeave : public SwImplEnterLeave
{
    SwRect aFrame;
public:
    SwFrameChangesLeave( const SwFrame* pF, sal_uLong nFunct, sal_uLong nAct, void* pPar )
        : SwImplEnterLeave( pF, nFunct, nAct, pPar ), aFrame( pF->Frame() ) {}
    virtual ~SwFrameChangesLeave() {}
    virtual void Enter() override;           // no message
    virtual void Leave() override;           // message when resizing the Frame area
};

void SwProtocol::Record( const SwFrame* pFrame, sal_uLong nFunction, sal_uLong nAct, void* pParam )
{
    if( Start() )
    {   // We reach this point if SwProtocol::nRecord is binary OR'd with PROT_INIT(0x1) using the debugger
        bool bFinit = false; // This gives the possibility to stop logging of this action in the debugger
        if( bFinit )
        {
            nRecord &= ~nFunction;  // Don't log this function any longer
            nRecord &= ~PROT_INIT;  // Always reset PROT_INIT
            return;
        }
        nRecord |= nFunction;       // Activate logging of this function
        nRecord &= ~PROT_INIT;      // Always reset PROT_INIT
        if( pImpl )
            pImpl->ChkStream();
    }
    if( !pImpl )                        // Create Impl object if needed
        pImpl = new SwImplProtocol();
    pImpl->Record( pFrame, nFunction, nAct, pParam ); // ...and start logging
}

// The following function gets called when pulling in the writer DLL through
// TextInit(..) and gives the possibility to release functions
// and/or FrameIds to the debugger

void SwProtocol::Init()
{
    nRecord = 0;
    OUString aName("dbg_lay.go");
    SvFileStream aStream( aName, StreamMode::READ );
    if( aStream.IsOpen() )
    {
        pImpl = new SwImplProtocol();
        pImpl->FileInit();
    }
    aStream.Close();
}

// End of logging

void SwProtocol::Stop()
{
     if( pImpl )
     {
        delete pImpl;
        pImpl = nullptr;
        if( pFntCache )
            pFntCache->Flush();
     }
     nRecord = 0;
}

SwImplProtocol::SwImplProtocol()
    : pStream( nullptr ), pFrameIds( nullptr ), nTypes( 0xffff ),
      nLineCount( 0 ), nMaxLines( USHRT_MAX ), nTestMode( 0 )
{
    NewStream();
}

bool SwImplProtocol::NewStream()
{
    OUString aName("dbg_lay.out");
    nLineCount = 0;
    pStream = new SvFileStream( aName, StreamMode::WRITE | StreamMode::TRUNC );
    if( pStream->GetError() )
    {
        delete pStream;
        pStream = nullptr;
    }
    return nullptr != pStream;
}

SwImplProtocol::~SwImplProtocol()
{
    if( pStream )
    {
        pStream->Close();
        delete pStream;
    }
    pFrameIds->clear();
    delete pFrameIds;
    aVars.clear();
}

/// analyze a line in the INI file
void SwImplProtocol::CheckLine( OString& rLine )
{
    rLine = rLine.toAsciiLowerCase(); // upper/lower case is the same
    rLine = rLine.replace( '\t', ' ' );
    if( '#' == rLine[0] )   // comments start with '#'
        return;
    if( '[' == rLine[0] )   // section: FrameIds, type or funciton
    {
        OString aTmp = rLine.getToken(0, ']');
        if (aTmp == "[frmid")      // section FrameIds
        {
            nInitFile = 1;
            pFrameIds->clear();
            delete pFrameIds;
            pFrameIds = nullptr;         // default: log all frames
        }
        else if (aTmp == "[frmtype")// section types
        {
            nInitFile = 2;
            nTypes = USHRT_MAX;     // default: log all frame types
        }
        else if (aTmp == "[record")// section functions
        {
            nInitFile = 3;
            SwProtocol::SetRecord( 0 );// default: don't log any function
        }
        else if (aTmp == "[test")// section functions
        {
            nInitFile = 4; // default:
            nTestMode = 0; // log outside of test formatting
        }
        else if (aTmp == "[max")// Max number of lines
        {
            nInitFile = 5; // default:
            nMaxLines = USHRT_MAX;
        }
        else if (aTmp == "[var")// variables
        {
            nInitFile = 6;
        }
        else
            nInitFile = 0;          // oops: unknown section?
        rLine = rLine.copy(aTmp.getLength() + 1);
    }

    // spaces (or tabs) are the delimiter
    sal_Int32 nIndex = 0;
    do
    {
        OString aTok = rLine.getToken( 0, ' ', nIndex );
        bool bNo = false;
        if( '!' == aTok[0] )
        {
            bNo = true;                 // remove this function/type
            aTok = aTok.copy(1);
        }
        if( !aTok.isEmpty() )
        {
            sal_uLong nVal;
            sscanf( aTok.getStr(), "%li", &nVal );
            switch ( nInitFile )
            {
                case 1: InsertFrame( sal_uInt16( nVal ) );    // add FrameId
                        break;
                case 2: {
                            sal_uInt16 nNew = (sal_uInt16)nVal;
                            if( bNo )
                                nTypes &= ~nNew;    // remove type
                            else
                                nTypes |= nNew;     // add type
                        }
                        break;
                case 3: {
                            sal_uLong nOld = SwProtocol::Record();
                            if( bNo )
                                nOld &= ~nVal;      // remove function
                            else
                                nOld |= nVal;       // remove function
                            SwProtocol::SetRecord( nOld );
                        }
                        break;
                case 4: {
                            sal_uInt8 nNew = (sal_uInt8)nVal;
                            if( bNo )
                                nTestMode &= ~nNew; // reset test mode
                            else
                                nTestMode |= nNew;      // set test mode
                        }
                        break;
                case 5: nMaxLines = (sal_uInt16)nVal;
                        break;
                case 6: aVars.push_back( (long)nVal );
                        break;
            }
        }
    }
    while ( nIndex >= 0 );
}

/// read the file "dbg_lay.ini" in the current directory and evaluate it.
void SwImplProtocol::FileInit()
{
    OUString aName("dbg_lay.ini");
    SvFileStream aStream( aName, StreamMode::READ );
    if( aStream.IsOpen() )
    {
        OString aLine;
        nInitFile = 0;
        while( aStream.good() )
        {
            sal_Char c;
            aStream.ReadChar( c );
            if( '\n' == c || '\r' == c )    // line ending
            {
                aLine = aLine.trim();
                if( !aLine.isEmpty() )
                    CheckLine( aLine );     // evaluate line
                aLine.clear();
            }
            else
                aLine = OString(c);
        }
        if( !aLine.isEmpty() )
            CheckLine( aLine );     // evaluate last line
    }
    aStream.Close();
}

/// enable indentation by two spaces during ACT_START and disable it again at ACT_END.
static void lcl_Start(OStringBuffer& rOut, OStringBuffer& rLay, sal_uLong nAction)
{
    if( nAction == ACT_START )
    {
        rLay.append("  ");
        rOut.append(" On");
    }
    else if( nAction == ACT_END )
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
    rOut.append(" Sz");
    rOut.append(pFrame->GetValidSizeFlag() ? '+' : '-');
    rOut.append(" Ps");
    rOut.append(pFrame->GetValidPosFlag() ? '+' : '-');
    rOut.append(" PA");
    rOut.append(pFrame->GetValidPrtAreaFlag() ? '+' : '-');
}

/// output the type of the frame as plain text.
static void lcl_FrameType( OStringBuffer& rOut, const SwFrame* pFrame )
{
    if( pFrame->IsTextFrame() )
        rOut.append("Text ");
    else if( pFrame->IsLayoutFrame() )
    {
        if( pFrame->IsPageFrame() )
            rOut.append("Page ");
        else if( pFrame->IsColumnFrame() )
            rOut.append("Col ");
        else if( pFrame->IsBodyFrame() )
        {
            if( pFrame->GetUpper() && pFrame->IsColBodyFrame() )
                rOut.append("(Col)");
            rOut.append("Body ");
        }
        else if( pFrame->IsRootFrame() )
            rOut.append("Root ");
        else if( pFrame->IsCellFrame() )
            rOut.append("Cell ");
        else if( pFrame->IsTabFrame() )
            rOut.append("Tab ");
        else if( pFrame->IsRowFrame() )
            rOut.append("Row ");
        else if( pFrame->IsSctFrame() )
            rOut.append("Sect ");
        else if( pFrame->IsHeaderFrame() )
            rOut.append("Header ");
        else if( pFrame->IsFooterFrame() )
            rOut.append("Footer ");
        else if( pFrame->IsFootnoteFrame() )
            rOut.append("Footnote ");
        else if( pFrame->IsFootnoteContFrame() )
            rOut.append("FootnoteCont ");
        else if( pFrame->IsFlyFrame() )
            rOut.append("Fly ");
        else
            rOut.append("Layout ");
    }
    else if( pFrame->IsNoTextFrame() )
        rOut.append("NoText ");
    else
        rOut.append("Not impl. ");
}

/**
 * Is only called if the PROTOCOL macro finds out,
 * that this function should be recorded ( @see{SwProtocol::nRecord} ).
 *
 * In this method we also check if FrameId and frame type should be logged.
 */
void SwImplProtocol::_Record( const SwFrame* pFrame, sal_uLong nFunction, sal_uLong nAct, void* pParam )
{
    sal_uInt16 nSpecial = 0;
    if( nSpecial )  // the possible debugger manipulations
    {
        sal_uInt16 nId = sal_uInt16(lcl_GetFrameId( pFrame ));
        switch ( nSpecial )
        {
            case 1: InsertFrame( nId ); break;
            case 2: DeleteFrame( nId ); break;
            case 3: pFrameIds->clear(); delete pFrameIds; pFrameIds = nullptr; break;
            case 4: delete pStream; pStream = nullptr; break;
        }
        return;
    }
    if( !pStream && !NewStream() )
        return; // still no stream

    if( pFrameIds && !pFrameIds->count( sal_uInt16(lcl_GetFrameId( pFrame )) ) )
        return; // doesn't belong to the wished FrameIds

    if( !(pFrame->GetType() & nTypes) )
        return; // the type is unwanted

    if( 1 == nTestMode && nFunction != PROT_TESTFORMAT )
        return; // we may only log inside a test formatting
    bool bTmp = false;
    OStringBuffer aOut(aLayer);
    aOut.append(static_cast<sal_Int64>(lcl_GetFrameId(pFrame)));
    aOut.append(' ');
    lcl_FrameType( aOut, pFrame );    // then the frame type
    switch ( nFunction )            // and the function
    {
        case PROT_SNAPSHOT: lcl_Flags( aOut, pFrame );
                            break;
        case PROT_MAKEALL:  aOut.append("MakeAll");
                            lcl_Start( aOut, aLayer, nAct );
                            if( nAct == ACT_START )
                                lcl_Flags( aOut, pFrame );
                            break;
        case PROT_MOVE_FWD: bTmp = true;
                            SAL_FALLTHROUGH;
        case PROT_MOVE_BWD:
                            if (nFunction == (bTmp ? 1U : 0U))
                                aOut.append("Fwd");
                            else
                                aOut.append("Bwd");
                            lcl_Start( aOut, aLayer, nAct );
                            if( pParam )
                            {
                                aOut.append(' ');
                                aOut.append(static_cast<sal_Int32>(*static_cast<sal_uInt16*>(pParam)));
                            }
                            break;
        case PROT_GROW_TST: if( ACT_START != nAct )
                                return;
                            aOut.append("TestGrow");
                            break;
        case PROT_SHRINK_TST: if( ACT_START != nAct )
                                return;
                            aOut.append("TestShrink");
                            break;
        case PROT_ADJUSTN :
        case PROT_SHRINK:   bTmp = true;
                            SAL_FALLTHROUGH;
        case PROT_GROW:
                            if (!bTmp)
                                aOut.append("Grow");
                            else
                            {
                                if (nFunction == PROT_SHRINK)
                                    aOut.append("Shrink");
                                else
                                    aOut.append("AdjustNgbhd");
                            }
                            lcl_Start( aOut, aLayer, nAct );
                            if( pParam )
                            {
                                aOut.append(' ');
                                aOut.append(static_cast<sal_Int64>(*static_cast<long*>(pParam)));
                            }
                            break;
        case PROT_POS:      break;
        case PROT_PRTAREA:  aOut.append("PrtArea");
                            lcl_Start( aOut, aLayer, nAct );
                            break;
        case PROT_SIZE:     aOut.append("Size");
                            lcl_Start( aOut, aLayer, nAct );
                            aOut.append(' ');
                            aOut.append(static_cast<sal_Int64>(pFrame->Frame().Height()));
                            break;
        case PROT_LEAF:     aOut.append("Prev/NextLeaf");
                            lcl_Start( aOut, aLayer, nAct );
                            aOut.append(' ');
                            if( pParam )
                            {
                                aOut.append(' ');
                                aOut.append(static_cast<sal_Int64>(lcl_GetFrameId(static_cast<SwFrame*>(pParam))));
                            }
                            break;
        case PROT_FILE_INIT: FileInit();
                            aOut.append("Initialize");
                            break;
        case PROT_SECTION:  SectFunc(aOut, pFrame, nAct, pParam);
                            break;
        case PROT_CUT:      bTmp = true;
                            SAL_FALLTHROUGH;
        case PROT_PASTE:
                            if (bTmp)
                                aOut.append("Cut from ");
                            else
                                aOut.append("Paste to ");
                            aOut.append(static_cast<sal_Int64>(lcl_GetFrameId(static_cast<SwFrame*>(pParam))));
                            break;
        case PROT_TESTFORMAT:
                            aOut.append("Test");
                            lcl_Start( aOut, aLayer, nAct );
                            if( ACT_START == nAct )
                                nTestMode |= 2;
                            else
                                nTestMode &= ~2;
                            break;
        case PROT_FRMCHANGES:
                            {
                                SwRect& rFrame = *static_cast<SwRect*>(pParam);
                                if( pFrame->Frame().Pos() != rFrame.Pos() )
                                {
                                    aOut.append("PosChg: (");
                                    aOut.append(static_cast<sal_Int64>(rFrame.Left()));
                                    aOut.append(", ");
                                    aOut.append(static_cast<sal_Int64>(rFrame.Top()));
                                    aOut.append(") (");
                                    aOut.append(static_cast<sal_Int64>(pFrame->Frame().Left()));
                                    aOut.append(", ");
                                    aOut.append(static_cast<sal_Int64>(pFrame->Frame().Top()));
                                    aOut.append(") ");
                                }
                                if( pFrame->Frame().Height() != rFrame.Height() )
                                {
                                    aOut.append("Height: ");
                                    aOut.append(static_cast<sal_Int64>(rFrame.Height()));
                                    aOut.append(" -> ");
                                    aOut.append(static_cast<sal_Int64>(pFrame->Frame().Height()));
                                    aOut.append(" ");
                                }
                                if( pFrame->Frame().Width() != rFrame.Width() )
                                {
                                    aOut.append("Width: ");
                                    aOut.append(static_cast<sal_Int64>(rFrame.Width()));
                                    aOut.append(" -> ");
                                    aOut.append(static_cast<sal_Int64>(pFrame->Frame().Width()));
                                    aOut.append(' ');
                                }
                                break;
                            }
    }
    pStream->WriteCharPtr( aOut.getStr() );
    (*pStream) << endl;  // output
    pStream->Flush();   // to the disk, so we can read it immediately
    if( ++nLineCount >= nMaxLines )     // max number of lines reached?
        SwProtocol::SetRecord( 0 );        // => end f logging
}

/// Handle the output of the SectionFrames.
void SwImplProtocol::SectFunc(OStringBuffer &rOut, const SwFrame* , sal_uLong nAct, void* pParam)
{
    bool bTmp = false;
    switch( nAct )
    {
        case ACT_MERGE:         rOut.append("Merge Section ");
                                rOut.append(static_cast<sal_Int64>(lcl_GetFrameId(static_cast<SwFrame*>(pParam))));
                                break;
        case ACT_CREATE_MASTER: bTmp = true;
                                SAL_FALLTHROUGH;
        case ACT_CREATE_FOLLOW: rOut.append("Create Section ");
                                if (bTmp)
                                    rOut.append("Master to ");
                                else
                                    rOut.append("Follow from ");
                                rOut.append(static_cast<sal_Int64>(lcl_GetFrameId(static_cast<SwFrame*>(pParam))));
                                break;
        case ACT_DEL_MASTER:    bTmp = true;
                                SAL_FALLTHROUGH;
        case ACT_DEL_FOLLOW:    rOut.append("Delete Section ");
                                if (bTmp)
                                    rOut.append("Master to ");
                                else
                                    rOut.append("Follow from ");
                                rOut.append(static_cast<sal_Int64>(lcl_GetFrameId(static_cast<SwFrame*>(pParam))));
                                break;
    }
}

/**
 * if pFrameIds==NULL all Frames will be logged. But as soon as pFrameIds are
 * set, only the added FrameIds are being logged.
 *
 * @param nId new FrameId for logging
 * @return TRUE if newly added, FALSE if FrameId is already under control
 */
bool SwImplProtocol::InsertFrame( sal_uInt16 nId )
{
    if( !pFrameIds )
        pFrameIds = new std::set<sal_uInt16>;
    if( pFrameIds->count( nId ) )
        return false;
    pFrameIds->insert( nId );
    return true;
}

/// Removes a FrameId from the pFrameIds array, so that it won't be logged anymore.
bool SwImplProtocol::DeleteFrame( sal_uInt16 nId )
{
    if( !pFrameIds )
        return false;
    if ( pFrameIds->erase(nId) )
        return true;
    return false;
}

/* SwEnterLeave::Ctor(..) is called from the (inline-)CTor if the function should
 * be logged.
 * The task here is to find the right SwImplEnterLeave object based on the
 * function; everything else is then done in his Ctor/Dtor.
 */
void SwEnterLeave::Ctor( const SwFrame* pFrame, sal_uLong nFunc, sal_uLong nAct, void* pPar )
{
    switch( nFunc )
    {
        case PROT_ADJUSTN :
        case PROT_GROW:
        case PROT_SHRINK : pImpl = new SwSizeEnterLeave( pFrame, nFunc, nAct, pPar ); break;
        case PROT_MOVE_FWD:
        case PROT_MOVE_BWD : pImpl = new SwUpperEnterLeave( pFrame, nFunc, nAct, pPar ); break;
        case PROT_FRMCHANGES : pImpl = new SwFrameChangesLeave( pFrame, nFunc, nAct, pPar ); break;
        default: pImpl = new SwImplEnterLeave( pFrame, nFunc, nAct, pPar ); break;
    }
    pImpl->Enter();
}

/* SwEnterLeave::Dtor() only calls the Dtor of the SwImplEnterLeave object. It's
 * just no inline because we don't want the SwImplEnterLeave definition inside
 * dbg_lay.hxx.
 */
void SwEnterLeave::Dtor()
{
    if( pImpl )
    {
        pImpl->Leave();
        delete pImpl;
    }
}

void SwImplEnterLeave::Enter()
{
    SwProtocol::Record( pFrame, nFunction, ACT_START, pParam );
}

void SwImplEnterLeave::Leave()
{
    SwProtocol::Record( pFrame, nFunction, ACT_END, pParam );
}

void SwSizeEnterLeave::Leave()
{
    nFrameHeight = pFrame->Frame().Height() - nFrameHeight;
    SwProtocol::Record( pFrame, nFunction, ACT_END, &nFrameHeight );
}

void SwUpperEnterLeave::Enter()
{
    nFrameId = pFrame->GetUpper() ? sal_uInt16(lcl_GetFrameId( pFrame->GetUpper() )) : 0;
    SwProtocol::Record( pFrame, nFunction, ACT_START, &nFrameId );
}

void SwUpperEnterLeave::Leave()
{
    nFrameId = pFrame->GetUpper() ? sal_uInt16(lcl_GetFrameId( pFrame->GetUpper() )) : 0;
    SwProtocol::Record( pFrame, nFunction, ACT_END, &nFrameId );
}

void SwFrameChangesLeave::Enter()
{
}

void SwFrameChangesLeave::Leave()
{
    if( pFrame->Frame() != aFrame )
        SwProtocol::Record( pFrame, PROT_FRMCHANGES, 0, &aFrame );
}

#endif // DBG_UTIL

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
