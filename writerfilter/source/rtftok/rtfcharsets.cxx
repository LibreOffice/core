/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include <rtfcharsets.hxx>
#include <sal/macros.h>

namespace writerfilter {
namespace rtftok {


RTFEncoding aRTFEncodings[] = {
    
    {0, 1252}, 
    {1, 0}, 
    {2, 42}, 
    {77, 10000}, 
    {78, 10001}, 
    {79, 10003}, 
    {80, 10008}, 
    {81, 10002}, 
    {83, 10005}, 
    {84, 10004}, 
    {85, 10006}, 
    {86, 10081}, 
    {87, 10021}, 
    {88, 10029}, 
    {89, 10007}, 
    {128, 932}, 
    {129, 949}, 
    {130, 1361}, 
    {134, 936}, 
    {136, 950}, 
    {161, 1253}, 
    {162, 1254}, 
    {163, 1258}, 
    {177, 1255}, 
    {178, 1256}, 
    {186, 1257}, 
    {204, 1251}, 
    {222, 874}, 
    {238, 1250}, 
    {254, 437}, 
    {255, 850}, 
};

int nRTFEncodings = SAL_N_ELEMENTS(aRTFEncodings);

} 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
