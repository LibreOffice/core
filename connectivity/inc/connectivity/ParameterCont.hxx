/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ParameterCont.hxx,v $
 * $Revision: 1.3 $
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

