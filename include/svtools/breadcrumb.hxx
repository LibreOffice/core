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

#include <vcl/layout.hxx>

#include <vector>

class FixedHyperlink;

#define SPACING 6

enum SvtBreadcrumbMode
{
    ONLY_CURRENT_PATH = 0,
    ALL_VISITED = 1
};

class CustomLink;

class SVT_DLLPUBLIC Breadcrumb : public VclHBox
{
    private:
        std::vector< VclPtr< CustomLink > > m_aLinks;
        std::vector< VclPtr< FixedText > > m_aSeparators;

        OUString m_sRootName;
        OUString m_sClickedURL;
        OUString m_aCurrentURL;

        SvtBreadcrumbMode m_eMode;

        Link<Breadcrumb*,void> m_aClickHdl;

        void appendField();
        bool showField( unsigned int nIndex, unsigned int nWidthMax );

        DECL_LINK( ClickLinkHdl, FixedHyperlink&, void );

    public:
        Breadcrumb( vcl::Window* pParent );
        virtual ~Breadcrumb() override;

        void dispose() override;
        void EnableFields( bool bEnable );

        void SetClickHdl( const Link<Breadcrumb*,void>& rLink );
        const OUString& GetHdlURL();

        void SetRootName( const OUString& rURL );
        void SetURL( const OUString& rURL );
        void SetMode( SvtBreadcrumbMode eMode );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
