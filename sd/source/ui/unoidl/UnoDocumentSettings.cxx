/*************************************************************************
 *
 *  $RCSfile: UnoDocumentSettings.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: cl $ $Date: 2001-04-06 14:08:56 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
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

#include <stl/vector>

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XMULTIPROPERTYSET_HPP_
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_I18N_XFORBIDDENCHARACTERS_HPP_
#include <com/sun/star/i18n/XForbiddenCharacters.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif

#ifndef _COMPHELPER_PROPERTYSETHELPER_HXX_
#include <comphelper/propertysethelper.hxx>
#endif

#ifndef _COMPHELPER_PROPERTSETINFO_HXX_
#include <comphelper/propertysetinfo.hxx>
#endif

#ifndef _SFXDOCINF_HXX
#include <sfx2/docinf.hxx>
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef _XTABLE_HXX
#include <svx/xtable.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#include "drawdoc.hxx"
#include "docshell.hxx"
#include "unomodel.hxx"

#define MAP_LEN(x) x, sizeof(x)-1

using namespace ::comphelper;
using namespace ::osl;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::i18n;

namespace sd
{
    class DocumentSettings : public WeakImplHelper3< XPropertySet, XMultiPropertySet, XServiceInfo >,
                             public comphelper::PropertySetHelper
    {
    public:
        DocumentSettings( SdXImpressDocument* pModel );
        virtual ~DocumentSettings();

        // XInterface
        virtual Any SAL_CALL queryInterface( const Type& aType ) throw (RuntimeException);
        virtual void SAL_CALL acquire(  ) throw ();
        virtual void SAL_CALL release(  ) throw ();

        // XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        // XMultiPropertySet
    //  virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addPropertiesChangeListener( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removePropertiesChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL firePropertiesChangeEvent( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException);

    protected:
        virtual void _setPropertyValues( const comphelper::PropertyMapEntry** ppEntries, const ::com::sun::star::uno::Any* pValues ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException );
        virtual void _getPropertyValues( const comphelper::PropertyMapEntry** ppEntries, ::com::sun::star::uno::Any* pValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException );

    private:
        Reference< XModel >     mxModel;
        SdXImpressDocument*     mpModel;
    };

    Reference< XInterface > SAL_CALL DocumentSettings_createInstance( SdXImpressDocument* pModel )
        throw( Exception )
    {
        DBG_ASSERT( pModel, "I need a model for the DocumentSettings!" );
        return (XWeak*)new DocumentSettings( pModel );
    }

enum SdDocumentSettingsPropertyHandles
{
    HANDLE_PRINTDRAWING, HANDLE_PRINTNOTES, HANDLE_PRINTHANDOUT, HANDLE_PRINTOUTLINE, HANDLE_MEASUREUNIT, HANDLE_SCALEX,
    HANDLE_SCALEY, HANDLE_TABSTOP, HANDLE_SNAPTO_GRID, HANDLE_SNAPTO_SNAPLINES, HANDLE_SNAPTO_PAGEMARGINS, HANDLE_SNAPTO_OBJECTFRAME,
    HANDLE_SNAPTO_OBJECTPOINTS, HANDLE_SNAPRANGE, HANDLE_SNAPTOGRID, HANDLE_VISIBLEGRID, HANDLE_GRIDRESX, HANDLE_GRIDRESY,
    HANDLE_GRIDSUBX, HANDLE_GRIDSUBY, HANDLE_GRIDAXISSYNC, HANDLE_PRINTPAGENAME, HANDLE_PRINTDATE, HANDLE_PRINTTIME,
    HANDLE_PRINTHIDENPAGES, HANDLE_PRINTFITPAGE, HANDLE_PRINTTILEPAGE, HANDLE_PRINTBOOKLET, HANDLE_PRINTBOOKLETFRONT,
    HANDLE_PRINTBOOKLETBACK, HANDLE_PRINTQUALITY, HANDLE_COLORTABLEURL, HANDLE_DASHTABLEURL, HANDLE_LINEENDTABLEURL, HANDLE_HATCHTABLEURL,
    HANDLE_GRADIENTTABLEURL, HANDLE_BITMAPTABLEURL, HANDLE_FORBIDDENCHARS, HANDLE_APPLYUSERDATA
};

    PropertySetInfo* createSettingsInfoImpl( sal_Bool bIsDraw )
    {
        static PropertyMapEntry aImpressSettingsInfoMap[] =
        {
/*
            { MAP_LEN("IsPrintDrawing"),        HANDLE_PRINTDRAWING,        &::getBooleanCppuType(),                0,  0 },
            { MAP_LEN("IsPrintNotes"),          HANDLE_PRINTNOTES,          &::getBooleanCppuType(),                0,  0 },
            { MAP_LEN("IsPrintHandout"),        HANDLE_PRINTHANDOUT,        &::getBooleanCppuType(),                0,  0 },
            { MAP_LEN("IsPrintOutline"),        HANDLE_PRINTOUTLINE,        &::getBooleanCppuType(),                0,  0 },
*/
            { NULL, 0, 0, NULL, 0, 0 }
        };

        static PropertyMapEntry aDrawSettingsInfoMap[] =
        {
/*
            { MAP_LEN("MeasureUnit"),           HANDLE_MEASUREUNIT,         &::getCppuType((const sal_Int32*)0),    0,  0 },
            { MAP_LEN("ScaleX"),                HANDLE_SCALEX,              &::getCppuType((const sal_Int32*)0),    0,  0 },
            { MAP_LEN("ScaleY"),                HANDLE_SCALEY,              &::getCppuType((const sal_Int32*)0),    0,  0 },
*/
            { NULL, 0, 0, NULL, 0, 0 }
        };

        static PropertyMapEntry aCommonSettingsInfoMap[] =
        {
/*
            { MAP_LEN("DefaultTabStop"),        HANDLE_TABSTOP,             &::getCppuType((const sal_Int32*)0),    0,  0 },
            { MAP_LEN("IsSnapToGrid"),          HANDLE_SNAPTO_GRID,         &::getBooleanCppuType(),                0,  0 },
            { MAP_LEN("IsSnapToSnapLines"),     HANDLE_SNAPTO_SNAPLINES,    &::getBooleanCppuType(),                0,  0 },
            { MAP_LEN("IsSnapToPageMargins"),   HANDLE_SNAPTO_PAGEMARGINS,  &::getBooleanCppuType(),                0,  0 },
            { MAP_LEN("IsSnapToObjectFrame"),   HANDLE_SNAPTO_OBJECTFRAME,  &::getBooleanCppuType(),                0,  0 },
            { MAP_LEN("IsSnapToObjectPoints"),  HANDLE_SNAPTO_OBJECTPOINTS, &::getBooleanCppuType(),                0,  0 },
            { MAP_LEN("SnapRange"),             HANDLE_SNAPRANGE,           &::getCppuType((const sal_Int32*)0),    0,  0 },
            { MAP_LEN("GridIsEnabled"),         HANDLE_SNAPTOGRID,          &::getBooleanCppuType(),                0,  0 },
            { MAP_LEN("GridIsVisible"),         HANDLE_VISIBLEGRID,         &::getBooleanCppuType(),                0,  0 },
            { MAP_LEN("GridResolutionX"),       HANDLE_GRIDRESX,            &::getCppuType((const sal_Int32*)0),    0,  0 },
            { MAP_LEN("GridResolutionY"),       HANDLE_GRIDRESY,            &::getCppuType((const sal_Int32*)0),    0,  0 },
            { MAP_LEN("GridSubdivisionX"),      HANDLE_GRIDSUBX,            &::getCppuType((const sal_Int32*)0),    0,  0 },
            { MAP_LEN("GridSubdivisionY"),      HANDLE_GRIDSUBY,            &::getCppuType((const sal_Int32*)0),    0,  0 },
            { MAP_LEN("IsGridAxisSynchronized"),HANDLE_GRIDAXISSYNC,        &::getBooleanCppuType(),                0,  0 },
            { MAP_LEN("IsPrintPageName"),       HANDLE_PRINTPAGENAME,       &::getBooleanCppuType(),                0,  0 },
            { MAP_LEN("IsPrintDate"),           HANDLE_PRINTDATE,           &::getBooleanCppuType(),                0,  0 },
            { MAP_LEN("IsPrintTime"),           HANDLE_PRINTTIME,           &::getBooleanCppuType(),                0,  0 },
            { MAP_LEN("IsPrintHiddenPages"),    HANDLE_PRINTHIDENPAGES,     &::getBooleanCppuType(),                0,  0 },
            { MAP_LEN("IsPrintFitPage"),        HANDLE_PRINTFITPAGE,        &::getBooleanCppuType(),                0,  0 },
            { MAP_LEN("IsPrintTilePage"),       HANDLE_PRINTTILEPAGE,       &::getBooleanCppuType(),                0,  0 },
            { MAP_LEN("IsPrintBooklet"),        HANDLE_PRINTBOOKLET,        &::getBooleanCppuType(),                0,  0 },
            { MAP_LEN("IsPrintBookletFront"),   HANDLE_PRINTBOOKLETFRONT,   &::getBooleanCppuType(),                0,  0 },
            { MAP_LEN("IsPrintBookletBack"),    HANDLE_PRINTBOOKLETBACK,    &::getBooleanCppuType(),                0,  0 },
            { MAP_LEN("PrintQuality"),          HANDLE_PRINTQUALITY,        &::getCppuType((const sal_Int32*)0),    0,  0 },
*/
            { MAP_LEN("ColorTableURL"),         HANDLE_COLORTABLEURL,       &::getCppuType((const OUString*)0),     0,  0 },
            { MAP_LEN("DashTableURL"),          HANDLE_DASHTABLEURL,        &::getCppuType((const OUString*)0),     0,  0 },
            { MAP_LEN("LineEndTableURL"),       HANDLE_LINEENDTABLEURL,     &::getCppuType((const OUString*)0),     0,  0 },
            { MAP_LEN("HatchTableURL"),         HANDLE_HATCHTABLEURL,       &::getCppuType((const OUString*)0),     0,  0 },
            { MAP_LEN("GradientTableURL"),      HANDLE_GRADIENTTABLEURL,    &::getCppuType((const OUString*)0),     0,  0 },
            { MAP_LEN("BitmapTableURL"),        HANDLE_BITMAPTABLEURL,      &::getCppuType((const OUString*)0),     0,  0 },

            { MAP_LEN("ForbiddenCharacters"),   HANDLE_FORBIDDENCHARS,      &::getCppuType((const Reference< XForbiddenCharacters >*)0),    0, 0 },
            { MAP_LEN("ApplyUserData"),         HANDLE_APPLYUSERDATA,       &::getBooleanCppuType(),                0,  0 },
            { NULL, 0, 0, NULL, 0, 0 }
        };

        PropertySetInfo* pInfo = new PropertySetInfo( aCommonSettingsInfoMap );
        pInfo->add( bIsDraw ? aDrawSettingsInfoMap : aImpressSettingsInfoMap );

        return pInfo;
    }
}

