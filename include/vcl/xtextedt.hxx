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
#ifndef INCLUDED_VCL_XTEXTEDT_HXX
#define INCLUDED_VCL_XTEXTEDT_HXX

#include <vcl/dllapi.h>
#include <vcl/texteng.hxx>
#include <vcl/textview.hxx>

namespace com {
namespace sun {
namespace star {
namespace util {
    struct SearchOptions;
}}}}

class VCL_DLLPUBLIC ExtTextEngine : public TextEngine
{
private:
    OUString              maGroupChars;

public:
                        ExtTextEngine();
                        virtual ~ExtTextEngine();

    TextSelection       MatchGroup( const TextPaM& rCursor ) const;
    bool                Search( TextSelection& rSel, const css::util::SearchOptions& rSearchOptions, bool bForward = true );
};

class VCL_DLLPUBLIC ExtTextView : public TextView
{
protected:
    bool            ImpIndentBlock( bool bRight );

public:
                    ExtTextView( ExtTextEngine* pEng, vcl::Window* pWindow );
                    virtual ~ExtTextView();

    bool            MatchGroup();

    bool            Search( const css::util::SearchOptions& rSearchOptions, bool bForward );
    sal_uInt16      Replace( const css::util::SearchOptions& rSearchOptions, bool bAll, bool bForward );

    bool            IndentBlock();
    bool            UnindentBlock();
};

#endif // INCLUDED_VCL_XTEXTEDT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
