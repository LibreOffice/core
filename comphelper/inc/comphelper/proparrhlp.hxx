/*************************************************************************
 *
 *  $RCSfile: proparrhlp.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-22 13:28:36 $
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

#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#define _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_

#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_AGGREGATION_HXX_
#include <comphelper/propagg.hxx>
#endif
#ifndef _CPPUHELPER_PROPSHLP_HXX
#include <cppuhelper/propshlp.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

namespace cppu {
    class IPropertyArrayHelper;
}

//... namespace comphelper ................................................
namespace comphelper
{
//.........................................................................

    namespace staruno   = ::com::sun::star::uno;
    namespace starbeans = ::com::sun::star::beans;


//==================================================================
template <class TYPE>
class OPropertyArrayUsageHelper
{
protected:
    static sal_Int32                        s_nRefCount;
    static ::cppu::IPropertyArrayHelper*    s_pProps;
    static ::osl::Mutex                     s_aMutex;

public:
    OPropertyArrayUsageHelper();
    virtual ~OPropertyArrayUsageHelper()
    {   // ARGHHHHHHH ..... would like to implement this in proparrhlp_impl.hxx (as we do with all other methods)
        // but SUNPRO 5 compiler (linker) doesn't like this
        ::osl::MutexGuard aGuard(s_aMutex);
        OSL_ENSURE(s_nRefCount > 0, "OPropertyArrayUsageHelper::~OPropertyArrayUsageHelper : suspicious call : have a refcount of 0 !");
        if (!--s_nRefCount)
        {
            delete s_pProps;
            s_pProps = NULL;
        }
    }

    /** call this in the getInfoHelper method of your derived class. The method returns the array helper of the
        class, which is created if neccessary.
    */
    ::cppu::IPropertyArrayHelper*   getArrayHelper();

protected:
    /** used to implement the creation of the array helper which is shared amongst all instances of the class.
        This method needs to be implemented in derived classes.
        <BR>
        The method gets called with s_aMutex acquired.
        <BR>
        as long as IPropertyArrayHelper has no virtual destructor, the implementation of ~OPropertyArrayUsageHelper
        assumes that you created an ::cppu::OPropertyArrayHelper when deleting s_pProps.
        @return                         an pointer to the newly created array helper. Must not be NULL.
    */
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const = 0;
};

//==================================================================
/** a OPropertyArrayUsageHelper which will create an OPropertyArrayAggregationHelper
*/
template <class TYPE>
class OAggregationArrayUsageHelper: public OPropertyArrayUsageHelper<TYPE>
{
protected:
    /** overwrite this in your derived class. initialize the two sequences with your and your aggregate's
        properties.
        <BR>
        The method gets called with s_aMutex acquired.
        @param      _rProps             out parameter to be filled with the property descriptions of your own class
        @param      _rAggregateProps    out parameter to be filled with the properties of your aggregate.
    */
    virtual void fillProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps,
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rAggregateProps
        ) const = 0;

    /** creates an OPropertyArrayAggregationHelper filled with properties for which's initialization
        fillProperties is called. getInfoService and getFirstAggregateId may be overwritten to determine
        the additional parameters of the OPropertyArrayAggregationHelper.
    */
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

    /** the return value is used for the construction of the OPropertyArrayAggregationHelper.
        Beware of the lifetime of the returned object, as it has to exist 'til the last instance
        of this class dies.
    */
    virtual IPropertyInfoService* getInfoService() const { return NULL; }

    /** the return value is used for the construction of the OPropertyArrayAggregationHelper.
    */
    virtual sal_Int32 getFirstAggregateId() const { return DEFAULT_AGGREGATE_PROPERTY_ID; }
};


#include <comphelper/proparrhlp_impl.hxx>

//.........................................................................
}
//... namespace comphelper ................................................

#endif _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_