using namespace ::sd;

DocumentSettings::DocumentSettings( SdXImpressDocument* pModel )
:   PropertySetHelper( createSettingsInfoImpl( !pModel->IsImpressDocument() ) ),
    mxModel( pModel ),
    mpModel( pModel )
{
}

DocumentSettings::~DocumentSettings()
{
}

void DocumentSettings::_setPropertyValues( const PropertyMapEntry** ppEntries, const Any* pValues ) throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException )
{
    SdDrawDocument* pDoc = mpModel->GetDoc();
    SdDrawDocShell* pDocSh = mpModel->GetDocShell();
    if( NULL == pDoc || NULL == pDocSh )
        throw UnknownPropertyException();

    sal_Bool bOk, bChanged = sal_False;

    while( *ppEntries )
    {
        bOk = sal_False;
        switch( (*ppEntries)->mnHandle )
        {
            case HANDLE_COLORTABLEURL:
                {
                    OUString aURLString;
                    if( *pValues >>= aURLString )
                    {
                        INetURLObject aURL( aURLString );
                        INetURLObject aPathURL( aURL );

                        aPathURL.removeSegment();
                        aPathURL.removeFinalSlash();

                        XColorTable* pColTab = new XColorTable( aPathURL.GetMainURL(), (XOutdevItemPool*)&pDoc->GetPool() );
                        pColTab->SetName( aURL.getName() );
                        if( pColTab->Load() )
                        {
                            pDoc->SetColorTable( pColTab );
                            bOk = sal_True;
                            bChanged = sal_True;
                        }
                    }
                }
                break;
            case HANDLE_DASHTABLEURL:
                {
                    OUString aURLString;
                    if( *pValues >>= aURLString )
                    {
                        INetURLObject aURL( aURLString );
                        INetURLObject aPathURL( aURL );

                        aPathURL.removeSegment();
                        aPathURL.removeFinalSlash();

                        XDashList* pDashTab = new XDashList( aPathURL.GetMainURL(), (XOutdevItemPool*)&pDoc->GetPool() );
                        pDashTab->SetName( aURL.getName() );
                        if( pDashTab->Load() )
                        {
                            pDoc->SetDashList( pDashTab );
                            bOk = sal_True;
                            bChanged = sal_True;
                        }
                    }
                }
                break;
            case HANDLE_LINEENDTABLEURL:
                {
                    OUString aURLString;
                    if( *pValues >>= aURLString )
                    {
                        INetURLObject aURL( aURLString );
                        INetURLObject aPathURL( aURL );

                        aPathURL.removeSegment();
                        aPathURL.removeFinalSlash();

                        XLineEndList* pTab = new XLineEndList( aPathURL.GetMainURL(), (XOutdevItemPool*)&pDoc->GetPool() );
                        pTab->SetName( aURL.getName() );
                        if( pTab->Load() )
                        {
                            pDoc->SetLineEndList( pTab );
                            bOk = sal_True;
                            bChanged = sal_True;
                        }
                    }
                }
                break;
            case HANDLE_HATCHTABLEURL:
                {
                    OUString aURLString;
                    if( *pValues >>= aURLString )
                    {
                        INetURLObject aURL( aURLString );
                        INetURLObject aPathURL( aURL );

                        aPathURL.removeSegment();
                        aPathURL.removeFinalSlash();

                        XHatchList* pTab = new XHatchList( aPathURL.GetMainURL(), (XOutdevItemPool*)&pDoc->GetPool() );
                        pTab->SetName( aURL.getName() );
                        if( pTab->Load() )
                        {
                            pDoc->SetHatchList( pTab );
                            bOk = sal_True;
                            bChanged = sal_True;
                        }
                    }
                }
                break;
            case HANDLE_GRADIENTTABLEURL:
                {
                    OUString aURLString;
                    if( *pValues >>= aURLString )
                    {
                        INetURLObject aURL( aURLString );
                        INetURLObject aPathURL( aURL );

                        aPathURL.removeSegment();
                        aPathURL.removeFinalSlash();

                        XGradientList* pTab = new XGradientList( aPathURL.GetMainURL(), (XOutdevItemPool*)&pDoc->GetPool() );
                        pTab->SetName( aURL.getName() );
                        if( pTab->Load() )
                        {
                            pDoc->SetGradientList( pTab );
                            bOk = sal_True;
                            bChanged = sal_True;
                        }
                    }
                }
                break;
            case HANDLE_BITMAPTABLEURL:
                {
                    OUString aURLString;
                    if( *pValues >>= aURLString )
                    {
                        INetURLObject aURL( aURLString );
                        INetURLObject aPathURL( aURL );

                        aPathURL.removeSegment();
                        aPathURL.removeFinalSlash();

                        XBitmapList* pTab = new XBitmapList( aPathURL.GetMainURL(), (XOutdevItemPool*)&pDoc->GetPool() );
                        pTab->SetName( aURL.getName() );
                        if( pTab->Load() )
                        {
                            pDoc->SetBitmapList( pTab );
                            bOk = sal_True;
                            bChanged = sal_True;
                        }
                    }
                }
                break;
            case HANDLE_FORBIDDENCHARS:
                {
                    bOk = sal_True;
                }
                break;
            case HANDLE_APPLYUSERDATA:
                {
                    sal_Bool bApplyUserData;
                    if( *pValues >>= bApplyUserData )
                    {
                        SfxDocumentInfo& rInfo = pDocSh->GetDocInfo();
                        if( rInfo.IsUseUserData() != bApplyUserData )
                        {
                            rInfo.SetUseUserData( bApplyUserData );
                            bChanged = sal_True;
                        }
                        bOk = sal_True;
                    }
                }
                break;
            case HANDLE_PRINTDRAWING:
            case HANDLE_PRINTNOTES:
            case HANDLE_PRINTHANDOUT:
            case HANDLE_PRINTOUTLINE:
            case HANDLE_MEASUREUNIT:
            case HANDLE_SCALEX:
            case HANDLE_SCALEY:
            case HANDLE_TABSTOP:
            case HANDLE_SNAPTO_GRID:
            case HANDLE_SNAPTO_SNAPLINES:
            case HANDLE_SNAPTO_PAGEMARGINS:
            case HANDLE_SNAPTO_OBJECTFRAME:
            case HANDLE_SNAPTO_OBJECTPOINTS:
            case HANDLE_SNAPRANGE:
            case HANDLE_SNAPTOGRID:
            case HANDLE_VISIBLEGRID:
            case HANDLE_GRIDRESX:
            case HANDLE_GRIDRESY:
            case HANDLE_GRIDSUBX:
            case HANDLE_GRIDSUBY:
            case HANDLE_GRIDAXISSYNC:
            case HANDLE_PRINTPAGENAME:
            case HANDLE_PRINTDATE:
            case HANDLE_PRINTTIME:
            case HANDLE_PRINTHIDENPAGES:
            case HANDLE_PRINTFITPAGE:
            case HANDLE_PRINTTILEPAGE:
            case HANDLE_PRINTBOOKLET:
            case HANDLE_PRINTBOOKLETFRONT:
            case HANDLE_PRINTBOOKLETBACK:
            case HANDLE_PRINTQUALITY:
            default:
                throw UnknownPropertyException();

        }

        if( !bOk )
            throw IllegalArgumentException();

        ppEntries++;
        pValues++;
    }

    if( bChanged )
        mpModel->SetModified( sal_True );
}

