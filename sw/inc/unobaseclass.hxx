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


#ifndef SW_UNOBASECLASS_HXX
#define SW_UNOBASECLASS_HXX

#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XEnumeration.hpp>

#include <cppuhelper/implbase2.hxx>


class SfxPoolItem;
class SwClient;
class SwDoc;


typedef ::cppu::WeakImplHelper2
<   ::com::sun::star::lang::XServiceInfo
,   ::com::sun::star::container::XEnumeration
>
SwSimpleEnumeration_Base;


/* -----------------29.04.98 07:35-------------------
 *
 * --------------------------------------------------*/
enum CursorType
{
    CURSOR_INVALID,
    CURSOR_BODY,
    CURSOR_FRAME,
    CURSOR_TBLTEXT,
    CURSOR_FOOTNOTE,
    CURSOR_HEADER,
    CURSOR_FOOTER,
    CURSOR_REDLINE,
    CURSOR_ALL,         // for Search&Replace
    CURSOR_SELECTION,   // create a paragraph enumeration from
                        // a text range or cursor
    CURSOR_SELECTION_IN_TABLE,
    CURSOR_META,         // meta/meta-field
};

/*-----------------04.03.98 11:54-------------------
    Start/EndAction or Start/EndAllAction
  -------------------------------------------------- */
class UnoActionContext
{
    private:
        SwDoc * m_pDoc;

    public:
        UnoActionContext(SwDoc *const pDoc);
        ~UnoActionContext();

        void InvalidateDocument() { m_pDoc = 0; }
};

/* -----------------07.07.98 12:03-------------------
    interrupt Actions for a little while
   -------------------------------------------------- */
class UnoActionRemoveContext
{
    private:
        SwDoc *const m_pDoc;

    public:
        UnoActionRemoveContext(SwDoc *const pDoc);
        ~UnoActionRemoveContext();
};


::com::sun::star::uno::Sequence< sal_Int8 > CreateUnoTunnelId();

/// helper function for implementing SwClient::Modify
void ClientModify(SwClient* pClient, const SfxPoolItem *pOld, const SfxPoolItem *pNew);


#include <boost/utility.hpp>
#include <osl/diagnose.h>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>

namespace sw {

    template<typename T> class UnoImplPtr
        : private ::boost::noncopyable
    {
        private:
            T * m_p;

        public:
            UnoImplPtr(T *const i_p)
                : m_p(i_p)
            {
                OSL_ENSURE(i_p, "UnoImplPtr: null");
            }

            ~UnoImplPtr()
            {
                ::vos::OGuard g(Application::GetSolarMutex());
                delete m_p; // #i105557#: call dtor with locked solar mutex
                m_p = 0;
            }

            T & operator * () const { return *m_p; }

            T * operator ->() const { return  m_p; }

            T * get        () const { return  m_p; }
    };

    template< class C > C *
    UnoTunnelGetImplementation(
            ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XUnoTunnel > const & xUnoTunnel)
    {
        if (!xUnoTunnel.is()) { return 0; }
        C *const pC( reinterpret_cast< C* >(
                        ::sal::static_int_cast< sal_IntPtr >(
                            xUnoTunnel->getSomething(C::getUnoTunnelId()))));
        return pC;
    }

    template< class C > sal_Int64
    UnoTunnelImpl(const ::com::sun::star::uno::Sequence< sal_Int8 > & rId,
                  C *const pThis)
    {
        if ((rId.getLength() == 16) &&
            (0 == rtl_compareMemory(C::getUnoTunnelId().getConstArray(),
                                    rId.getConstArray(), 16)))
        {
            return ::sal::static_int_cast< sal_Int64 >(
                    reinterpret_cast< sal_IntPtr >(pThis) );
        }
        return 0;
    }

    ::com::sun::star::uno::Sequence< ::rtl::OUString >
    GetSupportedServiceNamesImpl(
            size_t const nServices, char const*const pServices[]);
    sal_Bool SupportsServiceImpl(
            size_t const nServices, char const*const pServices[],
            ::rtl::OUString const & rServiceName);

} // namespace sw

#endif // SW_UNOBASECLASS_HXX

