/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



/*|     Author: Alexander Gelfenbain                    |*/


#ifndef __XLAT_H
#define __XLAT_H

//####include "sft.hxx"

namespace vcl
{
// TODO: sal_UCS4

    sal_uInt16 TranslateChar12(sal_uInt16);
    sal_uInt16 TranslateChar13(sal_uInt16);
    sal_uInt16 TranslateChar14(sal_uInt16);
    sal_uInt16 TranslateChar15(sal_uInt16);
    sal_uInt16 TranslateChar16(sal_uInt16);

    void TranslateString12(sal_uInt16 *, sal_uInt16 *, sal_uInt32);
    void TranslateString13(sal_uInt16 *, sal_uInt16 *, sal_uInt32);
    void TranslateString14(sal_uInt16 *, sal_uInt16 *, sal_uInt32);
    void TranslateString15(sal_uInt16 *, sal_uInt16 *, sal_uInt32);
    void TranslateString16(sal_uInt16 *, sal_uInt16 *, sal_uInt32);
}

#endif /* __XLAT_H */

