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

#include "xcl97dum.hxx"






const sal_uInt8 ExcDummy8_00a::pMyData[] = {
    0xe1, 0x00, 0x02, 0x00, 0xb0, 0x04,                     
    0xc1, 0x00, 0x02, 0x00, 0x00, 0x00,                     
    0xe2, 0x00, 0x00, 0x00,                                 
    0x5c, 0x00, 0x70, 0x00,                                 
    0x04, 0x00, 0x00,  'C',  'a',  'l',  'c', 0x20,         
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x42, 0x00, 0x02, 0x00, 0xb0, 0x04,                     
    0x61, 0x01, 0x02, 0x00, 0x00, 0x00                      
};
const sal_Size ExcDummy8_00a::nMyLen = sizeof(ExcDummy8_00a::pMyData);

                                                            

const sal_uInt8 ExcDummy8_00b::pMyData[] = {
    0x9c, 0x00, 0x02, 0x00, 0x0e, 0x00                      
};
const sal_Size ExcDummy8_00b::nMyLen = sizeof(ExcDummy8_00b::pMyData);


const sal_uInt8 ExcDummy8_040::pMyData[] = {
    0xaf, 0x01, 0x02, 0x00, 0x00, 0x00,                     
    0xbc, 0x01, 0x02, 0x00, 0x00, 0x00,                     



    0x40, 0x00, 0x02, 0x00, 0x00, 0x00,                     
    0x8d, 0x00, 0x02, 0x00, 0x00, 0x00                      
};
const sal_Size ExcDummy8_040::nMyLen = sizeof(ExcDummy8_040::pMyData);


const sal_uInt8 ExcDummy8_041::pMyData[] = {
    0xb7, 0x01, 0x02, 0x00, 0x00, 0x00,                     
    0xda, 0x00, 0x02, 0x00, 0x00, 0x00                      
};
const sal_Size ExcDummy8_041::nMyLen = sizeof(ExcDummy8_041::pMyData);



const sal_uInt8 ExcDummy8_02::pMyData[] = {
    0x5f, 0x00, 0x02, 0x00, 0x01, 0x00                      
    };
const sal_Size ExcDummy8_02::nMyLen = sizeof(ExcDummy8_02::pMyData);




sal_Size ExcDummy8_00a::GetLen() const
{
    return nMyLen;
}


const sal_uInt8* ExcDummy8_00a::GetData() const
{
    return pMyData;
}



sal_Size ExcDummy8_00b::GetLen() const
{
    return nMyLen;
}


const sal_uInt8* ExcDummy8_00b::GetData() const
{
    return pMyData;
}



sal_Size ExcDummy8_040::GetLen() const
{
    return nMyLen;
}


const sal_uInt8* ExcDummy8_040::GetData() const
{
    return pMyData;
}



sal_Size ExcDummy8_041::GetLen() const
{
    return nMyLen;
}


const sal_uInt8* ExcDummy8_041::GetData() const
{
    return pMyData;
}



sal_Size ExcDummy8_02::GetLen() const
{
    return nMyLen;
}


const sal_uInt8* ExcDummy8_02::GetData() const
{
    return pMyData;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
