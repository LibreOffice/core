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




template <class T>
ORefObj<T>::ORefObj(const T& Obj)
{
    m_Obj = Obj;

    m_RefCount.acquire();
}

template <class T>
inline ORefObj<T>::~ORefObj()
{
    VOS_ASSERT(m_RefCount.referenced() == 0);
}

template <class T>
inline T& ORefObj<T>::operator=(const T& Obj)
{
    m_Obj = Obj;

    return m_Obj;
}

template <class T>
inline ORefObj<T>::operator T&()
{
    return m_Obj;
}

template <class T>
inline ORefObj<T>::operator const T&() const
{
    return m_Obj;
}

template <class T>
inline T& ORefObj<T>::operator() ()
{
    return m_Obj;
}

template <class T>
inline const T& ORefObj<T>::operator() () const
{
    return m_Obj;
}

template <class T>
inline T& ORefObj<T>::getObj()
{
    return m_Obj;
}

template <class T>
inline const T& ORefObj<T>::getObj() const
{
    return m_Obj;
}

