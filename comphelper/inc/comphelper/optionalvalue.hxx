/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: optionalvalue.hxx,v $
 * $Revision: 1.5 $
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

#ifndef _COMPHELPER_OPTIONALVALUE_HXX
#define _COMPHELPER_OPTIONALVALUE_HXX

#include <com/sun/star/uno/Any.hxx>

namespace comphelper
{

/** @deprecated
    Use boost/optional.hpp instead.
*/



    /* Definition of OptionalValue template */

    /** This template provides 'optionality' for the given value type.

        Especially for PODs, optionality either needs to be achieved
        by special 'magic' values (i.e. an int value is not set when
        -1 etc.), or an additional bool denoting value
        validity. This template encapsulates the latter into an atomic
        data type.

        @tpl Element
        The value type that should be made optional
     */
    template< typename Element > class OptionalValue
    {
    public:
        typedef Element ValueType;

        /** Default-construct the value.

            A default-constructed value is not valid. You have to
            explicitely set a value.
         */
        OptionalValue() :
            maValue(),
            mbValid( false )
        {
        }

        /** Construct the value.

            An explicitely constructed value is valid. To create an
            invalid value, you have to default-construct it.
         */
        OptionalValue( const Element& rValue ) :
            maValue( rValue ),
            mbValid( true )
        {
        }

        // default copy/assignment operators are okay here
        //OptionalValue(const OptionalValue&);
        //OptionalValue& operator=( const OptionalValue& );

        /** Query whether the value is valid

            @return true, if this object contains a valid value.
         */
        bool isValid() const
        {
            return mbValid;
        }

        /** Set a value.

            After this call, the object contains a valid value.
         */
        void setValue( const Element& rValue )
        {
            maValue = rValue;
            mbValid = true;
        }

        /** Get the value.

            The return value of this method is undefined, if the
            object does not contain a valid value.
         */
        Element getValue() const
        {
            return maValue;
        }

        /** Clear the value.

            After this call, the object no longer contains a valid
            value.
         */
        void clearValue()
        {
            mbValid = false;
        }

        // NOTE: The following two methods would optimally have been
        // implemented as operator>>=/operator<<=
        // overloads. Unfortunately, there's already a templatized
        // version for those two methods, namely for UNO interface
        // types. Adding a second would lead to ambiguities.

        /** Export the value into an Any.

            This method extracts the value into an Any. If the value
            is invalid, the Any will be cleared.

            @return true, if the value has been successfully
            transferred to the Any. Clearing the Any from an invalid
            object is also considered a successful operation.
         */
        bool exportValue( ::com::sun::star::uno::Any& o_rAny )
        {
            o_rAny.clear();

            if( isValid() )
            {
                if( !(o_rAny <<= getValue()) )
                    return false;
            }

            return true;
        }

        /** Import the value from an Any.

            This method imports the value from an Any. If the Any
            is invalid, the object will get an invalid value.

            @return true, if the value has been successfully
            transferred from the Any. Setting the value to invalid
            from an empty Any is also considered a successful
            operation.
         */
        bool importValue( const ::com::sun::star::uno::Any& rAny )
        {
            clearValue();

            if( rAny.hasValue() )
            {
                Element tmp;

                if( !(rAny >>= tmp) )
                    return false;

                setValue( tmp );
            }

            return true;
        }

    private:
        Element     maValue;
        bool        mbValid;
    };

}

#endif /* _COMPHELPER_OPTIONALVALUE_HXX */
