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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlhelp.hxx"
#include <util/Decompressor.hxx>
#include <util/CompressorIterator.hxx>

using namespace xmlsearch;
using namespace xmlsearch::util;

const sal_Int32 Decompressor::BitsInByte = 8;
const sal_Int32 Decompressor::NBits = 32;

inline sal_Int32 getInteger_( const sal_Int8* v )
{
    return (((((( (v[0]&0xFF) ) << 8 )
                | (v[1]&0xFF) ) << 8 )
                | (v[2]&0xFF) ) << 8 )
                | (v[3]&0xFF);
}

sal_Int32 StreamDecompressor::getNextByte()
{
  sal_Int8 a[4];
  a[0] = a[1] = a[2] = 0;
  if( in_->readBytes( &a[3],1 ) != -1 )
    return getInteger_( a );
  else
    return -1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
