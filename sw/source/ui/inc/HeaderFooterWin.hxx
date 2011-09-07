/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * [ Copyright (C) 2011 SUSE <cbosdonnat@suse.com> (initial developer) ]
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
#ifndef _HEADERFOOTERWINDOW_HXX
#define _HEADERFOOTERWINDOW_HXX

#include <pagedesc.hxx>

#include <vcl/window.hxx>

class SwHeaderFooterButton;

/** Class for the header and footer separator control window.

    This control is showing the header / footer style name and provides
    a few useful actions to the user.
  */
class SwHeaderFooterWin : public Window
{
    SwEditWin*            m_pEditWin;
    rtl::OUString         m_sLabel;
    const SwPageFrm*      m_pPageFrm;
    bool                  m_bIsHeader;
    SwHeaderFooterButton* m_pButton;

public:
    SwHeaderFooterWin( SwEditWin* pEditWin, const SwPageFrm* pPageFrm, bool bHeader, Point aOffset );
    ~SwHeaderFooterWin( );

    virtual void Paint( const Rectangle& rRect );

    bool IsHeader() { return m_bIsHeader; };
    bool IsEmptyHeaderFooter( );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
