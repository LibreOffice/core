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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_linguistic.hxx"
#include <tools/debug.hxx>
#include <osl/mutex.hxx>

#include <linguistic/misc.hxx>

#include "thesdta.hxx"

//using namespace utl;
using namespace osl;
using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;


namespace linguistic
{

///////////////////////////////////////////////////////////////////////////


ThesaurusMeaning::ThesaurusMeaning(const OUString &rText,
                 const OUString &rLookUpText, sal_Int16 nLookUpLang ) :
    aText               (rText),
    aLookUpText         (rLookUpText),
    nLookUpLanguage     (nLookUpLang)
{
}

ThesaurusMeaning::~ThesaurusMeaning()
{
}

OUString SAL_CALL
        ThesaurusMeaning::getMeaning() throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return aText;
}

/*
uno::Sequence< OUString > SAL_CALL ThesaurusMeaning::querySynonyms()
        throw(RuntimeException)
{
}
*/

///////////////////////////////////////////////////////////////////////////

}   // namespace linguistic

