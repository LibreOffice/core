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

#ifndef INCLUDED_VCL_METAROUNDRECTACTION_HXX
#define INCLUDED_VCL_METAROUNDRECTACTION_HXX

#include <tools/gen.hxx>

#include <vcl/MetaAction.hxx>

class OutputDevice;
class SvStream;

class VCL_DLLPUBLIC MetaRoundRectAction final : public MetaAction
{
private:

    tools::Rectangle           maRect;
    sal_uInt32          mnHorzRound;
    sal_uInt32          mnVertRound;

public:
                        MetaRoundRectAction();
    MetaRoundRectAction(MetaRoundRectAction const &) = default;
    MetaRoundRectAction(MetaRoundRectAction &&) = default;
    MetaRoundRectAction & operator =(MetaRoundRectAction const &) = delete; // due to MetaAction
    MetaRoundRectAction & operator =(MetaRoundRectAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaRoundRectAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

                        MetaRoundRectAction( const tools::Rectangle& rRect,
                                             sal_uInt32 nHorzRound, sal_uInt32 nVertRound );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const tools::Rectangle&    GetRect() const { return maRect; }
    sal_uInt32          GetHorzRound() const { return mnHorzRound; }
    sal_uInt32          GetVertRound() const { return mnVertRound; }
};

#endif // INCLUDED_VCL_METAROUNDRECTACTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
