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

#ifndef INCLUDED_SVX_FONTWORKBAR_HXX
#define INCLUDED_SVX_FONTWORKBAR_HXX

#include <sfx2/shell.hxx>
#include <sfx2/module.hxx>
#include <svx/ifaceids.hxx>
#include <svx/svxdllapi.h>

class SfxViewShell;
class SdrView;

/************************************************************************/

namespace svx
{

bool SVX_DLLPUBLIC checkForSelectedFontWork( SdrView const * pSdrView, sal_uInt32& nCheckStatus );

class SAL_WARN_UNUSED SVX_DLLPUBLIC FontworkBar : public SfxShell
{
public:
    SFX_DECL_INTERFACE(SVX_INTERFACE_FONTWORK_BAR)

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
    FontworkBar(SfxViewShell* pViewShell);
    virtual ~FontworkBar() override;

    static void execute( SdrView* pSdrView, SfxRequest const & rReq, SfxBindings& rBindings );
    static void getState( SdrView const * pSdrView, SfxItemSet& rSet );
};

}

#endif // INCLUDED_SVX_FONTWORKBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
