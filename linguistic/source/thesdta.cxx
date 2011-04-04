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
#include "precompiled_linguistic.hxx"
#include <tools/debug.hxx>
#include <osl/mutex.hxx>

#include <linguistic/misc.hxx>

#include "thesdta.hxx"

using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;

using ::rtl::OUString;

namespace linguistic
{



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


}   // namespace linguistic

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
