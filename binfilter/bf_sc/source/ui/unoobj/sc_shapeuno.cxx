/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <comphelper/uno3.hxx>
#include <bf_svtools/unoevent.hxx>
#include <bf_svtools/unoimap.hxx>
#include <bf_svx/unoshape.hxx>
#include <bf_svx/unofield.hxx>


#include "shapeuno.hxx"
#include "miscuno.hxx"
#include "cellsuno.hxx"
#include "textuno.hxx"
#include "fielduno.hxx"
#include "docsh.hxx"
#include "drwlayer.hxx"
#include "userdat.hxx"
#include "unonames.hxx"
#include "unoguard.hxx"

#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
namespace binfilter {

using namespace ::com::sun::star;

//------------------------------------------------------------------------

DECLARE_STL_USTRINGACCESS_MAP( uno::Sequence< sal_Int8 > *,  ScShapeImplementationIdMap );

static ScShapeImplementationIdMap aImplementationIdMap;

const SfxItemPropertyMap* lcl_GetShapeMap()
{
    static SfxItemPropertyMap aShapeMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_IMAGEMAP),	0, &getCppuType((uno::Reference<container::XIndexContainer>*)0), 0, 0 },
        {0,0,0,0}
    };
    return aShapeMap_Impl;
}

// static
const SvEventDescription* ScShapeObj::GetSupportedMacroItems()
{
    static const SvEventDescription aMacroDescriptionsImpl[] =
    {
        { 0, NULL }
    };
    return aMacroDescriptionsImpl;
}

//------------------------------------------------------------------------

ScShapeObj::ScShapeObj( uno::Reference<drawing::XShape>& xShape )
    : pImplementationId(NULL),
      bIsTextShape(FALSE)
{
    ::comphelper::increment( m_refCount );

    {
        mxShapeAgg = uno::Reference<uno::XAggregation>( xShape, uno::UNO_QUERY );
        // extra block to force deletion of the temporary before setDelegator
    }

    if (mxShapeAgg.is())
    {
        xShape = NULL;		// during setDelegator, mxShapeAgg must be the only ref

        mxShapeAgg->setDelegator( (cppu::OWeakObject*)this );

        xShape = uno::Reference<drawing::XShape>( mxShapeAgg, uno::UNO_QUERY );

        bIsTextShape = ( SvxUnoTextBase::getImplementation( mxShapeAgg ) != NULL );
    }

    ::comphelper::decrement( m_refCount );
}

ScShapeObj::~ScShapeObj()
{
//	if (mxShapeAgg.is())
//		mxShapeAgg->setDelegator(uno::Reference<uno::XInterface>());
}

// XInterface

uno::Any SAL_CALL ScShapeObj::queryInterface( const uno::Type& rType )
                                                throw(uno::RuntimeException)
{
    SC_QUERYINTERFACE( beans::XPropertySet )
    SC_QUERYINTERFACE( beans::XPropertyState )
    SC_QUERYINTERFACE( text::XTextContent )
    SC_QUERYINTERFACE( lang::XComponent )
    if ( bIsTextShape )
    {
        //	#105585# for text shapes, XText (and parent interfaces) must
        //	be handled here, too (for ScCellFieldObj handling):
        SC_QUERYINTERFACE( text::XText )
        SC_QUERYINTERFACE( text::XSimpleText )
        SC_QUERYINTERFACE( text::XTextRange )
    }
    SC_QUERYINTERFACE( lang::XTypeProvider )

    uno::Any aRet = OWeakObject::queryInterface( rType );
    if ( !aRet.hasValue() && mxShapeAgg.is() )
        aRet = mxShapeAgg->queryAggregation( rType );

    return aRet;
}

void SAL_CALL ScShapeObj::acquire() throw()
{
    OWeakObject::acquire();
}

void SAL_CALL ScShapeObj::release() throw()
{
    OWeakObject::release();
}

uno::Reference<beans::XPropertySet> lcl_GetPropertySet( const uno::Reference<uno::XAggregation>& xAgg )
{
    uno::Reference<beans::XPropertySet> xRet;
    if ( xAgg.is() )
        xAgg->queryAggregation( getCppuType((uno::Reference<beans::XPropertySet>*) 0) ) >>= xRet;
    return xRet;
}

