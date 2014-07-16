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
#ifndef INCLUDED_SVL_INETHIST_HXX
#define INCLUDED_SVL_INETHIST_HXX

#include <svl/SfxBroadcaster.hxx>
#include <svl/hint.hxx>
#include <svl/svldllapi.h>
#include <tools/urlobj.hxx>

class INetURLHistory_Impl;
class SVL_DLLPUBLIC INetURLHistory : public SfxBroadcaster
{
    struct SAL_DLLPRIVATE StaticInstance
    {
        INetURLHistory * operator()();
    };
    friend INetURLHistory * StaticInstance::operator()();

    /** Representation.
    */
    INetURLHistory_Impl *m_pImpl;

    /** Construction/Destruction.
    */
    SAL_DLLPRIVATE INetURLHistory (void);
    SAL_DLLPRIVATE virtual ~INetURLHistory (void);

    /** Implementation.
    */
    SAL_DLLPRIVATE static void NormalizeUrl_Impl (INetURLObject &rUrl);

    void PutUrl_Impl   (const INetURLObject &rUrl);
    bool QueryUrl_Impl (const INetURLObject &rUrl);

    /** Not implemented.
    */
    SAL_DLLPRIVATE INetURLHistory (const INetURLHistory&);
    SAL_DLLPRIVATE INetURLHistory& operator= (const INetURLHistory&);

public:
    /** GetOrCreate.
    */
    static INetURLHistory* GetOrCreate (void);

    /** QueryProtocol.
    */
    bool QueryProtocol (INetProtocol eProto) const
    {
        return ((eProto == INET_PROT_FILE ) ||
                (eProto == INET_PROT_FTP  ) ||
                (eProto == INET_PROT_HTTP ) ||
                (eProto == INET_PROT_HTTPS)    );
    }

    /** QueryUrl.
    */
    bool QueryUrl (const INetURLObject &rUrl)
    {
        if (QueryProtocol (rUrl.GetProtocol()))
            return QueryUrl_Impl (rUrl);
        else
            return false;
    }

    bool QueryUrl (const OUString &rUrl)
    {
        INetProtocol eProto =
            INetURLObject::CompareProtocolScheme (rUrl);
        if (QueryProtocol (eProto))
            return QueryUrl_Impl (INetURLObject (rUrl));
        else
            return false;
    }

    /** PutUrl.
    */
    void PutUrl (const INetURLObject &rUrl)
    {
        if (QueryProtocol (rUrl.GetProtocol()))
            PutUrl_Impl (rUrl);
    }

    void PutUrl (const OUString &rUrl)
    {
        INetProtocol eProto =
            INetURLObject::CompareProtocolScheme (rUrl);
        if (QueryProtocol (eProto))
            PutUrl_Impl (INetURLObject (rUrl));
    }
};

// broadcasted from PutUrl().
DECL_PTRHINT (SVL_DLLPUBLIC, INetURLHistoryHint, const INetURLObject);

#endif // INCLUDED_SVL_INETHIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