void DocumentSettings::_getPropertyValues( const PropertyMapEntry** ppEntries, Any* pValue ) throw(UnknownPropertyException, WrappedTargetException )
{
    SdDrawDocument* pDoc = mpModel->GetDoc();
    SdDrawDocShell* pDocSh = mpModel->GetDocShell();
    if( NULL == pDoc || NULL == pDocSh )
        throw UnknownPropertyException();

    while( *ppEntries )
    {
        switch( (*ppEntries)->mnHandle )
        {
            case HANDLE_COLORTABLEURL:
                {
                    INetURLObject aPathURL( pDoc->GetColorTable()->GetPath() );
                    aPathURL.insertName( pDoc->GetColorTable()->GetName() );
                    String aExt( RTL_CONSTASCII_USTRINGPARAM("soc") );
                    aPathURL.setExtension( aExt );
                    OUString aPath( aPathURL.GetMainURL() );
                    *pValue <<= aPath;
                }
                break;
            case HANDLE_DASHTABLEURL:
                {
                    INetURLObject aPathURL( pDoc->GetDashList()->GetPath() );
                    aPathURL.insertName( pDoc->GetDashList()->GetName() );
                    String aExt( RTL_CONSTASCII_USTRINGPARAM("sod") );
                    aPathURL.setExtension( aExt );
                    OUString aPath( aPathURL.GetMainURL() );
                    *pValue <<= aPath;
                }
                break;
            case HANDLE_LINEENDTABLEURL:
                {
                    INetURLObject aPathURL( pDoc->GetLineEndList()->GetPath() );
                    aPathURL.insertName( pDoc->GetLineEndList()->GetName() );
                    String aExt( RTL_CONSTASCII_USTRINGPARAM("soe") );
                    aPathURL.setExtension( aExt );
                    OUString aPath( aPathURL.GetMainURL() );
                    *pValue <<= aPath;
                }
                break;
            case HANDLE_HATCHTABLEURL:
                {
                    INetURLObject aPathURL( pDoc->GetHatchList()->GetPath() );
                    aPathURL.insertName( pDoc->GetHatchList()->GetName() );
                    String aExt( RTL_CONSTASCII_USTRINGPARAM("soh") );
                    aPathURL.setExtension( aExt );
                    OUString aPath( aPathURL.GetMainURL() );
                    *pValue <<= aPath;
                }
                break;
            case HANDLE_GRADIENTTABLEURL:
                {
                    INetURLObject aPathURL( pDoc->GetGradientList()->GetPath() );
                    aPathURL.insertName( pDoc->GetGradientList()->GetName() );
                    String aExt( RTL_CONSTASCII_USTRINGPARAM("sog") );
                    aPathURL.setExtension( aExt );
                    OUString aPath( aPathURL.GetMainURL() );
                    *pValue <<= aPath;
                }
                break;
            case HANDLE_BITMAPTABLEURL:
                {
                    INetURLObject aPathURL( pDoc->GetBitmapList()->GetPath() );
                    aPathURL.insertName( pDoc->GetBitmapList()->GetName() );
                    String aExt( RTL_CONSTASCII_USTRINGPARAM("sob") );
                    aPathURL.setExtension( aExt );
                    OUString aPath( aPathURL.GetMainURL() );
                    *pValue <<= aPath;
                }
                break;
            case HANDLE_FORBIDDENCHARS:
                *pValue <<= mpModel->getForbiddenCharsTable();
                break;

            case HANDLE_APPLYUSERDATA:
                *pValue <<= (sal_Bool)pDocSh->GetDocInfo().IsUseUserData();
                break;

            case HANDLE_PRINTDRAWING:
            case HANDLE_PRINTNOTES:
            case HANDLE_PRINTHANDOUT:
            case HANDLE_PRINTOUTLINE:
            case HANDLE_MEASUREUNIT:
            case HANDLE_SCALEX:
            case HANDLE_SCALEY:
            case HANDLE_TABSTOP:
            case HANDLE_SNAPTO_GRID:
            case HANDLE_SNAPTO_SNAPLINES:
            case HANDLE_SNAPTO_PAGEMARGINS:
            case HANDLE_SNAPTO_OBJECTFRAME:
            case HANDLE_SNAPTO_OBJECTPOINTS:
            case HANDLE_SNAPRANGE:
            case HANDLE_SNAPTOGRID:
            case HANDLE_VISIBLEGRID:
            case HANDLE_GRIDRESX:
            case HANDLE_GRIDRESY:
            case HANDLE_GRIDSUBX:
            case HANDLE_GRIDSUBY:
            case HANDLE_GRIDAXISSYNC:
            case HANDLE_PRINTPAGENAME:
            case HANDLE_PRINTDATE:
            case HANDLE_PRINTTIME:
            case HANDLE_PRINTHIDENPAGES:
            case HANDLE_PRINTFITPAGE:
            case HANDLE_PRINTTILEPAGE:
            case HANDLE_PRINTBOOKLET:
            case HANDLE_PRINTBOOKLETFRONT:
            case HANDLE_PRINTBOOKLETBACK:
            case HANDLE_PRINTQUALITY:
            default:
                throw UnknownPropertyException();
        }

        ppEntries++;
        pValue++;
    }
}

