/*************************************************************************
 *
 *  $RCSfile: unocpres.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:42 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#include <cppuhelper/extract.hxx>

#include "unohelp.hxx"
#include "unomodel.hxx"
#include "drawdoc.hxx"
#include "unocpres.hxx"
#include "cusshow.hxx"
#include "unopage.hxx"

using namespace ::rtl;
using namespace ::vos;
using namespace ::com::sun::star;

SdXCustomPresentation::SdXCustomPresentation() throw()
:   mpSdCustomShow(NULL), mpModel(NULL)
{
}

SdXCustomPresentation::SdXCustomPresentation( SdCustomShow* pShow, SdXImpressDocument* pMyModel) throw()
:   mpSdCustomShow(pShow), mpModel(pMyModel)
{
}

SdXCustomPresentation::~SdXCustomPresentation() throw()
{
}

UNO3_GETIMPLEMENTATION_IMPL( SdXCustomPresentation );

// XServiceInfo
OUString SAL_CALL SdXCustomPresentation::getImplementationName()
    throw(uno::RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("SdXCustomPresentation") );
}

sal_Bool SAL_CALL SdXCustomPresentation::supportsService( const OUString& ServiceName )
    throw(uno::RuntimeException)
{
    return SvxServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames() );
}

uno::Sequence< OUString > SAL_CALL SdXCustomPresentation::getSupportedServiceNames()
    throw(uno::RuntimeException)
{
    OUString aSN( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.CustomPresentation") );
    uno::Sequence< OUString > aSeq( &aSN, 1 );
    return aSeq;
}

// XIndexContainer
void SAL_CALL SdXCustomPresentation::insertByIndex( sal_Int32 Index, const uno::Any& Element )
    throw(lang::IllegalArgumentException, lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( Index < 0 || Index > mpSdCustomShow->Count() )
        throw lang::IndexOutOfBoundsException();

    uno::Reference< drawing::XDrawPage > xPage;
    if(!::cppu::extractInterface(xPage, Element ))
        throw lang::IllegalArgumentException();

    SdDrawPage* pPage = SdDrawPage::getImplementation( xPage );

    if(pPage)
    {
        if( NULL == mpModel )
            mpModel = pPage->GetModel();

        if( NULL != mpModel && NULL == mpSdCustomShow && mpModel->GetDoc() )
            mpSdCustomShow = new SdCustomShow( mpModel->GetDoc() );

        mpSdCustomShow->Insert(pPage->GetSdrPage(), Index);
    }

    if( mpModel )
        mpModel->SetModified();
}

void SAL_CALL SdXCustomPresentation::removeByIndex( sal_Int32 Index )
    throw(lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(mpSdCustomShow)
    {
        uno::Any aPage( getByIndex( Index ) );

        uno::Reference< drawing::XDrawPage > xPage( *(uno::Reference< drawing::XDrawPage > *)aPage.getValue() );
        SvxDrawPage* pPage = SvxDrawPage::getImplementation(  xPage );
        if(pPage)
            mpSdCustomShow->Remove(pPage->GetSdrPage());
    }

    if( mpModel )
        mpModel->SetModified();
}

// XIndexReplace
void SAL_CALL SdXCustomPresentation::replaceByIndex( sal_Int32 Index, const uno::Any& Element )
    throw(lang::IllegalArgumentException, lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    removeByIndex( Index );
    insertByIndex( Index, Element );
}

// XElementAccess
uno::Type SAL_CALL SdXCustomPresentation::getElementType()
    throw(uno::RuntimeException)
{
    return ITYPE( drawing::XDrawPage );
}

sal_Bool SAL_CALL SdXCustomPresentation::hasElements()
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );
    return getCount() > 0;
}

// XIndexAccess
sal_Int32 SAL_CALL SdXCustomPresentation::getCount()
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );
    return mpSdCustomShow?mpSdCustomShow->Count():0;
}

uno::Any SAL_CALL SdXCustomPresentation::getByIndex( sal_Int32 Index )
    throw(lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( Index < 0 || Index >= mpSdCustomShow->Count() )
        throw lang::IndexOutOfBoundsException();

    uno::Any aAny;
    if(mpSdCustomShow && mpModel )
    {
        uno::Reference< drawing::XDrawPage > xRef( mpModel->CreateXDrawPage((SdPage*)mpSdCustomShow->GetObject(Index)));

        aAny <<= xRef;
    }

    return aAny;
}

// XNamed
OUString SAL_CALL SdXCustomPresentation::getName()
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(mpSdCustomShow)
        return mpSdCustomShow->GetName();

    return OUString();
}

void SAL_CALL SdXCustomPresentation::setName( const OUString& aName )
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(mpSdCustomShow)
        mpSdCustomShow->SetName( aName );
}

/*===========================================================================*
 *  class SdXCustomPresentationAccess : public XCustomPresentationAccess,    *
 *                                      public UsrObject                     *
 *===========================================================================*/

