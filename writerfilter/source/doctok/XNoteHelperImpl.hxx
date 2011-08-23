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

    if (aCpAndFcStart < aCpAndFcEnd)
        pResult = writerfilter::Reference<Stream>::Pointer_t
            (new WW8DocumentImpl(*mpDocument, aCpAndFcStart, aCpAndFcEnd));

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
