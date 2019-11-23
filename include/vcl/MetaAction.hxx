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

#ifndef INCLUDED_VCL_METAACTION_HXX
#define INCLUDED_VCL_METAACTION_HXX

#include <rtl/textenc.h>
#include <rtl/ref.hxx>
#include <salhelper/simplereferenceobject.hxx>

#include <vcl/dllapi.h>
#include <vcl/metaactiontypes.hxx>

class OutputDevice;
class SvStream;
namespace tools { class Polygon; }

void ImplScalePoly( tools::Polygon& rPoly, double fScaleX, double fScaleY );

struct ImplMetaReadData
{
    rtl_TextEncoding meActualCharSet;
    int mnParseDepth;

    ImplMetaReadData()
        : meActualCharSet(RTL_TEXTENCODING_ASCII_US)
        , mnParseDepth(0)
    {}
};

struct ImplMetaWriteData
{
    rtl_TextEncoding meActualCharSet;

    ImplMetaWriteData() :
        meActualCharSet( RTL_TEXTENCODING_ASCII_US )
    {}
};

class VCL_DLLPUBLIC MetaAction : public salhelper::SimpleReferenceObject
{
private:
    MetaActionType const mnType;

protected:
    virtual             ~MetaAction() override;

public:
                        MetaAction();
    explicit            MetaAction( MetaActionType nType );
                        MetaAction( MetaAction const & );

    virtual void        Execute( OutputDevice* pOut );

    oslInterlockedCount GetRefCount() const { return m_nCount; }

    virtual rtl::Reference<MetaAction> Clone();

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData );
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData );

    MetaActionType      GetType() const { return mnType; }

public:
    static MetaAction*  ReadMetaAction( SvStream& rIStm, ImplMetaReadData* pData );
};

#endif // INCLUDED_VCL_METAACTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
