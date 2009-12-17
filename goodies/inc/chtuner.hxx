/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: chtuner.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _CHTUNER_HXX
#define _CHTUNER_HXX

#include <vcl/timer.hxx>
#include <tools/list.hxx>
#ifndef _INETSESS_HXX
#include <inet/inetsess.hxx>
#endif


#include <svl/lstner.hxx>

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
