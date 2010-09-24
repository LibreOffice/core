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


#if (defined(OS2) && defined(ICC))
#define CAST_TO_IREFERENCE(p) ((IReference*)(p))
#else
#define CAST_TO_IREFERENCE(p) (p)
#endif

template <class T>
inline ORef<T>::ORef()
{
    m_refBody= 0;
}

template <class T>
inline ORef<T>::ORef(T* pBody)
{
    m_refBody= pBody;

    if (m_refBody)
        CAST_TO_IREFERENCE(m_refBody)->acquire();
}

template <class T>
inline ORef<T>::ORef(const ORef<T>& handle)
{
    m_refBody= handle.m_refBody;

    if (m_refBody)
        CAST_TO_IREFERENCE(m_refBody)->acquire();
}

template <class T>
inline ORef<T>::~ORef()
{
    if (m_refBody)
        CAST_TO_IREFERENCE(m_refBody)->release();
}

template <class T>
inline ORef<T>& ORef<T>::operator= (const ORef<T>& handle)
{
    if (m_refBody)
        CAST_TO_IREFERENCE(m_refBody)->release();

    m_refBody= handle.m_refBody;

    if (m_refBody)
        CAST_TO_IREFERENCE(m_refBody)->acquire();

    return *this;
}

template <class T>
inline void ORef<T>::bind(T* pBody)
{
    if (m_refBody)
        CAST_TO_IREFERENCE(m_refBody)->release();

    m_refBody= pBody;

    if (m_refBody)
        CAST_TO_IREFERENCE(m_refBody)->acquire();
}

template <class T>
inline ORef<T>& ORef<T>::unbind()
{
    if (m_refBody)
    {
        CAST_TO_IREFERENCE(m_refBody)->release();
        m_refBody = 0;
    }
    return *this;
}

template <class T>
inline void ORef<T>::operator= (T* pBody)
{
    bind(pBody);
}

template <class T>
inline T& ORef<T>::operator() () const
{
    VOS_PRECOND(m_refBody, "ORef::operator(): can't deref nil body!");
    return *m_refBody;
}

template <class T>
inline T& ORef<T>::operator* () const
{
    VOS_PRECOND(m_refBody, "ORef::operator*: can't deref nil body!");
    return *m_refBody;
}

template <class T>
inline T* ORef<T>::operator->() const
{
    VOS_PRECOND(m_refBody, "ORef::operator->: nil body!");
    return m_refBody;
}

template <class T>
inline T& ORef<T>::getBody() const
{
    VOS_PRECOND(m_refBody, "ORef::getBody(): can't deref nil body!");
    return *m_refBody;
}

template <class T>
inline T* ORef<T>::getBodyPtr() const
{
    // might be nil
    return m_refBody;
}

template <class T>
inline sal_Bool ORef<T>::isEmpty() const
{
    return m_refBody == 0;
}

template <class T>
inline sal_Bool ORef<T>::isValid() const
{
    return m_refBody != 0;
}

template <class T>
inline sal_Bool ORef<T>::isEqualBody(const ORef<T>& handle) const
{
    return m_refBody == handle.m_refBody;
}

template <class T>
inline sal_Bool ORef<T>::operator== (const ORef<T>& handle) const
{
    return m_refBody == handle.m_refBody;
}

template <class T>
inline sal_Bool ORef<T>::operator!= (const ORef<T>& handle) const
{
    return m_refBody != handle.m_refBody;
}

template <class T>
inline sal_Bool ORef<T>::operator== (const T* pBody) const
{
    return m_refBody == pBody;
}

template <class T>
inline sal_Bool ORef<T>::operator< (const ORef<T>& handle) const
{
    return m_refBody < handle.m_refBody;
}

template <class T>
inline sal_Bool ORef<T>::operator> (const ORef<T>& handle) const
{
    return m_refBody > handle.m_refBody;
}


