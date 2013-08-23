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

#ifndef SC_DDELINK_HXX
#define SC_DDELINK_HXX

#include "address.hxx"
#include <sfx2/lnkbase.hxx>
#include <svl/broadcast.hxx>
#include "types.hxx"

class ScDocument;
class ScMultipleReadHeader;
class ScMultipleWriteHeader;
class SvStream;

class ScDdeLink : public ::sfx2::SvBaseLink, public SvtBroadcaster
{
private:
static bool bIsInUpdate;

    ScDocument*     pDoc;

    String          aAppl;          // connection/ link data
    String          aTopic;
    String          aItem;
    sal_uInt8       nMode;          // number format mode

    bool            bNeedUpdate;    // is set, if update was not possible

    ScMatrixRef     pResult;

public:
    TYPEINFO();

            ScDdeLink( ScDocument* pD,
                        const String& rA, const String& rT, const String& rI,
                        sal_uInt8 nM );
            ScDdeLink( ScDocument* pD, SvStream& rStream, ScMultipleReadHeader& rHdr );
            ScDdeLink( ScDocument* pD, const ScDdeLink& rOther );
    virtual ~ScDdeLink();

    void            Store( SvStream& rStream, ScMultipleWriteHeader& rHdr ) const;

                    // overloaded by SvBaseLink:
    virtual ::sfx2::SvBaseLink::UpdateResult DataChanged(
        const OUString& rMimeType, const ::com::sun::star::uno::Any & rValue );

                    // overloaded by SvtBroadcaster:
    virtual void    ListenersGone();

                    // for interpreter:

    const ScMatrix* GetResult() const;
    void            SetResult( const ScMatrixRef& pRes );

                    // XML and Excel import after NewData()
    ScMatrixRef     GetModifiableResult();

    const String&   GetAppl() const     { return aAppl; }
    const String&   GetTopic() const    { return aTopic; }
    const String&   GetItem() const     { return aItem; }
    sal_uInt8       GetMode() const     { return nMode; }

    void            TryUpdate();

    bool            NeedsUpdate() const { return bNeedUpdate; }

    static bool     IsInUpdate()        { return bIsInUpdate; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