uno::Reference<beans::XPropertyState> lcl_GetPropertyState( const uno::Reference<uno::XAggregation>& xAgg )
{
    uno::Reference<beans::XPropertyState> xRet;
    if ( xAgg.is() )
        xAgg->queryAggregation( getCppuType((uno::Reference<beans::XPropertyState>*) 0) ) >>= xRet;
    return xRet;
}

uno::Reference<lang::XComponent> lcl_GetComponent( const uno::Reference<uno::XAggregation>& xAgg )
{
    uno::Reference<lang::XComponent> xRet;
    if ( xAgg.is() )
        xAgg->queryAggregation( getCppuType((uno::Reference<lang::XComponent>*) 0) ) >>= xRet;
    return xRet;
}

uno::Reference<text::XText> lcl_GetText( const uno::Reference<uno::XAggregation>& xAgg )
{
    uno::Reference<text::XText> xRet;
    if ( xAgg.is() )
        xAgg->queryAggregation( getCppuType((uno::Reference<text::XText>*) 0) ) >>= xRet;
    return xRet;
}

uno::Reference<text::XSimpleText> lcl_GetSimpleText( const uno::Reference<uno::XAggregation>& xAgg )
{
    uno::Reference<text::XSimpleText> xRet;
    if ( xAgg.is() )
        xAgg->queryAggregation( getCppuType((uno::Reference<text::XSimpleText>*) 0) ) >>= xRet;
    return xRet;
}

uno::Reference<text::XTextRange> lcl_GetTextRange( const uno::Reference<uno::XAggregation>& xAgg )
{
    uno::Reference<text::XTextRange> xRet;
    if ( xAgg.is() )
        xAgg->queryAggregation( getCppuType((uno::Reference<text::XTextRange>*) 0) ) >>= xRet;
    return xRet;
}

//	XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScShapeObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    //	mix own and aggregated properties:
    uno::Reference<beans::XPropertySetInfo> xRet;
    uno::Reference<beans::XPropertySet> xAggProp = lcl_GetPropertySet(mxShapeAgg);
    if ( xAggProp.is() )
    {
        uno::Reference<beans::XPropertySetInfo> xAggInfo = xAggProp->getPropertySetInfo();
        const uno::Sequence<beans::Property> aPropSeq = xAggInfo->getProperties();
        xRet = new SfxExtItemPropertySetInfo( lcl_GetShapeMap(), aPropSeq );
    }
    return xRet;
}

void SAL_CALL ScShapeObj::setPropertyValue(
                        const ::rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aNameString = aPropertyName;

    if ( aNameString.EqualsAscii( SC_UNONAME_IMAGEMAP ) )
    {
        SdrObject* pObj = GetSdrObject();
        if ( pObj )
        {
            ImageMap aImageMap;
            uno::Reference< uno::XInterface > xImageMapInt;
            aValue >>= xImageMapInt;

            if( !xImageMapInt.is() || !SvUnoImageMap_fillImageMap( xImageMapInt, aImageMap ) )
                throw lang::IllegalArgumentException();

            ScIMapInfo* pIMapInfo = ScDrawLayer::GetIMapInfo(pObj);
            if( pIMapInfo )
            {
                // replace existing image map
                pIMapInfo->SetImageMap( aImageMap );
            }
            else
            {
                // insert new user data with image map
                pObj->InsertUserData(new ScIMapInfo(aImageMap) );
            }
        }
    }
    else
    {
        uno::Reference<beans::XPropertySet> xAggProp = lcl_GetPropertySet(mxShapeAgg);
        if ( xAggProp.is() )
            xAggProp->setPropertyValue( aPropertyName, aValue );
    }
}

uno::Any SAL_CALL ScShapeObj::getPropertyValue( const ::rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aNameString = aPropertyName;

    uno::Any aAny;
    if ( aNameString.EqualsAscii( SC_UNONAME_IMAGEMAP ) )
    {
        uno::Reference< uno::XInterface > xImageMap;
        SdrObject* pObj = GetSdrObject();
        if ( pObj )
        {
            ScIMapInfo* pIMapInfo = ScDrawLayer::GetIMapInfo(GetSdrObject());
            if( pIMapInfo )
            {
                const ImageMap& rIMap = pIMapInfo->GetImageMap();
                xImageMap = SvUnoImageMap_createInstance( rIMap, GetSupportedMacroItems() );
            }
            else
                xImageMap = SvUnoImageMap_createInstance( GetSupportedMacroItems() );
        }
        aAny <<= uno::Reference< container::XIndexContainer >::query( xImageMap );
    }
    else
    {
        uno::Reference<beans::XPropertySet> xAggProp = lcl_GetPropertySet(mxShapeAgg);
        if ( xAggProp.is() )
            aAny = xAggProp->getPropertyValue( aPropertyName );
    }

    return aAny;
}

