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

#ifndef SC_TPHF_HXX
#define SC_TPHF_HXX


#include <bf_svx/hdft2.hxx>
namespace binfilter {

class ScStyleDlg;

//========================================================================

class ScHFPage : public SvxHFPage
{
public:
    virtual			~ScHFPage();

    virtual void	Reset( const SfxItemSet& rSet );
    virtual BOOL 	FillItemSet( SfxItemSet& rOutSet );

    void			SetPageStyle( const String& rName )    { aStrPageStyle = rName; }
    void			SetStyleDlg ( const ScStyleDlg* pDlg ) { pStyleDlg = pDlg; }

protected:
                    ScHFPage( Window* pParent,
                              USHORT nResId,
                              const SfxItemSet& rSet,
                              USHORT nSetId );

    virtual void	ActivatePage( const SfxItemSet& rSet );
    virtual int		DeactivatePage( SfxItemSet* pSet = 0 );

private:
    PushButton			aBtnEdit;
    SfxItemSet			aDataSet;
    String				aStrPageStyle;
    USHORT				nPageUsage;
    const ScStyleDlg*	pStyleDlg;

#ifdef _TPHF_CXX
private:
    DECL_LINK( BtnHdl, PushButton* );
    DECL_LINK( HFEditHdl, void* );
    DECL_LINK( TurnOnHdl, CheckBox* );
#endif
};

//========================================================================

class ScHeaderPage : public ScHFPage
{
public:
    static SfxTabPage* 	Create(	Window* pParent, const SfxItemSet& rSet );
    static USHORT*		GetRanges();

private:
    ScHeaderPage( Window* pParent, const SfxItemSet& rSet );
};

//========================================================================

class ScFooterPage : public ScHFPage
{
public:
    static SfxTabPage* 	Create(	Window* pParent, const SfxItemSet& rSet );
    static USHORT*		GetRanges();

private:
    ScFooterPage( Window* pParent, const SfxItemSet& rSet );
};


} //namespace binfilter
#endif // SC_TPHF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
