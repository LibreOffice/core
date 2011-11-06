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



#ifndef _COMPHELPER_PROPERTSETINFO_HXX_
#define _COMPHELPER_PROPERTSETINFO_HXX_

#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/stl_types.hxx>
#include "comphelper/comphelperdllapi.h"

//=========================================================================
//= property helper classes
//=========================================================================

//... namespace comphelper .......................................................
namespace comphelper
{
//.........................................................................

struct PropertyMapEntry
{
    const sal_Char* mpName;
    sal_uInt16 mnNameLen;
    sal_Int32 mnHandle;
    const com::sun::star::uno::Type* mpType;
    sal_Int16 mnAttributes;
    sal_uInt8 mnMemberId;
};

DECLARE_STL_USTRINGACCESS_MAP( PropertyMapEntry*, PropertyMap );

class PropertyMapImpl;

/** this class implements a XPropertySetInfo that is initialized with arrays of PropertyMapEntry.
    It is used by the class PropertySetHelper.
*/
class COMPHELPER_DLLPUBLIC PropertySetInfo : public ::cppu::WeakImplHelper1< ::com::sun::star::beans::XPropertySetInfo >
{
private:
    PropertyMapImpl* mpMap;
public:
    PropertySetInfo() throw();
    PropertySetInfo( PropertyMapEntry* pMap ) throw();
    virtual ~PropertySetInfo() throw();

    /** returns a stl map with all PropertyMapEntry pointer.<p>
        The key is the property name.
    */
    const PropertyMap* getPropertyMap() const throw();

    /** adds an array of PropertyMapEntry to this instance.<p>
        The end is marked with a PropertyMapEntry where mpName equals NULL</p>
    */
    void add( PropertyMapEntry* pMap ) throw();

    /** adds an array of PropertyMapEntry to this instance

        <p>At most the number of entries given will be added, if no terminating entry (<code>mpName == <NULL/></code>) is encountered.</p>

        <p>If <arg>nCount</arg> is less than 0, it is ignored and all entries (up to, but not including, the terminating
        one) are added.</p>
    */
    void add( PropertyMapEntry* pMap, sal_Int32 nCount ) throw();

    /** removes an already added PropertyMapEntry which string in mpName equals to aName */
    void remove( const rtl::OUString& aName ) throw();

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > SAL_CALL getProperties() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::beans::Property SAL_CALL getPropertyByName( const ::rtl::OUString& aName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasPropertyByName( const ::rtl::OUString& Name ) throw(::com::sun::star::uno::RuntimeException);
};

//.........................................................................
}
//... namespace comphelper .......................................................

#endif // _UTL_PROPERTSETINFO_HXX_