void SAL_CALL ScShapeObj::addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                            const uno::Reference<beans::XPropertyChangeListener>& aListener)
                            throw(beans::UnknownPropertyException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;

    uno::Reference<beans::XPropertySet> xAggProp = lcl_GetPropertySet(mxShapeAgg);
    if ( xAggProp.is() )
        xAggProp->addPropertyChangeListener( aPropertyName, aListener );
}

void SAL_CALL ScShapeObj::removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                            const uno::Reference<beans::XPropertyChangeListener>& aListener)
                            throw(beans::UnknownPropertyException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;

    uno::Reference<beans::XPropertySet> xAggProp = lcl_GetPropertySet(mxShapeAgg);
    if ( xAggProp.is() )
        xAggProp->removePropertyChangeListener( aPropertyName, aListener );
}

void SAL_CALL ScShapeObj::addVetoableChangeListener( const ::rtl::OUString& aPropertyName,
                            const uno::Reference<beans::XVetoableChangeListener>& aListener)
                            throw(beans::UnknownPropertyException,
                                lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;

    uno::Reference<beans::XPropertySet> xAggProp = lcl_GetPropertySet(mxShapeAgg);
    if ( xAggProp.is() )
        xAggProp->addVetoableChangeListener( aPropertyName, aListener );
}

void SAL_CALL ScShapeObj::removeVetoableChangeListener( const ::rtl::OUString& aPropertyName,
                            const uno::Reference<beans::XVetoableChangeListener>& aListener)
                            throw(beans::UnknownPropertyException,
                                lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;

    uno::Reference<beans::XPropertySet> xAggProp = lcl_GetPropertySet(mxShapeAgg);
    if ( xAggProp.is() )
        xAggProp->removeVetoableChangeListener( aPropertyName, aListener );
}

//	XPropertyState

beans::PropertyState SAL_CALL ScShapeObj::getPropertyState( const ::rtl::OUString& aPropertyName )
                                throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aNameString = aPropertyName;

    beans::PropertyState eRet = beans::PropertyState_DIRECT_VALUE;
    if ( aNameString.EqualsAscii( SC_UNONAME_IMAGEMAP ) )
    {
        // ImageMap is always "direct"
    }
    else
    {
        uno::Reference<beans::XPropertyState> xAggState = lcl_GetPropertyState(mxShapeAgg);
        if ( xAggState.is() )
            eRet = xAggState->getPropertyState( aPropertyName );
    }

    return eRet;
}

uno::Sequence<beans::PropertyState> SAL_CALL ScShapeObj::getPropertyStates(
                                const uno::Sequence< ::rtl::OUString>& aPropertyNames )
                            throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    ScUnoGuard aGuard;

    //	simple loop to get own and aggregated states

    const ::rtl::OUString* pNames = aPropertyNames.getConstArray();
    uno::Sequence<beans::PropertyState> aRet(aPropertyNames.getLength());
    beans::PropertyState* pStates = aRet.getArray();
    for(sal_Int32 i = 0; i < aPropertyNames.getLength(); i++)
        pStates[i] = getPropertyState(pNames[i]);
    return aRet;
}

void SAL_CALL ScShapeObj::setPropertyToDefault( const ::rtl::OUString& aPropertyName )
                            throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aNameString = aPropertyName;

    if ( aNameString.EqualsAscii( SC_UNONAME_IMAGEMAP ) )
    {
        SdrObject* pObj = GetSdrObject();
        if ( pObj )
        {
            ScIMapInfo* pIMapInfo = ScDrawLayer::GetIMapInfo(pObj);
            if( pIMapInfo )
            {
                ImageMap aEmpty;
                pIMapInfo->SetImageMap( aEmpty );	// replace with empty image map
            }
            else
            {
                // nothing to do (no need to insert user data for an empty map)
            }
        }
    }
    else
    {
        uno::Reference<beans::XPropertyState> xAggState = lcl_GetPropertyState(mxShapeAgg);
        if ( xAggState.is() )
            xAggState->setPropertyToDefault( aPropertyName );
    }
}

