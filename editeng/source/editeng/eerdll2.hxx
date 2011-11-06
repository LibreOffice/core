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



#ifndef _EERDLL2_HXX
#define _EERDLL2_HXX

#include <com/sun/star/linguistic2/XLanguageGuessing.hpp>
#include <editeng/forbiddencharacterstable.hxx>
#include <vos/ref.hxx>

class SfxPoolItem;

class GlobalEditData
{
private:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XLanguageGuessing >  xLanguageGuesser;
    SfxPoolItem**   ppDefItems;
    OutputDevice*   pStdRefDevice;

    vos::ORef<SvxForbiddenCharactersTable>  xForbiddenCharsTable;

public:
                    GlobalEditData();
                    ~GlobalEditData();

    SfxPoolItem**   GetDefItems();
    OutputDevice*   GetStdRefDevice();

    vos::ORef<SvxForbiddenCharactersTable>  GetForbiddenCharsTable();
    void            SetForbiddenCharsTable( vos::ORef<SvxForbiddenCharactersTable> xForbiddenChars ) { xForbiddenCharsTable = xForbiddenChars; }
    ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XLanguageGuessing > GetLanguageGuesser();
};


#endif //_EERDLL2_HXX

