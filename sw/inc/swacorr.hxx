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
#ifndef _SWACORR_HXX
#define _SWACORR_HXX

#include <editeng/svxacorr.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/embed/XStorage.hpp>

class SwAutoCorrect : public SvxAutoCorrect
{
    using  SvxAutoCorrect::PutText;

protected:
    // Return replacement text (only for SWG-format, all others can be obtained from wordlist!).
    // rShort is stream-name - encrypted!
    virtual sal_Bool GetLongText( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >&,
                                  const String& rFileName, const String& rShort, OUString& rLong );

    // Text with attributes (only SWG-format!).
    // rShort is stream-name - encrypted!
    virtual sal_Bool PutText( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >&,
                              const String& rFileName, const String& rShort, SfxObjectShell&, OUString& );

public:
    TYPEINFO();

    SwAutoCorrect( const SvxAutoCorrect& rACorr );
    virtual ~SwAutoCorrect();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
