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

#ifndef INCLUDED_X_NOTE_HELPER_IMPL_HXX
#define INCLUDED_X_NOTE_HELPER_IMPL_HXX

namespace writerfilter {
namespace doctok
{
template <class T>
sal_uInt32 XNoteHelper<T>::getCount() const
{
    sal_uInt32 nResult = 0;

    if (mpCps.get() != NULL && mpCps->getCount() > 8)
        nResult = ( mpCps->getCount() / 4) - 2;

    return nResult;
}

template <class T>
CpAndFc XNoteHelper<T>::getCpAndFc(sal_uInt32 nPos)
{
    // There are getCount() + 1 entries in mpOffsets => greater
    if (nPos > getCount())
        throw ExceptionNotFound("getCpAndFc");

    Cp aCp(mCpAndFcOffset.getCp() + mpCps->getU32(nPos * 4));
    Fc aFc(mpPieceTable->cp2fc(aCp));
    CpAndFc aCpAndFc(aCp, aFc, meType);

    return aCpAndFc;
}

template <class T>
CpAndFc XNoteHelper<T>::getRefCpAndFc(sal_uInt32 nPos)
{
    // There are getCount() entries in mpRefs => greater or equal
    if (nPos >= getCount())
        throw ExceptionNotFound("");

    Cp aCp(mpRefs->getFc(nPos));
    Fc aFc(mpPieceTable->cp2fc(aCp));
    CpAndFc aCpAndFc(aCp, aFc, meType);

    return aCpAndFc;
}

template <class T>
writerfilter::Reference<Stream>::Pointer_t
XNoteHelper<T>::get(sal_uInt32 nPos)
{
    // There are getCount() entries => greater or equal
    if (nPos >= getCount())
        throw ExceptionNotFound("get");

    writerfilter::Reference<Stream>::Pointer_t pResult;

    CpAndFc aCpAndFcStart(getCpAndFc(nPos));
    CpAndFc aCpAndFcEnd(getCpAndFc(nPos + 1));

    return pResult;
}

template <class T>
sal_uInt32 XNoteHelper<T>::getIndexOfCpAndFc(const CpAndFc & rCpAndFc)
{
   sal_uInt32 nResult = getCount();

   sal_uInt32 n = nResult;
   while (n > 0)
   {
       --n;

       Cp aCp(mpRefs->getFc(n));
       Fc aFc(mpPieceTable->cp2fc(aCp));
       CpAndFc aCpAndFc(aCp, aFc, meType);

       if (aCpAndFc <= rCpAndFc)
       {
           nResult = n;
           break;
       }
    }

    return nResult;
}

template <class T>
writerfilter::Reference<Stream>::Pointer_t
XNoteHelper<T>::get(const CpAndFc & rCpAndFc)
{
    writerfilter::Reference<Stream>::Pointer_t pResult;

    sal_uInt32 n = getIndexOfCpAndFc(rCpAndFc);

    if (n < getCount())
        pResult = get(n);

    return pResult;
}

template <class T>
T *
XNoteHelper<T>::getRef(sal_uInt32 nIndex)
{
    return mpRefs->getEntryPointer(nIndex);
}

template <class T>
T *
XNoteHelper<T>::getRef(const CpAndFc & rCpAndFc)
{
    T * pResult = NULL;

    sal_uInt32 n = getIndexOfCpAndFc(rCpAndFc);

    if (n < getCount())
    {
        pResult = getRef(n);
    }

    return pResult;
}

template <class T>
void XNoteHelper<T>::init()
{
    for (sal_uInt32 n = 0; n < getCount(); ++n)
    {
        CpAndFc aCpAndFc(getCpAndFc(n));
        mpDocument->insertCpAndFc(aCpAndFc);

        CpAndFc aCpAndFcRef(getRefCpAndFc(n));
        mpDocument->insertCpAndFc(aCpAndFcRef);
    }
}
}}

#endif // INCLUDED_X_NOTE_HELPER_IMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
