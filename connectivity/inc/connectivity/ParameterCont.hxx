/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ParameterCont.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:41:11 $
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
#ifndef CONNECTIVITY_PARAMETERCONTINUATION_HXX_INCLUDED
#define CONNECTIVITY_PARAMETERCONTINUATION_HXX_INCLUDED

#include <com/sun/star/sdb/XInteractionSupplyParameters.hpp>
#include <comphelper/interaction.hxx>
namespace dbtools
{
//====================================================================
    //= OParameterContinuation
    //====================================================================
    class OParameterContinuation : public comphelper::OInteraction< ::com::sun::star::sdb::XInteractionSupplyParameters >
    {
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >       m_aValues;

    public:
        OParameterContinuation() { }

        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >   getValues() const { return m_aValues; }

        // XInteractionSupplyParameters
        virtual void SAL_CALL setParameters( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rValues ) throw(::com::sun::star::uno::RuntimeException);

    protected:
        virtual ~OParameterContinuation() { }
    private:
        OParameterContinuation(const OParameterContinuation&);
        void operator =(const OParameterContinuation&);
    };
} // dbtools
#endif //CONNECTIVITY_PARAMETERCONTINUATION_HXX_INCLUDED

