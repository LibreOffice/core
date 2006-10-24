#ifndef INCLUDED_X_NOTE_HELPER_IMPL_HXX
#define INCLUDED_X_NOTE_HELPER_IMPL_HXX

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
doctok::Reference<Stream>::Pointer_t
XNoteHelper<T>::get(sal_uInt32 nPos)
{
    // There are getCount() entries => greater or equal
    if (nPos >= getCount())
        throw ExceptionNotFound("get");

    doctok::Reference<Stream>::Pointer_t pResult;

    CpAndFc aCpAndFcStart(getCpAndFc(nPos));
    CpAndFc aCpAndFcEnd(getCpAndFc(nPos + 1));

    if (aCpAndFcStart < aCpAndFcEnd)
        pResult = doctok::Reference<Stream>::Pointer_t
            (new WW8DocumentImpl(*mpDocument, aCpAndFcStart, aCpAndFcEnd));

    return pResult;
}

template <class T>
sal_uInt32 XNoteHelper<T>::getIndexOfCpAndFc(const CpAndFc & rCpAndFc)
{
    sal_uInt32 n = getCount();

    if (n > 0)
    {
        --n;

        Cp aCp(mpRefs->getFc(n));
        Fc aFc(mpPieceTable->cp2fc(aCp));
        CpAndFc aCpAndFc(aCp, aFc, meType);

        while (n > 0 && rCpAndFc < aCpAndFc)
            --n;
    }

    return n;
}

template <class T>
doctok::Reference<Stream>::Pointer_t
XNoteHelper<T>::get(const CpAndFc & rCpAndFc)
{
    doctok::Reference<Stream>::Pointer_t pResult;

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
}

#endif // INCLUDED_X_NOTE_HELPER_IMPL_HXX
