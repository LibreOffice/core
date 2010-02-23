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

