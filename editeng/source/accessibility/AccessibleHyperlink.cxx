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
#include "precompiled_editeng.hxx"

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <comphelper/accessiblekeybindinghelper.hxx>

#include "AccessibleHyperlink.hxx"
#include "editeng/unoedprx.hxx"
#include <editeng/flditem.hxx>
#include <vcl/keycodes.hxx>

using namespace ::com::sun::star;


//------------------------------------------------------------------------
//
// AccessibleHyperlink implementation
//
//------------------------------------------------------------------------

namespace accessibility
{

    AccessibleHyperlink::AccessibleHyperlink( SvxAccessibleTextAdapter& r, SvxFieldItem* p, sal_uInt16 nP, sal_uInt16 nR, sal_Int32 nStt, sal_Int32 nEnd, const ::rtl::OUString& rD )
    : rTA( r )
    {
        pFld = p;
        nPara = nP;
        nRealIdx = nR;
        nStartIdx = nStt;
        nEndIdx = nEnd;
        aDescription = rD;
    }

    AccessibleHyperlink::~AccessibleHyperlink()
    {
        delete pFld;
    }

    // XAccessibleAction
    sal_Int32 SAL_CALL AccessibleHyperlink::getAccessibleActionCount() throw (uno::RuntimeException)
    {
         return isValid() ? 1 : 0;
    }

    sal_Bool SAL_CALL AccessibleHyperlink::doAccessibleAction( sal_Int32 nIndex  ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        sal_Bool bRet = sal_False;
        if ( isValid() && ( nIndex == 0 ) )
        {
            rTA.FieldClicked( *pFld, nPara, nRealIdx );
            bRet = sal_True;
        }
        return bRet;
    }

    ::rtl::OUString  SAL_CALL AccessibleHyperlink::getAccessibleActionDescription( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        ::rtl::OUString aDesc;

        if ( isValid() && ( nIndex == 0 ) )
            aDesc = aDescription;

        return aDesc;
    }

