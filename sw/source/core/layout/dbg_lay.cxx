/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifdef DBG_UTIL

/*
 * And here's the description:
 *
 * The PROTOCOL macros allow you to log events in frame methods. In places where
 * logging is useful either one of the PROTOCOL(...) or PROTOCOL_ENTER(...) can
 * be used. PROTOCOL_ENTER(...) additionally logs the leaving of a method.

 * The PROTOCOL macros accept the following parameters:
 * 1.   A pointer to an SwFrm (usually "this" or "rThis")
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
 * directory. The file contains lines with FrmId, function group and additional
 * information.
 *
 * Was genau protokolliert wird, kann auf folgende Arten eingestellt werden:
 * What exactly is going to be logged, can be defined as follows:
 * 1.   The static variable SwProtocol::nRecord contains the function groups
 *      which shall be logged.
 *      A value of i.e. PROT_GROW causes calls to SwFrm::Grow to be
 *      logged; PROT_MAKEALL logs the calls to xxx::MakeAll.
 *      The PROT_XY values can be combined using binary OR, the default value
 *      is null - no method calls are logged.
 * 2.   The SwImplProtocol class contains a filter for frame types, only method
 *      call of frame types which are defined there are logged.
 *      The member nTypes can be set to values like FRM_PAGE or FRM_SECTION and
 *      may be combined using binary OR. The default values is 0xFFFF - meaning
 *      all frame types.

 * 3.   In der SwImplProtocol-Klasse gibt es einen ArrayPointer auf FrmIds, die zu ueberwachen sind.
 *      Ist der Pointer Null, so werden alle Frames protokolliert, ansonsten nur Frames,
 *      die in dem Array vermerkt sind.

 * 3.   The SwImplProtocol class contains an ArrayPointer to FrmIds which need to be
 *      tracked. If the pointer is null, all frames will be logged; otherwise
 *      only frames of linked from the array will be logged.
 *
 * Eine Aufzeichnung in Gang zu setzen, erfordert entweder Codemanipulation, z.B. in
 * SwProtocol::Init() einen anderen Default fuer nRecord setzen oder Debuggermanipulation.
 * Im Debugger gibt verschiedene, sich anbietende Stellen:

 * Code changes are needed to start logging; either change the default of nRecord
 * in SwProtocol::Init() or change the debugger. There are several possible
 * places in the debugger:

 * 1.   In SwProtocol::Init() einen Breakpoint setzen und dort nRecord manipulieren, ggf.
 *      FrmIds eintragen, dann beginnt die Aufzeichnung bereits beim Programmstart.

 * 1.   Set a breakpoint in SwProtocol::Init() and manipulate nRecord there, set
        FrmIds accordingly then start logging during program start.

 * 2.   Waehrend des Programmlaufs einen Breakpoint vor irgendein PROTOCOL oder PROTOCOL_ENTER-
 *      Makro setzen, dann am SwProtocol::nRecord das unterste Bit setzen (PROT_INIT). Dies
 *      bewirkt, dass die Funktionsgruppe des folgenden Makros aktiviert und in Zukunft
 *      protokolliert wird.

 * 2.   Set a breakpoint before any PROTOCOL or PROTOCOL_ENTER macro during
        program execution, then set the lowest bit (PROT_INIT) of
        SwProtocol::nRecord. This activates the function group of the following
        macro and causes it to be logged in the future.

 * 3.   Spezialfall von 2.: Wenn man 2. in der Methode SwRootFrm::Paint(..) anwendet, werden
 *      die Aufzeichnungseinstellung aus der Datei "dbg_lay.ini" ausgelesen!
 *      In dieser INI-Datei kann es Kommentarzeilen geben, diese beginnen mit '#', dann
 *      sind die Sektionen "[frmid]", "[frmtype]" und "[record]" relevant.
 *      Nach [frmid] koennen die FrameIds der zu protokollierenden Frames folgen. Gibt es
 *      dort keine Eintraege, werden alle Frames aufgezeichnet.
 *      Nach [frmtype] koennen FrameTypen folgen, die aufgezeichnet werden sollen, da der
 *      Default hier allerdings USHRT_MAX ist, werden sowieso alle aufgezeichnet. Man kann
 *      allerdings auch Typen entfernen, in dem man ein '!' vor den Wert setzt, z.B.
 *      !0xC000 nimmt die SwCntntFrms aus der Aufzeichnung heraus.
 *      Nach [record] folgen die Funktionsgruppen, die aufgezeichnet werden sollen, Default
 *      ist hier 0, also keine. Auch hier kann man mit einem vorgestellten '!' Funktionen
 *      wieder entfernen.
 *      Hier mal ein Beispiel fuer eine INI-Datei:
 *      ------------------------------------------
 *          #Funktionen: Alle, ausser PRTAREA
 *          [record] 0xFFFFFFE !0x200
 *          [frmid]
 *          #folgende FrmIds:
 *          1 2 12 13 14 15
 *          #keine Layoutframes ausser ColumnFrms
 *          [frmtype] !0x3FFF 0x4
 *      ------------------------------------------
 *
 * 3.   There's a special case for 2: If one uses 2. in SwRootFrm::Paint(..),
 *      the log settings are taken from the file "dbg_lay.ini"!
 *      In this INI-file you can have comment lines starting with a '#'.
 *      The sections "[frmid]", "[frmtype]" and "[record]" are relevant.
 *      In the [frmid] section, you can put FrameIds of the Frames to be logged.
 *      If there are no entries in this section, all Frames will be logged.
 *      In the [frmtype] section, the frame types which should be logged are
 *      listed; default is USHRT_MAX which means that all types are logged.
 *      It's possible to remove types from the list using '!' in front of a
 *      value. The value !0xC000 would for example exclude SwCntntFrms from
 *      logging.
 *      In the [record] section the functions group which should be logged are
 *      listed; default is 0 which means that none are logged. It's also
 *      possible to remove functions using '!'.
 *      An example INI file:
 *      ------------------------------------------
 *           #Functions: all, except PRTARE
 *           [record] 0xFFFFFFFE !0x200
 *           [frmid]
 *           #the following FrmIds:
 *           1 2 12 13 14 15
 *           #no layout frames, except ColumnFrms
 *           [frmtype] !0x3FFF 0x4
 *      ------------------------------------------

 * Wenn die Aufzeichnung erstmal laeuft, kann man in SwImplProtocol::_Record(...) mittels
 * Debugger vielfaeltige Manipulationen vornehmen, z.B. bezueglich FrameTypen oder FrmIds.
 *

 * As soon as the logging is in process, one can manipulate many things in
 * SwImplProtocol::_Record(...) using a debugger, especially concerning
 * frame types and FrmIds.

 * --------------------------------------------------*/