uno::Any SAL_CALL ScShapeObj::getPropertyDefault( const ::rtl::OUString& aPropertyName )
                                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aNameString = aPropertyName;

    uno::Any aAny;
    if ( aNameString.EqualsAscii( SC_UNONAME_IMAGEMAP ) )
    {
        //	default: empty ImageMap
        uno::Reference< uno::XInterface > xImageMap = SvUnoImageMap_createInstance( GetSupportedMacroItems() );
        aAny <<= uno::Reference< container::XIndexContainer >::query( xImageMap );
    }
    else
    {
        uno::Reference<beans::XPropertyState> xAggState = lcl_GetPropertyState(mxShapeAgg);
        if ( xAggState.is() )
            aAny = xAggState->getPropertyDefault( aPropertyName );
    }

    return aAny;
}

// XTextContent

void SAL_CALL ScShapeObj::attach( const uno::Reference<text::XTextRange>& xTextRange )
                                throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    ScUnoGuard aGuard;

    throw lang::IllegalArgumentException();		// anchor cannot be changed
}

BOOL lcl_GetPageNum( SdrPage* pPage, SdrModel& rModel, USHORT& rNum )
{
    USHORT nCount = rModel.GetPageCount();
    for (USHORT i=0; i<nCount; i++)
        if ( rModel.GetPage(i) == pPage )
        {
            rNum = i;
            return TRUE;
        }

    return FALSE;
}

uno::Reference<text::XTextRange> SAL_CALL ScShapeObj::getAnchor() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    uno::Reference<text::XTextRange> xRet;

    SdrObject* pObj = GetSdrObject();
    if( pObj )
    {
        ScDrawLayer* pModel = (ScDrawLayer*)pObj->GetModel();
        SdrPage* pPage = pObj->GetPage();
        if ( pModel )
        {
            ScDocument* pDoc = pModel->GetDocument();
            if ( pDoc )
            {
                SfxObjectShell* pObjSh = pDoc->GetDocumentShell();
                if ( pObjSh && pObjSh->ISA(ScDocShell) )
                {
                    ScDocShell* pDocSh = (ScDocShell*)pObjSh;

                    USHORT nTab = 0;
                    if ( lcl_GetPageNum( pPage, *pModel, nTab ) )
                    {
                        Point aPos = pObj->GetBoundRect().TopLeft();
                        ScRange aRange = pDoc->GetRange( nTab, Rectangle( aPos, aPos ) );

                        //	anchor is always the cell

                        xRet = new ScCellObj( pDocSh, aRange.aStart );
                    }
                }
            }
        }
    }

    return xRet;
}

// XComponent

void SAL_CALL ScShapeObj::dispose() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    uno::Reference<lang::XComponent> xAggComp = lcl_GetComponent(mxShapeAgg);
    if ( xAggComp.is() )
        xAggComp->dispose();
}

void SAL_CALL ScShapeObj::addEventListener(
                        const uno::Reference<lang::XEventListener>& xListener )
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    uno::Reference<lang::XComponent> xAggComp = lcl_GetComponent(mxShapeAgg);
    if ( xAggComp.is() )
        xAggComp->addEventListener(xListener);
}

void SAL_CALL ScShapeObj::removeEventListener(
                        const uno::Reference<lang::XEventListener>& xListener )
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    uno::Reference<lang::XComponent> xAggComp = lcl_GetComponent(mxShapeAgg);
    if ( xAggComp.is() )
        xAggComp->removeEventListener(xListener);
}

// XText
// (special handling for ScCellFieldObj)

void lcl_CopyOneProperty( beans::XPropertySet& rDest, beans::XPropertySet& rSource, const sal_Char* pName )
{
    ::rtl::OUString aNameStr = ::rtl::OUString::createFromAscii(pName);
    try
    {
        uno::Any aValue = rSource.getPropertyValue( aNameStr );
        rDest.setPropertyValue( aNameStr, aValue );
    }
    catch (uno::Exception&)
    {
        DBG_ERROR("Exception in text field");
    }
}

