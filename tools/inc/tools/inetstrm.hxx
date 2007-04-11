/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: inetstrm.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:12:57 $
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
#ifndef _TOOLS_INETSTRM_HXX
#define _TOOLS_INETSTRM_HXX

#ifndef INCLUDED_TOOLSDLLAPI_H
#include "tools/toolsdllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

class INetMessage;
class INetMIMEMessage;
class INetHTTPMessage;
class SvMemoryStream;
class SvStream;

/*=========================================================================
 *
 * INetStream Interface.
 *
 *=======================================================================*/
enum INetStreamStatus
{
    INETSTREAM_STATUS_LOADED     = -4,
    INETSTREAM_STATUS_WOULDBLOCK = -3,
    INETSTREAM_STATUS_OK         = -2,
    INETSTREAM_STATUS_ERROR      = -1
};

/*
 * INetIStream.
 */
class TOOLS_DLLPUBLIC INetIStream
{
    // Not implemented.
    INetIStream (const INetIStream& rStrm);
    INetIStream& operator= (const INetIStream& rStrm);

protected:
    virtual int GetData (sal_Char *pData, ULONG nSize) = 0;

public:
    INetIStream ();
    virtual ~INetIStream (void);

    int Read (sal_Char *pData, ULONG nSize);

    static void Decode64 (SvStream& rIn, SvStream& rOut);
    static void Encode64 (SvStream& rIn, SvStream& rOut);
};

/*
 * INetOStream.
 */
class INetOStream
{
    // Not implemented.
    INetOStream (const INetOStream& rStrm);
    INetOStream& operator= (const INetOStream& rStrm);

protected:
    virtual int PutData (
        const sal_Char *pData, ULONG nSize) = 0;

public:
    INetOStream ();
    virtual ~INetOStream (void);

    int Write (const sal_Char *pData, ULONG nSize);
};

/*
 * INetIOStream.
 */
class INetIOStream : public INetIStream, public INetOStream
{
    // Not implemented.
    INetIOStream (const INetIOStream& rStrm);
    INetIOStream& operator= (const INetIOStream& rStrm);

public:
    INetIOStream (ULONG nIBufferSize = 0, ULONG nOBufferSize = 0);
    virtual ~INetIOStream (void);
};

/*=========================================================================
 *
 * INetMessageStream Interface.
 *
 *=======================================================================*/
enum INetMessageStreamState
{
    INETMSG_EOL_BEGIN,
    INETMSG_EOL_DONE,
    INETMSG_EOL_SCR,
    INETMSG_EOL_FCR,
    INETMSG_EOL_FLF,
    INETMSG_EOL_FSP,
    INETMSG_EOL_FESC
};

/*
 * INetMessageIStream (Message Generator) Interface.
 */
class INetMessageIStream : public INetIStream
{
    INetMessage    *pSourceMsg;
    BOOL            bHeaderGenerated;

    ULONG           nBufSiz;
    sal_Char       *pBuffer;
    sal_Char       *pRead;
    sal_Char       *pWrite;

    SvStream       *pMsgStrm;
    SvMemoryStream *pMsgBuffer;
    sal_Char       *pMsgRead;
    sal_Char       *pMsgWrite;

    virtual int GetData (sal_Char *pData, ULONG nSize);

    // Not implemented.
    INetMessageIStream (const INetMessageIStream& rStrm);
    INetMessageIStream& operator= (const INetMessageIStream& rStrm);

protected:
    virtual int GetMsgLine (sal_Char *pData, ULONG nSize);

public:
    INetMessageIStream (ULONG nBufferSize = 2048);
    virtual ~INetMessageIStream (void);

    INetMessage *GetSourceMessage (void) const { return pSourceMsg; }
    void SetSourceMessage (INetMessage *pMsg) { pSourceMsg = pMsg; }

    void GenerateHeader (BOOL bGen = TRUE) { bHeaderGenerated = !bGen; }
    BOOL IsHeaderGenerated (void) const { return bHeaderGenerated; }
};

/*
 * INetMessageOStream (Message Parser) Interface.
 */
class INetMessageOStream : public INetOStream
{
    INetMessage            *pTargetMsg;
    BOOL                    bHeaderParsed;

    INetMessageStreamState  eOState;

    SvMemoryStream         *pMsgBuffer;

    virtual int PutData (const sal_Char *pData, ULONG nSize);

    // Not implemented.
    INetMessageOStream (const INetMessageOStream& rStrm);
    INetMessageOStream& operator= (const INetMessageOStream& rStrm);

protected:
    virtual int PutMsgLine (const sal_Char *pData, ULONG nSize);

public:
    INetMessageOStream (void);
    virtual ~INetMessageOStream (void);

    INetMessage *GetTargetMessage (void) const { return pTargetMsg; }
    void SetTargetMessage (INetMessage *pMsg) { pTargetMsg = pMsg; }

    void ParseHeader (BOOL bParse = TRUE) { bHeaderParsed = !bParse; }
    BOOL IsHeaderParsed (void) const { return bHeaderParsed; }
};

/*
 * INetMessageIOStream Interface.
 */
class INetMessageIOStream
    : public INetMessageIStream,
      public INetMessageOStream
{
    // Not implemented.
    INetMessageIOStream (const INetMessageIOStream& rStrm);
    INetMessageIOStream& operator= (const INetMessageIOStream& rStrm);

public:
    INetMessageIOStream (ULONG nBufferSize = 2048);
    virtual ~INetMessageIOStream (void);
};

/*=========================================================================
 *
 * INetMIMEMessageStream Interface.
 *
 *=======================================================================*/
enum INetMessageEncoding
{
    INETMSG_ENCODING_7BIT,
    INETMSG_ENCODING_8BIT,
    INETMSG_ENCODING_BINARY,
    INETMSG_ENCODING_QUOTED,
    INETMSG_ENCODING_BASE64
};

class TOOLS_DLLPUBLIC INetMIMEMessageStream : public INetMessageIOStream
{
    int                    eState;

    ULONG                  nChildIndex;
    INetMIMEMessageStream *pChildStrm;

    INetMessageEncoding    eEncoding;
    INetMessageIStream    *pEncodeStrm;
    INetMessageOStream    *pDecodeStrm;

    SvMemoryStream        *pMsgBuffer;

    static INetMessageEncoding GetMsgEncoding (
        const String& rContentType);

    // Not implemented.
    INetMIMEMessageStream (const INetMIMEMessageStream& rStrm);
    INetMIMEMessageStream& operator= (const INetMIMEMessageStream& rStrm);

protected:
    virtual int GetMsgLine (sal_Char *pData, ULONG nSize);
    virtual int PutMsgLine (const sal_Char *pData, ULONG nSize);

public:
    INetMIMEMessageStream (ULONG nBufferSize = 2048);
    virtual ~INetMIMEMessageStream (void);

    using INetMessageIStream::SetSourceMessage;
    void SetSourceMessage (INetMIMEMessage *pMsg)
    {
        INetMessageIStream::SetSourceMessage ((INetMessage *)pMsg);
    }
    INetMIMEMessage *GetSourceMessage (void) const
    {
        return ((INetMIMEMessage *)INetMessageIStream::GetSourceMessage());
    }

    using INetMessageOStream::SetTargetMessage;
    void SetTargetMessage (INetMIMEMessage *pMsg)
    {
        INetMessageOStream::SetTargetMessage ((INetMessage *)pMsg);
    }
    INetMIMEMessage *GetTargetMessage (void) const
    {
        return ((INetMIMEMessage *)INetMessageOStream::GetTargetMessage());
    }
};

#endif /* !_TOOLS_INETSTRM_HXX */

