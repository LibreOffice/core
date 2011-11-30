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


