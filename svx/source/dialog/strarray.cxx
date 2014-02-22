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

#include <tools/shl.hxx>

#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <svx/strarray.hxx>



SvxStringArray::SvxStringArray( sal_uInt32 nResId ) :

    ResStringArray( SVX_RES( nResId ) )

{
}



SvxStringArray::SvxStringArray( const ResId& rResId ) :
    ResStringArray( rResId )
{
}




SvxStringArray::~SvxStringArray()
{
}



const OUString SvxStringArray::GetStringByPos( sal_uInt32 nPos ) const
{
    if ( RESARRAY_INDEX_NOTFOUND != nPos && nPos < Count() )
        return ResStringArray::GetString( nPos );
    return OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
