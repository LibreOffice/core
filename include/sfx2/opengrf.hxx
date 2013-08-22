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
#ifndef _SFX2_OPENGRF_HXX
#define _SFX2_OPENGRF_HXX

#include <memory>       // auto_ptr
#include <vcl/graphicfilter.hxx>
#include <sfx2/dllapi.h>

struct  SvxOpenGrf_Impl;

class SFX2_DLLPUBLIC SvxOpenGraphicDialog
{
public:
    SvxOpenGraphicDialog    ( const OUString& rTitle );
    ~SvxOpenGraphicDialog   ();

    short                   Execute();

    void                    SetPath( const OUString& rPath );
    void                    SetPath( const OUString& rPath, sal_Bool bLinkState );
    OUString                GetPath() const;

    int                     GetGraphic(Graphic&) const;

    void                    EnableLink(sal_Bool);
    void                    AsLink(sal_Bool);
    sal_Bool                IsAsLink() const;

    OUString                GetCurrentFilter() const;
    void                    SetCurrentFilter(const OUString&);
private:
    // disable copy and assignment
    SFX2_DLLPRIVATE SvxOpenGraphicDialog    (const SvxOpenGraphicDialog&);
    SFX2_DLLPRIVATE SvxOpenGraphicDialog& operator = ( const SvxOpenGraphicDialog & );

    const std::auto_ptr< SvxOpenGrf_Impl >  mpImpl;
};

#endif // _SVX_OPENGRF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
