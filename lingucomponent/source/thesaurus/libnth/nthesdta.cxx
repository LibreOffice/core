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
#include "precompiled_lingucomponent.hxx"
#include <com/sun/star/uno/Reference.h>
#include <tools/debug.hxx>
#include <unotools/processfactory.hxx>
#include <osl/mutex.hxx>

#include "nthesdta.hxx"
#include <linguistic/misc.hxx>

// #include "lngsvcmgr.hxx"


using namespace utl;
using namespace osl;
using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;

namespace linguistic
{

///////////////////////////////////////////////////////////////////////////

Meaning::Meaning(
#if 0
            const OUString &rTerm, sal_Int16 nLang,
            const PropertyHelper_Thes &rHelper ) :
#else
            const OUString &rTerm, sal_Int16 nLang) :
#endif

    aSyn        ( Sequence< OUString >(1) ),
    aTerm       (rTerm),
    nLanguage   (nLang)

{
#if 0
    // this is for future use by a german thesaurus when one exists
    bIsGermanPreReform = rHelper.IsGermanPreReform;
#endif
}


Meaning::~Meaning()
{
}


OUString SAL_CALL Meaning::getMeaning()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return aTerm;
}


Sequence< OUString > SAL_CALL Meaning::querySynonyms()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
        return aSyn;
}


void Meaning::SetSynonyms( const Sequence< OUString > &rSyn )
{
        MutexGuard      aGuard( GetLinguMutex() );
        aSyn = rSyn;
}

void Meaning::SetMeaning( const OUString &rTerm )
{
        MutexGuard      aGuard( GetLinguMutex() );
        aTerm = rTerm;
}

///////////////////////////////////////////////////////////////////////////

}   // namespace linguistic






