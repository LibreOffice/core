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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#include <comphelper/accessiblekeybindinghelper.hxx>
#include <swurl.hxx>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <ndtxt.hxx>
#include <txtinet.hxx>
#include <accpara.hxx>
#include <acchyperlink.hxx>

//IAccessibility2 Implementation 2009-----
#include <comphelper/processfactory.hxx>
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XLINKTARGETSUPPLIER_HPP_
#include <com/sun/star/document/XLinkTargetSupplier.hpp>
#endif
//-----IAccessibility2 Implementation 2009

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using ::rtl::OUString;
using ::com::sun::star::lang::IndexOutOfBoundsException;

SwAccessibleHyperlink::SwAccessibleHyperlink( sal_uInt16 nHPos,
    SwAccessibleParagraph *p, sal_Int32 nStt, sal_Int32 nEnd ) :
    nHintPos( nHPos ),
    xPara( p ),
    nStartIdx( nStt ),
    nEndIdx( nEnd )
{
}

const SwTxtAttr *SwAccessibleHyperlink::GetTxtAttr() const
{
    const SwTxtAttr *pTxtAttr = 0;
    if( xPara.isValid() && xPara->GetMap() )
    {
        const SwTxtNode *pTxtNd = xPara->GetTxtNode();
        const SwpHints *pHints = pTxtNd->GetpSwpHints();
        if( pHints && nHintPos < pHints->Count() )
        {
            const SwTxtAttr *pHt = (*pHints)[nHintPos];
            if( RES_TXTATR_INETFMT == pHt->Which() )
                pTxtAttr = pHt;
        }
    }

    return pTxtAttr;
}


// XAccessibleAction
sal_Int32 SAL_CALL SwAccessibleHyperlink::getAccessibleActionCount()
        throw (uno::RuntimeException)
{
     return isValid() ? 1 : 0;
}

sal_Bool SAL_CALL SwAccessibleHyperlink::doAccessibleAction( sal_Int32 nIndex )
        throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    sal_Bool bRet = sal_False;

    //IAccessibility2 Implementation 2009-----
    if(nIndex != 0)
        throw new IndexOutOfBoundsException;
    const SwTxtAttr *pTxtAttr = GetTxtAttr();
    if( pTxtAttr /*&& 0 == nIndex*/ )
    //-----IAccessibility2 Implementation 2009
    {
        const SwFmtINetFmt& rINetFmt = pTxtAttr->GetINetFmt();
        if( rINetFmt.GetValue().Len() )
        {
            ViewShell *pVSh = xPara->GetShell();
            if( pVSh )
            {
                LoadURL( rINetFmt.GetValue(), pVSh, URLLOAD_NOFILTER,
                         &rINetFmt.GetTargetFrame() );
                ASSERT( pTxtAttr == rINetFmt.GetTxtINetFmt(),
                         "lost my txt attr" );
                const SwTxtINetFmt* pTxtAttr2 = rINetFmt.GetTxtINetFmt();
                if( pTxtAttr2 )
                {
                    const_cast<SwTxtINetFmt*>(pTxtAttr2)->SetVisited(true);
                    const_cast<SwTxtINetFmt*>(pTxtAttr2)->SetVisitedValid(true);
                }
                bRet = sal_True;
            }
        }
    }

    return bRet;
}

OUString SAL_CALL SwAccessibleHyperlink::getAccessibleActionDescription(
        sal_Int32 nIndex )
        throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    OUString sDesc;

    //IAccessibility2 Implementation 2009-----
    if(nIndex != 0)
        throw new IndexOutOfBoundsException;
    const SwTxtAttr *pTxtAttr = GetTxtAttr();
    if( pTxtAttr /*&& 0 == nIndex*/ )
    {
        const SwFmtINetFmt& rINetFmt = pTxtAttr->GetINetFmt();
        sDesc = OUString( rINetFmt.GetValue() );
    }
    //-----IAccessibility2 Implementation 2009
    return sDesc;
}

uno::Reference< XAccessibleKeyBinding > SAL_CALL
    SwAccessibleHyperlink::getAccessibleActionKeyBinding( sal_Int32 )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    uno::Reference< XAccessibleKeyBinding > xKeyBinding;

    //IAccessibility2 Implementation 2009-----
    if( isValid() /*&& 0 == nIndex*/ )
    //-----IAccessibility2 Implementation 2009
    {
        ::comphelper::OAccessibleKeyBindingHelper* pKeyBindingHelper =
               new ::comphelper::OAccessibleKeyBindingHelper();
        xKeyBinding = pKeyBindingHelper;

        awt::KeyStroke aKeyStroke;
        aKeyStroke.Modifiers = 0;
        aKeyStroke.KeyCode = KEY_RETURN;
        aKeyStroke.KeyChar = 0;
        aKeyStroke.KeyFunc = 0;
        pKeyBindingHelper->AddKeyBinding( aKeyStroke );
    }

    return xKeyBinding;
}