void SAL_CALL ScShapeObj::insertTextContent( const uno::Reference<text::XTextRange>& xRange,
                                                const uno::Reference<text::XTextContent>& xContent,
                                                sal_Bool bAbsorb )
                                    throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    ScUnoGuard aGuard;

    uno::Reference<text::XTextContent> xEffContent;

    ScCellFieldObj* pCellField = ScCellFieldObj::getImplementation( xContent );
    if ( pCellField )
    {
        //	#105585# createInstance("TextField.URL") from the document creates a ScCellFieldObj.
        //	To insert it into drawing text, a SvxUnoTextField is needed instead.
        //	The ScCellFieldObj object is left in non-inserted state.

        SvxUnoTextField* pDrawField = new SvxUnoTextField( ID_URLFIELD );
        xEffContent = pDrawField;
        lcl_CopyOneProperty( *pDrawField, *pCellField, SC_UNONAME_URL );
        lcl_CopyOneProperty( *pDrawField, *pCellField, SC_UNONAME_REPR );
        lcl_CopyOneProperty( *pDrawField, *pCellField, SC_UNONAME_TARGET );
    }
    else
        xEffContent = xContent;

    uno::Reference<text::XText> xAggText = lcl_GetText(mxShapeAgg);
    if ( xAggText.is() )
        xAggText->insertTextContent( xRange, xEffContent, bAbsorb );
}

void SAL_CALL ScShapeObj::removeTextContent( const uno::Reference<text::XTextContent>& xContent )
                                throw(container::NoSuchElementException, uno::RuntimeException)
{
    ScUnoGuard aGuard;

    //	ScCellFieldObj can't be used here.

    uno::Reference<text::XText> xAggText = lcl_GetText(mxShapeAgg);
    if ( xAggText.is() )
        xAggText->removeTextContent( xContent );
}

// XSimpleText (parent of XText)
// Use own SvxUnoTextCursor subclass - everything is just passed to aggregated object

uno::Reference<text::XTextCursor> SAL_CALL ScShapeObj::createTextCursor()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    if ( mxShapeAgg.is() )
    {
        //	ScDrawTextCursor must be used to ensure the ScShapeObj is returned by getText

        SvxUnoTextBase* pText = SvxUnoTextBase::getImplementation( mxShapeAgg );
        if (pText)
            return new ScDrawTextCursor( this, *pText );
    }

    return uno::Reference<text::XTextCursor>();
}

uno::Reference<text::XTextCursor> SAL_CALL ScShapeObj::createTextCursorByRange(
                                    const uno::Reference<text::XTextRange>& aTextPosition )
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    if ( mxShapeAgg.is() && aTextPosition.is() )
    {
        //	ScDrawTextCursor must be used to ensure the ScShapeObj is returned by getText

        SvxUnoTextBase* pText = SvxUnoTextBase::getImplementation( mxShapeAgg );
        SvxUnoTextRangeBase* pRange = SvxUnoTextRangeBase::getImplementation( aTextPosition );
        if ( pText && pRange )
        {
            SvxUnoTextCursor* pCursor = new ScDrawTextCursor( this, *pText );
            uno::Reference<text::XTextCursor> xCursor( pCursor );
            pCursor->SetSelection( pRange->GetSelection() );
            return xCursor;
        }
    }

    return uno::Reference<text::XTextCursor>();
}

void SAL_CALL ScShapeObj::insertString( const uno::Reference<text::XTextRange>& xRange,
                                        const ::rtl::OUString& aString, sal_Bool bAbsorb )
                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    uno::Reference<text::XSimpleText> xAggSimpleText = lcl_GetSimpleText(mxShapeAgg);
    if ( xAggSimpleText.is() )
        xAggSimpleText->insertString( xRange, aString, bAbsorb );
    else
        throw uno::RuntimeException();
}

void SAL_CALL ScShapeObj::insertControlCharacter( const uno::Reference<text::XTextRange>& xRange,
                                                sal_Int16 nControlCharacter, sal_Bool bAbsorb )
                                    throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    ScUnoGuard aGuard;

    uno::Reference<text::XSimpleText> xAggSimpleText = lcl_GetSimpleText(mxShapeAgg);
    if ( xAggSimpleText.is() )
        xAggSimpleText->insertControlCharacter( xRange, nControlCharacter, bAbsorb );
    else
        throw uno::RuntimeException();
}

// XTextRange
// (parent of XSimpleText)

uno::Reference<text::XText> SAL_CALL ScShapeObj::getText() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return this;
}

