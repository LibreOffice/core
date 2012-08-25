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

#ifndef BASCTL_BASDOC_HXX
#define BASCTL_BASDOC_HXX

#include <iderid.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/objsh.hxx>
#include <svx/ifaceids.hxx>

class SfxPrinter;

namespace basctl
{

class DocShell: public SfxObjectShell
{
    SfxPrinter*         pPrinter;

protected:
    virtual void    Draw( OutputDevice *, const JobSetup & rSetup,
                          sal_uInt16 nAspect = ASPECT_CONTENT );
    virtual void    FillClass( SvGlobalName * pClassName,
                               sal_uInt32 * pFormat,
                               String * pAppName,
                               String * pFullTypeName,
                               String * pShortTypeName,
                               sal_Int32 nVersion,
                               sal_Bool bTemplate = false ) const;

public:
                        TYPEINFO();

                        using SotObject::GetInterface;
                        SFX_DECL_OBJECTFACTORY();
                        SFX_DECL_INTERFACE( SVX_INTERFACE_BASIDE_DOCSH )
                        DocShell();
                        ~DocShell();

    SfxPrinter*         GetPrinter( bool bCreate );
    void                SetPrinter( SfxPrinter* pPrinter );
};

} // namespace basctl

// This typedef helps baside.sdi,
// because I don't know how to use nested names in it.
typedef basctl::DocShell basctl_DocShell;

#endif // BASCTL_BASDOC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