// XAccessibleHyperlink
uno::Any SAL_CALL SwAccessibleHyperlink::getAccessibleActionAnchor(
        sal_Int32 nIndex)
        throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    uno::Any aRet;
    //IAccessibility2 Implementation 2009-----
    if(nIndex != 0)
        throw new IndexOutOfBoundsException;
    //End Added.
    ::rtl::OUString text = OUString( xPara->GetString() );
    ::rtl::OUString retText =  text.copy(nStartIdx, nEndIdx - nStartIdx);
    aRet <<= retText;
    //-----IAccessibility2 Implementation 2009
    return aRet;
}

uno::Any SAL_CALL SwAccessibleHyperlink::getAccessibleActionObject(
            sal_Int32 nIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    //IAccessibility2 Implementation 2009-----
    if(nIndex != 0)
        throw new IndexOutOfBoundsException;
    //End Added.
    const SwTxtAttr *pTxtAttr = GetTxtAttr();
    ::rtl::OUString retText;
    if( pTxtAttr /*&& 0 == nIndex*/ )
    {
        const SwFmtINetFmt& rINetFmt = pTxtAttr->GetINetFmt();
        retText = OUString( rINetFmt.GetValue() );
    }
    uno::Any aRet;
    aRet <<= retText;
    return aRet;
    //-----IAccessibility2 Implementation 2009
}

sal_Int32 SAL_CALL SwAccessibleHyperlink::getStartIndex()
        throw (uno::RuntimeException)
{
    return nStartIdx;
}

sal_Int32 SAL_CALL SwAccessibleHyperlink::getEndIndex()
        throw (uno::RuntimeException)
{
    return nEndIdx;
}

sal_Bool SAL_CALL SwAccessibleHyperlink::isValid(  )
        throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    //IAccessibility2 Implementation 2009-----
    //  return xPara.isValid();
    if (xPara.isValid())
    {
        const SwTxtAttr *pTxtAttr = GetTxtAttr();
        ::rtl::OUString sText;
        if( pTxtAttr )
        {
            const SwFmtINetFmt& rINetFmt = pTxtAttr->GetINetFmt();
            sText = OUString( rINetFmt.GetValue() );
            ::rtl::OUString sToken = ::rtl::OUString::createFromAscii("#");
            sal_Int32 nPos = sText.indexOf(sToken);
            if (nPos==0)//document link
            {
                uno::Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
                if( ! xFactory.is() )
                    return sal_False;
                uno::Reference< com::sun::star::frame::XDesktop > xDesktop( xFactory->createInstance( OUString::createFromAscii( "com.sun.star.frame.Desktop" ) ),
                    uno::UNO_QUERY );
                if( !xDesktop.is() )
                    return sal_False;
                uno::Reference< lang::XComponent > xComp;
                xComp = xDesktop->getCurrentComponent();
                if( !xComp.is() )
                    return sal_False;
                uno::Reference< com::sun::star::document::XLinkTargetSupplier >  xLTS(xComp, uno::UNO_QUERY);
                if ( !xLTS.is())
                    return sal_False;

                uno::Reference< ::com::sun::star::container::XNameAccess > xLinks = xLTS->getLinks();
                uno::Reference< ::com::sun::star::container::XNameAccess > xSubLinks;
                const uno::Sequence< OUString > aNames( xLinks->getElementNames() );
                const sal_uLong nLinks = aNames.getLength();
                const OUString* pNames = aNames.getConstArray();

                for( sal_uLong i = 0; i < nLinks; i++ )
                {
                    uno::Any aAny;
                    OUString aLink( *pNames++ );
                    aAny = xLinks->getByName( aLink );
                    aAny >>= xSubLinks;
                    if (xSubLinks->hasByName(sText.copy(1)) )
                        return sal_True;
                }
            }
            else//internet
                return sal_True;
        }
    }//xpara valid
    return sal_False;
    //-----IAccessibility2 Implementation 2009
}

void SwAccessibleHyperlink::Invalidate()
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    xPara = 0;
}