// XInterface
Any SAL_CALL DocumentSettings::queryInterface( const Type& aType ) throw (RuntimeException)
{
    return WeakImplHelper3< XPropertySet, XMultiPropertySet, XServiceInfo >::queryInterface( aType );
}

void SAL_CALL DocumentSettings::acquire(  ) throw ()
{
    WeakImplHelper3< XPropertySet, XMultiPropertySet, XServiceInfo >::acquire();
}

void SAL_CALL DocumentSettings::release(  ) throw ()
{
    WeakImplHelper3< XPropertySet, XMultiPropertySet, XServiceInfo >::release();
}


// XPropertySet
Reference< XPropertySetInfo > SAL_CALL DocumentSettings::getPropertySetInfo(  ) throw(RuntimeException)
{
    return PropertySetHelper::getPropertySetInfo();
}

void SAL_CALL DocumentSettings::setPropertyValue( const OUString& aPropertyName, const Any& aValue ) throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    PropertySetHelper::setPropertyValue( aPropertyName, aValue );
}

Any SAL_CALL DocumentSettings::getPropertyValue( const OUString& PropertyName ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    return PropertySetHelper::getPropertyValue( PropertyName );
}

void SAL_CALL DocumentSettings::addPropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    PropertySetHelper::addPropertyChangeListener( aPropertyName, xListener );
}

