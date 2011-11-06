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


#ifndef DBACCESS_JACCESS_HXX
#define DBACCESS_JACCESS_HXX

#ifndef _TOOLKIT_AWT_VCLXACCESSIBLECOMPONENT_HXX_
#include <toolkit/awt/vclxaccessiblecomponent.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

namespace dbaui
{
    class OJoinTableView;
    typedef ::cppu::ImplHelper1< ::com::sun::star::accessibility::XAccessible
                                            > OJoinDesignViewAccess_BASE;
    /** the class OJoinDesignViewAccess represents the accessible object for join views
        like the QueryDesign and the RelationDesign
    */
    class OJoinDesignViewAccess     :   public VCLXAccessibleComponent, public OJoinDesignViewAccess_BASE
    {
        OJoinTableView* m_pTableView; // the window which I should give accessibility to

    protected:
        /** isEditable returns the current editable state
            @return true if the controller is not readonly otherwise false
        */
        virtual sal_Bool isEditable() const;
    public:
        /** OJoinDesignViewAccess needs a valid view
        */
        OJoinDesignViewAccess(  OJoinTableView* _pTableView);

        // XInterface
        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )

        // XServiceInfo - static methods
        static ::rtl::OUString getImplementationName_Static(void) throw( com::sun::star::uno::RuntimeException );

        virtual ::rtl::OUString SAL_CALL getImplementationName() throw(com::sun::star::uno::RuntimeException);

        // XAccessible
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException);

        // XAccessibleContext
        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException,::com::sun::star::uno::RuntimeException);
        virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException);

        OJoinTableView* getTableView() const { return m_pTableView; }

        void notifyAccessibleEvent(
                    const sal_Int16 _nEventId,
                    const ::com::sun::star::uno::Any& _rOldValue,
                    const ::com::sun::star::uno::Any& _rNewValue
                )
        {
            NotifyAccessibleEvent(_nEventId,_rOldValue,_rNewValue);
        }

        void clearTableView();
    };
}
#endif // DBACCESS_JACCESS_HXX
