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
#ifndef _XTEXTEDT_HXX
#define _XTEXTEDT_HXX

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
    String              maGroupChars;

public:
                        ExtTextEngine();
                        ~ExtTextEngine();

    const String&       GetGroupChars() const { return maGroupChars; }
    void                SetGroupChars( const String& r ) { maGroupChars = r; }
    TextSelection       MatchGroup( const TextPaM& rCursor ) const;

    sal_Bool                Search( TextSelection& rSel, const ::com::sun::star::util::SearchOptions& rSearchOptions, sal_Bool bForward = sal_True );
};

class VCL_DLLPUBLIC ExtTextView : public TextView
{
protected:
    sal_Bool                ImpIndentBlock( sal_Bool bRight );

public:
                        ExtTextView( ExtTextEngine* pEng, Window* pWindow );
                        ~ExtTextView();

    sal_Bool                MatchGroup();

    sal_Bool                Search( const ::com::sun::star::util::SearchOptions& rSearchOptions, sal_Bool bForward );
    sal_uInt16              Replace( const ::com::sun::star::util::SearchOptions& rSearchOptions, sal_Bool bAll, sal_Bool bForward );

    sal_Bool                IndentBlock();
    sal_Bool                UnindentBlock();
};

#endif // _XTEXTEDT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
