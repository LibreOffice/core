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

#ifndef SC_DDELINK_HXX
#define SC_DDELINK_HXX

#include "address.hxx"
#include <sfx2/lnkbase.hxx>
#include <svl/broadcast.hxx>
#include "scmatrix.hxx"

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
        const String& rMimeType, const ::com::sun::star::uno::Any & rValue );

                    // overloaded by SvtBroadcaster:
    virtual void    ListenersGone();

                    // for interpreter:

    const ScMatrix* GetResult() const           { return pResult.get(); }
    void            SetResult( ScMatrixRef pRes ) { pResult = pRes; }

                    // XML and Excel import after NewData()
    ScMatrixRef     GetModifiableResult()   { return pResult; }

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
