/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RefFunctor.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 15:31:43 $
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

#ifndef DBAUI_REFFUNCTOR_HXX
#define DBAUI_REFFUNCTOR_HXX

#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif
#ifndef INCLUDED_FUNCTIONAL
#define INCLUDED_FUNCTIONAL
#include <functional>
#endif // INCLUDED_FUNCTIONAL

namespace dbaui
{
    template <class T> class OUnaryRefFunctor : public ::std::unary_function< ::vos::ORef<T> ,void>
    {
        ::std::mem_fun_t<bool,T> m_aFunction;
    public:
        OUnaryRefFunctor(const ::std::mem_fun_t<bool,T>& _aFunc) : m_aFunction(_aFunc)
        {}
        inline void operator()(const ::vos::ORef<T>& _aType) const
        {
            m_aFunction(_aType.getBodyPtr());
        }
//      inline void operator()(const ::vos::ORef<T>& _aType)
//      {
//          m_aFunction(_aType.getBodyPtr());
//      }
    };
// -----------------------------------------------------------------------------
} // namespace dbaui
// -----------------------------------------------------------------------------
#endif //DBAUI_REFFUNCTOR_HXX

