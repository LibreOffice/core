/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: featuredispatcher.hxx,v $
 * $Revision: 1.4 $
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

#ifndef FORMS_SOLAR_DISPATCHER_HXX
#define FORMS_SOLAR_DISPATCHER_HXX

#include <rtl/ustring.hxx>

//.........................................................................
namespace frm
{
//.........................................................................

    //=========================================================================
    //= IFeatureDispatcher
    //=========================================================================
    class IFeatureDispatcher
    {
    public:
        /** dispatches a feature

            @param _nFeatureId
                the id of the feature to dispatch
        */
        virtual void    dispatch( sal_Int32 _nFeatureId ) const = 0;

        /** dispatches a feature, with an additional named parameter

            @param _nFeatureId
                the id of the feature to dispatch

            @param _pParamAsciiName
                the Ascii name of the parameter of the dispatch call

            @param _rParamValue
                the value of the parameter of the dispatch call
        */
        virtual void dispatchWithArgument(
                sal_Int32 _nFeatureId,
                const sal_Char* _pParamName,
                const ::com::sun::star::uno::Any& _rParamValue
             ) const = 0;

        /** checks whether a given feature is enabled
        */
        virtual bool    isEnabled( sal_Int32 _nFeatureId ) const = 0;

        /** returns the boolean state of a feature

            Certain features may support more status information than only the enabled/disabled
            state. The type of such additional information is fixed relative to a given feature, but
            may differ between different features.

            This method allows retrieving status information about features which have an additional
            boolean information associated with it.
        */
        virtual bool    getBooleanState( sal_Int32 _nFeatureId ) const = 0;

        /** returns the string state of a feature

            Certain features may support more status information than only the enabled/disabled
            state. The type of such additional information is fixed relative to a given feature, but
            may differ between different features.

            This method allows retrieving status information about features which have an additional
            string information associated with it.
        */
        virtual ::rtl::OUString getStringState( sal_Int32 _nFeatureId ) const = 0;

        /** returns the integer state of a feature

            Certain features may support more status information than only the enabled/disabled
            state. The type of such additional information is fixed relative to a given feature, but
            may differ between different features.

            This method allows retrieving status information about features which have an additional
            integer information associated with it.
        */
        virtual sal_Int32       getIntegerState( sal_Int32 _nFeatureId ) const = 0;
    };

//.........................................................................
}   // namespace frm
//.........................................................................

#endif // FORMS_SOLAR_DISPATCHER_HXX