SdXCustomPresentationAccess::SdXCustomPresentationAccess(SdXImpressDocument& rMyModel) throw()
: mrModel(rMyModel)
{
}

SdXCustomPresentationAccess::~SdXCustomPresentationAccess() throw()
{
}

// XServiceInfo
OUString SAL_CALL SdXCustomPresentationAccess::getImplementationName()
    throw(uno::RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("SdXCustomPresentationAccess") );
}

sal_Bool SAL_CALL SdXCustomPresentationAccess::supportsService( const OUString& ServiceName )
    throw(uno::RuntimeException)
{
    return SvxServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames() );
}

uno::Sequence< OUString > SAL_CALL SdXCustomPresentationAccess::getSupportedServiceNames()
    throw(uno::RuntimeException)
{
    const OUString aNS( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.CustomPresentationAccess") );
    uno::Sequence< OUString > aSeq( &aNS, 1 );
    return aSeq;
}

// XSingleServiceFactory
uno::Reference< uno::XInterface > SAL_CALL SdXCustomPresentationAccess::createInstance()
    throw(uno::Exception, uno::RuntimeException)
{
    uno::Reference< uno::XInterface >  xRef( (::cppu::OWeakObject*)new SdXCustomPresentation() );
    return xRef;
}

uno::Reference< uno::XInterface > SAL_CALL SdXCustomPresentationAccess::createInstanceWithArguments( const uno::Sequence< uno::Any >& aArguments )
    throw(uno::Exception, uno::RuntimeException)
{
    return createInstance();
}

// XNameContainer
void SAL_CALL SdXCustomPresentationAccess::insertByName( const OUString& aName, const uno::Any& aElement )
    throw(lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    // get the documents custom show list
    List* pList;
    if(mrModel.GetDoc())
        pList = mrModel.GetDoc()->GetCustomShowList(sal_True);

    // no list, no cookies
    if( NULL == pList)
        throw uno::RuntimeException();

    // do we have an container::XIndexContainer?
    uno::Reference< container::XIndexContainer > xContainer;
    ::cppu::extractInterface( xContainer, aElement);
    SdXCustomPresentation* pXShow = SdXCustomPresentation::getImplementation(xContainer);

    if( NULL == pXShow )
        throw lang::IllegalArgumentException();

    // get the internal custom show from the api wrapper
    SdCustomShow* pShow = pXShow->GetSdCustomShow();
    if( NULL == pShow )
    {
        pShow = new SdCustomShow( mrModel.GetDoc() );
        pXShow->SetSdCustomShow( pShow );
    }
    else
    {
        if( NULL == pXShow->GetModel() || *pXShow->GetModel() != mrModel )
            throw lang::IllegalArgumentException();
    }

    // give it a name
    pShow->SetName( aName);

    // check if this or another customshow with the same name already exists
    for( SdCustomShow* pCompare = (SdCustomShow*)pList->First();
         pCompare;
         pCompare = (SdCustomShow*)pList->Next() )
    {
        if( pCompare == pShow || pCompare->GetName() == pShow->GetName() )
            break;
    }

    if( CONTAINER_ENTRY_NOTFOUND != pList->GetPos( pShow ) )
    {
        uno::Any aAny;
        aAny <<= container::ElementExistException();

        lang::WrappedTargetException aExp;
        aExp.TargetException = aAny;

        throw aExp;
    }

    pList->Insert(pShow);

    mrModel.SetModified();
}

void SAL_CALL SdXCustomPresentationAccess::removeByName( const OUString& Name )
    throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    SdCustomShow* pShow = getSdCustomShow(Name);

    List* pList = GetCustomShowList();
    if(pList && pShow)
        delete (SdCustomShow*)pList->Remove( pShow );
    else
        throw container::NoSuchElementException();

    mrModel.SetModified();
}

