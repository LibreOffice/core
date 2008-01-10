/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XNoteHelperImpl.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 11:51:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
