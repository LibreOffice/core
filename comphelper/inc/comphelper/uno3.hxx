/*************************************************************************
 *
 *  $RCSfile: uno3.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-06 14:01:26 $
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

#ifndef _COMPHELPER_UNO3_HXX_
#define _COMPHELPER_UNO3_HXX_

#ifndef _OSL_INTERLOCK_H_
#include <osl/interlck.h>
#endif

#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_XAGGREGATION_HPP_
#include <com/sun/star/uno/XAggregation.hpp>
#endif

//.........................................................................
namespace comphelper
{
//.........................................................................

    namespace staruno = ::com::sun::star::uno;

//=========================================================================

    /// manipulate ref counts without calling acquire/release
    inline oslInterlockedCount increment(oslInterlockedCount& _counter) { return osl_incrementInterlockedCount(&_counter); }
    inline oslInterlockedCount decrement(oslInterlockedCount& _counter) { return osl_decrementInterlockedCount(&_counter); }

//=========================================================================

    /** used for declaring UNO3-Defaults, i.e. acquire/release
    */
    #define DECLARE_UNO3_DEFAULTS(classname, baseclass) \
        virtual void    SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException) { baseclass::acquire(); } \
        virtual void    SAL_CALL release() throw(::com::sun::star::uno::RuntimeException) { baseclass::release(); } \
        void            SAL_CALL PUT_SEMICOLON_AT_THE_END()

    /** used for declaring UNO3-Defaults, i.e. acquire/release if you want to forward all queryInterfaces to the base class,
        (e.g. if you overload queryAggregation)
    */
    #define DECLARE_UNO3_AGG_DEFAULTS(classname, baseclass) \
        virtual void            SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException) { baseclass::acquire(); } \
        virtual void            SAL_CALL release() throw(::com::sun::star::uno::RuntimeException) { baseclass::release(); } \
        virtual ::com::sun::star::uno::Any  SAL_CALL queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException) \
            { return baseclass::queryInterface(_rType); } \
        void                    SAL_CALL PUT_SEMICOLON_AT_THE_END()

//=========================================================================

    /** ask for an iface of an aggregated object
        usage:<br/>
            Reference<XFoo> xFoo;<br/>
            if (query_aggregation(xAggregatedObject, xFoo))<br/>
                ....
    */
    template <class iface>
    sal_Bool query_aggregation(const staruno::Reference<staruno::XAggregation>& _rxAggregate, staruno::Reference<iface>& _rxOut)
    {
        _rxOut = static_cast<iface*>(NULL);
        if (_rxAggregate.is())
        {
            staruno::Any aCheck = _rxAggregate->queryAggregation(::getCppuType((staruno::Reference<iface>*)NULL));
            if (aCheck.hasValue())
                _rxOut = *(staruno::Reference<iface>*)aCheck.getValue();
        }
        return _rxOut.is();
    }

    /** ask for an iface of an object
        usage:<br/>
            Reference<XFoo> xFoo;<br/>
            if (query_interface(xAnything, xFoo))<br/>
                ....
    */
    template <class iface>
    sal_Bool query_interface(const InterfaceRef& _rxObject, staruno::Reference<iface>& _rxOut)
    {
        _rxOut = static_cast<iface*>(NULL);
        if (_rxObject.is())
        {
            staruno::Any aCheck = _rxObject->queryInterface(::getCppuType((staruno::Reference<iface>*)NULL));
            if(aCheck.hasValue())
            {
                _rxOut = *(staruno::Reference<iface>*)aCheck.getValue();
                return _rxOut.is();
            }
        }
        return sal_False;
    }
    #define FORWARD_DECLARE_INTERFACE(NAME,XFACE) \
        namespace com \
        {   \
            namespace sun \
            {\
                namespace star \
                {\
                    namespace NAME \
                    {\
                        class XFACE; \
                    }\
                }\
            }\
        }\


//.........................................................................
}   // namespace comphelper
//.........................................................................

#endif // _COMPHELPER_UNO3_HXX_