// XNameReplace
void SAL_CALL SdXCustomPresentationAccess::replaceByName( const OUString& aName, const uno::Any& aElement )
    throw(lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    removeByName( aName );
    insertByName( aName, aElement );
}

// XNameAccess
uno::Any SAL_CALL SdXCustomPresentationAccess::getByName( const OUString& aName )
    throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    uno::Any aAny;

    SdCustomShow* pShow = getSdCustomShow(aName);
    if(pShow)
    {
        uno::Reference< container::XIndexContainer >  xRef( (container::XIndexContainer*)new SdXCustomPresentation(pShow, &mrModel) );
        aAny <<= xRef;
    }
    else
    {
        throw container::NoSuchElementException();
    }

    return aAny;
}

uno::Sequence< OUString > SAL_CALL SdXCustomPresentationAccess::getElementNames()
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    List* pList = GetCustomShowList();
    const sal_uInt32 nCount = pList?pList->Count():0;

    uno::Sequence< OUString > aSequence( nCount );
    OUString* pStringList = aSequence.getArray();

    sal_uInt32 nIdx = 0;
    while( nIdx < nCount )
    {
        const SdCustomShow* pShow = (const SdCustomShow*)pList->GetObject(nIdx);
        pStringList[nIdx] = pShow->GetName();
        nIdx++;
    }

    return aSequence;
}


sal_Bool SAL_CALL SdXCustomPresentationAccess::hasByName( const OUString& aName )
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );
    return getSdCustomShow(aName) != NULL;
}

// XElementAccess
uno::Type SAL_CALL SdXCustomPresentationAccess::getElementType()
    throw(uno::RuntimeException)
{
    return ITYPE( container::XIndexContainer );
}

sal_Bool SAL_CALL SdXCustomPresentationAccess::hasElements()
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    List* pList = GetCustomShowList();
    return pList && pList->Count() > 0;
}

SdCustomShow * SdXCustomPresentationAccess::getSdCustomShow( const OUString& Name ) const throw()
{
    sal_uInt32 nIdx = 0;

    List* pList = GetCustomShowList();
    const sal_uInt32 nCount = pList?pList->Count():0;

    const String aName( Name );

    while( nIdx < nCount )
    {
        SdCustomShow* pShow = (SdCustomShow*)pList->GetObject(nIdx);
        if( pShow->GetName() == aName )
            return pShow;
        nIdx++;
    }
    return NULL;
}

sal_Int32 SdXCustomPresentationAccess::getSdCustomShowIdx( const OUString& Name ) const throw()
{
    sal_Int32 nIdx = 0;

    List* pList = GetCustomShowList();
    const sal_Int32 nCount = pList?pList->Count():0;

    const String aName( Name );

    while( nIdx < nCount )
    {
        SdCustomShow* pShow = (SdCustomShow*)pList->GetObject(nIdx);
        if( pShow->GetName() == aName )
            return nIdx;
        nIdx++;
    }
    return -1;
}