#include "dbg_lay.hxx"
#include <tools/stream.hxx>

#include <stdio.h>

#include "frame.hxx"
#include "layfrm.hxx"
#include "flyfrm.hxx"
#include "txtfrm.hxx"
#include "ndtxt.hxx"
#include "dflyobj.hxx"
#include <fntcache.hxx>
#include <sortedobjs.hxx> // #i28701#

#include <rtl/strbuf.hxx>
#include <comphelper/string.hxx>

sal_uLong SwProtocol::nRecord = 0;
SwImplProtocol* SwProtocol::pImpl = NULL;

static sal_uLong lcl_GetFrameId( const SwFrm* pFrm )
{
#if OSL_DEBUG_LEVEL > 1
    static sal_Bool bFrameId = sal_False;
    if( bFrameId )
        return pFrm->GetFrmId();
#endif
    if( pFrm && pFrm->IsTxtFrm() )
        return ((SwTxtFrm*)pFrm)->GetTxtNode()->GetIndex();
    return 0;
}

class SwImplProtocol
{
    SvFileStream *pStream;          // output stream
    std::set<sal_uInt16> *pFrmIds;  // which FrmIds shall be logged ( NULL == all)
    std::vector<long> aVars;        // variables
    rtl::OStringBuffer aLayer;      // indentation of output ("  " per start/end)
    sal_uInt16 nTypes;              // which types shall be logged
    sal_uInt16 nLineCount;          // printed lines
    sal_uInt16 nMaxLines;           // max lines to be printed
    sal_uInt8 nInitFile;            // range (FrmId,FrmType,Record) during reading of the INI file
    sal_uInt8 nTestMode;            // special for test formating, logging may only be done in test formating.
    void _Record( const SwFrm* pFrm, sal_uLong nFunction, sal_uLong nAct, void* pParam );
    sal_Bool NewStream();
    void CheckLine( rtl::OString& rLine );
    void SectFunc( rtl::OStringBuffer& rOut, const SwFrm* pFrm, sal_uLong nAct, void* pParam );
public:
    SwImplProtocol();
    ~SwImplProtocol();
    // logging
    void Record( const SwFrm* pFrm, sal_uLong nFunction, sal_uLong nAct, void* pParam )
        { if( pStream ) _Record( pFrm, nFunction, nAct, pParam ); }
    sal_Bool InsertFrm( sal_uInt16 nFrmId );    // take FrmId for logging
    sal_Bool DeleteFrm( sal_uInt16 nFrmId );    // remove FrmId; don't log him anymore
    void FileInit();                    // read the INI file
    void ChkStream() { if( !pStream ) NewStream(); }
};

