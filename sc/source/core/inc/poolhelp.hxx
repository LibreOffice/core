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

#ifndef SC_POOLHELP_HXX
#define SC_POOLHELP_HXX

#include <rtl/ref.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include <tools/link.hxx>
#include "docoptio.hxx"

class ScDocument;
class ScDocumentPool;
class ScStyleSheetPool;
class SvNumberFormatter;
class SfxItemPool;


class ScPoolHelper : public salhelper::SimpleReferenceObject
{
private:
    ScDocOptions        aOpt;
    ScDocumentPool*     pDocPool;
    rtl::Reference< ScStyleSheetPool > mxStylePool;
    mutable SvNumberFormatter*  pFormTable;
    mutable SfxItemPool*        pEditPool;                      // EditTextObjectPool
    mutable SfxItemPool*        pEnginePool;                    // EditEnginePool
    ScDocument*         m_pSourceDoc;

    void                UseDocOptions() const;

public:
                ScPoolHelper( ScDocument* pSourceDoc );
    virtual     ~ScPoolHelper();

                // called in dtor of main document
    void        SourceDocumentGone();

                // access to pointers (are never 0):
    ScDocumentPool*     GetDocPool() const      { return pDocPool; }
    ScStyleSheetPool*   GetStylePool() const    { return mxStylePool.get(); }
    SvNumberFormatter*  GetFormTable() const;
    SfxItemPool*        GetEditPool() const;
    SfxItemPool*        GetEnginePool() const;

    void                SetFormTableOpt(const ScDocOptions& rOpt);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