void SAL_CALL DocumentSettings::removePropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& aListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    PropertySetHelper::removePropertyChangeListener( aPropertyName, aListener );
}

void SAL_CALL DocumentSettings::addVetoableChangeListener( const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    PropertySetHelper::addVetoableChangeListener( PropertyName, aListener );
}

void SAL_CALL DocumentSettings::removeVetoableChangeListener( const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    PropertySetHelper::removeVetoableChangeListener( PropertyName, aListener );
}

// XMultiPropertySet
void SAL_CALL DocumentSettings::setPropertyValues( const Sequence< OUString >& aPropertyNames, const Sequence< Any >& aValues ) throw(PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    PropertySetHelper::setPropertyValues( aPropertyNames, aValues );
}

Sequence< Any > SAL_CALL DocumentSettings::getPropertyValues( const Sequence< OUString >& aPropertyNames ) throw(RuntimeException)
{
    return PropertySetHelper::getPropertyValues( aPropertyNames );
}

void SAL_CALL DocumentSettings::addPropertiesChangeListener( const Sequence< OUString >& aPropertyNames, const Reference< XPropertiesChangeListener >& xListener ) throw(RuntimeException)
{
    PropertySetHelper::addPropertiesChangeListener( aPropertyNames, xListener );
}

void SAL_CALL DocumentSettings::removePropertiesChangeListener( const Reference< XPropertiesChangeListener >& xListener ) throw(RuntimeException)
{
    PropertySetHelper::removePropertiesChangeListener( xListener );
}

