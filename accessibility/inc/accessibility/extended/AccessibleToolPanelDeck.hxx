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



#ifndef ACC_ACCESSIBLETOOLPANELDECK_HXX
#define ACC_ACCESSIBLETOOLPANELDECK_HXX

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <cppuhelper/implbase1.hxx>
#include <toolkit/awt/vclxaccessiblecomponent.hxx>

#include <boost/scoped_ptr.hpp>

namespace svt
{
    class ToolPanelDeck;
}

//......................................................................................................................
namespace accessibility
{
//......................................................................................................................

    //==================================================================================================================
    //= AccessibleToolPanelDeck
    //==================================================================================================================
    class AccessibleToolPanelDeck_Impl;
    typedef VCLXAccessibleComponent   AccessibleToolPanelDeck_Base;
    class AccessibleToolPanelDeck : public AccessibleToolPanelDeck_Base
    {
    public:
        AccessibleToolPanelDeck(
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& i_rAccessibleParent,
            ::svt::ToolPanelDeck& i_rPanelDeck
        );

        using AccessibleToolPanelDeck_Base::NotifyAccessibleEvent;

    protected:
        virtual ~AccessibleToolPanelDeck();

        // XAccessibleContext
        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException);

        // XAccessibleComponent
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL grabFocus(  ) throw (::com::sun::star::uno::RuntimeException);

        // OComponentHelper
        virtual void SAL_CALL disposing();

        // VCLXAccessibleComponent
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > GetChildAccessible( const VclWindowEvent& i_rVclWindowEvent );
        virtual void FillAccessibleStateSet( ::utl::AccessibleStateSetHelper& i_rStateSet );

    private:
        ::boost::scoped_ptr< AccessibleToolPanelDeck_Impl > m_pImpl;
    };

//......................................................................................................................
} // namespace accessibility
//......................................................................................................................

#endif // ACC_ACCESSIBLETOOLPANELDECK_HXX