/* --------------------------------------------------
 * Through the PROTOCOL_ENTER macro a SwEnterLeave object gets created. If the
 * current function should be logged a SwImplEnterLeace object gets created.
 * The funny thing here is, that the Ctor of the Impl object is automatically
 * called at the beginning of the function and the Dtor is automatically called
 * when leaving the function. In the base implementation the Ctor calls only
 * PROTOCOL(..) with ACT_START and in the Dtor a PROTOCOL(..) with ACT_END.
 * It's possible to derive from this class, for example to be able to document
 * frame resize while leaving a function. To do this, one only needs to add the
 * desired SwImplEnterLeave class in SwEnterLeave::Ctor().
 * --------------------------------------------------*/

class SwImplEnterLeave
{
protected:
    const SwFrm* pFrm;              // the frame
    sal_uLong nFunction, nAction;   // the function, the action if needed
    void* pParam;                   // further parameter
public:
    SwImplEnterLeave( const SwFrm* pF, sal_uLong nFunct, sal_uLong nAct, void* pPar )
        : pFrm( pF ), nFunction( nFunct ), nAction( nAct ), pParam( pPar ) {}
    virtual ~SwImplEnterLeave() {}
    virtual void Enter();           // message when entering
    virtual void Leave();           // message when leaving
};

class SwSizeEnterLeave : public SwImplEnterLeave
{
    long nFrmHeight;
public:
    SwSizeEnterLeave( const SwFrm* pF, sal_uLong nFunct, sal_uLong nAct, void* pPar )
        : SwImplEnterLeave( pF, nFunct, nAct, pPar ), nFrmHeight( pF->Frm().Height() ) {}
    virtual ~SwSizeEnterLeave() {}
    virtual void Leave();           // resize message
};

class SwUpperEnterLeave : public SwImplEnterLeave
{
    sal_uInt16 nFrmId;
public:
    SwUpperEnterLeave( const SwFrm* pF, sal_uLong nFunct, sal_uLong nAct, void* pPar )
        : SwImplEnterLeave( pF, nFunct, nAct, pPar ), nFrmId( 0 ) {}
    virtual ~SwUpperEnterLeave() {}
    virtual void Enter();           // message
    virtual void Leave();           // message of FrmId from upper
};

class SwFrmChangesLeave : public SwImplEnterLeave
{
    SwRect aFrm;
public:
    SwFrmChangesLeave( const SwFrm* pF, sal_uLong nFunct, sal_uLong nAct, void* pPar )
        : SwImplEnterLeave( pF, nFunct, nAct, pPar ), aFrm( pF->Frm() ) {}
    virtual ~SwFrmChangesLeave() {}
    virtual void Enter();           // no message
    virtual void Leave();           // message when resizing the Frm area
};

void SwProtocol::Record( const SwFrm* pFrm, sal_uLong nFunction, sal_uLong nAct, void* pParam )
{
    if( Start() )
    {   // We reach this point if SwProtocol::nRecord is binary OR'd with PROT_INIT(0x1) using the debugger
        sal_Bool bFinit = sal_False; // This gives the possibility to stop logging of this action in the debugger
        if( bFinit )
        {
            nRecord &= ~nFunction;  // Don't log this function any longer
            nRecord &= ~PROT_INIT;  // Always reset PROT_INIT
            return;
        }
        nRecord |= nFunction;       // Acitivate logging of this function
        nRecord &= ~PROT_INIT;      // Always reset PROT_INIT
        if( pImpl )
            pImpl->ChkStream();
    }
    if( !pImpl )                        // Create Impl object if needed
        pImpl = new SwImplProtocol();
    pImpl->Record( pFrm, nFunction, nAct, pParam ); // ...and start logging
}

// The following function gets called when pulling in the writer DLL through
// TxtInit(..) and gives the possibility to release functions
// and/or FrmIds to the debugger

void SwProtocol::Init()
{
    nRecord = 0;
    rtl::OUString aName("dbg_lay.go");
    SvFileStream aStream( aName, STREAM_READ );
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
        pImpl = NULL;
        if( pFntCache )
            pFntCache->Flush();
     }
     nRecord = 0;
}

