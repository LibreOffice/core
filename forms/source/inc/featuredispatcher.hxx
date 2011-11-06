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
        virtual void    dispatch( sal_Int16 _nFeatureId ) const = 0;

        /** dispatches a feature, with an additional named parameter

            @param _nFeatureId
                the id of the feature to dispatch

            @param _pParamAsciiName
                the Ascii name of the parameter of the dispatch call

            @param _rParamValue
                the value of the parameter of the dispatch call
        */
        virtual void dispatchWithArgument(
                sal_Int16 _nFeatureId,
                const sal_Char* _pParamName,
                const ::com::sun::star::uno::Any& _rParamValue
             ) const = 0;

        /** checks whether a given feature is enabled
        */
        virtual bool    isEnabled( sal_Int16 _nFeatureId ) const = 0;

        /** returns the boolean state of a feature

            Certain features may support more status information than only the enabled/disabled
            state. The type of such additional information is fixed relative to a given feature, but
            may differ between different features.

            This method allows retrieving status information about features which have an additional
            boolean information associated with it.
        */
        virtual bool    getBooleanState( sal_Int16 _nFeatureId ) const = 0;

        /** returns the string state of a feature

            Certain features may support more status information than only the enabled/disabled
            state. The type of such additional information is fixed relative to a given feature, but
            may differ between different features.

            This method allows retrieving status information about features which have an additional
            string information associated with it.
        */
        virtual ::rtl::OUString getStringState( sal_Int16 _nFeatureId ) const = 0;

        /** returns the integer state of a feature

            Certain features may support more status information than only the enabled/disabled
            state. The type of such additional information is fixed relative to a given feature, but
            may differ between different features.

            This method allows retrieving status information about features which have an additional
            integer information associated with it.
        */
        virtual sal_Int32       getIntegerState( sal_Int16 _nFeatureId ) const = 0;
    };

//.........................................................................
}   // namespace frm
//.........................................................................

#endif // FORMS_SOLAR_DISPATCHER_HXX
