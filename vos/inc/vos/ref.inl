/*************************************************************************
 *
 *  $RCSfile: ref.inl,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:18:12 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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