SwImplProtocol::SwImplProtocol()
    : pStream( NULL ), pFrmIds( NULL ), nTypes( 0xffff ),
      nLineCount( 0 ), nMaxLines( USHRT_MAX ), nTestMode( 0 )
{
    NewStream();
}

sal_Bool SwImplProtocol::NewStream()
{
    rtl::OUString aName("dbg_lay.out");
    nLineCount = 0;
    pStream = new SvFileStream( aName, STREAM_WRITE | STREAM_TRUNC );
    if( pStream->GetError() )
    {
        delete pStream;
        pStream = NULL;
    }
    return 0 != pStream;
}

SwImplProtocol::~SwImplProtocol()
{
    if( pStream )
    {
        pStream->Close();
        delete pStream;
    }
    pFrmIds->clear();
    delete pFrmIds;
    aVars.clear();
}

/* --------------------------------------------------
 * SwImplProtocol::CheckLine analyzes a line in the INI file
 * --------------------------------------------------*/

void SwImplProtocol::CheckLine( rtl::OString& rLine )
{
    rLine = rLine.toAsciiLowerCase(); // upper/lower case is the same
    rLine = rLine.replace( '\t', ' ' );
    if( '#' == rLine[0] )   // comments start with '#'
        return;
    if( '[' == rLine[0] )   // section: FrmIds, type or funciton
    {
        rtl::OString aTmp = comphelper::string::getToken(rLine, 0, ']');
        if (aTmp.equalsL(RTL_CONSTASCII_STRINGPARAM("[frmid")))      // section FrmIds
        {
            nInitFile = 1;
            pFrmIds->clear();
            delete pFrmIds;
            pFrmIds = NULL;         // default: log all frames
        }
        else if (aTmp.equalsL(RTL_CONSTASCII_STRINGPARAM("[frmtype")))// section types
        {
            nInitFile = 2;
            nTypes = USHRT_MAX;     // default: log all frame types
        }
        else if (aTmp.equalsL(RTL_CONSTASCII_STRINGPARAM("[record")))// section functions
        {
            nInitFile = 3;
            SwProtocol::SetRecord( 0 );// default: don't log any function
        }
        else if (aTmp.equalsL(RTL_CONSTASCII_STRINGPARAM("[test")))// section functions
        {
            nInitFile = 4; // default:
            nTestMode = 0; // log outside of test formating
        }
        else if (aTmp.equalsL(RTL_CONSTASCII_STRINGPARAM("[max")))// Max number of lines
        {
            nInitFile = 5; // default:
            nMaxLines = USHRT_MAX;
        }
        else if (aTmp.equalsL(RTL_CONSTASCII_STRINGPARAM("[var")))// variables
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
        rtl::OString aTok = rLine.getToken( 0, ' ', nIndex );
        sal_Bool bNo = sal_False;
        if( '!' == aTok[0] )
        {
            bNo = sal_True;                 // remove this function/type
            aTok = aTok.copy(1);
        }
        if( !aTok.isEmpty() )
        {
            sal_uLong nVal;
            sscanf( aTok.getStr(), "%li", &nVal );
            switch ( nInitFile )
            {
                case 1: InsertFrm( sal_uInt16( nVal ) );    // add FrmId
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

/* --------------------------------------------------
 * SwImplProtocol::FileInit() reads the file "dbg_lay.ini"
 * in the current directory and evaluates it.
 * --------------------------------------------------*/
void SwImplProtocol::FileInit()
{
    rtl::OUString aName("dbg_lay.ini");
    SvFileStream aStream( aName, STREAM_READ );
    if( aStream.IsOpen() )
    {
        rtl::OString aLine;
        nInitFile = 0;
        while( aStream.good() )
        {
            sal_Char c;
            aStream >> c;
            if( '\n' == c || '\r' == c )    // line ending
            {
                aLine = aLine.trim();
                if( !aLine.isEmpty() )
                    CheckLine( aLine );     // evaluate line
                aLine = rtl::OString();
            }
            else
                aLine = rtl::OString(c);
        }
        if( !aLine.isEmpty() )
            CheckLine( aLine );     // evaluate last line
    }
    aStream.Close();
}

/* --------------------------------------------------
 * lcl_Start enables indentation by two spaces during ACT_START and disables
 * it again at ACT_END.
 * --------------------------------------------------*/
static void lcl_Start(rtl::OStringBuffer& rOut, rtl::OStringBuffer& rLay, sal_uLong nAction)
{
    if( nAction == ACT_START )
    {
        rLay.append(RTL_CONSTASCII_STRINGPARAM("  "));
        rOut.append(RTL_CONSTASCII_STRINGPARAM(" On"));
    }
    else if( nAction == ACT_END )
    {
        if( rLay.getLength() > 1 )
        {
            rLay.remove(rLay.getLength() - 2, rLay.getLength());
            rOut.remove(0, 2);
        }
        rOut.append(RTL_CONSTASCII_STRINGPARAM(" Off"));
    }
}

/* --------------------------------------------------
 * lcl_Flags outputs the ValidSize-, ValidPos- and ValidPrtArea-Flag ("Sz","Ps","PA")
 * of the frame; "+" stands for valid, "-" stands for invalid.
 * --------------------------------------------------*/

static void lcl_Flags(rtl::OStringBuffer& rOut, const SwFrm* pFrm)
{
    rOut.append(RTL_CONSTASCII_STRINGPARAM(" Sz"));
    rOut.append(pFrm->GetValidSizeFlag() ? '+' : '-');
    rOut.append(RTL_CONSTASCII_STRINGPARAM(" Ps"));
    rOut.append(pFrm->GetValidPosFlag() ? '+' : '-');
    rOut.append(RTL_CONSTASCII_STRINGPARAM(" PA"));
    rOut.append(pFrm->GetValidPrtAreaFlag() ? '+' : '-');
}

/* --------------------------------------------------
 * lcl_FrameType outputs the type of the frame as clear text.
 * --------------------------------------------------*/

static void lcl_FrameType( rtl::OStringBuffer& rOut, const SwFrm* pFrm )
{
    if( pFrm->IsTxtFrm() )
        rOut.append(RTL_CONSTASCII_STRINGPARAM("Txt "));
    else if( pFrm->IsLayoutFrm() )
    {
        if( pFrm->IsPageFrm() )
            rOut.append(RTL_CONSTASCII_STRINGPARAM("Page "));
        else if( pFrm->IsColumnFrm() )
            rOut.append(RTL_CONSTASCII_STRINGPARAM("Col "));
        else if( pFrm->IsBodyFrm() )
        {
            if( pFrm->GetUpper() && pFrm->IsColBodyFrm() )
                rOut.append(RTL_CONSTASCII_STRINGPARAM("(Col)"));
            rOut.append(RTL_CONSTASCII_STRINGPARAM("Body "));
        }
        else if( pFrm->IsRootFrm() )
            rOut.append(RTL_CONSTASCII_STRINGPARAM("Root "));
        else if( pFrm->IsCellFrm() )
            rOut.append(RTL_CONSTASCII_STRINGPARAM("Cell "));
        else if( pFrm->IsTabFrm() )
            rOut.append(RTL_CONSTASCII_STRINGPARAM("Tab "));
        else if( pFrm->IsRowFrm() )
            rOut.append(RTL_CONSTASCII_STRINGPARAM("Row "));
        else if( pFrm->IsSctFrm() )
            rOut.append(RTL_CONSTASCII_STRINGPARAM("Sect "));
        else if( pFrm->IsHeaderFrm() )
            rOut.append(RTL_CONSTASCII_STRINGPARAM("Header "));
        else if( pFrm->IsFooterFrm() )
            rOut.append(RTL_CONSTASCII_STRINGPARAM("Footer "));
        else if( pFrm->IsFtnFrm() )
            rOut.append(RTL_CONSTASCII_STRINGPARAM("Ftn "));
        else if( pFrm->IsFtnContFrm() )
            rOut.append(RTL_CONSTASCII_STRINGPARAM("FtnCont "));
        else if( pFrm->IsFlyFrm() )
            rOut.append(RTL_CONSTASCII_STRINGPARAM("Fly "));
        else
            rOut.append(RTL_CONSTASCII_STRINGPARAM("Layout "));
    }
    else if( pFrm->IsNoTxtFrm() )
        rOut.append(RTL_CONSTASCII_STRINGPARAM("NoTxt "));
    else
        rOut.append(RTL_CONSTASCII_STRINGPARAM("Not impl. "));
}

/* --------------------------------------------------
 * SwImplProtocol::Record(..) is only called if the PROTOCOL macro finds out,
 * that this function should be recorded ( SwProtocol::nRecord ).
 * In this method we also check if FrmId and frame type should be logged.
 * --------------------------------------------------*/

void SwImplProtocol::_Record( const SwFrm* pFrm, sal_uLong nFunction, sal_uLong nAct, void* pParam )
{
    sal_uInt16 nSpecial = 0;
    if( nSpecial )  // the possible debugger manipulations
    {
        sal_uInt16 nId = sal_uInt16(lcl_GetFrameId( pFrm ));
        switch ( nSpecial )
        {
            case 1: InsertFrm( nId ); break;
            case 2: DeleteFrm( nId ); break;
            case 3: pFrmIds->clear(); delete pFrmIds; pFrmIds = NULL; break;
            case 4: delete pStream; pStream = NULL; break;
        }
        return;
    }
    if( !pStream && !NewStream() )
        return; // still no stream

    if( pFrmIds && !pFrmIds->count( sal_uInt16(lcl_GetFrameId( pFrm )) ) )
        return; // doesn't belong to the wished FrmIds

    if( !(pFrm->GetType() & nTypes) )
        return; // the type is unwanted

    if( 1 == nTestMode && nFunction != PROT_TESTFORMAT )
        return; // we may only log inside a test formating
    sal_Bool bTmp = sal_False;
    rtl::OStringBuffer aOut(aLayer);
    aOut.append(static_cast<sal_Int64>(lcl_GetFrameId(pFrm)));
    aOut.append(' ');
    lcl_FrameType( aOut, pFrm );    // then the frame type
    switch ( nFunction )            // and the function
    {
        case PROT_SNAPSHOT: lcl_Flags( aOut, pFrm );
                            break;
        case PROT_MAKEALL:  aOut.append(RTL_CONSTASCII_STRINGPARAM("MakeAll"));
                            lcl_Start( aOut, aLayer, nAct );
                            if( nAct == ACT_START )
                                lcl_Flags( aOut, pFrm );
                            break;
        case PROT_MOVE_FWD: bTmp = sal_True; // NoBreak
        case PROT_MOVE_BWD:
                            if (nFunction == bTmp)
                                aOut.append(RTL_CONSTASCII_STRINGPARAM("Fwd"));
                            else
                                aOut.append(RTL_CONSTASCII_STRINGPARAM("Bwd"));
                            lcl_Start( aOut, aLayer, nAct );
                            if( pParam )
                            {
                                aOut.append(' ');
                                aOut.append(static_cast<sal_Int32>(*((sal_uInt16*)pParam)));
                            }
                            break;
        case PROT_GROW_TST: if( ACT_START != nAct )
                                return;
                            aOut.append(RTL_CONSTASCII_STRINGPARAM("TestGrow"));
                            break;
        case PROT_SHRINK_TST: if( ACT_START != nAct )
                                return;
                            aOut.append(RTL_CONSTASCII_STRINGPARAM("TestShrink"));
                            break;
        case PROT_ADJUSTN :
        case PROT_SHRINK:   bTmp = sal_True; // NoBreak
        case PROT_GROW:
                            if (!bTmp)
                                aOut.append(RTL_CONSTASCII_STRINGPARAM("Grow"));
                            else
                            {
                                if (nFunction == PROT_SHRINK)
                                    aOut.append(RTL_CONSTASCII_STRINGPARAM("Shrink"));
                                else
                                    aOut.append(RTL_CONSTASCII_STRINGPARAM("AdjustNgbhd"));
                            }
                            lcl_Start( aOut, aLayer, nAct );
                            if( pParam )
                            {
                                aOut.append(' ');
                                aOut.append(static_cast<sal_Int64>(*((long*)pParam)));
                            }
                            break;
        case PROT_POS:      break;
        case PROT_PRTAREA:  aOut.append(RTL_CONSTASCII_STRINGPARAM("PrtArea"));
                            lcl_Start( aOut, aLayer, nAct );
                            break;
        case PROT_SIZE:     aOut.append(RTL_CONSTASCII_STRINGPARAM("Size"));
                            lcl_Start( aOut, aLayer, nAct );
                            aOut.append(' ');
                            aOut.append(static_cast<sal_Int64>(pFrm->Frm().Height()));
                            break;
        case PROT_LEAF:     aOut.append(RTL_CONSTASCII_STRINGPARAM("Prev/NextLeaf"));
                            lcl_Start( aOut, aLayer, nAct );
                            aOut.append(' ');
                            if( pParam )
                            {
                                aOut.append(' ');
                                aOut.append(static_cast<sal_Int64>(lcl_GetFrameId((SwFrm*)pParam)));
                            }
                            break;
        case PROT_FILE_INIT: FileInit();
                            aOut.append(RTL_CONSTASCII_STRINGPARAM("Initialize"));
                            break;
        case PROT_SECTION:  SectFunc(aOut, pFrm, nAct, pParam);
                            break;
        case PROT_CUT:      bTmp = sal_True; // NoBreak
        case PROT_PASTE:
                            if (bTmp)
                                aOut.append(RTL_CONSTASCII_STRINGPARAM("Cut from "));
                            else
                                aOut.append(RTL_CONSTASCII_STRINGPARAM("Paste to "));
                            aOut.append(static_cast<sal_Int64>(lcl_GetFrameId((SwFrm*)pParam)));
                            break;
        case PROT_TESTFORMAT:
                            aOut.append(RTL_CONSTASCII_STRINGPARAM("Test"));
                            lcl_Start( aOut, aLayer, nAct );
                            if( ACT_START == nAct )
                                nTestMode |= 2;
                            else
                                nTestMode &= ~2;
                            break;
        case PROT_FRMCHANGES:
                            {
                                SwRect& rFrm = *((SwRect*)pParam);
                                if( pFrm->Frm().Pos() != rFrm.Pos() )
                                {
                                    aOut.append(RTL_CONSTASCII_STRINGPARAM("PosChg: ("));
                                    aOut.append(static_cast<sal_Int64>(rFrm.Left()));
                                    aOut.append(RTL_CONSTASCII_STRINGPARAM(", "));
                                    aOut.append(static_cast<sal_Int64>(rFrm.Top()));
                                    aOut.append(RTL_CONSTASCII_STRINGPARAM(") ("));
                                    aOut.append(static_cast<sal_Int64>(pFrm->Frm().Left()));
                                    aOut.append(RTL_CONSTASCII_STRINGPARAM(", "));
                                    aOut.append(static_cast<sal_Int64>(pFrm->Frm().Top()));
                                    aOut.append(RTL_CONSTASCII_STRINGPARAM(") "));
                                }
                                if( pFrm->Frm().Height() != rFrm.Height() )
                                {
                                    aOut.append(RTL_CONSTASCII_STRINGPARAM("Height: "));
                                    aOut.append(static_cast<sal_Int64>(rFrm.Height()));
                                    aOut.append(RTL_CONSTASCII_STRINGPARAM(" -> "));
                                    aOut.append(static_cast<sal_Int64>(pFrm->Frm().Height()));
                                    aOut.append(RTL_CONSTASCII_STRINGPARAM(" "));
                                }
                                if( pFrm->Frm().Width() != rFrm.Width() )
                                {
                                    aOut.append(RTL_CONSTASCII_STRINGPARAM("Width: "));
                                    aOut.append(static_cast<sal_Int64>(rFrm.Width()));
                                    aOut.append(RTL_CONSTASCII_STRINGPARAM(" -> "));
                                    aOut.append(static_cast<sal_Int64>(pFrm->Frm().Width()));
                                    aOut.append(' ');
                                }
                                break;
                            }
    }
    *pStream << aOut.getStr() << endl;  // output
    pStream->Flush();   // to the disk, so we can read it immediately
    if( ++nLineCount >= nMaxLines )     // max number of lines reached?
        SwProtocol::SetRecord( 0 );        // => end f logging
}

/* --------------------------------------------------
 * SwImplProtocol::SectFunc(...) is called from SwImplProtocol::_Record(..)
 * here we handle the output of the SectionFrms.
 * --------------------------------------------------*/

void SwImplProtocol::SectFunc(rtl::OStringBuffer &rOut, const SwFrm* , sal_uLong nAct, void* pParam)
{
    sal_Bool bTmp = sal_False;
    switch( nAct )
    {
        case ACT_MERGE:         rOut.append(RTL_CONSTASCII_STRINGPARAM("Merge Section "));
                                rOut.append(static_cast<sal_Int64>(lcl_GetFrameId((SwFrm*)pParam)));
                                break;
        case ACT_CREATE_MASTER: bTmp = sal_True; // NoBreak
        case ACT_CREATE_FOLLOW: rOut.append(RTL_CONSTASCII_STRINGPARAM("Create Section "));
                                if (bTmp)
                                    rOut.append(RTL_CONSTASCII_STRINGPARAM("Master to "));
                                else
                                    rOut.append(RTL_CONSTASCII_STRINGPARAM("Follow from "));
                                rOut.append(static_cast<sal_Int64>(lcl_GetFrameId((SwFrm*)pParam)));
                                break;
        case ACT_DEL_MASTER:    bTmp = sal_True; // NoBreak
        case ACT_DEL_FOLLOW:    rOut.append(RTL_CONSTASCII_STRINGPARAM("Delete Section "));
                                if (bTmp)
                                    rOut.append(RTL_CONSTASCII_STRINGPARAM("Master to "));
                                else
                                    rOut.append(RTL_CONSTASCII_STRINGPARAM("Follow from "));
                                rOut.append(static_cast<sal_Int64>(lcl_GetFrameId((SwFrm*)pParam)));
                                break;
    }
}

/* --------------------------------------------------
 * SwImplProtocol::InsertFrm(..) takes a new FrmId for logging; if pFrmIds==NULL
 * all are going to be logged but as soon as pFrmIds are set through
 * InsertFrm(..) only the added FrmIds are being logged.
 * --------------------------------------------------*/

sal_Bool SwImplProtocol::InsertFrm( sal_uInt16 nId )
{
    if( !pFrmIds )
        pFrmIds = new std::set<sal_uInt16>;
    if( pFrmIds->count( nId ) )
        return sal_False;
    pFrmIds->insert( nId );
    return sal_True;
}

/* --------------------------------------------------
 * SwImplProtocol::DeleteFrm(..) removes a FrmId from the pFrmIds array, so they
 * won't be logged anymore.
 * --------------------------------------------------*/
sal_Bool SwImplProtocol::DeleteFrm( sal_uInt16 nId )
{
    if( !pFrmIds )
        return sal_False;
    if ( pFrmIds->erase(nId) )
        return sal_True;
    return sal_False;
}

/* --------------------------------------------------
 * SwEnterLeave::Ctor(..) is called from the (inline-)CTor if the function should
 * be logged.
 * The task here is to find the right SwImplEnterLeave object based on the
 * function; everything else is then done in his Ctor/Dtor.
 * --------------------------------------------------*/
void SwEnterLeave::Ctor( const SwFrm* pFrm, sal_uLong nFunc, sal_uLong nAct, void* pPar )
{
    switch( nFunc )
    {
        case PROT_ADJUSTN :
        case PROT_GROW:
        case PROT_SHRINK : pImpl = new SwSizeEnterLeave( pFrm, nFunc, nAct, pPar ); break;
        case PROT_MOVE_FWD:
        case PROT_MOVE_BWD : pImpl = new SwUpperEnterLeave( pFrm, nFunc, nAct, pPar ); break;
        case PROT_FRMCHANGES : pImpl = new SwFrmChangesLeave( pFrm, nFunc, nAct, pPar ); break;
        default: pImpl = new SwImplEnterLeave( pFrm, nFunc, nAct, pPar ); break;
    }
    pImpl->Enter();
}

/* --------------------------------------------------
 * SwEnterLeave::Dtor() only calls the Dtor of the SwImplEnterLeave object. It's
 * just no inline because we don't want the SwImplEnterLeave definition inside
 * dbg_lay.hxx.
 * --------------------------------------------------*/

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
    SwProtocol::Record( pFrm, nFunction, ACT_START, pParam );
}

void SwImplEnterLeave::Leave()
{
    SwProtocol::Record( pFrm, nFunction, ACT_END, pParam );
}

void SwSizeEnterLeave::Leave()
{
    nFrmHeight = pFrm->Frm().Height() - nFrmHeight;
    SwProtocol::Record( pFrm, nFunction, ACT_END, &nFrmHeight );
}

void SwUpperEnterLeave::Enter()
{
    nFrmId = pFrm->GetUpper() ? sal_uInt16(lcl_GetFrameId( pFrm->GetUpper() )) : 0;
    SwProtocol::Record( pFrm, nFunction, ACT_START, &nFrmId );
}

void SwUpperEnterLeave::Leave()
{
    nFrmId = pFrm->GetUpper() ? sal_uInt16(lcl_GetFrameId( pFrm->GetUpper() )) : 0;
    SwProtocol::Record( pFrm, nFunction, ACT_END, &nFrmId );
}

void SwFrmChangesLeave::Enter()
{
}

void SwFrmChangesLeave::Leave()
{
    if( pFrm->Frm() != aFrm )
        SwProtocol::Record( pFrm, PROT_FRMCHANGES, 0, &aFrm );
}

#endif // DBG_UTIL

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
