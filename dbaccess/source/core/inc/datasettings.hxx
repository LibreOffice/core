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



#ifndef _DBA_CORE_DATASETTINGS_HXX_
#define _DBA_CORE_DATASETTINGS_HXX_

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTDESCRIPTOR_HPP_
#include <com/sun/star/awt/FontDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef COMPHELPER_PROPERTYSTATECONTAINER_HXX
#include <comphelper/propertystatecontainer.hxx>
#endif

//........................................................................
namespace dbaccess
{
//........................................................................

//==========================================================================
//= ODataSettings_Base - a base class which implements the property member
//=                 for an object implementing the sdb::DataSettings
//=                 service
//= the properties have to to be registered when used
//==========================================================================
class ODataSettings_Base
{
public:
// <properties>
    ::rtl::OUString                             m_sFilter;
    ::rtl::OUString                             m_sHavingClause;
    ::rtl::OUString                             m_sGroupBy;
    ::rtl::OUString                             m_sOrder;
    sal_Bool                                    m_bApplyFilter;     // no BitField ! the base class needs a pointer to this member !
    ::com::sun::star::awt::FontDescriptor       m_aFont;
    ::com::sun::star::uno::Any                  m_aRowHeight;
    ::com::sun::star::uno::Any                  m_aTextColor;
    ::com::sun::star::uno::Any                  m_aTextLineColor;
    sal_Int16                                   m_nFontEmphasis;
    sal_Int16                                   m_nFontRelief;
// </properties>

protected:
    ODataSettings_Base();
    ODataSettings_Base(const ODataSettings_Base& _rSource);
    ~ODataSettings_Base();
};
//==========================================================================
//= ODataSettings - a base class which implements the property handling
//=                 for an object implementing the sdb::DataSettings
//=                 service
//==========================================================================

class ODataSettings : public ::comphelper::OPropertyStateContainer
                    , public ODataSettings_Base
{
    sal_Bool m_bQuery;
protected:
    ODataSettings(::cppu::OBroadcastHelper& _rBHelper,sal_Bool _bQuery = sal_False);
    virtual void getPropertyDefaultByHandle( sal_Int32 _nHandle, ::com::sun::star::uno::Any& _rDefault ) const;

    /** register the properties from the param given. The parameter instance must be alive as long as tis object live.
        @param  _pItem
            The database settings, can be <br>this</br>
    */
    void registerPropertiesFor(ODataSettings_Base* _pItem);
};

//........................................................................
}   // namespace dbaccess
//........................................................................

#endif // _DBA_CORE_DATASETTINGS_HXX_

