/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVTOOLS_BREADCRUMB_HXX
#define INCLUDED_SVTOOLS_BREADCRUMB_HXX

#include <svtools/svtdllapi.h>

#include <tools/urlobj.hxx>

#include <vcl/fixedhyper.hxx>
#include <vcl/layout.hxx>

#include <vector>

class SVT_DLLPUBLIC Breadcrumb : public VclHBox
{
    private:
        std::vector< VclPtr< FixedHyperlink > > m_aLinks;
        std::vector< VclPtr< FixedText > > m_aSeparators;

        OUString m_sRootName;
        OUString m_sClickedURL;

        Link<> m_aClickHdl;

        void appendField();

        DECL_LINK ( ClickLinkHdl, FixedHyperlink* );

    public:
        Breadcrumb( vcl::Window* pParent, WinBits nWinStyle = 0 );
        ~Breadcrumb();

        void dispose();

        void SetClickHdl( const Link<>& rLink );
        OUString GetHdlURL();

        void SetRootName( const OUString& rURL );
        void SetURL( const OUString& rURL );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
