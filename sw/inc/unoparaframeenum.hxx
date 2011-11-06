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



#ifndef SW_UNOPARAFRAMEENUM_HXX
#define SW_UNOPARAFRAMEENUM_HXX

#include <deque>

#include <boost/shared_ptr.hpp>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/text/XTextContent.hpp>

#include <cppuhelper/implbase2.hxx>

#include <unobaseclass.hxx>


class SwDepend;
class SwNodeIndex;
class SwPaM;
class SwFrmFmt;


struct FrameDependSortListEntry
{
    xub_StrLen nIndex;
    sal_uInt32 nOrder;
    ::boost::shared_ptr<SwDepend> pFrameDepend;

    FrameDependSortListEntry (xub_StrLen const i_nIndex,
                sal_uInt32 const i_nOrder, SwDepend * const i_pDepend)
        : nIndex(i_nIndex), nOrder(i_nOrder), pFrameDepend(i_pDepend) { }
};

typedef ::std::deque< FrameDependSortListEntry >
    FrameDependSortList_t;

typedef ::std::deque< ::boost::shared_ptr<SwDepend> >
    FrameDependList_t;


// OD 2004-05-07 #i28701# - adjust 4th parameter
void CollectFrameAtNode( SwClient& rClnt, const SwNodeIndex& rIdx,
                         FrameDependSortList_t & rFrames,
                         const bool _bAtCharAnchoredObjs );


/* -----------------23.03.99 12:57-------------------
 *
 * --------------------------------------------------*/

enum ParaFrameMode
{
    PARAFRAME_PORTION_PARAGRAPH,
    PARAFRAME_PORTION_CHAR,
    PARAFRAME_PORTION_TEXTRANGE,
};

typedef ::cppu::WeakImplHelper2
<   ::com::sun::star::lang::XServiceInfo
,   ::com::sun::star::container::XEnumeration
> SwXParaFrameEnumeration_Base;

class SwXParaFrameEnumeration
    : public SwXParaFrameEnumeration_Base
{

private:

    class Impl;
    ::sw::UnoImplPtr<Impl> m_pImpl;

    virtual ~SwXParaFrameEnumeration();

public:

    SwXParaFrameEnumeration(const SwPaM& rPaM,
        const enum ParaFrameMode eParaFrameMode, SwFrmFmt *const pFmt = 0);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService(
            const ::rtl::OUString& rServiceName)
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
        getSupportedServiceNames()
        throw (::com::sun::star::uno::RuntimeException);

    // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement()
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException);

};

#endif // SW_UNOPARAFRAMEENUM_HXX

