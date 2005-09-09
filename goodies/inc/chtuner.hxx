/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: chtuner.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:16:24 $
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

#ifndef _CHTUNER_HXX
#define _CHTUNER_HXX

#ifndef _SV_TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif
#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#ifndef _INETSESS_HXX
#include <inet/inetsess.hxx>
#endif


#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif

class ChannelItem;
class ChannelList;
class INetRequest;
class Downloader;
class StringList;

struct RetryInformation
{
    String  aURL;
    Time    aErrorTime;
};

DECLARE_LIST(ReqList,INetRequest*)
DECLARE_LIST(RetryList,RetryInformation*)

class ChannelTuner : public SfxListener
{
    ChannelList*        pChannelList;
    INetSessionRef      xINetSession;
    ReqList*            pReqList;

    Link                aBTXCallBack;
    Downloader*         pBTXDecoder;
    String              aBTXFileName;

    AutoTimer           aRetryTimer;
    RetryList*          pRetryList;

    void                SendHttpReq(const INetURLObject& rURL);
    void                SendFtpReq(const INetURLObject& rURL);
    void                SendBTXReq(const INetURLObject& rURL);
    void                GetFileTrans(const INetURLObject& rURL);

    void                RemoveRequest(INetRequest* pReq);
    void                NotifyChannelList(String& rStr);

    virtual void        SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType,
                                const SfxHint& rHint, const TypeId& rHintType);

    DECL_LINK(RetryCallBack, void*);
    DECL_LINK(BTXCallBack, void*);
public:
    ChannelTuner(INetSession* pSess, ChannelList* pChList);
    ~ChannelTuner();

    BOOL RequestChannel(ChannelItem* pItem, const Link& rBTXCallBack);

    void SetINetSession(INetSession* pNewSess);
    void SetBTXDecoder(Downloader* pDecoder);
};

#endif // _CHTUNER_HXX
