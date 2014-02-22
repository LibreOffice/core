/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

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
