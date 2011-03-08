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

#ifndef SC_SORTDLG_HXX
#define SC_SORTDLG_HXX

#include <sfx2/tabdlg.hxx>

#if TEST_LAYOUT
#define LAYOUT_SFX_TABDIALOG_BROKEN 0
#else /* !TEST_LAYOUT */
#ifndef LAYOUT_SFX_TABDIALOG_BROKEN
#define LAYOUT_SFX_TABDIALOG_BROKEN 1
#endif /* !LAYOUT_SFX_TABDIALOG_BROKEN */
#endif /* !TEST_LAYOUT */

#if !LAYOUT_SFX_TABDIALOG_BROKEN
#include <sfx2/layout.hxx>
#include <layout/layout-pre.hxx>
#endif

class ScSortDlg : public SfxTabDialog
{
public:
                ScSortDlg( Window*           pParent,
                           const SfxItemSet* pArgSet );
                ~ScSortDlg();

    void    SetHeaders( BOOL bHeaders );
    void    SetByRows ( BOOL bByRows );
    BOOL    GetHeaders() const;
    BOOL    GetByRows () const;


private:
    BOOL    bIsHeaders;
    BOOL    bIsByRows;
};

inline void ScSortDlg::SetHeaders( BOOL bHeaders )  { bIsHeaders = bHeaders; }
inline void ScSortDlg::SetByRows ( BOOL bByRows  )  { bIsByRows = bByRows; }
inline BOOL ScSortDlg::GetHeaders() const           { return bIsHeaders; }
inline BOOL ScSortDlg::GetByRows () const           { return bIsByRows; }

#if !LAYOUT_SFX_TABDIALOG_BROKEN
#include <layout/layout-post.hxx>
#endif

#endif // SC_SORTDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
