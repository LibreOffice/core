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

#ifndef _LIST_HXX
#define _LIST_HXX

#include <tools/solar.h>
#include <tools/contnr.hxx>

#define LIST_APPEND           CONTAINER_APPEND
#define LIST_ENTRY_NOTFOUND   CONTAINER_ENTRY_NOTFOUND

class List : private Container
{
public:
            using Container::Insert;
            using Container::Remove;
            using Container::Replace;
            using Container::Clear;
            using Container::Count;
            using Container::GetCurObject;
            using Container::GetCurPos;
            using Container::GetObject;
            using Container::GetPos;
            using Container::Seek;
            using Container::First;
            using Container::Last;
            using Container::Next;
            using Container::Prev;

            List( sal_uInt16 _nInitSize = 16, sal_uInt16 _nReSize = 16 ) :
                Container( 1024, _nInitSize, _nReSize ) {}
            List( sal_uInt16 _nBlockSize, sal_uInt16 _nInitSize, sal_uInt16 _nReSize ) :
                Container( _nBlockSize, _nInitSize, _nReSize ) {}
            List( const List& rList ) : Container( rList ) {}

    List&   operator =( const List& rList )
                { Container::operator =( rList ); return *this; }

    sal_Bool    operator ==( const List& rList ) const
                { return Container::operator ==( rList ); }
    sal_Bool    operator !=( const List& rList ) const
                { return Container::operator !=( rList ); }
};

#endif // _LIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
