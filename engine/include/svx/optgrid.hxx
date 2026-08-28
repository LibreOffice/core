/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#ifndef INCLUDED_SVX_OPTGRID_HXX
#define INCLUDED_SVX_OPTGRID_HXX

#include <sfx2/tabdlg.hxx>
#include <sfx2/htmlmode.hxx>
#include <svx/svxdllapi.h>

namespace weld { class CheckButton; }
namespace weld { class MetricSpinButton; }
namespace weld { class SpinButton; }
namespace weld { class Widget; }

class SVX_DLLPUBLIC SvxOptionsGrid
{
protected:
    sal_uInt32  m_nFldDrawX;
    sal_uInt32  m_nFldDivisionX;
    sal_uInt32  m_nFldDrawY;
    sal_uInt32  m_nFldDivisionY;
    bool        m_bUseGridsnap:1;
    bool        m_bSynchronize:1;
    bool        m_bGridVisible:1;

public:
    SvxOptionsGrid();

    void    SetFieldDrawX(    sal_uInt32 nSet){m_nFldDrawX      = nSet;}
    void    SetFieldDivisionX(sal_uInt32 nSet){m_nFldDivisionX  = nSet;}
    void    SetFieldDrawY   ( sal_uInt32 nSet){m_nFldDrawY      = nSet;}
    void    SetFieldDivisionY(sal_uInt32 nSet){m_nFldDivisionY  = nSet;}
    void    SetUseGridSnap( bool bSet ) {m_bUseGridsnap   = bSet;}
    void    SetSynchronize( bool bSet ) {m_bSynchronize   = bSet;}
    void    SetGridVisible( bool bSet ) {m_bGridVisible   = bSet;}

    sal_uInt32  GetFieldDrawX(    ) const {  return m_nFldDrawX;    }
    sal_uInt32  GetFieldDivisionX() const {  return m_nFldDivisionX;}
    sal_uInt32  GetFieldDrawY   ( ) const {  return m_nFldDrawY;    }
    sal_uInt32  GetFieldDivisionY() const {  return m_nFldDivisionY;}
    bool        GetUseGridSnap( ) const {  return m_bUseGridsnap; }
    bool        GetSynchronize( ) const {  return m_bSynchronize; }
    bool        GetGridVisible( ) const {  return m_bGridVisible; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
