/*************************************************************************
 *
 *  $RCSfile: XPropertyTable.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: cl $ $Date: 2001-03-08 11:37:51 $
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

#ifndef _COM_SUN_STAR_DRAWING_POLYPOLYGONBEZIERCOORDS_HPP_
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINEDASH_HPP_
#include <com/sun/star/drawing/LineDash.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_GRADIENT_HPP_
#include <com/sun/star/awt/Gradient.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_HATCH_HPP_
#include <com/sun/star/drawing/Hatch.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

#include <cppuhelper/implbase2.hxx>

#ifndef _XTABLE_HXX
#include "xtable.hxx"
#endif

#ifndef _SVX_UNOPOLYHELPER_HXX
#include "unopolyhelper.hxx"
#endif

#ifndef _XDEF_HXX
#include "xdef.hxx"
#endif

#include "unoapi.hxx"
#include "unoprnms.hxx"

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;
using namespace cppu;
using namespace rtl;

class SvxUnoXPropertyTable : public WeakImplHelper2< XNameContainer, XServiceInfo >
{
private:
    sal_Int16 mnWhich;
    XPropertyList*  mpList;
    XPropertyTable* mpTable;

    long getCount() const { return mpList ? mpList->Count() : (mpTable?mpTable->Count():0); }
    XPropertyEntry* get( long index ) const;
public:
    SvxUnoXPropertyTable( sal_Int16 nWhich, XPropertyList* pList ) throw();
    SvxUnoXPropertyTable( sal_Int16 nWhich, XPropertyTable* pTable ) throw();

    virtual ~SvxUnoXPropertyTable() throw();

    virtual Any getAny( const XPropertyEntry* pEntry ) const throw() = 0;
    virtual XPropertyEntry* getEntry( const OUString& rName, const Any& rAny ) const throw() = 0;

    // XServiceInfo
    virtual sal_Bool SAL_CALL supportsService( const  OUString& ServiceName ) throw( RuntimeException);

    // XNameContainer
    virtual void SAL_CALL insertByName( const  OUString& aName, const  Any& aElement ) throw( IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException);
    virtual void SAL_CALL removeByName( const  OUString& Name ) throw( NoSuchElementException, WrappedTargetException, RuntimeException);

    // XNameReplace
    virtual void SAL_CALL replaceByName( const  OUString& aName, const  Any& aElement ) throw( IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException);

    // XNameAccess
    virtual Any SAL_CALL getByName( const  OUString& aName ) throw( NoSuchElementException, WrappedTargetException, RuntimeException);
    virtual Sequence<  OUString > SAL_CALL getElementNames(  ) throw( RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const  OUString& aName ) throw( RuntimeException);

    // XElementAccess
    virtual sal_Bool SAL_CALL hasElements(  ) throw( RuntimeException);
};

SvxUnoXPropertyTable::SvxUnoXPropertyTable( sal_Int16 nWhich, XPropertyTable* pTable ) throw()
: mpTable( pTable ), mpList( NULL ), mnWhich( nWhich )
{
}

SvxUnoXPropertyTable::SvxUnoXPropertyTable( sal_Int16 nWhich, XPropertyList* pList ) throw()
: mpTable( NULL ), mpList( pList ), mnWhich( nWhich )
{
}

SvxUnoXPropertyTable::~SvxUnoXPropertyTable() throw()
{
}

XPropertyEntry* SvxUnoXPropertyTable::get( long index ) const
{
    if( mpTable )
        return mpTable->Get( index, 0 );
    else if( mpList )
        return mpList->Get( index, 0 );
    else
        return NULL;
}

// XServiceInfo
sal_Bool SAL_CALL SvxUnoXPropertyTable::supportsService( const  OUString& ServiceName )
    throw( RuntimeException)
{
    const Sequence< OUString > aServices( getSupportedServiceNames() );
    const OUString* pServices = aServices.getConstArray();
    const sal_Int32 nCount = aServices.getLength();
    sal_Int32 i;
    for( i = 0; i < nCount; i++ )
    {
        if( *pServices++ == ServiceName )
            return sal_True;
    }

    return sal_False;
}

// XNameContainer
void SAL_CALL SvxUnoXPropertyTable::insertByName( const  OUString& aName, const  Any& aElement )
    throw( IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
{
    if( NULL == mpList && NULL == mpTable )
        throw IllegalArgumentException();

    if( hasByName( aName ) )
        throw ElementExistException();

    String aInternalName;
    SvxUnogetInternalNameForItem( mnWhich, aName, aInternalName );

    XPropertyEntry* pNewEntry = getEntry( aInternalName, aElement );
    if( NULL == pNewEntry )
        throw IllegalArgumentException();

    if( mpList )
        mpList->Insert( pNewEntry );
    else
        mpTable->Insert( mpTable->Count(), pNewEntry );
}

void SAL_CALL SvxUnoXPropertyTable::removeByName( const  OUString& Name )
    throw( NoSuchElementException, WrappedTargetException, RuntimeException)
{
    String aInternalName;
    SvxUnogetInternalNameForItem( mnWhich, Name, aInternalName );

    const long nCount = getCount();
    long i;
    XPropertyEntry* pEntry;
    for( i = 0; i < nCount; i++ )
    {
        pEntry = get( i );
        if( pEntry && pEntry->GetName() == aInternalName )
        {
            if( mpList )
                delete mpList->Remove( i, 0 );
            else
                delete mpTable->Remove( i, 0 );
            return;
        }
    }

    throw NoSuchElementException();
}

// XNameReplace
void SAL_CALL SvxUnoXPropertyTable::replaceByName( const  OUString& aName, const  Any& aElement )
    throw( IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
{
    String aInternalName;
    SvxUnogetInternalNameForItem( mnWhich, aName, aInternalName );

    const long nCount = getCount();
    long i;
    XPropertyEntry* pEntry;
    for( i = 0; i < nCount; i++ )
    {
        pEntry = get( i );
        if( pEntry && pEntry->GetName() == aInternalName )
        {
            XPropertyEntry* pNewEntry = getEntry( aInternalName, aElement );
            if( NULL == pNewEntry )
                throw IllegalArgumentException();

            if( mpList )
                delete mpList->Replace( pNewEntry, i );
            else
                delete mpTable->Replace( i, pNewEntry );
            return;
        }
    }

    throw NoSuchElementException();
}

// XNameAccess
Any SAL_CALL SvxUnoXPropertyTable::getByName( const  OUString& aName )
    throw( NoSuchElementException, WrappedTargetException, RuntimeException)
{
    String aInternalName;
    SvxUnogetInternalNameForItem( mnWhich, aName, aInternalName );

    const long nCount = getCount();
    long i;
    XPropertyEntry* pEntry;
    for( i = 0; i < nCount; i++ )
    {
        pEntry = get( i );

        if( pEntry && pEntry->GetName() == aInternalName )
            return getAny( pEntry );
    }

    throw NoSuchElementException();
}

Sequence<  OUString > SAL_CALL SvxUnoXPropertyTable::getElementNames()
    throw( RuntimeException)
{
    const long nCount = getCount();
    Sequence< OUString > aNames( nCount );
    OUString* pNames = aNames.getArray();
    long i;
    XPropertyEntry* pEntry;
    for( i = 0; i < nCount; i++ )
    {
        pEntry = get( i );

        if( pEntry )
        {
            SvxUnogetApiNameForItem( mnWhich, pEntry->GetName(), *pNames );
            pNames++;
        }
    }

    return aNames;
}

sal_Bool SAL_CALL SvxUnoXPropertyTable::hasByName( const  OUString& aName )
    throw( RuntimeException)
{
    String aInternalName;
    SvxUnogetInternalNameForItem( mnWhich, aName, aInternalName );

    const long nCount = mpList?mpList->Count():0;
    long i;
    XPropertyEntry* pEntry;
    for( i = 0; i < nCount; i++ )
    {
        pEntry = get( i );
        if( pEntry && pEntry->GetName() == aInternalName )
            return sal_True;
    }

    return sal_False;
}

// XElementAccess
sal_Bool SAL_CALL SvxUnoXPropertyTable::hasElements(  )
    throw( RuntimeException)
{
    return getCount() != 0;
}

///////////////////////////////////////////////////////////////////////

class SvxUnoXColorTable : public SvxUnoXPropertyTable
{
public:
    SvxUnoXColorTable( XPropertyTable* pTable ) throw() : SvxUnoXPropertyTable( XATTR_LINECOLOR, pTable ) {};

    // SvxUnoXPropertyTable
    virtual Any getAny( const XPropertyEntry* pEntry ) const throw();
    virtual XPropertyEntry* getEntry( const OUString& rName, const Any& rAny ) const throw();

    // XElementAccess
    virtual Type SAL_CALL getElementType() throw( RuntimeException );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw( uno::RuntimeException );
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) throw( uno::RuntimeException);
};

Reference< XInterface > SvxUnoXColorTable_createInstance( XPropertyTable* pTable ) throw()
{
    return (OWeakObject*) new SvxUnoXColorTable( pTable );
}

// SvxUnoXPropertyTable
Any SvxUnoXColorTable::getAny( const XPropertyEntry* pEntry ) const throw()
{
    Any aAny;
    aAny <<= (sal_Int32)((XColorEntry*)pEntry)->GetColor().GetColor();
    return aAny;
}

XPropertyEntry* SvxUnoXColorTable::getEntry( const OUString& rName, const Any& rAny ) const throw()
{
    sal_Int32 nColor;
    if( !(rAny >>= nColor) )
        return NULL;

    const Color aColor( (ColorData)nColor );
    const String aName( rName );
    return new XColorEntry( aColor, aName );
}

// XElementAccess
Type SAL_CALL SvxUnoXColorTable::getElementType()
    throw( RuntimeException )
{
    return ::getCppuType((const sal_Int32*)0);
}

// XServiceInfo
OUString SAL_CALL SvxUnoXColorTable::getImplementationName(  ) throw( uno::RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SvxUnoXColorTable" ) );
}

uno::Sequence<  OUString > SAL_CALL SvxUnoXColorTable::getSupportedServiceNames(  ) throw( uno::RuntimeException)
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.ColorTable" ) );
    Sequence< OUString > aServices( &aServiceName, 1 );
    return aServices;
}

///////////////////////////////////////////////////////////////////////

class SvxUnoXLineEndTable : public SvxUnoXPropertyTable
{
public:
    SvxUnoXLineEndTable( XPropertyList* pTable ) throw() : SvxUnoXPropertyTable( XATTR_LINEEND, pTable ) {};

    // SvxUnoXPropertyTable
    virtual Any getAny( const XPropertyEntry* pEntry ) const throw();
    virtual XPropertyEntry* getEntry( const OUString& rName, const Any& rAny ) const throw();

    // XElementAccess
    virtual Type SAL_CALL getElementType() throw( RuntimeException );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw( uno::RuntimeException );
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) throw( uno::RuntimeException);
};

Reference< XInterface > SvxUnoXLineEndTable_createInstance( XPropertyList* pTable ) throw()
{
    return (OWeakObject*)new SvxUnoXLineEndTable( pTable );
}

// SvxUnoXPropertyTable
Any SvxUnoXLineEndTable::getAny( const XPropertyEntry* pEntry ) const throw()
{

    Any aAny;
    drawing::PolyPolygonBezierCoords aBezier;
    SvxConvertXPolygonToPolyPolygonBezier( ((XLineEndEntry*)pEntry)->GetLineEnd(), aBezier );
    aAny <<= aBezier;
    return aAny;
}

XPropertyEntry* SvxUnoXLineEndTable::getEntry( const OUString& rName, const Any& rAny ) const throw()
{

    if( !rAny.getValue() || rAny.getValueType() != ::getCppuType((const drawing::PolyPolygonBezierCoords*)0) )
        return NULL;

    XPolygon aPolygon;
    drawing::PolyPolygonBezierCoords* pCoords = (drawing::PolyPolygonBezierCoords*)rAny.getValue();
    if( pCoords->Coordinates.getLength() > 0 )
        SvxConvertPolyPolygonBezierToXPolygon( pCoords, aPolygon );

    const String aName( rName );
    return new XLineEndEntry( aPolygon, aName );
}

// XElementAccess
Type SAL_CALL SvxUnoXLineEndTable::getElementType()
    throw( RuntimeException )
{
    return ::getCppuType((const drawing::PolyPolygonBezierCoords*)0);
}

// XServiceInfo
OUString SAL_CALL SvxUnoXLineEndTable::getImplementationName(  ) throw( uno::RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SvxUnoXLineEndTable" ) );
}

uno::Sequence<  OUString > SAL_CALL SvxUnoXLineEndTable::getSupportedServiceNames(  ) throw( uno::RuntimeException)
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.LineEndTable" ) );
    Sequence< OUString > aServices( &aServiceName, 1 );
    return aServices;
}

///////////////////////////////////////////////////////////////////////

class SvxUnoXDashTable : public SvxUnoXPropertyTable
{
public:
    SvxUnoXDashTable( XPropertyList* pTable ) throw() : SvxUnoXPropertyTable( XATTR_LINEDASH, pTable ) {};

    // SvxUnoXPropertyTable
    virtual Any getAny( const XPropertyEntry* pEntry ) const throw();
    virtual XPropertyEntry* getEntry( const OUString& rName, const Any& rAny ) const throw();

    // XElementAccess
    virtual Type SAL_CALL getElementType() throw( RuntimeException );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw( uno::RuntimeException );
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) throw( uno::RuntimeException);
};

Reference< XInterface > SvxUnoXDashTable_createInstance( XPropertyList* pTable ) throw()
{
    return (OWeakObject*)new SvxUnoXDashTable( pTable );
}

// SvxUnoXPropertyTable
Any SvxUnoXDashTable::getAny( const XPropertyEntry* pEntry ) const throw()
{
    const XDash& rXD = ((XDashEntry*)pEntry)->GetDash();

    drawing::LineDash aLineDash;

    aLineDash.Style = (::com::sun::star::drawing::DashStyle)((UINT16)rXD.GetDashStyle());
    aLineDash.Dots = rXD.GetDots();
    aLineDash.DotLen = rXD.GetDotLen();
    aLineDash.Dashes = rXD.GetDashes();
    aLineDash.DashLen = rXD.GetDashLen();
    aLineDash.Distance = rXD.GetDistance();

    Any aAny;
    aAny <<= aLineDash;
    return aAny;
}

XPropertyEntry* SvxUnoXDashTable::getEntry( const OUString& rName, const Any& rAny ) const throw()
{
    drawing::LineDash aLineDash;
    if(!(rAny >>= aLineDash))
        return NULL;

    XDash aXDash;

    aXDash.SetDashStyle((XDashStyle)((UINT16)(aLineDash.Style)));
    aXDash.SetDots(aLineDash.Dots);
    aXDash.SetDotLen(aLineDash.DotLen);
    aXDash.SetDashes(aLineDash.Dashes);
    aXDash.SetDashLen(aLineDash.DashLen);
    aXDash.SetDistance(aLineDash.Distance);

    const String aName( rName );
    return new XDashEntry( aXDash, aName );
}

// XElementAccess
Type SAL_CALL SvxUnoXDashTable::getElementType()
    throw( RuntimeException )
{
    return ::getCppuType((const drawing::LineDash*)0);
}

// XServiceInfo
OUString SAL_CALL SvxUnoXDashTable::getImplementationName(  ) throw( uno::RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SvxUnoXDashTable" ) );
}

uno::Sequence<  OUString > SAL_CALL SvxUnoXDashTable::getSupportedServiceNames(  ) throw( uno::RuntimeException)
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.DashTable" ) );
    Sequence< OUString > aServices( &aServiceName, 1 );
    return aServices;
}

///////////////////////////////////////////////////////////////////////

class SvxUnoXHatchTable : public SvxUnoXPropertyTable
{
public:
    SvxUnoXHatchTable( XPropertyList* pTable ) throw() : SvxUnoXPropertyTable( XATTR_FILLHATCH, pTable ) {};

    // SvxUnoXPropertyTable
    virtual Any getAny( const XPropertyEntry* pEntry ) const throw();
    virtual XPropertyEntry* getEntry( const OUString& rName, const Any& rAny ) const throw();

    // XElementAccess
    virtual Type SAL_CALL getElementType() throw( RuntimeException );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw( uno::RuntimeException );
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) throw( uno::RuntimeException);
};

Reference< XInterface > SvxUnoXHatchTable_createInstance( XPropertyList* pTable ) throw()
{
    return (OWeakObject*)new SvxUnoXHatchTable( pTable );
}

// SvxUnoXPropertyTable
Any SvxUnoXHatchTable::getAny( const XPropertyEntry* pEntry ) const throw()
{
    const XHatch& aHatch = ((XHatchEntry*)pEntry)->GetHatch();

    drawing::Hatch aUnoHatch;

    aUnoHatch.Style = (drawing::HatchStyle)aHatch.GetHatchStyle();
    aUnoHatch.Color = aHatch.GetColor().GetColor();
    aUnoHatch.Distance = aHatch.GetDistance();
    aUnoHatch.Angle = aHatch.GetAngle();

    Any aAny;
    aAny <<= aUnoHatch;
    return aAny;
}

XPropertyEntry* SvxUnoXHatchTable::getEntry( const OUString& rName, const Any& rAny ) const throw()
{
    drawing::Hatch aUnoHatch;
    if(!(rAny >>= aUnoHatch))
        return NULL;

    XHatch aXHatch;
    aXHatch.SetHatchStyle( (XHatchStyle)aUnoHatch.Style );
    aXHatch.SetColor( aUnoHatch.Color );
    aXHatch.SetDistance( aUnoHatch.Distance );
    aXHatch.SetAngle( aUnoHatch.Angle );

    const String aName( rName );
    return new XHatchEntry( aXHatch, aName );
}

// XElementAccess
Type SAL_CALL SvxUnoXHatchTable::getElementType()
    throw( RuntimeException )
{
    return ::getCppuType((const drawing::Hatch*)0);
}

// XServiceInfo
OUString SAL_CALL SvxUnoXHatchTable::getImplementationName(  ) throw( uno::RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SvxUnoXHatchTable" ) );
}

uno::Sequence<  OUString > SAL_CALL SvxUnoXHatchTable::getSupportedServiceNames(  ) throw( uno::RuntimeException)
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.HatchTable" ) );
    Sequence< OUString > aServices( &aServiceName, 1 );
    return aServices;
}

///////////////////////////////////////////////////////////////////////

class SvxUnoXGradientTable : public SvxUnoXPropertyTable
{
public:
    SvxUnoXGradientTable( XPropertyList* pTable ) throw() : SvxUnoXPropertyTable( XATTR_FILLGRADIENT, pTable ) {};

    // SvxUnoXPropertyTable
    virtual Any getAny( const XPropertyEntry* pEntry ) const throw();
    virtual XPropertyEntry* getEntry( const OUString& rName, const Any& rAny ) const throw();

    // XElementAccess
    virtual Type SAL_CALL getElementType() throw( RuntimeException );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw( uno::RuntimeException );
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) throw( uno::RuntimeException);
};

Reference< XInterface > SvxUnoXGradientTable_createInstance( XPropertyList* pTable ) throw()
{
    return (OWeakObject*)new SvxUnoXGradientTable( pTable );
}

// SvxUnoXPropertyTable
Any SvxUnoXGradientTable::getAny( const XPropertyEntry* pEntry ) const throw()
{
    const XGradient& aXGradient = ((XGradientEntry*)pEntry)->GetGradient();
    awt::Gradient aGradient;

    aGradient.Style = (awt::GradientStyle) aXGradient.GetGradientStyle();
    aGradient.StartColor = (INT32)aXGradient.GetStartColor().GetColor();
    aGradient.EndColor = (INT32)aXGradient.GetEndColor().GetColor();
    aGradient.Angle = (short)aXGradient.GetAngle();
    aGradient.Border = aXGradient.GetBorder();
    aGradient.XOffset = aXGradient.GetXOffset();
    aGradient.YOffset = aXGradient.GetYOffset();
    aGradient.StartIntensity = aXGradient.GetStartIntens();
    aGradient.EndIntensity = aXGradient.GetEndIntens();
    aGradient.StepCount = aXGradient.GetSteps();

    Any aAny;
    aAny <<= aGradient;
    return aAny;
}

XPropertyEntry* SvxUnoXGradientTable::getEntry( const OUString& rName, const Any& rAny ) const throw()
{
    awt::Gradient aGradient;
    if(!(rAny >>= aGradient))
        return NULL;

    XGradient aXGradient;

    aXGradient.SetGradientStyle( (XGradientStyle) aGradient.Style );
    aXGradient.SetStartColor( aGradient.StartColor );
    aXGradient.SetEndColor( aGradient.EndColor );
    aXGradient.SetAngle( aGradient.Angle );
    aXGradient.SetBorder( aGradient.Border );
    aXGradient.SetXOffset( aGradient.XOffset );
    aXGradient.SetYOffset( aGradient.YOffset );
    aXGradient.SetStartIntens( aGradient.StartIntensity );
    aXGradient.SetEndIntens( aGradient.EndIntensity );
    aXGradient.SetSteps( aGradient.StepCount );

    const String aName( rName );
    return new XGradientEntry( aXGradient, aName );
}

// XElementAccess
Type SAL_CALL SvxUnoXGradientTable::getElementType()
    throw( RuntimeException )
{
    return ::getCppuType((const awt::Gradient*)0);
}

// XServiceInfo
OUString SAL_CALL SvxUnoXGradientTable::getImplementationName(  ) throw( uno::RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SvxUnoXGradientTable" ) );
}

uno::Sequence<  OUString > SAL_CALL SvxUnoXGradientTable::getSupportedServiceNames(  ) throw( uno::RuntimeException)
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.GradientTable" ) );
    Sequence< OUString > aServices( &aServiceName, 1 );
    return aServices;
}

///////////////////////////////////////////////////////////////////////

class SvxUnoXBitmapTable : public SvxUnoXPropertyTable
{
public:
    SvxUnoXBitmapTable( XPropertyList* pTable ) throw() : SvxUnoXPropertyTable( XATTR_FILLBITMAP, pTable ) {};

    // SvxUnoXPropertyTable
    virtual Any getAny( const XPropertyEntry* pEntry ) const throw();
    virtual XPropertyEntry* getEntry( const OUString& rName, const Any& rAny ) const throw();

    // XElementAccess
    virtual Type SAL_CALL getElementType() throw( RuntimeException );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw( uno::RuntimeException );
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) throw( uno::RuntimeException);
};

Reference< XInterface > SvxUnoXBitmapTable_createInstance( XPropertyList* pTable ) throw()
{
    return (OWeakObject*)new SvxUnoXBitmapTable( pTable );
}

// SvxUnoXPropertyTable
Any SvxUnoXBitmapTable::getAny( const XPropertyEntry* pEntry ) const throw()
{
    OUString aURL( RTL_CONSTASCII_USTRINGPARAM(UNO_NAME_GRAPHOBJ_URLPREFIX));
    aURL += OUString::createFromAscii( ((XBitmapEntry*)pEntry)->GetXBitmap().GetGraphicObject().GetUniqueID().GetBuffer() );

    Any aAny;
    aAny <<= aURL;
    return aAny;
}

XPropertyEntry* SvxUnoXBitmapTable::getEntry( const OUString& rName, const Any& rAny ) const throw()
{
    OUString aURL;
    if(!(rAny >>= aURL))
        return NULL;

    GraphicObject aGrafObj( CreateGraphicObjectFromURL( aURL ) );
    XOBitmap aBMP( aGrafObj );

    const String aName( rName );
    return new XBitmapEntry( aBMP, aName );
}

// XElementAccess
Type SAL_CALL SvxUnoXBitmapTable::getElementType()
    throw( RuntimeException )
{
    return ::getCppuType((const OUString*)0);
}

// XServiceInfo
OUString SAL_CALL SvxUnoXBitmapTable::getImplementationName(  ) throw( uno::RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SvxUnoXBitmapTable" ) );
}

uno::Sequence<  OUString > SAL_CALL SvxUnoXBitmapTable::getSupportedServiceNames(  ) throw( uno::RuntimeException)
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.BitmapTable" ) );
    Sequence< OUString > aServices( &aServiceName, 1 );
    return aServices;
}
