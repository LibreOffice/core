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

#pragma once

#include <rtl/ref.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include <docoptio.hxx>
#include <svl/itempool.hxx>
#include <mutex>

class ScDocument;
class ScDocumentPool;
class ScStyleSheetPool;
class SvNumberFormatter;
class SfxItemPool;

class ScPoolHelper final : public salhelper::SimpleReferenceObject
{
private:
    mutable std::mutex maMtxCreateNumFormatter;
    ScDocOptions        aOpt;
    rtl::Reference<ScDocumentPool> pDocPool;
    rtl::Reference< ScStyleSheetPool > mxStylePool;
    mutable std::unique_ptr<SvNumberFormatter> pFormTable;
    mutable rtl::Reference<SfxItemPool>  mpEditTextObjectPool;     // EditTextObjectPool
    mutable rtl::Reference<SfxItemPool>  mpEditEnginePool;   // EditEnginePool

public:
                ScPoolHelper( ScDocument& rSourceDoc );
    virtual     ~ScPoolHelper() override;

                // called in dtor of main document
    void        SourceDocumentGone();

                // access to pointers (are never 0):
    ScDocumentPool*     GetDocPool() const      { return pDocPool.get(); }
    ScStyleSheetPool*   GetStylePool() const    { return mxStylePool.get(); }
    SvNumberFormatter*  GetFormTable() const;
    SfxItemPool*        GetEditTextObjectPool() const;
    SfxItemPool*        GetEditEnginePool() const;

    void                SetFormTableOpt(const ScDocOptions& rOpt);

    std::unique_ptr<SvNumberFormatter> CreateNumberFormatter() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