uno::Reference<text::XTextRange> SAL_CALL ScShapeObj::getStart() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    uno::Reference<text::XTextRange> xAggTextRange = lcl_GetTextRange(mxShapeAgg);
    if ( xAggTextRange.is() )
        return xAggTextRange->getStart();
    else
        throw uno::RuntimeException();

    return uno::Reference<text::XTextRange>();
}

uno::Reference<text::XTextRange> SAL_CALL ScShapeObj::getEnd() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    uno::Reference<text::XTextRange> xAggTextRange = lcl_GetTextRange(mxShapeAgg);
    if ( xAggTextRange.is() )
        return xAggTextRange->getEnd();
    else
        throw uno::RuntimeException();

    return uno::Reference<text::XTextRange>();
}

::rtl::OUString SAL_CALL ScShapeObj::getString() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    uno::Reference<text::XTextRange> xAggTextRange = lcl_GetTextRange(mxShapeAgg);
    if ( xAggTextRange.is() )
        return xAggTextRange->getString();
    else
        throw uno::RuntimeException();

    return ::rtl::OUString();
}

void SAL_CALL ScShapeObj::setString( const ::rtl::OUString& aText ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    uno::Reference<text::XTextRange> xAggTextRange = lcl_GetTextRange(mxShapeAgg);
    if ( xAggTextRange.is() )
        xAggTextRange->setString( aText );
    else
        throw uno::RuntimeException();
}

// XTypeProvider

uno::Sequence<uno::Type> SAL_CALL ScShapeObj::getTypes() throw(uno::RuntimeException)
{
    uno::Sequence< uno::Type > aTypeSequence;

    uno::Reference<lang::XTypeProvider> xBaseProvider;
    if ( mxShapeAgg.is() )
        mxShapeAgg->queryAggregation( getCppuType((uno::Reference<lang::XTypeProvider>*) 0) ) >>= xBaseProvider;

    DBG_ASSERT( xBaseProvider.is(), "ScShapeObj: No XTypeProvider from aggregated shape!" );
    if( xBaseProvider.is() )
    {
        aTypeSequence = xBaseProvider->getTypes();
        long nBaseLen = aTypeSequence.getLength();

        aTypeSequence.realloc( nBaseLen + 1 );
        uno::Type* pPtr = aTypeSequence.getArray();
        pPtr[nBaseLen + 0] = getCppuType((const uno::Reference<text::XTextContent>*)0);
    }
    return aTypeSequence;
}

uno::Sequence<sal_Int8> SAL_CALL ScShapeObj::getImplementationId()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    // do we need to compute the implementation id for this instance?
    if( !pImplementationId && mxShapeAgg.is())
    {
        uno::Reference< drawing::XShape > xAggShape;
        mxShapeAgg->queryAggregation( ::getCppuType((uno::Reference< drawing::XShape >*)0) ) >>= xAggShape;

        if( xAggShape.is() )
        {
            const ::rtl::OUString aShapeType( xAggShape->getShapeType() );
            // did we already compute an implementation id for the agregated shape type?
            ScShapeImplementationIdMap::iterator aIter( aImplementationIdMap.find(aShapeType ) );
            if( aIter == aImplementationIdMap.end() )
            {
                // we need to create a new implementation id for this
                // note: this memory is not free'd until application exists
                //		 but since we have a fixed set of shapetypes and the
                //		 memory will be reused this is ok.
                pImplementationId = new uno::Sequence< sal_Int8 >( 16 );
                rtl_createUuid( (sal_uInt8 *) pImplementationId->getArray(), 0, sal_True );
                aImplementationIdMap[ aShapeType ] = pImplementationId;
            }
            else
            {
                // use the already computed implementation id
                pImplementationId = (*aIter).second;
            }
        }
    }
    if( NULL == pImplementationId )
    {
        DBG_ERROR( "Could not create an implementation id for a ScXShape!" );
        return uno::Sequence< sal_Int8 > ();
    }
    else
    {
        return *pImplementationId;
    }
}

SdrObject* ScShapeObj::GetSdrObject() const throw()
{
    if(mxShapeAgg.is())
    {
        SvxShape* pShape = SvxShape::getImplementation( mxShapeAgg );
        if(pShape)
            return pShape->GetSdrObject();
    }

    return NULL;
}

}
