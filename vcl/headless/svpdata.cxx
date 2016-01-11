/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <unx/gendata.hxx>
#include <headless/svpinst.hxx>

class SvpSalData : public SalGenericData
{
public:
    explicit SvpSalData( SalInstance *pInstance ) : SalGenericData( SAL_DATA_SVP, pInstance ) {}
    virtual void ErrorTrapPush() override {}
    virtual bool ErrorTrapPop( bool ) override { return false; }
};

// plugin factory function
SalInstance* svp_create_SalInstance()
{
    SvpSalInstance* pInstance = new SvpSalInstance( new SalYieldMutex() );
    new SvpSalData( pInstance );
    return pInstance;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
