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
#ifndef _SFX_HELPER_HXX
#define _SFX_HELPER_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/ucb/NameClash.hpp>

#include <tools/string.hxx>
#include <tools/errcode.hxx>

// class SfxContentHelper ------------------------------------------------

class SfxContentHelper
{
public:
    static ::com::sun::star::uno::Sequence< ::rtl::OUString >
                                GetResultSet( const String& rURL );
    static ::com::sun::star::uno::Sequence< ::rtl::OUString >
                                GetHelpTreeViewContents( const String& rURL );
    static String               GetActiveHelpString( const String& rURL );
    static sal_Bool             IsHelpErrorDocument( const String& rURL );

    static sal_uIntPtr              GetSize( const String& rContent );
};

#endif // #ifndef _SFX_HELPER_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
