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

#include <osl/mutex.hxx>

#include "nthesdta.hxx"
#include <linguistic/misc.hxx>

using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;

namespace linguistic
{

Meaning::Meaning(
            const OUString &rTerm) :

    aSyn        ( Sequence< OUString >(1) ),
    aTerm       (rTerm)

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
{
    MutexGuard  aGuard( GetLinguMutex() );
    return aTerm;
}

Sequence< OUString > SAL_CALL Meaning::querySynonyms()
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

}   // namespace linguistic

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