    uno::Reference< ::com::sun::star::accessibility::XAccessibleKeyBinding > SAL_CALL AccessibleHyperlink::getAccessibleActionKeyBinding( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        uno::Reference< ::com::sun::star::accessibility::XAccessibleKeyBinding > xKeyBinding;

        if( isValid() && ( nIndex == 0 ) )
        {
            ::comphelper::OAccessibleKeyBindingHelper* pKeyBindingHelper = new ::comphelper::OAccessibleKeyBindingHelper();
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
    uno::Any SAL_CALL AccessibleHyperlink::getAccessibleActionAnchor( sal_Int32 /*nIndex*/ ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        return uno::Any();
    }

    uno::Any SAL_CALL AccessibleHyperlink::getAccessibleActionObject( sal_Int32 /*nIndex*/ ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        return uno::Any();
    }

    sal_Int32 SAL_CALL AccessibleHyperlink::getStartIndex() throw (uno::RuntimeException)
    {
        return nStartIdx;
    }

    sal_Int32 SAL_CALL AccessibleHyperlink::getEndIndex() throw (uno::RuntimeException)
    {
        return nEndIdx;
    }

    sal_Bool SAL_CALL AccessibleHyperlink::isValid(  ) throw (uno::RuntimeException)
    {
        return rTA.IsValid();
    }

}  // end of namespace accessibility

//------------------------------------------------------------------------

// MT IA2: Accessiblehyperlink.hxx from IA2 CWS - meanwhile we also introduced one in DEV300 (above)
// Keeping this for reference - we probably should get support for image maps in our implementation...


/*

class SVX_DLLPUBLIC SvxAccessibleHyperlink :
        public ::cppu::WeakImplHelper1<
        ::com::sun::star::accessibility::XAccessibleHyperlink >
{
    SvxURLField* mpField;
    sal_Int32 nStartIdx;
    sal_Int32 nEndIdx;

    ImageMap* mpImageMap;
    SdrObject* m_pShape;
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >  shapeParent;

public:

    SvxAccessibleHyperlink(){};
    //SvxAccessibleHyperlink(::rtl::OUString name, const Imagemap* pImageMap);
    SvxAccessibleHyperlink(const SvxURLField* p, sal_Int32 nStt, sal_Int32 nEnd);
    SvxAccessibleHyperlink(SdrObject* p, ::accessibility::AccessibleShape* pAcc);
    virtual ~SvxAccessibleHyperlink();
    //void setImageMap(ImageMap* pMap);
    //void setXAccessibleImage(::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > parent);
    ::rtl::OUString GetHyperlinkURL(sal_Int32 nIndex) throw (::com::sun::star::lang::IndexOutOfBoundsException);
    sal_Bool IsValidHyperlink();

    // XAccessibleAction
    virtual sal_Int32 SAL_CALL getAccessibleActionCount()
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL doAccessibleAction( sal_Int32 nIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getAccessibleActionDescription(
                sal_Int32 nIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleKeyBinding > SAL_CALL
            getAccessibleActionKeyBinding( sal_Int32 nIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException);

    // XAccessibleHyperlink
    virtual ::com::sun::star::uno::Any SAL_CALL getAccessibleActionAnchor(
                sal_Int32 nIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getAccessibleActionObject(
            sal_Int32 nIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getStartIndex()
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getEndIndex()
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isValid(  )
        throw (::com::sun::star::uno::RuntimeException);
};


SvxAccessibleHyperlink::SvxAccessibleHyperlink( const SvxURLField *p,
                                          sal_Int32 nStt, sal_Int32 nEnd ) :
    nStartIdx( nStt ),
    nEndIdx( nEnd ),
    m_pShape(NULL),
    shapeParent(NULL)
{
    if(p)
        mpField = (SvxURLField*)p->Clone();
    else
        mpField = NULL;
}

SvxAccessibleHyperlink::SvxAccessibleHyperlink(SdrObject* p,
                                            ::accessibility::AccessibleShape* pAcc) :
    nStartIdx( -1 ),
    nEndIdx( -1 ),
    mpField(NULL),
    m_pShape(p)
{
    mpImageMap = m_pShape->GetModel()->GetImageMapForObject(m_pShape);
    shapeParent = dynamic_cast< XAccessible* >(pAcc);
}

SvxAccessibleHyperlink::~SvxAccessibleHyperlink()
{
    if(mpField)
        delete mpField;
}

::rtl::OUString SvxAccessibleHyperlink::GetHyperlinkURL(sal_Int32 nIndex) throw (::com::sun::star::lang::IndexOutOfBoundsException)
{
    if( mpField )
    {
        if (nIndex != 0)
            throw ::com::sun::star::lang::IndexOutOfBoundsException();
        return ::rtl::OUString( mpField->GetURL() );
    }
    else if (mpImageMap)
    {
        if (nIndex < 0 || nIndex >=mpImageMap->GetIMapObjectCount())
            throw IndexOutOfBoundsException();

        IMapObject* pMapObj = mpImageMap->GetIMapObject(sal_uInt16(nIndex));
        if (pMapObj->GetURL().Len())
            return ::rtl::OUString( pMapObj->GetURL() );
    }
    else
    {
        if (nIndex != 0)
            throw ::com::sun::star::lang::IndexOutOfBoundsException();

        SdrUnoObj* pUnoCtrl = dynamic_cast< SdrUnoObj* >( m_pShape );

        if(pUnoCtrl)
        {
            try
            {
                uno::Reference< awt::XControlModel > xControlModel( pUnoCtrl->GetUnoControlModel(), uno::UNO_QUERY_THROW );
                uno::Reference< beans::XPropertySet > xPropSet( xControlModel, uno::UNO_QUERY_THROW );
                uno::Reference< beans::XPropertySetInfo > xPropInfo( xPropSet->getPropertySetInfo(), uno::UNO_QUERY_THROW );

                form::FormButtonType eButtonType = form::FormButtonType_URL;
                const ::rtl::OUString sButtonType( RTL_CONSTASCII_USTRINGPARAM( "ButtonType" ) );
                if(xPropInfo->hasPropertyByName( sButtonType ) && (xPropSet->getPropertyValue( sButtonType ) >>= eButtonType ) )
                {
                    ::rtl::OUString aString;

                    // URL
                    const ::rtl::OUString sTargetURL(RTL_CONSTASCII_USTRINGPARAM( "TargetURL" ));
                    if(xPropInfo->hasPropertyByName(sTargetURL))
                    {
                        if( xPropSet->getPropertyValue(sTargetURL) >>= aString )
                            return aString;
                    }
                }
            }
            catch( uno::Exception& )
            {
            }
        }
        // If hyperlink can't be got from sdrobject, query the corresponding document to retrieve the link info
        uno::Reference< XAccessibleGroupPosition > xGroupPosition (shapeParent, uno::UNO_QUERY);
        if (xGroupPosition.is())
            return xGroupPosition->getObjectLink( uno::makeAny( shapeParent ) );
    }
    return ::rtl::OUString();
}

// Just check whether the first hyperlink is valid
sal_Bool SvxAccessibleHyperlink::IsValidHyperlink()
{
    ::rtl::OUString url = GetHyperlinkURL(0);
    if (url.getLength() > 0)
        return sal_True;
    else
        return sal_False;
}
// XAccessibleAction
sal_Int32 SAL_CALL SvxAccessibleHyperlink::getAccessibleActionCount()
        throw (RuntimeException)
{
    if (mpImageMap)
        return mpImageMap->GetIMapObjectCount();
    else
        return 1;   // only shape link or url field

    //return mpField ? 1 : (mpImageMap ? mpImageMap->GetIMapObjectCount() : 0);
}

sal_Bool SAL_CALL SvxAccessibleHyperlink::doAccessibleAction( sal_Int32 nIndex )
        throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    sal_Bool bRet = sal_False;

    OUString url = GetHyperlinkURL(nIndex);

    if( url.getLength() > 0 )
    {
        SfxStringItem aStrItem(SID_FILE_NAME, url);
        const SfxObjectShell* pDocSh = SfxObjectShell::Current();
        if( pDocSh )
        {
            SfxMedium* pSfxMedium = pDocSh->GetMedium();
            if( pSfxMedium)
            {
                SfxStringItem aReferer(SID_REFERER, pSfxMedium->GetName());
                SfxBoolItem aBrowseItem( SID_BROWSE, TRUE );
                SfxViewFrame* pFrame = SfxViewFrame::Current();
                if( pFrame )
                {
                    pFrame->GetDispatcher()->Execute(SID_OPENDOC, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                                &aStrItem, &aBrowseItem, &aReferer, 0L);
                    bRet = sal_True;
                }
            }
        }
    }

    return bRet;
}

OUString SAL_CALL SvxAccessibleHyperlink::getAccessibleActionDescription(
        sal_Int32 nIndex )
        throw (IndexOutOfBoundsException, RuntimeException)
{
    return GetHyperlinkURL(nIndex);
}

::com::sun::star::uno::Reference< XAccessibleKeyBinding > SAL_CALL
    SvxAccessibleHyperlink::getAccessibleActionKeyBinding( sal_Int32 )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    ::com::sun::star::uno::Reference< XAccessibleKeyBinding > xKeyBinding;

    if( mpField || m_pShape)
    {
        ::comphelper::OAccessibleKeyBindingHelper* pKeyBindingHelper =
            new ::comphelper::OAccessibleKeyBindingHelper();
        xKeyBinding = pKeyBindingHelper;

        ::com::sun::star::awt::KeyStroke aKeyStroke;
        aKeyStroke.Modifiers = 0;
        aKeyStroke.KeyCode = KEY_RETURN;
        aKeyStroke.KeyChar = 0;
        aKeyStroke.KeyFunc = 0;
        pKeyBindingHelper->AddKeyBinding( aKeyStroke );
    }

    return xKeyBinding;
}

// XAccessibleHyperlink
Any SAL_CALL SvxAccessibleHyperlink::getAccessibleActionAnchor(
        sal_Int32 nIndex )
        throw (IndexOutOfBoundsException, RuntimeException)
{
    Any aRet;

    ::rtl::OUString retText;
    if(mpField && nIndex == 0)
    {
        retText = mpField->GetRepresentation();
        aRet <<= retText;
        return aRet;
    }
    else if(mpImageMap)
    {
        IMapObject* pMapObj = mpImageMap->GetIMapObject(sal_uInt16(nIndex));
        if(pMapObj && pMapObj->GetURL().Len())
            aRet <<= shapeParent;
            return aRet;
    }
    else if (nIndex == 0)
    {
        aRet <<= shapeParent;
        return aRet;
    }
    return aRet;
}

Any SAL_CALL SvxAccessibleHyperlink::getAccessibleActionObject(
            sal_Int32 nIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    ::rtl::OUString retText = GetHyperlinkURL(nIndex);
    Any aRet;
    aRet <<= retText;
    return aRet;
}

sal_Int32 SAL_CALL SvxAccessibleHyperlink::getStartIndex()
        throw (RuntimeException)
{
    return nStartIdx;
}

sal_Int32 SAL_CALL SvxAccessibleHyperlink::getEndIndex()
        throw (RuntimeException)
{
    return nEndIdx;
}

sal_Bool SAL_CALL SvxAccessibleHyperlink::isValid(  )
        throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    //return mpField ? sal_True: ( mpImageMap ? sal_True : sal_False );
    if (mpField || m_pShape)
        return sal_True;
    else
        return sal_False;
}

*/



