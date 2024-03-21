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
#include <memory>

class INetURLHistory_Impl;
class SVL_DLLPUBLIC INetURLHistory final : public SfxBroadcaster
{
    /** Representation.
    */
    std::unique_ptr<INetURLHistory_Impl> m_pImpl;

    /** Construction/Destruction.
    */
    SAL_DLLPRIVATE INetURLHistory();
    SAL_DLLPRIVATE virtual ~INetURLHistory() override;

    /** Implementation.
    */
    SAL_DLLPRIVATE static void NormalizeUrl_Impl (INetURLObject &rUrl);

    void PutUrl_Impl   (const INetURLObject &rUrl);
    bool QueryUrl_Impl (INetURLObject rUrl) const;

    /** Not implemented.
    */
    INetURLHistory (const INetURLHistory&) = delete;
    INetURLHistory& operator= (const INetURLHistory&) = delete;

public:
    /** GetOrCreate.
    */
    static INetURLHistory* GetOrCreate();

    /** QueryProtocol.
    */
    static bool QueryProtocol (INetProtocol eProto)
    {
        return ((eProto == INetProtocol::File ) ||
                (eProto == INetProtocol::Ftp  ) ||
                (eProto == INetProtocol::Http ) ||
                (eProto == INetProtocol::Https)    );
    }

    /** QueryUrl.
    */
    bool QueryUrl (const INetURLObject &rUrl) const
    {
        if (QueryProtocol (rUrl.GetProtocol()))
            return QueryUrl_Impl (rUrl);
        else
            return false;
    }

    bool QueryUrl (std::u16string_view rUrl) const;

    /** PutUrl.
    */
    void PutUrl (const INetURLObject &rUrl)
    {
        if (QueryProtocol (rUrl.GetProtocol()))
            PutUrl_Impl (rUrl);
    }
};

// broadcasted from PutUrl().
class SVL_DLLPUBLIC INetURLHistoryHint final : public SfxHint
{
    const INetURLObject* pObj;
public:
    explicit INetURLHistoryHint( const INetURLObject* Object ) : pObj(Object) {}
    const INetURLObject* GetObject() const { return pObj; }
};

#endif // INCLUDED_SVL_INETHIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
