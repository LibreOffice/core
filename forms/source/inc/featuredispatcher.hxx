/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: featuredispatcher.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 22:55:57 $
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

#ifndef FORMS_SOLAR_DISPATCHER_HXX
#define FORMS_SOLAR_DISPATCHER_HXX

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

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
