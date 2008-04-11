/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: IdPropArrayHelper.hxx,v $
 * $Revision: 1.10 $
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
#ifndef COMPHELPER_IDPROPERTYARRAYUSAGEHELPER_HXX
#define COMPHELPER_IDPROPERTYARRAYUSAGEHELPER_HXX

#include <cppuhelper/component.hxx>
#include <osl/mutex.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <osl/diagnose.h>
#include <comphelper/stl_types.hxx>
#include <rtl/instance.hxx>
#include <cppuhelper/propshlp.hxx>

namespace cppu { class IPropertyArrayHelper; }

namespace comphelper
{
    //************************************************************
    //  OIdPropertyArrayUsageHelper
    //************************************************************
    template <typename TYPE> struct OIdPropertyArrayUsageHelperMutex
            : public rtl::Static< ::osl::Mutex, OIdPropertyArrayUsageHelperMutex<TYPE> > {};

    typedef std::map< sal_Int32, ::cppu::IPropertyArrayHelper*, std::less< sal_Int32 > > OIdPropertyArrayMap;
    template <class TYPE>
    class OIdPropertyArrayUsageHelper
    {
    protected:
        static sal_Int32                        s_nRefCount;
        static OIdPropertyArrayMap*             s_pMap;

    public:
        OIdPropertyArrayUsageHelper();
        virtual ~OIdPropertyArrayUsageHelper()
        {
            ::osl::MutexGuard aGuard(OIdPropertyArrayUsageHelperMutex<TYPE>::get());
            OSL_ENSURE(s_nRefCount > 0, "OIdPropertyArrayUsageHelper::~OIdPropertyArrayUsageHelper : suspicious call : have a refcount of 0 !");
            if (!--s_nRefCount)
            {
                // delete the element
                for (OIdPropertyArrayMap::iterator i = s_pMap->begin(); i != s_pMap->end(); ++i)
                    delete (*i).second;
                delete s_pMap;
                s_pMap = NULL;
            }
        }

        /** call this in the getInfoHelper method of your derived class. The method returns the array helper of the
            class, which is created if neccessary.
        */
        ::cppu::IPropertyArrayHelper* getArrayHelper(sal_Int32 nId);

    protected:
        /** used to implement the creation of the array helper which is shared amongst all instances of the class.
            This method needs to be implemented in derived classes.
            <BR>
            The method gets called with Mutex acquired.
            <BR>
            as long as IPropertyArrayHelper has no virtual destructor, the implementation of ~OPropertyArrayUsageHelper
            assumes that you created an ::cppu::OPropertyArrayHelper when deleting s_pProps.
            @return                         an pointer to the newly created array helper. Must not be NULL.
        */
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper(sal_Int32 nId) const = 0;
    };

    //------------------------------------------------------------------
    template<class TYPE>
    sal_Int32                       OIdPropertyArrayUsageHelper< TYPE >::s_nRefCount    = 0;

    template<class TYPE>
    OIdPropertyArrayMap*            OIdPropertyArrayUsageHelper< TYPE >::s_pMap = NULL;

    //------------------------------------------------------------------
    template <class TYPE>
    OIdPropertyArrayUsageHelper<TYPE>::OIdPropertyArrayUsageHelper()
    {
        ::osl::MutexGuard aGuard(OIdPropertyArrayUsageHelperMutex<TYPE>::get());
        // create the map if necessary
        if (s_pMap == NULL)
            s_pMap = new OIdPropertyArrayMap();
        ++s_nRefCount;
    }

    //------------------------------------------------------------------
    template <class TYPE>
    ::cppu::IPropertyArrayHelper* OIdPropertyArrayUsageHelper<TYPE>::getArrayHelper(sal_Int32 nId)
    {
        OSL_ENSURE(s_nRefCount, "OIdPropertyArrayUsageHelper::getArrayHelper : suspicious call : have a refcount of 0 !");
        ::osl::MutexGuard aGuard(OIdPropertyArrayUsageHelperMutex<TYPE>::get());
        // do we have the array already?
        if (! (*s_pMap)[nId] )
        {
            (*s_pMap)[nId] = createArrayHelper(nId);
            OSL_ENSURE((*s_pMap)[nId], "OIdPropertyArrayUsageHelper::getArrayHelper : createArrayHelper returned nonsense !");
        }
        return (*s_pMap)[nId];
    }
}
#endif // COMPHELPER_IDPROPERTYARRAYUSAGEHELPER_HXX
