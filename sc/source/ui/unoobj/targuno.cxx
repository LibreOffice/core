/*************************************************************************
 *
 *  $RCSfile: targuno.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:08 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include <vcl/image.hxx>
#include <vcl/virdev.hxx>
//#include <toolkit/unoiface.hxx>
#include <toolkit/unohlp.hxx>
#include <svtools/itemprop.hxx>
#include <svtools/smplhint.hxx>

#ifndef _COM_SUN_STAR_AWT_XBITMAP_HPP_
#include <com/sun/star/awt/XBitmap.hpp>
#endif

#include "targuno.hxx"
#include "miscuno.hxx"
#include "docuno.hxx"
#include "datauno.hxx"
#include "nameuno.hxx"
#include "docsh.hxx"
#include "content.hxx"
#include "unoguard.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "unonames.hxx"

using  namespace ::com::sun::star;

//------------------------------------------------------------------------

sal_uInt16 nTypeResIds[SC_LINKTARGETTYPE_COUNT] =
{
    SCSTR_CONTENT_TABLE,        // SC_LINKTARGETTYPE_SHEET
    SCSTR_CONTENT_RANGENAME,    // SC_LINKTARGETTYPE_RANGENAME
    SCSTR_CONTENT_DBAREA        // SC_LINKTARGETTYPE_DBAREA
};

const SfxItemPropertyMap* lcl_GetLinkTargetMap()
{
    static SfxItemPropertyMap aLinkTargetMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNO_LINKDISPBIT),  0,  &getCppuType((const uno::Reference<awt::XBitmap>*)0),   beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNO_LINKDISPNAME), 0,  &getCppuType((const ::rtl::OUString*)0),                beans::PropertyAttribute::READONLY, 0 },
        {0,0,0,0}
    };
    return aLinkTargetMap_Impl;
}

//------------------------------------------------------------------------

// service for ScLinkTargetTypeObj is not defined
//  must not support document::LinkTarget because the target type cannot be used as a target

SC_SIMPLE_SERVICE_INFO( ScLinkTargetTypesObj, "ScLinkTargetTypesObj", "com.sun.star.document.LinkTargets" )
SC_SIMPLE_SERVICE_INFO( ScLinkTargetTypeObj,  "ScLinkTargetTypeObj",  "com.sun.star.document.LinkTargetSupplier" )
SC_SIMPLE_SERVICE_INFO( ScLinkTargetsObj,     "ScLinkTargetsObj",     "com.sun.star.document.LinkTargets" )

//------------------------------------------------------------------------

ScLinkTargetTypesObj::ScLinkTargetTypesObj(ScDocShell* pDocSh) :
    pDocShell( pDocSh )
{
    pDocShell->GetDocument()->AddUnoObject(*this);

    for (sal_uInt16 i=0; i<SC_LINKTARGETTYPE_COUNT; i++)
        aNames[i] = String( ScResId( nTypeResIds[i] ) );
}

ScLinkTargetTypesObj::~ScLinkTargetTypesObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScLinkTargetTypesObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA( SfxSimpleHint ) && ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
        pDocShell = NULL;       // document gone
}

// container::XNameAccess

uno::Any SAL_CALL ScLinkTargetTypesObj::getByName(const rtl::OUString& aName)
        throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
    if (pDocShell)
    {
        String aNameStr = aName;
        for (sal_uInt16 i=0; i<SC_LINKTARGETTYPE_COUNT; i++)
            if ( aNames[i] == aNameStr )
            {
                uno::Reference< beans::XPropertySet >  xInt = new ScLinkTargetTypeObj( pDocShell, i );
                uno::Any aRet;
                aRet <<= xInt;
                return aRet;
            }
    }

    return uno::Any();  //! exception?
}

uno::Sequence<rtl::OUString> SAL_CALL ScLinkTargetTypesObj::getElementNames(void) throw( uno::RuntimeException )
{
    uno::Sequence<rtl::OUString> aRet(SC_LINKTARGETTYPE_COUNT);
    rtl::OUString* pArray = aRet.getArray();
    for (sal_uInt16 i=0; i<SC_LINKTARGETTYPE_COUNT; i++)
        pArray[i] = aNames[i];
    return aRet;
}

sal_Bool SAL_CALL ScLinkTargetTypesObj::hasByName(const rtl::OUString& aName) throw( uno::RuntimeException )
{
    String aNameStr = aName;
    for (sal_uInt16 i=0; i<SC_LINKTARGETTYPE_COUNT; i++)
        if ( aNames[i] == aNameStr )
            return sal_True;
    return sal_False;
}

// container::XElementAccess

uno::Type SAL_CALL ScLinkTargetTypesObj::getElementType(void) throw( uno::RuntimeException )
{
    return ::getCppuType((const uno::Reference< beans::XPropertySet >*)0);
}

sal_Bool SAL_CALL ScLinkTargetTypesObj::hasElements(void) throw( uno::RuntimeException )
{
    return sal_True;
}

//------------------------------------------------------------------------

ScLinkTargetTypeObj::ScLinkTargetTypeObj(ScDocShell* pDocSh, sal_uInt16 nT) :
    pDocShell( pDocSh ),
    nType( nT )
{
    pDocShell->GetDocument()->AddUnoObject(*this);
    aName = String( ScResId( nTypeResIds[nType] ) );    //! on demand?
}

ScLinkTargetTypeObj::~ScLinkTargetTypeObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScLinkTargetTypeObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA( SfxSimpleHint ) && ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
        pDocShell = NULL;       // document gone
}

// document::XLinkTargetSupplier

uno::Reference< container::XNameAccess > SAL_CALL  ScLinkTargetTypeObj::getLinks(void) throw( uno::RuntimeException )
{
    uno::Reference< container::XNameAccess >  xCollection;

    if ( pDocShell )
    {
        switch ( nType )
        {
            case SC_LINKTARGETTYPE_SHEET:
                xCollection = new ScTableSheetsObj(pDocShell);
                break;
            case SC_LINKTARGETTYPE_RANGENAME:
                xCollection = new ScNamedRangesObj(pDocShell);
                break;
            case SC_LINKTARGETTYPE_DBAREA:
                xCollection = new ScDatabaseRangesObj(pDocShell);
                break;
            default:
                DBG_ERROR("invalid type");
        }
    }

    //  wrap collection in ScLinkTargetsObj because service document::LinkTargets requires
    //  beans::XPropertySet as ElementType in container::XNameAccess.
    if ( xCollection.is() )
        return new ScLinkTargetsObj( xCollection );
    return NULL;
}

// beans::XPropertySet

uno::Reference< beans::XPropertySetInfo > SAL_CALL  ScLinkTargetTypeObj::getPropertySetInfo(void) throw( uno::RuntimeException )
{
    ScUnoGuard aGuard;
    static uno::Reference< beans::XPropertySetInfo >  aRef = new SfxItemPropertySetInfo( lcl_GetLinkTargetMap() );
    return aRef;
}

void SAL_CALL ScLinkTargetTypeObj::setPropertyValue(const rtl::OUString& aPropertyName,
            const uno::Any& aValue)
        throw(  beans::UnknownPropertyException,
                beans::PropertyVetoException,
                lang::IllegalArgumentException,
                lang::WrappedTargetException,
                 uno::RuntimeException )
{
    //  everything is read-only
    //! exception?
}

//  static
void ScLinkTargetTypeObj::SetLinkTargetBitmap( uno::Any& rRet, sal_uInt16 nType )
{
    sal_uInt16 nImgId = 0;
    switch ( nType )
    {
        case SC_LINKTARGETTYPE_SHEET:
            nImgId = SC_CONTENT_TABLE;
            break;
        case SC_LINKTARGETTYPE_RANGENAME:
            nImgId = SC_CONTENT_RANGENAME;
            break;
        case SC_LINKTARGETTYPE_DBAREA:
            nImgId = SC_CONTENT_DBAREA;
            break;
    }
    if (nImgId)
    {
        ImageList aEntryImages( ScResId( RID_IMAGELIST_NAVCONT ) );
        const Image& rImage = aEntryImages.GetImage( nImgId );
        Size aSize = rImage.GetSizePixel();

        VirtualDevice aVDev;
        aVDev.SetOutputSizePixel( aSize );
        aVDev.DrawImage( Point(0,0), rImage );
        Bitmap aBitmap = aVDev.GetBitmap( Point(0,0), aSize );

        BitmapEx aBitmapEx( aBitmap );
        uno::Reference< awt::XBitmap > xBmp = VCLUnoHelper::CreateBitmap( aBitmapEx );
        rRet <<= xBmp;
    }
}

uno::Any SAL_CALL ScLinkTargetTypeObj::getPropertyValue(const rtl::OUString& PropertyName)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    uno::Any aRet;
    String aNameStr = PropertyName;
    if ( aNameStr.EqualsAscii( SC_UNO_LINKDISPBIT ) )
        SetLinkTargetBitmap( aRet, nType );
    else if ( aNameStr.EqualsAscii( SC_UNO_LINKDISPNAME ) )
        aRet <<= rtl::OUString( aName );

    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScLinkTargetTypeObj )

//------------------------------------------------------------------------

ScLinkTargetsObj::ScLinkTargetsObj( const uno::Reference< container::XNameAccess > & rColl ) :
    xCollection( rColl )
{
    DBG_ASSERT( xCollection.is(), "ScLinkTargetsObj: NULL" );
}

ScLinkTargetsObj::~ScLinkTargetsObj()
{
}

// container::XNameAccess

uno::Any SAL_CALL ScLinkTargetsObj::getByName(const rtl::OUString& aName)
        throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
    uno::Reference< uno::XInterface >  xInt = ScUnoHelpFunctions::AnyToInterface( xCollection->getByName(aName) );
    uno::Reference< beans::XPropertySet >  xProp( xInt, uno::UNO_QUERY );
    if (xProp.is())
    {
        uno::Any aRet;
        aRet <<= xProp;
        return aRet;
    }

    return uno::Any();  //! exception?
}

uno::Sequence<rtl::OUString> SAL_CALL ScLinkTargetsObj::getElementNames(void) throw( uno::RuntimeException )
{
    return xCollection->getElementNames();
}

sal_Bool SAL_CALL ScLinkTargetsObj::hasByName(const rtl::OUString& aName) throw( uno::RuntimeException )
{
    return xCollection->hasByName(aName);
}

// container::XElementAccess

uno::Type SAL_CALL ScLinkTargetsObj::getElementType(void) throw( uno::RuntimeException )
{
    return ::getCppuType((const uno::Reference< beans::XPropertySet >*)0);
}

sal_Bool SAL_CALL ScLinkTargetsObj::hasElements(void) throw( uno::RuntimeException )
{
    return xCollection->hasElements();
}



