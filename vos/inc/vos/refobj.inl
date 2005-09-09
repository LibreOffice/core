/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: refobj.inl,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:03:07 $
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