void SAL_CALL DocumentSettings::firePropertiesChangeEvent( const Sequence< OUString >& aPropertyNames, const Reference< XPropertiesChangeListener >& xListener ) throw(RuntimeException)
{
    PropertySetHelper::firePropertiesChangeEvent( aPropertyNames, xListener );
}

// XServiceInfo
OUString SAL_CALL DocumentSettings::getImplementationName(  )
    throw(RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Draw.DocumentSettings"));
}

sal_Bool SAL_CALL DocumentSettings::supportsService( const OUString& ServiceName )
    throw(RuntimeException)
{
    const Sequence< OUString > aSeq( getSupportedServiceNames() );
    sal_Int32 nCount = aSeq.getLength();
    const OUString* pServices = aSeq.getConstArray();
    while( nCount-- )
    {
        if( *pServices++ == ServiceName )
            return sal_True;
    }

    return sal_True;
}

Sequence< OUString > SAL_CALL DocumentSettings::getSupportedServiceNames(  )
    throw(RuntimeException)
{
    Sequence< OUString > aSeq( 2 );
    aSeq[0] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.Settings") );
    if( mpModel->IsImpressDocument() )
    {
        aSeq[1] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Impress.Settings") );
    }
    else
    {
        aSeq[1] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Draw.Settings") );
    }

    return aSeq;
}

