/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: inethist.hxx,v $
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
#ifndef _INETHIST_HXX
#define _INETHIST_HXX

#include "svtools/svldllapi.h"
#include <tools/solar.h>
#include <tools/string.hxx>
#include <tools/urlobj.hxx>
#include <svtools/brdcst.hxx>
#include <svtools/hint.hxx>

/*========================================================================
 *
 * INetURLHistory interface.
 *
 *=======================================================================*/
class INetURLHistory_Impl;
class INetURLHistory : public SfxBroadcaster
{
    struct StaticInstance
    {
        INetURLHistory * operator()();
    };
    friend INetURLHistory * StaticInstance::operator()();

    /** Representation.
    */
    INetURLHistory_Impl *m_pImpl;

    /** Construction/Destruction.
    */
    INetURLHistory (void);
    virtual ~INetURLHistory (void);

    /** Implementation.
    */
    static void NormalizeUrl_Impl (INetURLObject &rUrl);

    SVL_DLLPUBLIC void PutUrl_Impl   (const INetURLObject &rUrl);
    SVL_DLLPUBLIC BOOL QueryUrl_Impl (const INetURLObject &rUrl);

    /** Not implemented.
    */
    INetURLHistory (const INetURLHistory&);
    INetURLHistory& operator= (const INetURLHistory&);

public:
    /** GetOrCreate.
    */
    SVL_DLLPUBLIC static INetURLHistory* GetOrCreate (void);

    /** QueryProtocol.
    */
    BOOL QueryProtocol (INetProtocol eProto) const
    {
        return ((eProto == INET_PROT_FILE ) ||
                (eProto == INET_PROT_FTP  ) ||
                (eProto == INET_PROT_HTTP ) ||
                (eProto == INET_PROT_HTTPS)    );
    }

    /** QueryUrl.
    */
    BOOL QueryUrl (const INetURLObject &rUrl)
    {
        if (QueryProtocol (rUrl.GetProtocol()))
            return QueryUrl_Impl (rUrl);
        else
            return FALSE;
    }

    BOOL QueryUrl (const String &rUrl)
    {
        INetProtocol eProto =
            INetURLObject::CompareProtocolScheme (rUrl);
        if (QueryProtocol (eProto))
            return QueryUrl_Impl (INetURLObject (rUrl));
        else
            return FALSE;
    }

    /** PutUrl.
    */
    void PutUrl (const INetURLObject &rUrl)
    {
        if (QueryProtocol (rUrl.GetProtocol()))
            PutUrl_Impl (rUrl);
    }

    void PutUrl (const String &rUrl)
    {
        INetProtocol eProto =
            INetURLObject::CompareProtocolScheme (rUrl);
        if (QueryProtocol (eProto))
            PutUrl_Impl (INetURLObject (rUrl));
    }
};

/*========================================================================
 *
 * INetURLHistoryHint (broadcasted from PutUrl()).
 *
 *=======================================================================*/
DECL_PTRHINT (SVL_DLLPUBLIC, INetURLHistoryHint, const INetURLObject);

#endif /* _INETHIST_HXX */

