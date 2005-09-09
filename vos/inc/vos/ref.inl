/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ref.inl,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:02:12 $
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


