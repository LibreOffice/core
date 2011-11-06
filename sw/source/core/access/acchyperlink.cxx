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

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using ::rtl::OUString;

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

    const SwTxtAttr *pTxtAttr = GetTxtAttr();
    if( pTxtAttr && 0 == nIndex )
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

    const SwTxtAttr *pTxtAttr = GetTxtAttr();
    if( pTxtAttr && 0 == nIndex )
    {
        const SwFmtINetFmt& rINetFmt = pTxtAttr->GetINetFmt();
        sDesc = OUString( rINetFmt.GetValue() );
    }

    return sDesc;
}

uno::Reference< XAccessibleKeyBinding > SAL_CALL
    SwAccessibleHyperlink::getAccessibleActionKeyBinding( sal_Int32 nIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    uno::Reference< XAccessibleKeyBinding > xKeyBinding;

    if( isValid() && 0==nIndex )
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
        sal_Int32 /*nIndex*/ )
        throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    return uno::Any();
}

uno::Any SAL_CALL SwAccessibleHyperlink::getAccessibleActionObject(
            sal_Int32 /*nIndex*/ )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    return uno::Any();
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
    return xPara.isValid();
}

void SwAccessibleHyperlink::Invalidate()
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    xPara = 0;
}

