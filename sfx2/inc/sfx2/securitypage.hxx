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
#ifndef _SECURITYPAGE_HXX_
#define _SECURITYPAGE_HXX_

#include "sfx2/tabdlg.hxx"
#include "sfx2/htmlmode.hxx"

#include <memory>


//////////////////////////////////////////////////////////////////////

struct SfxSecurityPage_Impl;

class SfxSecurityPage : public SfxTabPage
{
    std::auto_ptr< SfxSecurityPage_Impl > m_pImpl;

protected:
    SfxSecurityPage( Window* pParent, const SfxItemSet& );
    virtual ~SfxSecurityPage();

    virtual sal_Bool            FillItemSet( SfxItemSet& );
    virtual void            Reset( const SfxItemSet& );

public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& );
};

//////////////////////////////////////////////////////////////////////

#endif // #ifndef _SECURITYPAGE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
