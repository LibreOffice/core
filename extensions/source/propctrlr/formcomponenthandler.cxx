/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "controltype.hxx"
#include "propctrlr.hrc"
#include "extensio.hrc"
#include "fontdialog.hxx"
#include "formcomponenthandler.hxx"
#include "formlinkdialog.hxx"
#include "formmetadata.hxx"
#include "formresid.hrc"
#include "formstrings.hxx"
#include "handlerhelper.hxx"
#include "listselectiondlg.hxx"
#include "pcrcommon.hxx"
#include "selectlabeldialog.hxx"
#include "taborder.hxx"
#include "usercontrol.hxx"

#include <com/sun/star/lang/NullPointerException.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/sdb/OrderDialog.hpp>
#include <com/sun/star/sdb/FilterDialog.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/form/XGridColumnFactory.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/form/ListSourceType.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/awt/XTabControllerModel.hpp>
#include <com/sun/star/form/FormSubmitEncoding.hpp>
#include <com/sun/star/awt/VisualEffect.hpp>
#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/inspection/PropertyControlType.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/inspection/XObjectInspectorUI.hpp>
#include <com/sun/star/inspection/PropertyLineElement.hpp>
#include <com/sun/star/resource/XStringResourceManager.hpp>
#include <com/sun/star/resource/MissingResourceException.hpp>
#include <com/sun/star/graphic/GraphicObject.hpp>
#include <com/sun/star/text/WritingMode2.hpp>

#include <comphelper/extract.hxx>
#include <connectivity/dbconversion.hxx>
#include <connectivity/dbexception.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <sfx2/app.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/filedlghelper.hxx>
#include <svl/ctloptions.hxx>
#include <svtools/colrdlg.hxx>
#include <svl/filenotation.hxx>
#include <svl/intitem.hxx>
#include <svl/itemset.hxx>
#include <unotools/moduleoptions.hxx>
#include <svl/numuno.hxx>
#include <svl/urihelper.hxx>
#include <svx/dialogs.hrc>
#include <svx/numinf.hxx>
#include <svx/svxdlg.hxx>
#include <svx/svxids.hrc>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/msgbox.hxx>
#include <vcl/stdtext.hxx>
#include <vcl/wrkwin.hxx>
#include <tools/StringListResource.hxx>
#include <sal/macros.h>

#include <limits>

#define GRAPHOBJ_URLPREFIX "vnd.sun.star.GraphicObject:"
//------------------------------------------------------------------------
extern "C" void SAL_CALL createRegistryInfo_FormComponentPropertyHandler()
{
    ::pcr::FormComponentPropertyHandler::registerImplementation();
}

//........................................................................
namespace pcr
{
//........................................................................

    using namespace ::com::sun::star;
    using namespace uno;
    using namespace lang;
    using namespace beans;
    using namespace frame;
    using namespace script;
    using namespace form;
    using namespace util;
    using namespace awt;
    using namespace sdb;
    using namespace sdbc;
    using namespace sdbcx;
    using namespace container;
    using namespace ui::dialogs;
    using namespace inspection;
    using namespace ::dbtools;

    namespace WritingMode2 = ::com::sun::star::text::WritingMode2;

    //====================================================================
    //= FormComponentPropertyHandler
    //====================================================================
    DBG_NAME( FormComponentPropertyHandler )
#define PROPERTY_ID_ROWSET 1
    //--------------------------------------------------------------------
    FormComponentPropertyHandler::FormComponentPropertyHandler( const Reference< XComponentContext >& _rxContext )
        :FormComponentPropertyHandler_Base( _rxContext )
        ,::comphelper::OPropertyContainer(FormComponentPropertyHandler_Base::rBHelper)
        ,m_sDefaultValueString( PcrRes(RID_STR_STANDARD).toString() )
        ,m_eComponentClass( eUnknown )
        ,m_bComponentIsSubForm( false )
        ,m_bHaveListSource( false )
        ,m_bHaveCommand( false )
        ,m_nClassId( 0 )
    {
        DBG_CTOR( FormComponentPropertyHandler, NULL );
        registerProperty(PROPERTY_ROWSET,PROPERTY_ID_ROWSET,0,&m_xRowSet,::getCppuType(&m_xRowSet));
    }

    //--------------------------------------------------------------------
    FormComponentPropertyHandler::~FormComponentPropertyHandler()
    {
        DBG_DTOR( FormComponentPropertyHandler, NULL );
    }
    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_XINTERFACE2(FormComponentPropertyHandler,FormComponentPropertyHandler_Base,::comphelper::OPropertyContainer)
    //--------------------------------------------------------------------
    OUString SAL_CALL FormComponentPropertyHandler::getImplementationName_static(  ) throw (RuntimeException)
    {
        return OUString(  "com.sun.star.comp.extensions.FormComponentPropertyHandler"  );
    }

    //--------------------------------------------------------------------
    Sequence< OUString > SAL_CALL FormComponentPropertyHandler::getSupportedServiceNames_static(  ) throw (RuntimeException)
    {
        Sequence< OUString > aSupported( 1 );
        aSupported[0] = OUString(  "com.sun.star.form.inspection.FormComponentPropertyHandler"  );
        return aSupported;
    }

    //============================================
    // TODO: -> export from toolkit
    struct LanguageDependentProp
    {
        const char* pPropName;
        sal_Int32   nPropNameLength;
    };

    static const LanguageDependentProp aLanguageDependentProp[] =
    {
        { "Text",            4 },
        { "Label",           5 },
        { "Title",           5 },
        { "HelpText",        8 },
        { "CurrencySymbol", 14 },
        { "StringItemList", 14 },
        { 0, 0                 }
    };

    namespace
    {
        bool lcl_isLanguageDependentProperty( OUString aName )
        {
            bool bRet = false;

            const LanguageDependentProp* pLangDepProp = aLanguageDependentProp;
            while( pLangDepProp->pPropName != 0 )
            {
                if( aName.equalsAsciiL( pLangDepProp->pPropName, pLangDepProp->nPropNameLength ))
                {
                    bRet = true;
                    break;
                }
                pLangDepProp++;
            }
            return bRet;
        }

        Reference< resource::XStringResourceResolver > lcl_getStringResourceResolverForProperty
            ( Reference< XPropertySet > _xComponent, const OUString& _rPropertyName,
              const Any& _rPropertyValue )
        {
            Reference< resource::XStringResourceResolver > xRet;
            const TypeClass eType = _rPropertyValue.getValueType().getTypeClass();
            if ( (eType == TypeClass_STRING || eType == TypeClass_SEQUENCE) &&
                    lcl_isLanguageDependentProperty( _rPropertyName ) )
            {
                static const OUString s_sResourceResolverPropName("ResourceResolver");

                Reference< resource::XStringResourceResolver > xStringResourceResolver;
                try
                {
                    xStringResourceResolver.set( _xComponent->getPropertyValue( s_sResourceResolverPropName ),UNO_QUERY);
                    if( xStringResourceResolver.is() &&
                        xStringResourceResolver->getLocales().getLength() > 0 )
                    {
                        xRet = xStringResourceResolver;
                    }
                }
                catch(const UnknownPropertyException&)
                {
                    // nii
                }
            }

            return xRet;
        }
    }

    //--------------------------------------------------------------------
    Any FormComponentPropertyHandler::impl_getPropertyValue_throw( const OUString& _rPropertyName ) const
    {
        const PropertyId nPropId( impl_getPropertyId_throw( _rPropertyName ) );

        Any aPropertyValue( m_xComponent->getPropertyValue( _rPropertyName ) );

        Reference< resource::XStringResourceResolver > xStringResourceResolver
            = lcl_getStringResourceResolverForProperty( m_xComponent, _rPropertyName, aPropertyValue );
        if( xStringResourceResolver.is() )
        {
            TypeClass eType = aPropertyValue.getValueType().getTypeClass();
            if( eType == TypeClass_STRING )
            {
                OUString aPropStr;
                aPropertyValue >>= aPropStr;
                if( aPropStr.getLength() > 1 )
                {
                    OUString aPureIdStr = aPropStr.copy( 1 );
                    if( xStringResourceResolver->hasEntryForId( aPureIdStr ) )
                    {
                        OUString aResourceStr = xStringResourceResolver->resolveString( aPureIdStr );
                        aPropertyValue <<= aResourceStr;
                    }
                }
            }
            // StringItemList?
            else if( eType == TypeClass_SEQUENCE )
            {
                Sequence< OUString > aStrings;
                aPropertyValue >>= aStrings;

                const OUString* pStrings = aStrings.getConstArray();
                sal_Int32 nCount = aStrings.getLength();

                Sequence< OUString > aResolvedStrings;
                aResolvedStrings.realloc( nCount );
                OUString* pResolvedStrings = aResolvedStrings.getArray();
                try
                {
                    for ( sal_Int32 i = 0; i < nCount; ++i )
                    {
                        OUString aIdStr = pStrings[i];
                        OUString aPureIdStr = aIdStr.copy( 1 );
                        if( xStringResourceResolver->hasEntryForId( aPureIdStr ) )
                            pResolvedStrings[i] = xStringResourceResolver->resolveString( aPureIdStr );
                        else
                            pResolvedStrings[i] = aIdStr;
                    }
                }
                catch( const resource::MissingResourceException & )
                {}
                aPropertyValue <<= aResolvedStrings;
            }
        }
        else
            impl_normalizePropertyValue_nothrow( aPropertyValue, nPropId );

        return aPropertyValue;
    }

    //--------------------------------------------------------------------
    Any SAL_CALL FormComponentPropertyHandler::getPropertyValue( const OUString& _rPropertyName ) throw (UnknownPropertyException, RuntimeException)
    {
        if( _rPropertyName == PROPERTY_ROWSET )
            return ::comphelper::OPropertyContainer::getPropertyValue( _rPropertyName );

        ::osl::MutexGuard aGuard( m_aMutex );
        return impl_getPropertyValue_throw( _rPropertyName );
    }

    //--------------------------------------------------------------------
    void SAL_CALL FormComponentPropertyHandler::setPropertyValue( const OUString& _rPropertyName, const Any& _rValue ) throw (UnknownPropertyException, RuntimeException)
    {
        if( _rPropertyName == PROPERTY_ROWSET )
        {
            ::comphelper::OPropertyContainer::setPropertyValue( _rPropertyName, _rValue );
            return;
        }

        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyId nPropId( impl_getPropertyId_throw( _rPropertyName ) ); // check if property is known by the handler

        Reference< graphic::XGraphicObject > xGrfObj;
        if ( PROPERTY_ID_IMAGE_URL == nPropId && ( _rValue >>= xGrfObj ) )
        {
            DBG_ASSERT( xGrfObj.is(), "FormComponentPropertyHandler::setPropertyValue() xGrfObj is invalid");
            OUString sObjectID(  GRAPHOBJ_URLPREFIX  );
            sObjectID = sObjectID + xGrfObj->getUniqueID();
            m_xComponent->setPropertyValue( _rPropertyName, uno::makeAny( sObjectID ) );
        }
        else if ( PROPERTY_ID_FONT == nPropId )
        {
            // special handling, the value is a faked value we generated ourself in impl_executeFontDialog_nothrow
            Sequence< NamedValue > aFontPropertyValues;
            OSL_VERIFY( _rValue >>= aFontPropertyValues );
            const NamedValue* fontPropertyValue = aFontPropertyValues.getConstArray();
            const NamedValue* fontPropertyValueEnd = fontPropertyValue + aFontPropertyValues.getLength();
            for ( ; fontPropertyValue != fontPropertyValueEnd; ++fontPropertyValue )
                m_xComponent->setPropertyValue( fontPropertyValue->Name, fontPropertyValue->Value );
        }
        else
        {
            Any aValue = _rValue;

            Reference< resource::XStringResourceResolver > xStringResourceResolver
                = lcl_getStringResourceResolverForProperty( m_xComponent, _rPropertyName, _rValue );
            if( xStringResourceResolver.is() )
            {
                Reference< resource::XStringResourceManager >
                    xStringResourceManager( xStringResourceResolver, UNO_QUERY );
                if( xStringResourceManager.is() )
                {
                    Any aPropertyValue( m_xComponent->getPropertyValue( _rPropertyName ) );
                    TypeClass eType = aPropertyValue.getValueType().getTypeClass();
                    if( eType == TypeClass_STRING )
                    {
                        OUString aPropStr;
                        aPropertyValue >>= aPropStr;
                        if( aPropStr.getLength() > 1 )
                        {
                            OUString aPureIdStr = aPropStr.copy( 1 );
                            OUString aValueStr;
                            _rValue >>= aValueStr;
                            xStringResourceManager->setString( aPureIdStr, aValueStr );
                            aValue = aPropertyValue;    // set value to force modified
                        }
                    }
                    // StringItemList?
                    else if( eType == TypeClass_SEQUENCE )
                    {
                        static OUString aDot(".");
                        static OUString aEsc("&");

                        // Put strings into resource using new ids
                        Sequence< OUString > aNewStrings;
                        _rValue >>= aNewStrings;

                        const OUString* pNewStrings = aNewStrings.getConstArray();
                        sal_Int32 nNewCount = aNewStrings.getLength();

                        // Create new Ids
                        OUString* pNewPureIds = new OUString[nNewCount];
                        OUString aIdStrBase = aDot;
                        Any aNameAny = m_xComponent->getPropertyValue(PROPERTY_NAME);
                        OUString sControlName;
                        aNameAny >>= sControlName;
                        aIdStrBase += sControlName;
                        aIdStrBase += aDot;
                        aIdStrBase += _rPropertyName;
                        sal_Int32 i;
                        OUString aDummyStr;
                        for ( i = 0; i < nNewCount; ++i )
                        {
                            sal_Int32 nUniqueId = xStringResourceManager->getUniqueNumericId();
                            OUString aPureIdStr = OUString::valueOf( nUniqueId );
                            aPureIdStr += aIdStrBase;
                            pNewPureIds[i] = aPureIdStr;
                            // Force usage of next Unique Id
                            xStringResourceManager->setString( aPureIdStr, aDummyStr );
                        }

                        // Move strings to new Ids for all locales
                        Sequence< Locale > aLocaleSeq = xStringResourceManager->getLocales();
                        const Locale* pLocale = aLocaleSeq.getConstArray();
                        sal_Int32 nLocaleCount = aLocaleSeq.getLength();
                        Sequence< OUString > aOldIdStrings;
                        aPropertyValue >>= aOldIdStrings;
                        try
                        {
                            const OUString* pOldIdStrings = aOldIdStrings.getConstArray();
                            sal_Int32 nOldIdCount = aOldIdStrings.getLength();
                            for ( i = 0; i < nNewCount; ++i )
                            {
                                OUString aOldIdStr;
                                OUString aOldPureIdStr;
                                if( i < nOldIdCount )
                                {
                                    aOldIdStr = pOldIdStrings[i];
                                    aOldPureIdStr = aOldIdStr.copy( 1 );
                                }
                                OUString aNewPureIdStr = pNewPureIds[i];

                                for ( sal_Int32 iLocale = 0; iLocale < nLocaleCount; ++iLocale )
                                {
                                    Locale aLocale = pLocale[iLocale];

                                    OUString aResourceStr;
                                    if( !aOldPureIdStr.isEmpty() )
                                    {
                                        if( xStringResourceManager->hasEntryForIdAndLocale( aOldPureIdStr, aLocale ) )
                                        {
                                            aResourceStr = xStringResourceManager->
                                                resolveStringForLocale( aOldPureIdStr, aLocale );
                                        }
                                    }
                                    xStringResourceManager->setStringForLocale( aNewPureIdStr, aResourceStr, aLocale );
                                }
                            }
                        }
                        catch( const resource::MissingResourceException & )
                        {}


                        // Set new strings for current locale and create
                        // new Id sequence as new property value
                        Sequence< OUString > aNewIdStrings;
                        aNewIdStrings.realloc( nNewCount );
                        OUString* pNewIdStrings = aNewIdStrings.getArray();
                        for ( i = 0; i < nNewCount; ++i )
                        {
                            OUString aPureIdStr = pNewPureIds[i];
                            OUString aStr = pNewStrings[i];
                            xStringResourceManager->setString( aPureIdStr, aStr );

                            OUString aIdStr = aEsc;
                            aIdStr += aPureIdStr;
                            pNewIdStrings[i] = aIdStr;
                        }
                        aValue <<= aNewIdStrings;

                        // Remove old ids from resource for all locales
                        const OUString* pOldIdStrings = aOldIdStrings.getConstArray();
                        sal_Int32 nOldIdCount = aOldIdStrings.getLength();
                        for( i = 0 ; i < nOldIdCount ; ++i )
                        {
                            OUString aIdStr = pOldIdStrings[i];
                            OUString aPureIdStr = aIdStr.copy( 1 );
                            for ( sal_Int32 iLocale = 0; iLocale < nLocaleCount; ++iLocale )
                            {
                                Locale aLocale = pLocale[iLocale];
                                try
                                {
                                    xStringResourceManager->removeIdForLocale( aPureIdStr, aLocale );
                                }
                                catch( const resource::MissingResourceException & )
                                {}
                            }
                        }
                        delete[] pNewPureIds;
                    }
                }
            }

            m_xComponent->setPropertyValue( _rPropertyName, aValue );
        }
    }

    //--------------------------------------------------------------------
    Any SAL_CALL FormComponentPropertyHandler::convertToPropertyValue( const OUString& _rPropertyName, const Any& _rControlValue ) throw (UnknownPropertyException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyId nPropId( impl_getPropertyId_throw( _rPropertyName ) );
        Property aProperty( impl_getPropertyFromId_throw( nPropId ) );

        Any aPropertyValue( _rControlValue );
        if ( !aPropertyValue.hasValue() )
        {
            if ( ( aProperty.Attributes & PropertyAttribute::MAYBEVOID ) == 0 )
                // default construct an instance of the proper type
                aPropertyValue = Any( NULL, aProperty.Type );
            // nothing to do
            return aPropertyValue;
        }

        /// care for the special "default" string, translate it to VOID
        if ( m_aPropertiesWithDefListEntry.find( _rPropertyName ) != m_aPropertiesWithDefListEntry.end() )
        {
            // it's a control with a string list
            OUString sStringValue;
            if ( _rControlValue >>= sStringValue )
            {   // note that ColorListBoxes might transfer values either as string or as css.util.Color,
                // so this check here is important
                if ( sStringValue == m_sDefaultValueString )
                    return Any();
            }
        }

        switch ( nPropId )
        {
        case PROPERTY_ID_DATASOURCE:
        {
            OUString sControlValue;
            OSL_VERIFY( _rControlValue >>= sControlValue );

            if ( !sControlValue.isEmpty() )
            {
                Reference< XDatabaseContext > xDatabaseContext = sdb::DatabaseContext::create( m_xContext );
                if ( !xDatabaseContext->hasByName( sControlValue ) )
                {
                    ::svt::OFileNotation aTransformer(sControlValue);
                    aPropertyValue <<= OUString( aTransformer.get( ::svt::OFileNotation::N_URL ) );
                }
            }
        }
        break;  // case PROPERTY_ID_DATASOURCE

        case PROPERTY_ID_SHOW_POSITION:
        case PROPERTY_ID_SHOW_NAVIGATION:
        case PROPERTY_ID_SHOW_RECORDACTIONS:
        case PROPERTY_ID_SHOW_FILTERSORT:
        {
            OUString sControlValue;
            OSL_VERIFY( _rControlValue >>= sControlValue );

            ::std::vector< OUString > aListEntries;
            tools::StringListResource aRes( PcrRes( RID_RSC_ENUM_SHOWHIDE ), aListEntries );
            OSL_ENSURE( aListEntries.size() == 2, "FormComponentPropertyHandler::convertToPropertyValue: broken resource for Show/Hide!" );
            sal_Bool bShow = ( aListEntries.size() < 2 ) || ( sControlValue == aListEntries[1] );

            aPropertyValue <<= bShow;
        }
        break;

        case PROPERTY_ID_TARGET_URL:
        case PROPERTY_ID_IMAGE_URL:
        {
            OUString sControlValue;
            OSL_VERIFY( _rControlValue >>= sControlValue );
            // Don't convert a placeholder
            if ( nPropId == PROPERTY_ID_IMAGE_URL && sControlValue.equals( PcrRes(RID_EMBED_IMAGE_PLACEHOLDER).toString() ) )
                aPropertyValue <<= sControlValue;
            else
            {
                INetURLObject aDocURL( impl_getDocumentURL_nothrow() );
                aPropertyValue <<= (OUString)URIHelper::SmartRel2Abs( aDocURL, sControlValue, Link(), false, true, INetURLObject::WAS_ENCODED, INetURLObject::DECODE_TO_IURI );
            }
        }
        break;

        case PROPERTY_ID_DATEMIN:
        case PROPERTY_ID_DATEMAX:
        case PROPERTY_ID_DEFAULT_DATE:
        case PROPERTY_ID_DATE:
        {
            util::Date aDate;
            OSL_VERIFY( _rControlValue >>= aDate );
            aPropertyValue <<= aDate;
        }
        break;

        case PROPERTY_ID_TIMEMIN:
        case PROPERTY_ID_TIMEMAX:
        case PROPERTY_ID_DEFAULT_TIME:
        case PROPERTY_ID_TIME:
        {
            util::Time aTime;
            OSL_VERIFY( _rControlValue >>= aTime );
            aPropertyValue <<= aTime;
        }
        break;

        case PROPERTY_ID_WRITING_MODE:
        {
            aPropertyValue = FormComponentPropertyHandler_Base::convertToPropertyValue( _rPropertyName, _rControlValue );

            sal_Int16 nNormalizedValue( 2 );
            OSL_VERIFY( aPropertyValue >>= nNormalizedValue );
            sal_Int16 nWritingMode = WritingMode2::CONTEXT;
            switch ( nNormalizedValue )
            {
            case 0: nWritingMode = WritingMode2::LR_TB;      break;
            case 1: nWritingMode = WritingMode2::RL_TB;      break;
            case 2: nWritingMode = WritingMode2::CONTEXT;    break;
            default:
                OSL_FAIL( "FormComponentPropertyHandler::convertToPropertyValue: unexpected 'normalized value' for WritingMode!" );
                nWritingMode = WritingMode2::CONTEXT;
                break;
            }

            aPropertyValue <<= nWritingMode;
        }
        break;

        default:
            aPropertyValue = FormComponentPropertyHandler_Base::convertToPropertyValue( _rPropertyName, _rControlValue );
            break;  // default

        }   // switch ( nPropId )

        return aPropertyValue;
    }

    //--------------------------------------------------------------------
    Any SAL_CALL FormComponentPropertyHandler::convertToControlValue( const OUString& _rPropertyName, const Any& _rPropertyValue, const Type& _rControlValueType ) throw (UnknownPropertyException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        sal_Int32 nPropId = m_pInfoService->getPropertyId( _rPropertyName );
        DBG_ASSERT( nPropId != -1, "FormComponentPropertyHandler::convertToPropertyValue: not one of my properties!!" );

        Property aProperty( impl_getPropertyFromId_throw( nPropId ) );

        Any aControlValue( _rPropertyValue );
        if ( !aControlValue.hasValue() )
        {
            // if the property is represented with a list box or color list box, we need to
            // translate this into the string "Default"
            if ( m_aPropertiesWithDefListEntry.find( _rPropertyName ) != m_aPropertiesWithDefListEntry.end() )
                aControlValue <<= m_sDefaultValueString;

            return aControlValue;
        }

        switch ( nPropId )
        {
        //////////////////////////////////////////////////////////////
        case PROPERTY_ID_SHOW_POSITION:
        case PROPERTY_ID_SHOW_NAVIGATION:
        case PROPERTY_ID_SHOW_RECORDACTIONS:
        case PROPERTY_ID_SHOW_FILTERSORT:
        {
            ::std::vector< OUString > aListEntries;
            tools::StringListResource aRes( PcrRes( RID_RSC_ENUM_SHOWHIDE ), aListEntries );
            OSL_ENSURE( aListEntries.size() == 2, "FormComponentPropertyHandler::convertToControlValue: broken resource for Show/Hide!" );

            if ( aListEntries.size() == 2 )
            {
                OUString sControlValue =     ::comphelper::getBOOL( _rPropertyValue )
                                                ?   aListEntries[1]
                                                :   aListEntries[0];
                aControlValue <<= sControlValue;
            }
        }
        break;

        //////////////////////////////////////////////////////////////
        case PROPERTY_ID_DATASOURCE:
        {
            OSL_ENSURE( _rControlValueType.getTypeClass() == TypeClass_STRING,
                "FormComponentPropertyHandler::convertToControlValue: wrong ControlValueType!" );

            OUString sDataSource;
            _rPropertyValue >>= sDataSource;
            if ( !sDataSource.isEmpty() )
            {
                ::svt::OFileNotation aTransformer( sDataSource );
                sDataSource = aTransformer.get( ::svt::OFileNotation::N_SYSTEM );
            }
            aControlValue <<= sDataSource;
        }
        break;

        //////////////////////////////////////////////////////////////
        case PROPERTY_ID_CONTROLLABEL:
        {
            OUString sControlValue;

            Reference< XPropertySet >  xSet;
            _rPropertyValue >>= xSet;
            Reference< XPropertySetInfo > xPSI;
            if ( xSet.is() )
                xPSI = xSet->getPropertySetInfo();
            if ( xPSI.is() && xPSI->hasPropertyByName( PROPERTY_LABEL ) )
            {
                OUStringBuffer aValue;
                aValue.append( (sal_Unicode)'<' );
                OUString sLabel;
                OSL_VERIFY( xSet->getPropertyValue( PROPERTY_LABEL ) >>= sLabel );
                aValue.append( sLabel );
                aValue.append( (sal_Unicode)'>' );
                sControlValue = aValue.makeStringAndClear();
            }

            aControlValue <<= sControlValue;
        }
        break;

        //////////////////////////////////////////////////////////////
        case PROPERTY_ID_DATEMIN:
        case PROPERTY_ID_DATEMAX:
        case PROPERTY_ID_DEFAULT_DATE:
        case PROPERTY_ID_DATE:
        {
            sal_Int32 nDate = 0;
            OSL_VERIFY( _rPropertyValue >>= nDate );
            aControlValue <<= DBTypeConversion::toDate( nDate );
        }
        break;

        case PROPERTY_ID_TIMEMIN:
        case PROPERTY_ID_TIMEMAX:
        case PROPERTY_ID_DEFAULT_TIME:
        case PROPERTY_ID_TIME:
        {
            sal_Int64 nTime = 0;
            OSL_VERIFY( _rPropertyValue >>= nTime );
            aControlValue <<= DBTypeConversion::toTime( nTime );
        }
        break;

        case PROPERTY_ID_WRITING_MODE:
        {
            sal_Int16 nWritingMode( WritingMode2::CONTEXT );
            OSL_VERIFY( _rPropertyValue >>= nWritingMode );
            sal_Int16 nNormalized = 2;
            switch ( nWritingMode )
            {
            case WritingMode2::LR_TB:   nNormalized = 0;    break;
            case WritingMode2::RL_TB:   nNormalized = 1;    break;
            case WritingMode2::CONTEXT: nNormalized = 2;    break;
            default:
                OSL_FAIL( "FormComponentPropertyHandler::convertToControlValue: unsupported API value for WritingMode!" );
                nNormalized = 2;
                break;
            }

            aControlValue = FormComponentPropertyHandler_Base::convertToControlValue( _rPropertyName, makeAny( nNormalized ), _rControlValueType );
        }
        break;

        case PROPERTY_ID_FONT:
        {
            FontDescriptor aFont;
            OSL_VERIFY( _rPropertyValue >>= aFont );

            OUStringBuffer displayName;
            if ( aFont.Name.isEmpty() )
            {
                displayName.append( PcrRes(RID_STR_FONT_DEFAULT).toString() );
            }
            else
            {
                // font name
                displayName.append( aFont.Name );
                displayName.appendAscii( ", " );

                // font style
                ::FontWeight  eWeight = VCLUnoHelper::ConvertFontWeight( aFont.Weight );
                sal_uInt16 nStyleResID = RID_STR_FONTSTYLE_REGULAR;
                if ( aFont.Slant == FontSlant_ITALIC )
                {
                    if ( eWeight > WEIGHT_NORMAL )
                        nStyleResID = RID_STR_FONTSTYLE_BOLD_ITALIC;
                    else
                        nStyleResID = RID_STR_FONTSTYLE_ITALIC;
                }
                else
                {
                    if ( eWeight > WEIGHT_NORMAL )
                        nStyleResID = RID_STR_FONTSTYLE_BOLD;
                }
                displayName.append(PcrRes(nStyleResID).toString());

                // font size
                if ( aFont.Height )
                {
                    displayName.appendAscii( ", " );
                    displayName.append( sal_Int32( aFont.Height ) );
                }
            }

            aControlValue <<= displayName.makeStringAndClear();
        }
        break;

        default:
            aControlValue = FormComponentPropertyHandler_Base::convertToControlValue( _rPropertyName, _rPropertyValue, _rControlValueType );
            break;

        }   // switch ( nPropId )

        return aControlValue;
    }

    //--------------------------------------------------------------------
    PropertyState SAL_CALL FormComponentPropertyHandler::getPropertyState( const OUString& _rPropertyName ) throw (UnknownPropertyException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( m_xPropertyState.is() )
            return m_xPropertyState->getPropertyState( _rPropertyName );
        return PropertyState_DIRECT_VALUE;
    }

    //--------------------------------------------------------------------
    void SAL_CALL FormComponentPropertyHandler::addPropertyChangeListener( const Reference< XPropertyChangeListener >& _rxListener ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        FormComponentPropertyHandler_Base::addPropertyChangeListener( _rxListener );
        if ( m_xComponent.is() )
            m_xComponent->addPropertyChangeListener( OUString(), _rxListener );
    }

    //--------------------------------------------------------------------
    void SAL_CALL FormComponentPropertyHandler::removePropertyChangeListener( const Reference< XPropertyChangeListener >& _rxListener ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( m_xComponent.is() )
            m_xComponent->removePropertyChangeListener( OUString(), _rxListener );
        FormComponentPropertyHandler_Base::removePropertyChangeListener( _rxListener );
    }

    //--------------------------------------------------------------------
    void FormComponentPropertyHandler::onNewComponent()
    {
        FormComponentPropertyHandler_Base::onNewComponent();
        if ( !m_xComponentPropertyInfo.is() && m_xComponent.is() )
            throw NullPointerException();

        m_xPropertyState.set( m_xComponent, UNO_QUERY );
        m_eComponentClass = eUnknown;
        m_bComponentIsSubForm = m_bHaveListSource = m_bHaveCommand = false;
        m_nClassId = 0;

        impl_initComponentMetaData_throw();
    }

    //--------------------------------------------------------------------
    Sequence< Property > SAL_CALL FormComponentPropertyHandler::doDescribeSupportedProperties() const
    {
        if ( !m_xComponentPropertyInfo.is() )
            return Sequence< Property >();

        ::std::vector< Property > aProperties;

        Sequence< Property > aAllProperties( m_xComponentPropertyInfo->getProperties() );
        aProperties.reserve( aAllProperties.getLength() );

        // filter the properties
        PropertyId nPropId( 0 );
        OUString sDisplayName;

        Property* pProperty = aAllProperties.getArray();
        Property* pPropertiesEnd = pProperty + aAllProperties.getLength();
        for ( ; pProperty != pPropertiesEnd; ++pProperty )
        {
            nPropId = m_pInfoService->getPropertyId( pProperty->Name );
            if ( nPropId == -1 )
                continue;
            pProperty->Handle = nPropId;

            sDisplayName = m_pInfoService->getPropertyTranslation( nPropId );
            if ( sDisplayName.isEmpty() )
                continue;

            sal_uInt32  nPropertyUIFlags = m_pInfoService->getPropertyUIFlags( nPropId );
            bool bIsVisibleForForms   = ( nPropertyUIFlags & PROP_FLAG_FORM_VISIBLE   ) != 0;
            bool bIsVisibleForDialogs = ( nPropertyUIFlags & PROP_FLAG_DIALOG_VISIBLE ) != 0;

            // depending on whether we're working for a form or a UNO dialog, some
            // properties are not displayed
            if  (  ( m_eComponentClass == eFormControl   && !bIsVisibleForForms )
                || ( m_eComponentClass == eDialogControl && !bIsVisibleForDialogs )
                )
                continue;

            // some generic sanity checks
            if ( impl_shouldExcludeProperty_nothrow( *pProperty ) )
                continue;

            switch ( nPropId )
            {
            case PROPERTY_ID_BORDER:
            case PROPERTY_ID_TABSTOP:
                // BORDER and TABSTOP are normalized (see impl_normalizePropertyValue_nothrow)
                // to not allow VOID values
                pProperty->Attributes &= ~( PropertyAttribute::MAYBEVOID );
                break;

            case PROPERTY_ID_LISTSOURCE:
                // no cursor source if no Base is installed. #124939#
                // This fix is not intendend to appear on the main trunk. If you find it there,
                // please tell me! frank.schoenheit@sun.com
                if ( SvtModuleOptions().IsModuleInstalled( SvtModuleOptions::E_SDATABASE ) )
                    const_cast< FormComponentPropertyHandler* >( this )->m_bHaveListSource = true;
                break;

            case PROPERTY_ID_COMMAND:
                // no cursor source if no Base is installed. #124939#
                // This fix is not intendend to appear on the main trunk. If you find it there,
                // please tell me! frank.schoenheit@sun.com
                if ( SvtModuleOptions().IsModuleInstalled( SvtModuleOptions::E_SDATABASE ) )
                    const_cast< FormComponentPropertyHandler* >( this )->m_bHaveCommand = true;
                break;
            }   // switch ( nPropId )

            aProperties.push_back( *pProperty );
        }

        if ( aProperties.empty() )
            return Sequence< Property >();
        return Sequence< Property >( &(*aProperties.begin()), aProperties.size() );
    }

    //--------------------------------------------------------------------
    Sequence< OUString > SAL_CALL FormComponentPropertyHandler::getSupersededProperties( ) throw (RuntimeException)
    {
        return Sequence< OUString >( );
    }

    //--------------------------------------------------------------------
    Sequence< OUString > SAL_CALL FormComponentPropertyHandler::getActuatingProperties( ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        ::std::vector< OUString > aInterestingProperties;
        aInterestingProperties.push_back(  static_cast<const OUString&>(PROPERTY_DATASOURCE) );
        aInterestingProperties.push_back(  static_cast<const OUString&>(PROPERTY_COMMAND) );
        aInterestingProperties.push_back(  static_cast<const OUString&>(PROPERTY_COMMANDTYPE) );
        aInterestingProperties.push_back(  static_cast<const OUString&>(PROPERTY_LISTSOURCE) );
        aInterestingProperties.push_back(  static_cast<const OUString&>(PROPERTY_LISTSOURCETYPE) );
        aInterestingProperties.push_back(  static_cast<const OUString&>(PROPERTY_SUBMIT_ENCODING) );
        aInterestingProperties.push_back(  static_cast<const OUString&>(PROPERTY_REPEAT) );
        aInterestingProperties.push_back(  static_cast<const OUString&>(PROPERTY_TABSTOP) );
        aInterestingProperties.push_back(  static_cast<const OUString&>(PROPERTY_BORDER) );
        aInterestingProperties.push_back(  static_cast<const OUString&>(PROPERTY_CONTROLSOURCE) );
        aInterestingProperties.push_back(  static_cast<const OUString&>(PROPERTY_DROPDOWN) );
        aInterestingProperties.push_back(  static_cast<const OUString&>(PROPERTY_IMAGE_URL) );
        aInterestingProperties.push_back(  static_cast<const OUString&>(PROPERTY_TARGET_URL) );
        aInterestingProperties.push_back(  static_cast<const OUString&>(PROPERTY_STRINGITEMLIST) );
        aInterestingProperties.push_back(  static_cast<const OUString&>(PROPERTY_BUTTONTYPE) );
        aInterestingProperties.push_back(  static_cast<const OUString&>(PROPERTY_ESCAPE_PROCESSING) );
        aInterestingProperties.push_back(  static_cast<const OUString&>(PROPERTY_TRISTATE) );
        aInterestingProperties.push_back(  static_cast<const OUString&>(PROPERTY_DECIMAL_ACCURACY) );
        aInterestingProperties.push_back(  static_cast<const OUString&>(PROPERTY_SHOWTHOUSANDSEP) );
        aInterestingProperties.push_back(  static_cast<const OUString&>(PROPERTY_FORMATKEY) );
        aInterestingProperties.push_back(  static_cast<const OUString&>(PROPERTY_EMPTY_IS_NULL) );
        aInterestingProperties.push_back(  static_cast<const OUString&>(PROPERTY_TOGGLE) );
        return Sequence< OUString >( &(*aInterestingProperties.begin()), aInterestingProperties.size() );
    }

    //--------------------------------------------------------------------
    LineDescriptor SAL_CALL FormComponentPropertyHandler::describePropertyLine( const OUString& _rPropertyName,
        const Reference< XPropertyControlFactory >& _rxControlFactory )
        throw (UnknownPropertyException, NullPointerException, RuntimeException)
    {
        if ( !_rxControlFactory.is() )
            throw NullPointerException();

        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyId nPropId( impl_getPropertyId_throw( _rPropertyName ) );
        Property aProperty( impl_getPropertyFromId_throw( nPropId ) );

        //////////////////////////////////////////////////////////////////////
        // for the MultiLine property, we have different UI translations depending on the control
        // type
        if ( nPropId == PROPERTY_ID_MULTILINE )
        {
            if (  ( m_nClassId == FormComponentType::FIXEDTEXT )
               || ( m_nClassId == FormComponentType::COMMANDBUTTON )
               || ( m_nClassId == FormComponentType::RADIOBUTTON )
               || ( m_nClassId == FormComponentType::CHECKBOX )
               )
                nPropId = PROPERTY_ID_WORDBREAK;
        }

        String sDisplayName = m_pInfoService->getPropertyTranslation( nPropId );
        if ( !sDisplayName.Len() )
        {
            OSL_FAIL( "FormComponentPropertyHandler::describePropertyLine: did getSupportedProperties not work properly?" );
            throw UnknownPropertyException();
        }

        //////////////////////////////////////////////////////////////////////

        LineDescriptor aDescriptor;
        aDescriptor.HelpURL = HelpIdUrl::getHelpURL( m_pInfoService->getPropertyHelpId( nPropId ) );
        aDescriptor.DisplayName = sDisplayName;

        // for the moment, assume a text field
        sal_Int16 nControlType = PropertyControlType::TextField;
        sal_Bool bReadOnly = sal_False;
        aDescriptor.Control.clear();

        //////////////////////////////////////////////////////////////////////

        bool bNeedDefaultStringIfVoidAllowed = false;

        TypeClass eType = aProperty.Type.getTypeClass();

        switch ( nPropId )
        {
        case PROPERTY_ID_DEFAULT_SELECT_SEQ:
        case PROPERTY_ID_SELECTEDITEMS:
            aDescriptor.PrimaryButtonId = OUString::createFromAscii(UID_PROP_DLG_SELECTION);
            break;

        case PROPERTY_ID_FILTER:
            aDescriptor.PrimaryButtonId = OUString::createFromAscii(UID_PROP_DLG_FILTER);
            break;

        case PROPERTY_ID_SORT:
            aDescriptor.PrimaryButtonId = OUString::createFromAscii(UID_PROP_DLG_ORDER);
            break;

        case PROPERTY_ID_MASTERFIELDS:
        case PROPERTY_ID_DETAILFIELDS:
            nControlType = PropertyControlType::StringListField;
            aDescriptor.PrimaryButtonId = OUString::createFromAscii(UID_PROP_DLG_FORMLINKFIELDS);
            break;

        case PROPERTY_ID_COMMAND:
            aDescriptor.PrimaryButtonId = OUString::createFromAscii(UID_PROP_DLG_SQLCOMMAND);
            break;

        case PROPERTY_ID_TABINDEX:
        {
            Reference< XControlContainer > xControlContext( impl_getContextControlContainer_nothrow() );
            if ( xControlContext.is() )
                aDescriptor.PrimaryButtonId = OUString::createFromAscii(UID_PROP_DLG_TABINDEX);
            nControlType = PropertyControlType::NumericField;
        };
        break;

        case PROPERTY_ID_FONT:
            bReadOnly = sal_True;
            aDescriptor.PrimaryButtonId = OUString::createFromAscii(UID_PROP_DLG_FONT_TYPE);
            break;

        case PROPERTY_ID_TARGET_URL:
        case PROPERTY_ID_IMAGE_URL:
        {
            aDescriptor.Control = new OFileUrlControl( impl_getDefaultDialogParent_nothrow(), WB_TABSTOP | WB_BORDER );

            aDescriptor.PrimaryButtonId = OUString::createFromAscii(( PROPERTY_ID_TARGET_URL == nPropId )
                ? UID_PROP_DLG_ATTR_TARGET_URL : UID_PROP_DLG_IMAGE_URL);
        }
        break;

        case PROPERTY_ID_ECHO_CHAR:
            nControlType = PropertyControlType::CharacterField;
            break;

        case PROPERTY_ID_BACKGROUNDCOLOR:
        case PROPERTY_ID_FILLCOLOR:
        case PROPERTY_ID_SYMBOLCOLOR:
        case PROPERTY_ID_BORDERCOLOR:
            nControlType = PropertyControlType::ColorListBox;

            switch( nPropId )
            {
            case PROPERTY_ID_BACKGROUNDCOLOR:
                aDescriptor.PrimaryButtonId = OUString::createFromAscii(UID_PROP_DLG_BACKGROUNDCOLOR); break;
            case PROPERTY_ID_FILLCOLOR:
                aDescriptor.PrimaryButtonId = OUString::createFromAscii(UID_PROP_DLG_FILLCOLOR); break;
            case PROPERTY_ID_SYMBOLCOLOR:
                aDescriptor.PrimaryButtonId = OUString::createFromAscii(UID_PROP_DLG_SYMBOLCOLOR); break;
            case PROPERTY_ID_BORDERCOLOR:
                aDescriptor.PrimaryButtonId = OUString::createFromAscii(UID_PROP_DLG_BORDERCOLOR); break;
            }
            break;

        case PROPERTY_ID_LABEL:
            nControlType = PropertyControlType::MultiLineTextField;
            break;

        case PROPERTY_ID_DEFAULT_TEXT:
        {
            if (FormComponentType::FILECONTROL == m_nClassId)
                nControlType = PropertyControlType::TextField;
            else
                nControlType = PropertyControlType::MultiLineTextField;
        }
        break;

        case PROPERTY_ID_TEXT:
            if ( impl_componentHasProperty_throw( PROPERTY_MULTILINE ) )
                nControlType = PropertyControlType::MultiLineTextField;
            break;

        case PROPERTY_ID_CONTROLLABEL:
            bReadOnly = sal_True;
            aDescriptor.PrimaryButtonId = OUString::createFromAscii(UID_PROP_DLG_CONTROLLABEL);
            break;

        case PROPERTY_ID_FORMATKEY:
        case PROPERTY_ID_EFFECTIVE_MIN:
        case PROPERTY_ID_EFFECTIVE_MAX:
        case PROPERTY_ID_EFFECTIVE_DEFAULT:
        case PROPERTY_ID_EFFECTIVE_VALUE:
        {
            // and the supplier is really available
            Reference< XNumberFormatsSupplier >  xSupplier;
            m_xComponent->getPropertyValue( PROPERTY_FORMATSSUPPLIER ) >>= xSupplier;
            if (xSupplier.is())
            {
                Reference< XUnoTunnel > xTunnel(xSupplier,UNO_QUERY);
                DBG_ASSERT(xTunnel.is(), "FormComponentPropertyHandler::describePropertyLine : xTunnel is invalid!");
                SvNumberFormatsSupplierObj* pSupplier = reinterpret_cast<SvNumberFormatsSupplierObj*>(xTunnel->getSomething(SvNumberFormatsSupplierObj::getUnoTunnelId()));

                if (pSupplier != NULL)
                {
                    sal_Bool bIsFormatKey = (PROPERTY_ID_FORMATKEY == nPropId);

                    bReadOnly = bIsFormatKey;

                    if ( bIsFormatKey )
                    {
                        OFormatSampleControl* pControl = new OFormatSampleControl( impl_getDefaultDialogParent_nothrow(), WB_READONLY | WB_TABSTOP | WB_BORDER );
                        aDescriptor.Control = pControl;
                        pControl->SetFormatSupplier( pSupplier );

                        aDescriptor.PrimaryButtonId = OUString::createFromAscii(UID_PROP_DLG_NUMBER_FORMAT);
                    }
                    else
                    {
                        OFormattedNumericControl* pControl = new OFormattedNumericControl( impl_getDefaultDialogParent_nothrow(), WB_TABSTOP | WB_BORDER );
                        aDescriptor.Control = pControl;

                        FormatDescription aDesc;
                        aDesc.pSupplier = pSupplier;
                        Any aFormatKeyValue = m_xComponent->getPropertyValue(PROPERTY_FORMATKEY);
                        if ( !( aFormatKeyValue >>= aDesc.nKey ) )
                            aDesc.nKey = 0;

                        pControl->SetFormatDescription( aDesc );
                    }
                }
            }
        }
        break;

        case PROPERTY_ID_DATEMIN:
        case PROPERTY_ID_DATEMAX:
        case PROPERTY_ID_DEFAULT_DATE:
        case PROPERTY_ID_DATE:
            nControlType = PropertyControlType::DateField;
            break;

        case PROPERTY_ID_TIMEMIN:
        case PROPERTY_ID_TIMEMAX:
        case PROPERTY_ID_DEFAULT_TIME:
        case PROPERTY_ID_TIME:
            nControlType = PropertyControlType::TimeField;
            break;

        case PROPERTY_ID_VALUEMIN:
        case PROPERTY_ID_VALUEMAX:
        case PROPERTY_ID_DEFAULT_VALUE:
        case PROPERTY_ID_VALUE:
            {
                OFormattedNumericControl* pControl = new OFormattedNumericControl( impl_getDefaultDialogParent_nothrow(), WB_TABSTOP | WB_BORDER | WB_SPIN | WB_REPEAT );
                aDescriptor.Control = pControl;

                // we don't set a formatter so the control uses a default (which uses the application
                // language and a default numeric format)
                // but we set the decimal digits
                pControl->SetDecimalDigits(
                    ::comphelper::getINT16( m_xComponent->getPropertyValue( PROPERTY_DECIMAL_ACCURACY ) )
                );

                // and the thousands separator
                pControl->SetThousandsSep(
                    ::comphelper::getBOOL( m_xComponent->getPropertyValue(PROPERTY_SHOWTHOUSANDSEP) )
                );

                // and the default value for the property
                try
                {
                    if (m_xPropertyState.is() && ((PROPERTY_ID_VALUEMIN == nPropId) || (PROPERTY_ID_VALUEMAX == nPropId)))
                    {
                        double nDefault = 0;
                        if ( m_xPropertyState->getPropertyDefault( aProperty.Name ) >>= nDefault )
                            pControl->SetDefaultValue( nDefault );
                    }
                }
                catch (const Exception&)
                {
                    // just ignore it
                }

                // and allow empty values only for the default value and the value
                pControl->EnableEmptyField( ( PROPERTY_ID_DEFAULT_VALUE == nPropId )
                                        ||  ( PROPERTY_ID_VALUE == nPropId ) );
            }
            break;

        default:
            if ( TypeClass_BYTE <= eType && eType <= TypeClass_DOUBLE )
            {
                sal_Int16 nDigits = 0;
                sal_Int16 nValueUnit = -1;
                sal_Int16 nDisplayUnit = -1;
                if ( m_eComponentClass == eFormControl )
                {
                    if  (  ( nPropId == PROPERTY_ID_WIDTH )
                        || ( nPropId == PROPERTY_ID_ROWHEIGHT )
                        || ( nPropId == PROPERTY_ID_HEIGHT )
                        )
                    {
                        nValueUnit = MeasureUnit::MM_10TH;
                        nDisplayUnit = impl_getDocumentMeasurementUnit_throw();
                        nDigits = 2;
                    }
                }

                Optional< double > aValueNotPresent( sal_False, 0 );
                aDescriptor.Control = PropertyHandlerHelper::createNumericControl(
                    _rxControlFactory, nDigits, aValueNotPresent, aValueNotPresent, sal_False );

                Reference< XNumericControl > xNumericControl( aDescriptor.Control, UNO_QUERY_THROW );
                if ( nValueUnit != -1 )
                    xNumericControl->setValueUnit( nValueUnit );
                if ( nDisplayUnit != -1 )
                    xNumericControl->setDisplayUnit( nDisplayUnit );
            }
            break;
        }

        //////////////////////////////////////////////////////////////////////
        if ( eType == TypeClass_SEQUENCE )
            nControlType = PropertyControlType::StringListField;

        //////////////////////////////////////////////////////////////////////
        // boolean values
        if ( eType == TypeClass_BOOLEAN )
        {
            sal_uInt16 nResId = RID_RSC_ENUM_YESNO;
            if  (   ( nPropId == PROPERTY_ID_SHOW_POSITION )
                ||  ( nPropId == PROPERTY_ID_SHOW_NAVIGATION )
                ||  ( nPropId == PROPERTY_ID_SHOW_RECORDACTIONS )
                ||  ( nPropId == PROPERTY_ID_SHOW_FILTERSORT )
                )
                nResId = RID_RSC_ENUM_SHOWHIDE;

            ::std::vector< OUString > aListEntries;
            tools::StringListResource aRes(PcrRes(nResId),aListEntries);
            aDescriptor.Control = PropertyHandlerHelper::createListBoxControl( _rxControlFactory, aListEntries, sal_False, sal_False );
            bNeedDefaultStringIfVoidAllowed = true;
        }

        //////////////////////////////////////////////////////////////////////
        // enum properties
        sal_uInt32 nPropertyUIFlags = m_pInfoService->getPropertyUIFlags( nPropId );
        bool bIsEnumProperty = ( nPropertyUIFlags & PROP_FLAG_ENUM ) != 0;
        if ( bIsEnumProperty || ( PROPERTY_ID_TARGET_FRAME == nPropId ) )
        {
            ::std::vector< OUString > aEnumValues = m_pInfoService->getPropertyEnumRepresentations( nPropId );
            ::std::vector< OUString >::const_iterator pStart = aEnumValues.begin();
            ::std::vector< OUString >::const_iterator pEnd = aEnumValues.end();

            // for a checkbox: if "ambiguous" is not allowed, remove this from the sequence
            if  (   ( PROPERTY_ID_DEFAULT_STATE == nPropId )
                ||  ( PROPERTY_ID_STATE == nPropId )
                )
            {
                if ( impl_componentHasProperty_throw( PROPERTY_TRISTATE ) )
                {
                    if ( !::comphelper::getBOOL( m_xComponent->getPropertyValue( PROPERTY_TRISTATE ) ) )
                    {   // remove the last sequence element
                        if ( pEnd > pStart )
                            --pEnd;
                    }
                }
                else
                    --pEnd;
            }

            if ( PROPERTY_ID_LISTSOURCETYPE == nPropId )
                if ( FormComponentType::COMBOBOX == m_nClassId )
                    // remove the first sequence element -> value list not possible for combo boxes
                    ++pStart;

            // copy the sequence
            ::std::vector< OUString > aListEntries( pEnd - pStart );
            ::std::copy( pStart, pEnd, aListEntries.begin() );

            // create the control
            if ( PROPERTY_ID_TARGET_FRAME == nPropId )
                aDescriptor.Control = PropertyHandlerHelper::createComboBoxControl( _rxControlFactory, aListEntries, sal_False, sal_False );
            else
            {
                aDescriptor.Control = PropertyHandlerHelper::createListBoxControl( _rxControlFactory, aListEntries, sal_False, sal_False );
                bNeedDefaultStringIfVoidAllowed = true;
            }
        }

        //////////////////////////////////////////////////////////////////////
        switch( nPropId )
        {
            case PROPERTY_ID_REPEAT_DELAY:
            {
                OTimeDurationControl* pControl = new OTimeDurationControl( impl_getDefaultDialogParent_nothrow(), WB_BORDER | WB_TABSTOP );
                aDescriptor.Control = pControl;

                pControl->setMinValue( Optional< double >( sal_True, 0 ) );
                pControl->setMaxValue( Optional< double >( sal_True, ::std::numeric_limits< double >::max() ) );
            }
            break;

            case PROPERTY_ID_TABINDEX:
            case PROPERTY_ID_BOUNDCOLUMN:
            case PROPERTY_ID_VISIBLESIZE:
            case PROPERTY_ID_MAXTEXTLEN:
            case PROPERTY_ID_LINEINCREMENT:
            case PROPERTY_ID_BLOCKINCREMENT:
            case PROPERTY_ID_SPININCREMENT:
            {
                Optional< double > aMinValue( sal_True, 0 );
                Optional< double > aMaxValue( sal_True, 0x7FFFFFFF );

                if ( nPropId == PROPERTY_ID_MAXTEXTLEN ||  nPropId == PROPERTY_ID_BOUNDCOLUMN )
                    aMinValue.Value = -1;
                else if ( nPropId == PROPERTY_ID_VISIBLESIZE )
                    aMinValue.Value = 1;
                else
                    aMinValue.Value = 0;

                aDescriptor.Control = PropertyHandlerHelper::createNumericControl(
                    _rxControlFactory, 0, aMinValue, aMaxValue, sal_False );
            }
            break;

            case PROPERTY_ID_DECIMAL_ACCURACY:
            {
                Optional< double > aMinValue( sal_True, 0 );
                Optional< double > aMaxValue( sal_True, 20 );

                aDescriptor.Control = PropertyHandlerHelper::createNumericControl(
                    _rxControlFactory, 0, aMinValue, aMaxValue, sal_False );
            }
            break;

            //////////////////////////////////////////////////////////////////////
            // DataSource
            case PROPERTY_ID_DATASOURCE:
            {
                aDescriptor.PrimaryButtonId = OUString::createFromAscii(UID_PROP_DLG_ATTR_DATASOURCE);

                ::std::vector< OUString > aListEntries;

                Reference< XDatabaseContext > xDatabaseContext = sdb::DatabaseContext::create( m_xContext );
                Sequence< OUString > aDatasources = xDatabaseContext->getElementNames();
                aListEntries.resize( aDatasources.getLength() );
                ::std::copy( aDatasources.getConstArray(), aDatasources.getConstArray() + aDatasources.getLength(),
                    aListEntries.begin() );
                aDescriptor.Control = PropertyHandlerHelper::createComboBoxControl(
                    _rxControlFactory, aListEntries, sal_False, sal_True );
            }
            break;

            case PROPERTY_ID_CONTROLSOURCE:
            {
                ::std::vector< OUString > aFieldNames;
                impl_initFieldList_nothrow( aFieldNames );
                aDescriptor.Control = PropertyHandlerHelper::createComboBoxControl(
                    _rxControlFactory, aFieldNames, sal_False, sal_False );
            }
            break;

            case PROPERTY_ID_COMMAND:
                impl_describeCursorSource_nothrow( aDescriptor, _rxControlFactory );
                break;

            case PROPERTY_ID_LISTSOURCE:
                impl_describeListSourceUI_throw( aDescriptor, _rxControlFactory );
                break;
        }

        if ( !aDescriptor.Control.is() )
            aDescriptor.Control = _rxControlFactory->createPropertyControl( nControlType, bReadOnly );

        if ( ( aProperty.Attributes & PropertyAttribute::MAYBEVOID ) != 0 )
        {
            // insert the string "Default" string, if necessary
            if ( bNeedDefaultStringIfVoidAllowed || ( nControlType == PropertyControlType::ColorListBox ) )
            {
                Reference< XStringListControl > xStringList( aDescriptor.Control, UNO_QUERY_THROW );
                xStringList->prependListEntry( m_sDefaultValueString );
                m_aPropertiesWithDefListEntry.insert( _rPropertyName );
            }
        }

        if ( !aDescriptor.PrimaryButtonId.isEmpty() )
            aDescriptor.HasPrimaryButton = sal_True;
        if ( !aDescriptor.SecondaryButtonId.isEmpty() )
            aDescriptor.HasSecondaryButton = sal_True;

        bool bIsDataProperty = ( nPropertyUIFlags & PROP_FLAG_DATA_PROPERTY ) != 0;
        aDescriptor.Category = bIsDataProperty ? OUString("Data") : OUString("General");
        return aDescriptor;
    }

    //--------------------------------------------------------------------
    InteractiveSelectionResult SAL_CALL FormComponentPropertyHandler::onInteractivePropertySelection( const OUString& _rPropertyName, sal_Bool /*_bPrimary*/, Any& _rData, const Reference< XObjectInspectorUI >& _rxInspectorUI ) throw (UnknownPropertyException, NullPointerException, RuntimeException)
    {
        if ( !_rxInspectorUI.is() )
            throw NullPointerException();

        ::osl::ClearableMutexGuard aGuard( m_aMutex );
        PropertyId nPropId( impl_getPropertyId_throw( _rPropertyName ) );

        InteractiveSelectionResult eResult = InteractiveSelectionResult_Cancelled;
        switch ( nPropId )
        {
        case PROPERTY_ID_DEFAULT_SELECT_SEQ:
        case PROPERTY_ID_SELECTEDITEMS:
            if ( impl_dialogListSelection_nothrow( _rPropertyName, aGuard ) )
                eResult = InteractiveSelectionResult_Success;
            break;

        case PROPERTY_ID_FILTER:
        case PROPERTY_ID_SORT:
        {
            OUString sClause;
            if ( impl_dialogFilterOrSort_nothrow( PROPERTY_ID_FILTER == nPropId, sClause, aGuard ) )
            {
                _rData <<= sClause;
                eResult = InteractiveSelectionResult_ObtainedValue;
            }
        }
        break;

        case PROPERTY_ID_MASTERFIELDS:
        case PROPERTY_ID_DETAILFIELDS:
            if ( impl_dialogLinkedFormFields_nothrow( aGuard ) )
                eResult = InteractiveSelectionResult_Success;
            break;

        case PROPERTY_ID_FORMATKEY:
            if ( impl_dialogFormatting_nothrow( _rData, aGuard ) )
                eResult = InteractiveSelectionResult_ObtainedValue;
            break;

        case PROPERTY_ID_IMAGE_URL:
            if ( impl_browseForImage_nothrow( _rData, aGuard ) )
                eResult = InteractiveSelectionResult_ObtainedValue;
            break;

        case PROPERTY_ID_TARGET_URL:
            if ( impl_browseForTargetURL_nothrow( _rData, aGuard ) )
                eResult = InteractiveSelectionResult_ObtainedValue;
            break;

        case PROPERTY_ID_FONT:
            if ( impl_executeFontDialog_nothrow( _rData, aGuard ) )
                eResult = InteractiveSelectionResult_ObtainedValue;
            break;

        case PROPERTY_ID_DATASOURCE:
            if ( impl_browseForDatabaseDocument_throw( _rData, aGuard ) )
                eResult = InteractiveSelectionResult_ObtainedValue;
            break;

        case PROPERTY_ID_BACKGROUNDCOLOR:
        case PROPERTY_ID_FILLCOLOR:
        case PROPERTY_ID_SYMBOLCOLOR:
        case PROPERTY_ID_BORDERCOLOR:
            if ( impl_dialogColorChooser_throw( nPropId, _rData, aGuard ) )
                eResult = InteractiveSelectionResult_ObtainedValue;
            break;

        case PROPERTY_ID_CONTROLLABEL:
            if ( impl_dialogChooseLabelControl_nothrow( _rData, aGuard ) )
                eResult = InteractiveSelectionResult_ObtainedValue;
            break;

        case PROPERTY_ID_TABINDEX:
            if ( impl_dialogChangeTabOrder_nothrow( aGuard ) )
                eResult = InteractiveSelectionResult_Success;
            break;

        case PROPERTY_ID_COMMAND:
        case PROPERTY_ID_LISTSOURCE:
            if ( impl_doDesignSQLCommand_nothrow( _rxInspectorUI, nPropId ) )
                eResult = InteractiveSelectionResult_Pending;
            break;
        default:
            OSL_FAIL( "FormComponentPropertyHandler::onInteractivePropertySelection: request for a property which does not have dedicated UI!" );
            break;
        }
        return eResult;
    }

    //--------------------------------------------------------------------
    namespace
    {
        void lcl_rebuildAndResetCommand( const Reference< XObjectInspectorUI >& _rxInspectorUI, const Reference< XPropertyHandler >& _rxHandler )
        {
            OSL_PRECOND( _rxInspectorUI.is(), "lcl_rebuildAndResetCommand: invalid BrowserUI!" );
            OSL_PRECOND( _rxHandler.is(), "lcl_rebuildAndResetCommand: invalid handler!" );
            _rxInspectorUI->rebuildPropertyUI( PROPERTY_COMMAND );
            _rxHandler->setPropertyValue( PROPERTY_COMMAND, makeAny( OUString() ) );
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL FormComponentPropertyHandler::actuatingPropertyChanged( const OUString& _rActuatingPropertyName, const Any& _rNewValue, const Any& /*_rOldValue*/, const Reference< XObjectInspectorUI >& _rxInspectorUI, sal_Bool _bFirstTimeInit ) throw (NullPointerException, RuntimeException)
    {
        if ( !_rxInspectorUI.is() )
            throw NullPointerException();

        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyId nActuatingPropId( impl_getPropertyId_throw( _rActuatingPropertyName ) );

        ::std::vector< PropertyId > aDependentProperties;

        switch ( nActuatingPropId )
        {
        // ----- EscapeProcessing -----
        case PROPERTY_ID_ESCAPE_PROCESSING:
            aDependentProperties.push_back( PROPERTY_ID_FILTER );
            aDependentProperties.push_back( PROPERTY_ID_SORT );
            break;  // case PROPERTY_ID_ESCAPE_PROCESSING

        // ----- CommandType -----
        case PROPERTY_ID_COMMANDTYPE:
            // available commands (tables or queries) might have changed
            if ( !_bFirstTimeInit && m_bHaveCommand )
                lcl_rebuildAndResetCommand( _rxInspectorUI, this );
            aDependentProperties.push_back( PROPERTY_ID_COMMAND );
            break;  // case PROPERTY_ID_COMMANDTYPE

        // ----- DataSourceName -----
        case PROPERTY_ID_DATASOURCE:
            // reset the connection, now that we have a new data source
            impl_clearRowsetConnection_nothrow();

            // available list source values (tables or queries) might have changed
            if ( !_bFirstTimeInit && m_bHaveListSource )
                _rxInspectorUI->rebuildPropertyUI( PROPERTY_LISTSOURCE );

            // available commands (tables or queries) might have changed
            if ( !_bFirstTimeInit && m_bHaveCommand )
                lcl_rebuildAndResetCommand( _rxInspectorUI, this );

            // Command also depends on DataSource
            aDependentProperties.push_back( PROPERTY_ID_COMMAND );
            // NO break!

        // ----- Command -----
        case PROPERTY_ID_COMMAND:
            aDependentProperties.push_back( PROPERTY_ID_FILTER );
            aDependentProperties.push_back( PROPERTY_ID_SORT );
            if ( m_bComponentIsSubForm )
                aDependentProperties.push_back( PROPERTY_ID_DETAILFIELDS );
            break;

        // ----- ListSourceType -----
        case PROPERTY_ID_LISTSOURCETYPE:
            if ( !_bFirstTimeInit && m_bHaveListSource )
                // available list source values (tables or queries) might have changed
                _rxInspectorUI->rebuildPropertyUI( PROPERTY_LISTSOURCE );
            aDependentProperties.push_back( PROPERTY_ID_STRINGITEMLIST );
            aDependentProperties.push_back( PROPERTY_ID_BOUNDCOLUMN );
            // NO break!

        // ----- StringItemList -----
        case PROPERTY_ID_STRINGITEMLIST:
            aDependentProperties.push_back( PROPERTY_ID_SELECTEDITEMS );
            aDependentProperties.push_back( PROPERTY_ID_DEFAULT_SELECT_SEQ );
            break;

        // ----- ListSource -----
        case PROPERTY_ID_LISTSOURCE:
            aDependentProperties.push_back( PROPERTY_ID_STRINGITEMLIST );
            break;

        // ----- DataField -----
        case PROPERTY_ID_CONTROLSOURCE:
        {
            OUString sControlSource;
            _rNewValue >>= sControlSource;
            if ( impl_componentHasProperty_throw( PROPERTY_FILTERPROPOSAL ) )
                _rxInspectorUI->enablePropertyUI( PROPERTY_FILTERPROPOSAL, !sControlSource.isEmpty() );
            if ( impl_componentHasProperty_throw( PROPERTY_EMPTY_IS_NULL ) )
                _rxInspectorUI->enablePropertyUI( PROPERTY_EMPTY_IS_NULL, !sControlSource.isEmpty() );

            aDependentProperties.push_back( PROPERTY_ID_BOUNDCOLUMN );
            aDependentProperties.push_back( PROPERTY_ID_SCALEIMAGE );
            aDependentProperties.push_back( PROPERTY_ID_SCALE_MODE );
            aDependentProperties.push_back( PROPERTY_ID_INPUT_REQUIRED );
        }
        break;

        case PROPERTY_ID_EMPTY_IS_NULL:
            aDependentProperties.push_back( PROPERTY_ID_INPUT_REQUIRED );
            break;

        // ----- SubmitEncoding -----
        case PROPERTY_ID_SUBMIT_ENCODING:
        {
            FormSubmitEncoding eEncoding = FormSubmitEncoding_URL;
            OSL_VERIFY( _rNewValue >>= eEncoding );
            _rxInspectorUI->enablePropertyUI( PROPERTY_SUBMIT_METHOD, eEncoding == FormSubmitEncoding_URL );
        }
        break;

        // ----- Repeat -----
        case PROPERTY_ID_REPEAT:
        {
            sal_Bool bIsRepeating = sal_False;
            OSL_VERIFY( _rNewValue >>= bIsRepeating );
            _rxInspectorUI->enablePropertyUI( PROPERTY_REPEAT_DELAY, bIsRepeating );
        }
        break;

        // ----- TabStop -----
        case PROPERTY_ID_TABSTOP:
        {
            if ( !impl_componentHasProperty_throw( PROPERTY_TABINDEX ) )
                break;
            sal_Bool bHasTabStop = sal_False;
            _rNewValue >>= bHasTabStop;
            _rxInspectorUI->enablePropertyUI( PROPERTY_TABINDEX, bHasTabStop );
        }
        break;

        // ----- Border -----
        case PROPERTY_ID_BORDER:
        {
            sal_Int16 nBordeType = VisualEffect::NONE;
            OSL_VERIFY( _rNewValue >>= nBordeType );
            _rxInspectorUI->enablePropertyUI( PROPERTY_BORDERCOLOR, nBordeType == VisualEffect::FLAT );
        }
        break;

        // ----- DropDown -----
        case PROPERTY_ID_DROPDOWN:
        {
            if ( impl_isSupportedProperty_nothrow( PROPERTY_ID_LINECOUNT ) )
            {
                sal_Bool bDropDown = sal_True;
                _rNewValue >>= bDropDown;
                _rxInspectorUI->enablePropertyUI( PROPERTY_LINECOUNT, bDropDown );
            }
        }
        break;

        // ----- ImageURL -----
        case PROPERTY_ID_IMAGE_URL:
        {
            if ( impl_isSupportedProperty_nothrow( PROPERTY_ID_IMAGEPOSITION ) )
            {
                OUString sImageURL;
                OSL_VERIFY( _rNewValue >>= sImageURL );
                _rxInspectorUI->enablePropertyUI( PROPERTY_IMAGEPOSITION, !sImageURL.isEmpty() );
            }

            aDependentProperties.push_back( PROPERTY_ID_SCALEIMAGE );
            aDependentProperties.push_back( PROPERTY_ID_SCALE_MODE );
        }
        break;

        // ----- ButtonType -----
        case PROPERTY_ID_BUTTONTYPE:
        {
            FormButtonType eButtonType( FormButtonType_PUSH );
            OSL_VERIFY( _rNewValue >>= eButtonType );
            _rxInspectorUI->enablePropertyUI( PROPERTY_TARGET_URL, FormButtonType_URL == eButtonType );
        }
        // NO break!

        // ----- TargetURL -----
        case PROPERTY_ID_TARGET_URL:
            aDependentProperties.push_back( PROPERTY_ID_TARGET_FRAME );
            break;  // case PROPERTY_ID_TARGET_URL

        // ----- TriState -----
        case PROPERTY_ID_TRISTATE:
            if ( !_bFirstTimeInit )
                _rxInspectorUI->rebuildPropertyUI( m_eComponentClass == eFormControl ? OUString(PROPERTY_DEFAULT_STATE) : OUString(PROPERTY_STATE) );
            break;  // case PROPERTY_ID_TRISTATE

        // ----- DecimalAccuracy -----
        case PROPERTY_ID_DECIMAL_ACCURACY:
        // ----- ShowThousandsSeparator -----
        case PROPERTY_ID_SHOWTHOUSANDSEP:
        {
            sal_Bool bAccuracy = (PROPERTY_ID_DECIMAL_ACCURACY == nActuatingPropId);
            sal_uInt16  nNewDigits = 0;
            sal_Bool    bUseSep = sal_False;
            if ( bAccuracy )
                OSL_VERIFY( _rNewValue >>= nNewDigits );
            else
                OSL_VERIFY( _rNewValue >>= bUseSep );

            // propagate the changes to the min/max/default fields
            Any aCurrentProp;
            OUString aAffectedProps[] = { OUString(PROPERTY_VALUE), OUString(PROPERTY_DEFAULT_VALUE), OUString(PROPERTY_VALUEMIN), OUString(PROPERTY_VALUEMAX) };
            for (sal_uInt16 i=0; i<SAL_N_ELEMENTS(aAffectedProps); ++i)
            {
                Reference< XPropertyControl > xControl;
                try
                {
                    xControl = _rxInspectorUI->getPropertyControl( aAffectedProps[i] );
                }
                catch( const UnknownPropertyException& ) {}
                if ( xControl.is() )
                {
                    OFormattedNumericControl* pControl = dynamic_cast< OFormattedNumericControl* >( xControl.get() );
                    DBG_ASSERT( pControl, "FormComponentPropertyHandler::actuatingPropertyChanged: invalid control!" );
                    if ( pControl )
                    {
                        if ( bAccuracy )
                            pControl->SetDecimalDigits( nNewDigits );
                        else
                            pControl->SetThousandsSep( bUseSep );
                    }
                }
            }
        }
        break;

        // ----- FormatKey -----
        case PROPERTY_ID_FORMATKEY:
        {
            FormatDescription aNewDesc;

            Reference< XNumberFormatsSupplier >  xSupplier;
            OSL_VERIFY( m_xComponent->getPropertyValue( PROPERTY_FORMATSSUPPLIER ) >>= xSupplier );

            Reference< XUnoTunnel > xTunnel( xSupplier, UNO_QUERY );
            DBG_ASSERT(xTunnel.is(), "FormComponentPropertyHandler::actuatingPropertyChanged: xTunnel is invalid!");
            if ( xTunnel.is() )
            {
                SvNumberFormatsSupplierObj* pSupplier = reinterpret_cast<SvNumberFormatsSupplierObj*>(xTunnel->getSomething(SvNumberFormatsSupplierObj::getUnoTunnelId()));
                    // the same again

                aNewDesc.pSupplier = pSupplier;
                if ( !( _rNewValue >>= aNewDesc.nKey ) )
                    aNewDesc.nKey = 0;

                // give each control which has to know this an own copy of the description
                OUString aFormattedPropertyControls[] = {
                    OUString(PROPERTY_EFFECTIVE_MIN), OUString(PROPERTY_EFFECTIVE_MAX), OUString(PROPERTY_EFFECTIVE_DEFAULT), OUString(PROPERTY_EFFECTIVE_VALUE)
                };
                for ( sal_uInt16 i=0; i<SAL_N_ELEMENTS(aFormattedPropertyControls); ++i )
                {
                    Reference< XPropertyControl > xControl;
                    try
                    {
                        xControl = _rxInspectorUI->getPropertyControl( aFormattedPropertyControls[i] );
                    }
                    catch( const UnknownPropertyException& ) {}
                    if ( xControl.is() )
                    {
                        OFormattedNumericControl* pControl = dynamic_cast< OFormattedNumericControl* >( xControl.get() );
                        DBG_ASSERT( pControl, "FormComponentPropertyHandler::actuatingPropertyChanged: invalid control!" );
                        if ( pControl )
                            pControl->SetFormatDescription( aNewDesc );
                    }
                }
            }
        }
        break;

        case PROPERTY_ID_TOGGLE:
        {
            sal_Bool bIsToggleButton = sal_False;
            OSL_VERIFY( _rNewValue >>= bIsToggleButton );
            _rxInspectorUI->enablePropertyUI( PROPERTY_DEFAULT_STATE, bIsToggleButton );
        }
        break;

        default:
            OSL_FAIL( "FormComponentPropertyHandler::actuatingPropertyChanged: did not register for this property!" );
            break;

        }   // switch ( nActuatingPropId )

        for ( ::std::vector< PropertyId >::const_iterator loopAffected = aDependentProperties.begin();
              loopAffected != aDependentProperties.end();
              ++loopAffected
            )
        {
            if ( impl_isSupportedProperty_nothrow( *loopAffected ) )
                impl_updateDependentProperty_nothrow( *loopAffected, _rxInspectorUI );
        }
    }

    //------------------------------------------------------------------------
    void FormComponentPropertyHandler::impl_updateDependentProperty_nothrow( PropertyId _nPropId, const Reference< XObjectInspectorUI >& _rxInspectorUI ) const
    {
        try
        {
            switch ( _nPropId )
            {
            // ----- StringItemList -----
            case PROPERTY_ID_STRINGITEMLIST:
            {
                ListSourceType eLSType = ListSourceType_VALUELIST;
                OSL_VERIFY( impl_getPropertyValue_throw( PROPERTY_LISTSOURCETYPE ) >>= eLSType );

                OUString sListSource;
                {
                    Sequence< OUString > aListSource;
                    Any aListSourceValue( impl_getPropertyValue_throw( PROPERTY_LISTSOURCE ) );
                    if ( aListSourceValue >>= aListSource )
                    {
                        if ( aListSource.getLength() )
                            sListSource = aListSource[0];
                    }
                    else
                        OSL_VERIFY( aListSourceValue >>= sListSource );
                }

                sal_Bool bIsEnabled =   (  ( eLSType == ListSourceType_VALUELIST )
                                        || ( sListSource.isEmpty() )
                                        );
                _rxInspectorUI->enablePropertyUI( PROPERTY_STRINGITEMLIST, bIsEnabled );
            }
            break;  // case PROPERTY_ID_STRINGITEMLIST

            // ----- BoundColumn -----
            case PROPERTY_ID_BOUNDCOLUMN:
            {
                ListSourceType eLSType = ListSourceType_VALUELIST;
                OSL_VERIFY( impl_getPropertyValue_throw( PROPERTY_LISTSOURCETYPE ) >>= eLSType );

                _rxInspectorUI->enablePropertyUI( PROPERTY_BOUNDCOLUMN,
                        ( eLSType != ListSourceType_VALUELIST )
                );
            }
            break;  // case PROPERTY_ID_BOUNDCOLUMN

            // ----- ScaleImage, ScaleMode -----
            case PROPERTY_ID_SCALEIMAGE:
            case PROPERTY_ID_SCALE_MODE:
            {
                OUString sControlSource;
                if ( impl_isSupportedProperty_nothrow( PROPERTY_ID_CONTROLSOURCE ) )
                    impl_getPropertyValue_throw( PROPERTY_CONTROLSOURCE ) >>= sControlSource;

                OUString sImageURL;
                impl_getPropertyValue_throw( PROPERTY_IMAGE_URL ) >>= sImageURL;

                _rxInspectorUI->enablePropertyUI( impl_getPropertyNameFromId_nothrow( _nPropId ),
                    ( !sControlSource.isEmpty() ) || ( !sImageURL.isEmpty() )
                );
            }
            break;  // case PROPERTY_ID_SCALEIMAGE, PROPERTY_ID_SCALE_MODE

            // ----- InputRequired -----
            case PROPERTY_ID_INPUT_REQUIRED:
            {
                OUString sControlSource;
                OSL_VERIFY( impl_getPropertyValue_throw( PROPERTY_CONTROLSOURCE ) >>= sControlSource );

                sal_Bool bEmptyIsNULL = sal_False;
                sal_Bool bHasEmptyIsNULL = impl_componentHasProperty_throw( PROPERTY_EMPTY_IS_NULL );
                if ( bHasEmptyIsNULL )
                    OSL_VERIFY( impl_getPropertyValue_throw( PROPERTY_EMPTY_IS_NULL ) >>= bEmptyIsNULL );

                // if the control is not bound to a DB field, there is no sense in having the "Input required"
                // property
                // Also, if an empty input of this control are *not* written as NULL, but as empty strings,
                // then "Input required" does not make sense, too (since there's always an input, even if the control
                // is empty).
                _rxInspectorUI->enablePropertyUI( PROPERTY_INPUT_REQUIRED,
                    ( !sControlSource.isEmpty() ) && ( !bHasEmptyIsNULL || bEmptyIsNULL )
                );
            }
            break;

            // ----- SelectedItems, DefaultSelection -----
            case PROPERTY_ID_SELECTEDITEMS:
            case PROPERTY_ID_DEFAULT_SELECT_SEQ:
            {
                Sequence< OUString > aEntries;
                impl_getPropertyValue_throw( PROPERTY_STRINGITEMLIST ) >>= aEntries;
                bool isEnabled = aEntries.getLength() != 0;

                if ( ( m_nClassId == FormComponentType::LISTBOX ) && ( m_eComponentClass == eFormControl ) )
                {
                    ListSourceType eLSType = ListSourceType_VALUELIST;
                    impl_getPropertyValue_throw( PROPERTY_LISTSOURCETYPE ) >>= eLSType;
                    isEnabled &= ( eLSType == ListSourceType_VALUELIST );
                }
                _rxInspectorUI->enablePropertyUIElements( impl_getPropertyNameFromId_nothrow( _nPropId ),
                    PropertyLineElement::PrimaryButton, isEnabled );
            }
            break;  // case PROPERTY_ID_DEFAULT_SELECT_SEQ

            // ----- TargetFrame ------
            case PROPERTY_ID_TARGET_FRAME:
            {
                OUString sTargetURL;
                impl_getPropertyValue_throw( PROPERTY_TARGET_URL ) >>= sTargetURL;
                FormButtonType eButtonType( FormButtonType_URL );
                if ( 0 != m_nClassId )
                {
                    OSL_VERIFY( impl_getPropertyValue_throw( PROPERTY_BUTTONTYPE ) >>= eButtonType );
                }
                // if m_nClassId is 0, then we're inspecting a form. In this case, eButtonType is always
                // FormButtonType_URL here
                _rxInspectorUI->enablePropertyUI( PROPERTY_TARGET_FRAME,
                    ( eButtonType == FormButtonType_URL ) && ( !sTargetURL.isEmpty() )
                );
            }
            break;

            // ----- Order ------
            case PROPERTY_ID_SORT:
            // ----- Filter ------
            case PROPERTY_ID_FILTER:
            {
                Reference< XConnection > xConnection;
                bool bAllowEmptyDS = ::dbtools::isEmbeddedInDatabase( m_xComponent, xConnection );

                // if there's no escape processing, we cannot enter any values for this property
                sal_Bool  bDoEscapeProcessing( sal_False );
                impl_getPropertyValue_throw( PROPERTY_ESCAPE_PROCESSING ) >>= bDoEscapeProcessing;
                _rxInspectorUI->enablePropertyUI(
                    impl_getPropertyNameFromId_nothrow( _nPropId ),
                    bDoEscapeProcessing
                );

                // also care for the browse button - enabled if we have escape processing, and a valid
                // data source signature
                _rxInspectorUI->enablePropertyUIElements(
                    impl_getPropertyNameFromId_nothrow( _nPropId ),
                    PropertyLineElement::PrimaryButton,
                        impl_hasValidDataSourceSignature_nothrow( m_xComponent, bAllowEmptyDS )
                    &&  bDoEscapeProcessing
                );
            }
            break;  // case PROPERTY_ID_FILTER:

            // ----- Command -----
            case PROPERTY_ID_COMMAND:
            {
                sal_Int32   nCommandType( CommandType::COMMAND );
                OSL_VERIFY( impl_getPropertyValue_throw( PROPERTY_COMMANDTYPE ) >>= nCommandType );

                impl_ensureRowsetConnection_nothrow();
                Reference< XConnection > xConnection = m_xRowSetConnection.getTyped();
                bool bAllowEmptyDS = false;
                if ( !xConnection.is() )
                    bAllowEmptyDS = ::dbtools::isEmbeddedInDatabase( m_xComponent, xConnection );

                bool doEnable = ( nCommandType == CommandType::COMMAND )
                            &&  (  m_xRowSetConnection.is()
                                || xConnection.is()
                                || impl_hasValidDataSourceSignature_nothrow( m_xComponent, bAllowEmptyDS)
                                );

                _rxInspectorUI->enablePropertyUIElements(
                    PROPERTY_COMMAND,
                    PropertyLineElement::PrimaryButton,
                    doEnable
                );
            }
            break;  // case PROPERTY_ID_COMMAND

            // ----- DetailFields -----
            case PROPERTY_ID_DETAILFIELDS:
            {
                Reference< XConnection > xConnection;
                bool bAllowEmptyDS = ::dbtools::isEmbeddedInDatabase( m_xComponent, xConnection );

                // both our current form, and it's parent form, need to have a valid
                // data source signature
                bool bDoEnableMasterDetailFields =
                        impl_hasValidDataSourceSignature_nothrow( m_xComponent, bAllowEmptyDS )
                    &&  impl_hasValidDataSourceSignature_nothrow( Reference< XPropertySet >( m_xObjectParent, UNO_QUERY ), bAllowEmptyDS );

                // in opposite to the other properties, here in real *two* properties are
                // affected
                _rxInspectorUI->enablePropertyUIElements( PROPERTY_DETAILFIELDS, PropertyLineElement::PrimaryButton, bDoEnableMasterDetailFields );
                _rxInspectorUI->enablePropertyUIElements( PROPERTY_MASTERFIELDS, PropertyLineElement::PrimaryButton, bDoEnableMasterDetailFields );
            }
            break;

            default:
                OSL_FAIL( "FormComponentPropertyHandler::impl_updateDependentProperty_nothrow: unexpected property to update!" );
                break;

            }   // switch
        }
        catch( const Exception& )
        {
            OSL_FAIL( "FormComponentPropertyHandler::impl_updateDependentProperty_nothrow: caught an exception!" );
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //------------------------------------------------------------------------
    void SAL_CALL FormComponentPropertyHandler::disposing()
    {
        FormComponentPropertyHandler_Base::disposing();
        if ( m_xCommandDesigner.is() && m_xCommandDesigner->isActive() )
            m_xCommandDesigner->dispose();
    }

    //------------------------------------------------------------------------
    sal_Bool SAL_CALL FormComponentPropertyHandler::suspend( sal_Bool _bSuspend ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( _bSuspend )
            if ( m_xCommandDesigner.is() && m_xCommandDesigner->isActive() )
                return m_xCommandDesigner->suspend();
        return sal_True;
    }

    //------------------------------------------------------------------------
    void FormComponentPropertyHandler::impl_initComponentMetaData_throw()
    {
        try
        {
            //////////////////////////////////////////////////////////////////
            // component class
            m_eComponentClass = eUnknown;

            if  (   impl_componentHasProperty_throw( PROPERTY_WIDTH )
                &&  impl_componentHasProperty_throw( PROPERTY_HEIGHT )
                &&  impl_componentHasProperty_throw( PROPERTY_POSITIONX )
                &&  impl_componentHasProperty_throw( PROPERTY_POSITIONY )
                &&  impl_componentHasProperty_throw( PROPERTY_STEP )
                &&  impl_componentHasProperty_throw( PROPERTY_TABINDEX )
                )
            {
                m_eComponentClass = eDialogControl;
            }
            else
            {
                m_eComponentClass = eFormControl;
            }

            //////////////////////////////////////////////////////////////////
            // (database) sub form?
            Reference< XForm > xAsForm( m_xComponent, UNO_QUERY );
            if ( xAsForm.is() )
            {
                Reference< XForm > xFormsParent = xFormsParent.query( xAsForm->getParent() );
                m_bComponentIsSubForm = xFormsParent.is();
            }

            //////////////////////////////////////////////////////////////////
            // ClassId
            Reference< XChild > xCompAsChild( m_xComponent, UNO_QUERY );
            if ( xCompAsChild.is() )
                m_xObjectParent = xCompAsChild->getParent();

            //////////////////////////////////////////////////////////////////
            // ClassId
            impl_classifyControlModel_throw();
        }
        catch( const RuntimeException& )
        {
            throw;
        }
        catch( const Exception& )
        {
            OSL_FAIL( "FormComponentPropertyHandler::impl_initComponentMetaData_throw: caught an exception!" );
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //------------------------------------------------------------------------
    void FormComponentPropertyHandler::impl_classifyControlModel_throw( )
    {
        if ( impl_componentHasProperty_throw( PROPERTY_CLASSID ) )
        {
            OSL_VERIFY( m_xComponent->getPropertyValue( PROPERTY_CLASSID ) >>= m_nClassId );
        }
        else if ( eDialogControl == m_eComponentClass )
        {
            Reference< XControlModel > xControlModel( m_xComponent, UNO_QUERY );
            Reference< XServiceInfo > xServiceInfo( m_xComponent, UNO_QUERY );
            if ( xServiceInfo.is() )
            {
                // it's a control model, and can tell about it's supported services
                m_nClassId = FormComponentType::CONTROL;

                const sal_Char* aControlModelServiceNames[] =
                {
                    "UnoControlButtonModel",
                    "UnoControlCheckBoxModel",
                    "UnoControlComboBoxModel",
                    "UnoControlCurrencyFieldModel",
                    "UnoControlDateFieldModel",
                    "UnoControlEditModel",
                    "UnoControlFileControlModel",
                    "UnoControlFixedTextModel",
                    "UnoControlGroupBoxModel",
                    "UnoControlImageControlModel",
                    "UnoControlListBoxModel",
                    "UnoControlNumericFieldModel",
                    "UnoControlPatternFieldModel",
                    "UnoControlRadioButtonModel",
                    "UnoControlScrollBarModel",
                    "UnoControlSpinButtonModel",
                    "UnoControlTimeFieldModel",

                    "UnoControlFixedLineModel",
                    "UnoControlFormattedFieldModel",
                    "UnoControlProgressBarModel"
                };
                const sal_Int16 nClassIDs[] =
                {
                    FormComponentType::COMMANDBUTTON,
                    FormComponentType::CHECKBOX,
                    FormComponentType::COMBOBOX,
                    FormComponentType::CURRENCYFIELD,
                    FormComponentType::DATEFIELD,
                    FormComponentType::TEXTFIELD,
                    FormComponentType::FILECONTROL,
                    FormComponentType::FIXEDTEXT,
                    FormComponentType::GROUPBOX,
                    FormComponentType::IMAGECONTROL,
                    FormComponentType::LISTBOX,
                    FormComponentType::NUMERICFIELD,
                    FormComponentType::PATTERNFIELD,
                    FormComponentType::RADIOBUTTON,
                    FormComponentType::SCROLLBAR,
                    FormComponentType::SPINBUTTON,
                    FormComponentType::TIMEFIELD,

                    ControlType::FIXEDLINE,
                    ControlType::FORMATTEDFIELD,
                    ControlType::PROGRESSBAR
                };

                sal_Int32 nKnownControlTypes = SAL_N_ELEMENTS( aControlModelServiceNames );
                OSL_ENSURE( nKnownControlTypes == SAL_N_ELEMENTS( nClassIDs ),
                    "FormComponentPropertyHandler::impl_classifyControlModel_throw: inconsistence" );

                for ( sal_Int32 i = 0; i < nKnownControlTypes; ++i )
                {
                    OUString sServiceName(  "com.sun.star.awt."  );
                    sServiceName += OUString::createFromAscii( aControlModelServiceNames[ i ] );

                    if ( xServiceInfo->supportsService( sServiceName ) )
                    {
                        m_nClassId = nClassIDs[ i ];
                        break;
                    }
                }
            }
        }
    }

    //------------------------------------------------------------------------
    void FormComponentPropertyHandler::impl_normalizePropertyValue_nothrow( Any& _rValue, PropertyId _nPropId ) const
    {
        switch ( _nPropId )
        {
        case PROPERTY_ID_TABSTOP:
            if ( !_rValue.hasValue() )
            {
                switch ( m_nClassId )
                {
                case FormComponentType::COMMANDBUTTON:
                case FormComponentType::RADIOBUTTON:
                case FormComponentType::CHECKBOX:
                case FormComponentType::TEXTFIELD:
                case FormComponentType::LISTBOX:
                case FormComponentType::COMBOBOX:
                case FormComponentType::FILECONTROL:
                case FormComponentType::DATEFIELD:
                case FormComponentType::TIMEFIELD:
                case FormComponentType::NUMERICFIELD:
                case ControlType::FORMATTEDFIELD:
                case FormComponentType::CURRENCYFIELD:
                case FormComponentType::PATTERNFIELD:
                    _rValue = makeAny( (sal_Bool)sal_True );
                    break;
                default:
                    _rValue = makeAny( (sal_Bool)sal_False );
                    break;
                }
            }
            break;
        }
    }

    //------------------------------------------------------------------------
    bool FormComponentPropertyHandler::impl_shouldExcludeProperty_nothrow( const Property& _rProperty ) const
    {
        OSL_ENSURE( _rProperty.Handle == m_pInfoService->getPropertyId( _rProperty.Name ),
            "FormComponentPropertyHandler::impl_shouldExcludeProperty_nothrow: insonsistency in the property!" );

        if ( _rProperty.Handle == PROPERTY_ID_CONTROLLABEL )
            // prevent that this is caught below
            return false;

        if  (   ( _rProperty.Type.getTypeClass() == TypeClass_INTERFACE )
            ||  ( _rProperty.Type.getTypeClass() == TypeClass_ARRAY )
            ||  ( _rProperty.Type.getTypeClass() == TypeClass_UNKNOWN )
            )
            return true;

        if ( ( _rProperty.Attributes & PropertyAttribute::TRANSIENT ) && ( m_eComponentClass != eDialogControl ) )
            // strange enough, dialog controls declare a lot of their properties as transient
            return true;

        if ( _rProperty.Attributes & PropertyAttribute::READONLY )
            return true;

        switch ( _rProperty.Handle )
        {
        case PROPERTY_ID_MASTERFIELDS:
        case PROPERTY_ID_DETAILFIELDS:
            if ( !m_bComponentIsSubForm )
                // no master and detail fields for forms which are no sub forms
                return true;
            break;

        case PROPERTY_ID_DATASOURCE:
        {
            // don't show DataSource if the component is part of an embedded form document
            Reference< XConnection > xConn;
            if ( isEmbeddedInDatabase( m_xComponent, xConn ) )
                return true;
        }
        break;

        case PROPERTY_ID_TEXT:
            // don't show the "Text" property of formatted fields
            if ( ControlType::FORMATTEDFIELD == m_nClassId )
                return true;
            break;

        case PROPERTY_ID_FORMATKEY:
        case PROPERTY_ID_EFFECTIVE_MIN:
        case PROPERTY_ID_EFFECTIVE_MAX:
        case PROPERTY_ID_EFFECTIVE_DEFAULT:
        case PROPERTY_ID_EFFECTIVE_VALUE:
            // only if the set has a formats supplier, too
            if  ( !impl_componentHasProperty_throw( PROPERTY_FORMATSSUPPLIER ) )
                return true;
            // (form) date and time fields also have a formats supplier, but the format itself
            // is reflected in another property
            if  (   ( FormComponentType::DATEFIELD == m_nClassId )
                ||  ( FormComponentType::TIMEFIELD == m_nClassId )
                )
                return true;
            break;

        case PROPERTY_ID_SCALEIMAGE:
            if ( impl_componentHasProperty_throw( PROPERTY_SCALE_MODE ) )
                // ScaleImage is superseded by ScaleMode
                return true;
            break;

        case PROPERTY_ID_WRITING_MODE:
            if ( !SvtCTLOptions().IsCTLFontEnabled() )
                return true;
            break;
        }

        sal_uInt32 nPropertyUIFlags = m_pInfoService->getPropertyUIFlags( _rProperty.Handle );

        // don't show experimental properties unless allowed to do so
        if ( ( nPropertyUIFlags & PROP_FLAG_EXPERIMENTAL ) != 0 )
        {
            if ( true ) // TODO
                return true;
        }

        // no data properties if no Base is installed. #124939#
        // This fix is not intendend to appear on the main trunk. If you find it there,
        // please tell me! frank.schoenheit@sun.com
        if ( ( nPropertyUIFlags & PROP_FLAG_DATA_PROPERTY ) != 0 )
            if ( !SvtModuleOptions().IsModuleInstalled( SvtModuleOptions::E_SDATABASE ) )
                return true;

        return false;
    }

    //------------------------------------------------------------------------
    Reference< XRowSet > FormComponentPropertyHandler::impl_getRowSet_throw( ) const
    {
        Reference< XRowSet > xRowSet = m_xRowSet;
        if ( !xRowSet.is() )
        {
            xRowSet.set( m_xComponent, UNO_QUERY );
            if ( !xRowSet.is() )
            {
                xRowSet = Reference< XRowSet >( m_xObjectParent, UNO_QUERY );
                if ( !xRowSet.is() )
                {
                    // are we inspecting a grid column?
                    if (Reference< XGridColumnFactory >( m_xObjectParent, UNO_QUERY) .is())
                    {   // yes
                        Reference< XChild > xParentAsChild( m_xObjectParent, UNO_QUERY );
                        if ( xParentAsChild.is() )
                            xRowSet = Reference< XRowSet >( xParentAsChild->getParent(), UNO_QUERY );
                    }
                }
                if ( !xRowSet.is() )
                    xRowSet = m_xRowSet;
            }
            DBG_ASSERT( xRowSet.is(), "FormComponentPropertyHandler::impl_getRowSet_throw: could not obtain the rowset for the introspectee!" );
        }
        return xRowSet;
    }

    //------------------------------------------------------------------------
    Reference< XRowSet > FormComponentPropertyHandler::impl_getRowSet_nothrow( ) const
    {
        Reference< XRowSet > xReturn;
        try
        {
            xReturn = impl_getRowSet_throw();
        }
        catch( const Exception& )
        {
            OSL_FAIL( "FormComponentPropertyHandler::impl_getRowSet_nothrow: caught an exception!" );
            DBG_UNHANDLED_EXCEPTION();
        }
        return xReturn;
    }

    //------------------------------------------------------------------------
    void FormComponentPropertyHandler::impl_initFieldList_nothrow( ::std::vector< OUString >& _rFieldNames ) const
    {
        clearContainer( _rFieldNames );
        try
        {
            WaitCursor aWaitCursor( impl_getDefaultDialogParent_nothrow() );

            Reference< XPreparedStatement >  xStatement;

            // get the form of the control we're inspecting
            Reference< XPropertySet > xFormSet( impl_getRowSet_throw(), UNO_QUERY );
            if ( !xFormSet.is() )
                return;

            OUString sObjectName;
            OSL_VERIFY( xFormSet->getPropertyValue( PROPERTY_COMMAND ) >>= sObjectName );
            // when there is no command we don't need to ask for columns
            if ( !sObjectName.isEmpty() && impl_ensureRowsetConnection_nothrow() )
            {
                OUString aDatabaseName;
                OSL_VERIFY( xFormSet->getPropertyValue( PROPERTY_DATASOURCE ) >>= aDatabaseName );
                sal_Int32 nObjectType = CommandType::COMMAND;
                OSL_VERIFY( xFormSet->getPropertyValue( PROPERTY_COMMANDTYPE ) >>= nObjectType );

                Sequence< OUString > aFields( ::dbtools::getFieldNamesByCommandDescriptor( m_xRowSetConnection, nObjectType, sObjectName ) );

                const OUString* pFields = aFields.getConstArray();
                for ( sal_Int32 i = 0; i < aFields.getLength(); ++i, ++pFields )
                    _rFieldNames.push_back( *pFields );
            }
        }
        catch (const Exception&)
        {
            OSL_FAIL( "FormComponentPropertyHandler::impl_initFieldList_nothrow: caught an exception!" );
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //------------------------------------------------------------------------
    void FormComponentPropertyHandler::impl_clearRowsetConnection_nothrow()
    {
        m_xRowSetConnection.clear();
    }

    //------------------------------------------------------------------------
    void FormComponentPropertyHandler::impl_displaySQLError_nothrow( const ::dbtools::SQLExceptionInfo& _rErrorDescriptor ) const
    {
        ::dbtools::showError( _rErrorDescriptor, VCLUnoHelper::GetInterface( impl_getDefaultDialogParent_nothrow() ), m_xContext );
    }

    //------------------------------------------------------------------------
    bool FormComponentPropertyHandler::impl_ensureRowsetConnection_nothrow() const
    {
        if ( !m_xRowSetConnection.is() )
        {
            uno::Reference<sdbc::XConnection> xConnection;
            Any any = m_xContext->getValueByName( "ActiveConnection" );
            any >>= xConnection;
            m_xRowSetConnection.reset(xConnection,::dbtools::SharedConnection::NoTakeOwnership);
        }
        if ( m_xRowSetConnection.is() )
            return true;

        Reference< XRowSet > xRowSet( impl_getRowSet_throw() );
        Reference< XPropertySet > xRowSetProps( xRowSet, UNO_QUERY );

        // connect the row set - this is delegated to elsewhere - while observing errors
        SQLExceptionInfo aError;
        try
        {
            if ( xRowSetProps.is() )
            {
                WaitCursor aWaitCursor( impl_getDefaultDialogParent_nothrow() );
                m_xRowSetConnection = ::dbtools::ensureRowSetConnection( xRowSet, m_xContext, false );
            }
        }
        catch ( const SQLException& ) { aError = SQLExceptionInfo( ::cppu::getCaughtException() ); }
        catch ( const WrappedTargetException& e ) { aError = SQLExceptionInfo( e.TargetException ); }
        catch ( const Exception& ) { DBG_UNHANDLED_EXCEPTION(); }

        // report errors, if necessary
        if ( aError.isValid() )
        {
            OUString sDataSourceName;
            try
            {
                xRowSetProps->getPropertyValue( PROPERTY_DATASOURCE ) >>= sDataSourceName;
            }
            catch( const Exception& )
            {
                OSL_FAIL( "FormComponentPropertyHandler::impl_ensureRowsetConnection_nothrow: caught an exception during error handling!" );
                DBG_UNHANDLED_EXCEPTION();
            }
            // additional info about what happended
            INetURLObject aParser( sDataSourceName );
            if ( aParser.GetProtocol() != INET_PROT_NOT_VALID )
                sDataSourceName = aParser.getBase( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );
            OUString sInfo(PcrRes(RID_STR_UNABLETOCONNECT).toString().replaceAll("$name$", sDataSourceName));
            SQLContext aContext;
            aContext.Message = sInfo;
            aContext.NextException = aError.get();
            impl_displaySQLError_nothrow( aContext );
        }

        return m_xRowSetConnection.is();
    }

    //------------------------------------------------------------------------
    void FormComponentPropertyHandler::impl_describeCursorSource_nothrow( LineDescriptor& _out_rProperty, const Reference< XPropertyControlFactory >& _rxControlFactory ) const
    {
        try
        {
            WaitCursor aWaitCursor( impl_getDefaultDialogParent_nothrow() );

            ////////////////////////////////////////////////////////////
            // Setzen der UI-Daten
            _out_rProperty.DisplayName = m_pInfoService->getPropertyTranslation( PROPERTY_ID_COMMAND );

            _out_rProperty.HelpURL = HelpIdUrl::getHelpURL( m_pInfoService->getPropertyHelpId( PROPERTY_ID_COMMAND ) );
            _out_rProperty.PrimaryButtonId = OUString::createFromAscii(UID_PROP_DLG_SQLCOMMAND);

            ////////////////////////////////////////////////////////////
            sal_Int32 nCommandType = CommandType::COMMAND;
            impl_getPropertyValue_throw( PROPERTY_COMMANDTYPE ) >>= nCommandType;

            switch ( nCommandType )
            {
            case CommandType::TABLE:
            case CommandType::QUERY:
            {
                ::std::vector< OUString > aNames;
                if ( impl_ensureRowsetConnection_nothrow() )
                {
                    if ( nCommandType == CommandType::TABLE )
                        impl_fillTableNames_throw( aNames );
                    else
                        impl_fillQueryNames_throw( aNames );
                }
                _out_rProperty.Control = PropertyHandlerHelper::createComboBoxControl( _rxControlFactory, aNames, sal_False, sal_True );
            }
            break;

            default:
                _out_rProperty.Control = _rxControlFactory->createPropertyControl( PropertyControlType::MultiLineTextField, sal_False );
                break;
            }
        }
        catch (const Exception&)
        {
            OSL_FAIL("FormComponentPropertyHandler::impl_describeCursorSource_nothrow: caught an exception !");
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //------------------------------------------------------------------------
    void FormComponentPropertyHandler::impl_fillTableNames_throw( ::std::vector< OUString >& _out_rNames ) const
    {
        OSL_PRECOND( m_xRowSetConnection.is(), "FormComponentPropertyHandler::impl_fillTableNames_throw: need a connection!" );
        _out_rNames.resize( 0 );

        Reference< XTablesSupplier > xSupplyTables( m_xRowSetConnection, UNO_QUERY );
        Reference< XNameAccess > xTableNames;
        if ( xSupplyTables.is() )
            xTableNames = xSupplyTables->getTables();
        DBG_ASSERT( xTableNames.is(), "FormComponentPropertyHandler::impl_fillTableNames_throw: no way to obtain the tables of the connection!" );
        if ( !xTableNames.is() )
            return;

        Sequence< OUString> aTableNames = xTableNames->getElementNames();
        sal_uInt32 nCount = aTableNames.getLength();
        const OUString* pTableNames = aTableNames.getConstArray();

        for ( sal_uInt32 i=0; i<nCount; ++i ,++pTableNames )
            _out_rNames.push_back( *pTableNames );
    }

    //------------------------------------------------------------------------
    void FormComponentPropertyHandler::impl_fillQueryNames_throw( ::std::vector< OUString >& _out_rNames ) const
    {
        OSL_PRECOND( m_xRowSetConnection.is(), "FormComponentPropertyHandler::impl_fillQueryNames_throw: need a connection!" );
        _out_rNames.resize( 0 );

        Reference< XQueriesSupplier > xSupplyQueries( m_xRowSetConnection, UNO_QUERY );
        Reference< XNameAccess > xQueryNames;
        if ( xSupplyQueries.is() )
        {
            xQueryNames = xSupplyQueries->getQueries();
            impl_fillQueryNames_throw(xQueryNames,_out_rNames);
        }
    }
    //------------------------------------------------------------------------
    void FormComponentPropertyHandler::impl_fillQueryNames_throw( const Reference< XNameAccess >& _xQueryNames,::std::vector< OUString >& _out_rNames,const OUString& _sName ) const
    {
        DBG_ASSERT( _xQueryNames.is(), "FormComponentPropertyHandler::impl_fillQueryNames_throw: no way to obtain the queries of the connection!" );
        if ( !_xQueryNames.is() )
            return;

        Sequence< OUString> aQueryNames = _xQueryNames->getElementNames();
        sal_uInt32 nCount = aQueryNames.getLength();
        const OUString* pQueryNames = aQueryNames.getConstArray();
        sal_Bool bAdd = !_sName.isEmpty();

        for ( sal_uInt32 i=0; i<nCount; i++, ++pQueryNames )
        {
            OUStringBuffer sTemp;
            if ( bAdd )
            {
                sTemp.append(_sName);
                sTemp.appendAscii("/");
            }
            sTemp.append(*pQueryNames);
            Reference< XNameAccess > xSubQueries(_xQueryNames->getByName(*pQueryNames),UNO_QUERY);
            if ( xSubQueries.is() )
                impl_fillQueryNames_throw(xSubQueries,_out_rNames,sTemp.makeStringAndClear());
            else
                _out_rNames.push_back( sTemp.makeStringAndClear() );
        }
    }

    //------------------------------------------------------------------------
    void FormComponentPropertyHandler::impl_describeListSourceUI_throw( LineDescriptor& _out_rDescriptor, const Reference< XPropertyControlFactory >& _rxControlFactory ) const
    {
        OSL_PRECOND( m_xComponent.is(), "FormComponentPropertyHandler::impl_describeListSourceUI_throw: no component!" );

        ////////////////////////////////////////////////////////////
        // Auslesen des ListSourceTypes
        Any aListSourceType( m_xComponent->getPropertyValue( PROPERTY_LISTSOURCETYPE ) );

        sal_Int32 nListSourceType = ListSourceType_VALUELIST;
        ::cppu::enum2int( nListSourceType, aListSourceType );

        _out_rDescriptor.DisplayName = m_pInfoService->getPropertyTranslation( PROPERTY_ID_LISTSOURCE );
        _out_rDescriptor.HelpURL = HelpIdUrl::getHelpURL( m_pInfoService->getPropertyHelpId( PROPERTY_ID_LISTSOURCE ) );

        ////////////////////////////////////////////////////////////
        // Enums setzen
        switch( nListSourceType )
        {
        case ListSourceType_VALUELIST:
            _out_rDescriptor.Control = _rxControlFactory->createPropertyControl( PropertyControlType::StringListField, sal_False );
            break;

        case ListSourceType_TABLEFIELDS:
        case ListSourceType_TABLE:
        case ListSourceType_QUERY:
        {
            ::std::vector< OUString > aListEntries;
            if ( impl_ensureRowsetConnection_nothrow() )
            {
                if ( nListSourceType == ListSourceType_QUERY )
                    impl_fillQueryNames_throw( aListEntries );
                else
                    impl_fillTableNames_throw( aListEntries );
            }
            _out_rDescriptor.Control = PropertyHandlerHelper::createComboBoxControl( _rxControlFactory, aListEntries, sal_False, sal_False );
        }
        break;
        case ListSourceType_SQL:
        case ListSourceType_SQLPASSTHROUGH:
            impl_ensureRowsetConnection_nothrow();
            _out_rDescriptor.HasPrimaryButton = m_xRowSetConnection.is();
            break;
        }
    }

    //------------------------------------------------------------------------
    bool FormComponentPropertyHandler::impl_dialogListSelection_nothrow( const OUString& _rProperty, ::osl::ClearableMutexGuard& _rClearBeforeDialog ) const
    {
        OSL_PRECOND( m_pInfoService.get(), "FormComponentPropertyHandler::impl_dialogListSelection_nothrow: no property meta data!" );

        String sPropertyUIName( m_pInfoService->getPropertyTranslation( m_pInfoService->getPropertyId( _rProperty ) ) );
        ListSelectionDialog aDialog( impl_getDefaultDialogParent_nothrow(), m_xComponent, _rProperty, sPropertyUIName );
        _rClearBeforeDialog.clear();
        return ( RET_OK == aDialog.Execute() );
    }

    //------------------------------------------------------------------------
    bool FormComponentPropertyHandler::impl_dialogFilterOrSort_nothrow( bool _bFilter, OUString& _out_rSelectedClause, ::osl::ClearableMutexGuard& _rClearBeforeDialog ) const
    {
        OSL_PRECOND( Reference< XRowSet >( m_xComponent, UNO_QUERY ).is(),
            "FormComponentPropertyHandler::impl_dialogFilterOrSort_nothrow: to be called for forms only!" );

        _out_rSelectedClause = OUString();
        bool bSuccess = false;
        SQLExceptionInfo aErrorInfo;
        try
        {
            if ( !impl_ensureRowsetConnection_nothrow() )
                return false;

            // get a composer for the statement which the form is currently based on
            Reference< XSingleSelectQueryComposer > xComposer( ::dbtools::getCurrentSettingsComposer( m_xComponent, m_xContext ) );
            OSL_ENSURE( xComposer.is(), "FormComponentPropertyHandler::impl_dialogFilterOrSort_nothrow: could not obtain a composer!" );
            if ( !xComposer.is() )
                return false;

            OUString sPropertyUIName( m_pInfoService->getPropertyTranslation( _bFilter ? PROPERTY_ID_FILTER : PROPERTY_ID_SORT ) );

            // create the dialog
            Reference< XExecutableDialog > xDialog;
            if ( _bFilter)
            {
                xDialog.set( sdb::FilterDialog::createDefault(m_xContext) );
            }
            else
            {
                xDialog.set( sdb::OrderDialog::createDefault(m_xContext) );
            }


            // initialize the dialog
            Reference< XPropertySet > xDialogProps( xDialog, UNO_QUERY_THROW );
            xDialogProps->setPropertyValue("QueryComposer", makeAny( xComposer ) );
            xDialogProps->setPropertyValue("RowSet",        makeAny( m_xComponent ) );
            xDialogProps->setPropertyValue("ParentWindow",  makeAny( VCLUnoHelper::GetInterface( impl_getDefaultDialogParent_nothrow() ) ) );
            xDialogProps->setPropertyValue("Title",         makeAny( sPropertyUIName ) );

            _rClearBeforeDialog.clear();
            bSuccess = ( xDialog->execute() != 0 );
            if ( bSuccess )
                _out_rSelectedClause = _bFilter ? xComposer->getFilter() : xComposer->getOrder();
        }
        catch (const SQLContext& e) { aErrorInfo = e; }
        catch (const SQLWarning& e) { aErrorInfo = e; }
        catch (const SQLException& e) { aErrorInfo = e; }
        catch( const Exception& )
        {
            OSL_FAIL( "FormComponentPropertyHandler::impl_dialogFilterOrSort_nothrow: caught an exception!" );
            DBG_UNHANDLED_EXCEPTION();
        }

        if ( aErrorInfo.isValid() )
            impl_displaySQLError_nothrow( aErrorInfo );

        return bSuccess;
    }

    //------------------------------------------------------------------------
    bool FormComponentPropertyHandler::impl_dialogLinkedFormFields_nothrow( ::osl::ClearableMutexGuard& _rClearBeforeDialog ) const
    {
        Reference< XForm > xDetailForm( m_xComponent, UNO_QUERY );
        Reference< XForm > xMasterForm( m_xObjectParent, UNO_QUERY );
        uno::Reference<beans::XPropertySet> xMasterProp(m_xObjectParent,uno::UNO_QUERY);
        OSL_PRECOND( xDetailForm.is() && xMasterForm.is(), "FormComponentPropertyHandler::impl_dialogLinkedFormFields_nothrow: no forms!" );
        if ( !xDetailForm.is() || !xMasterForm.is() )
            return false;


        FormLinkDialog aDialog( impl_getDefaultDialogParent_nothrow(), m_xComponent, xMasterProp, m_xContext );
        _rClearBeforeDialog.clear();
        return ( RET_OK == aDialog.Execute() );
    }

    //------------------------------------------------------------------------
    bool FormComponentPropertyHandler::impl_dialogFormatting_nothrow( Any& _out_rNewValue, ::osl::ClearableMutexGuard& _rClearBeforeDialog ) const
    {
        bool bChanged = false;
        try
        {
            // create the itemset for the dialog
            SfxItemSet aCoreSet(SFX_APP()->GetPool(),
                SID_ATTR_NUMBERFORMAT_VALUE, SID_ATTR_NUMBERFORMAT_VALUE,
                SID_ATTR_NUMBERFORMAT_INFO, SID_ATTR_NUMBERFORMAT_INFO,
                0);     // ripped this somewhere ... don't understand it :(

            // get the number formats supplier
            Reference< XNumberFormatsSupplier >  xSupplier;
            m_xComponent->getPropertyValue( PROPERTY_FORMATSSUPPLIER ) >>= xSupplier;

            DBG_ASSERT(xSupplier.is(), "FormComponentPropertyHandler::impl_dialogFormatting_nothrow: invalid call !" );
            Reference< XUnoTunnel > xTunnel( xSupplier, UNO_QUERY_THROW );
            SvNumberFormatsSupplierObj* pSupplier =
                reinterpret_cast< SvNumberFormatsSupplierObj* >( xTunnel->getSomething( SvNumberFormatsSupplierObj::getUnoTunnelId() ) );
            DBG_ASSERT( pSupplier != NULL, "FormComponentPropertyHandler::impl_dialogFormatting_nothrow: invalid call !" );

            sal_Int32 nFormatKey = 0;
            impl_getPropertyValue_throw( PROPERTY_FORMATKEY ) >>= nFormatKey;
            aCoreSet.Put( SfxUInt32Item( SID_ATTR_NUMBERFORMAT_VALUE, nFormatKey ) );

            SvNumberFormatter* pFormatter = pSupplier->GetNumberFormatter();
            double dPreviewVal = OFormatSampleControl::getPreviewValue(pFormatter,nFormatKey);
            SvxNumberInfoItem aFormatter( pFormatter, dPreviewVal, PcrRes(RID_STR_TEXT_FORMAT).toString(), SID_ATTR_NUMBERFORMAT_INFO );
            aCoreSet.Put( aFormatter );

            // a tab dialog with a single page
            boost::scoped_ptr< SfxSingleTabDialog > xDialog(new SfxSingleTabDialog(
                impl_getDefaultDialogParent_nothrow(), aCoreSet,
                "FormatNumberDialog", "cui/ui/formatnumberdialog.ui"));
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            DBG_ASSERT( pFact, "CreateFactory fail!" );
            ::CreateTabPage fnCreatePage = pFact->GetTabPageCreatorFunc( RID_SVXPAGE_NUMBERFORMAT );
            if ( !fnCreatePage )
                throw RuntimeException();   // caught below

            SfxTabPage* pPage = (*fnCreatePage)( xDialog->get_content_area(), aCoreSet );
            xDialog->setTabPage( pPage );

            _rClearBeforeDialog.clear();
            if ( RET_OK == xDialog->Execute() )
            {
                const SfxItemSet* pResult = xDialog->GetOutputItemSet();

                const SfxPoolItem* pItem = pResult->GetItem( SID_ATTR_NUMBERFORMAT_INFO );
                const SvxNumberInfoItem* pInfoItem = dynamic_cast< const SvxNumberInfoItem* >( pItem );
                if (pInfoItem && pInfoItem->GetDelCount())
                {
                    const sal_uInt32* pDeletedKeys = pInfoItem->GetDelArray();

                    for (sal_uInt16 i=0; i< pInfoItem->GetDelCount(); ++i, ++pDeletedKeys)
                        pFormatter->DeleteEntry(*pDeletedKeys);
                }

                pItem = NULL;
                if ( SFX_ITEM_SET == pResult->GetItemState( SID_ATTR_NUMBERFORMAT_VALUE, sal_False, &pItem ) )
                {
                    _out_rNewValue <<= (sal_Int32)( static_cast< const SfxUInt32Item* >( pItem )->GetValue() );
                    bChanged = true;
                }
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "FormComponentPropertyHandler::impl_dialogFormatting_nothrow: : caught an exception!" );
            DBG_UNHANDLED_EXCEPTION();
        }
        return bChanged;
    }

    //------------------------------------------------------------------------
    bool FormComponentPropertyHandler::impl_browseForImage_nothrow( Any& _out_rNewValue, ::osl::ClearableMutexGuard& _rClearBeforeDialog ) const
    {
        bool bIsLink = true;// reflect the legacy behavior
        OUString aStrTrans = m_pInfoService->getPropertyTranslation( PROPERTY_ID_IMAGE_URL );

        ::sfx2::FileDialogHelper aFileDlg(
                ui::dialogs::TemplateDescription::FILEOPEN_LINK_PREVIEW,
                SFXWB_GRAPHIC);

        aFileDlg.SetTitle(aStrTrans);
        // non-linked images ( e.g. those located in the document
        // stream ) only if document is available
        Reference< XModel > xModel( impl_getContextDocument_nothrow() );
        bool bHandleNonLink = false;

        Reference< XFilePickerControlAccess > xController(aFileDlg.GetFilePicker(), UNO_QUERY);
        DBG_ASSERT(xController.is(), "FormComponentPropertyHandler::impl_browseForImage_nothrow: missing the controller interface on the file picker!");
        if (xController.is())
        {
            // do a preview by default
            xController->setValue(ExtendedFilePickerElementIds::CHECKBOX_PREVIEW, 0, ::cppu::bool2any(sal_True));

            xController->setValue(ExtendedFilePickerElementIds::CHECKBOX_LINK, 0, ::cppu::bool2any(bIsLink));
            xController->enableControl(ExtendedFilePickerElementIds::CHECKBOX_LINK, bHandleNonLink );

        }

        OUString sCurValue;
        OSL_VERIFY( impl_getPropertyValue_throw( PROPERTY_IMAGE_URL ) >>= sCurValue );
        if ( !sCurValue.isEmpty() && !sCurValue.startsWith(GRAPHOBJ_URLPREFIX) )
        {
            aFileDlg.SetDisplayDirectory( sCurValue );
            // TODO: need to set the display directory _and_ the default name
        }

        _rClearBeforeDialog.clear();
        bool bSuccess = ( 0 == aFileDlg.Execute() );
        if ( bSuccess )
        {
            if ( bHandleNonLink && xController.is() )
            {
                xController->getValue(ExtendedFilePickerElementIds::CHECKBOX_LINK, 0) >>= bIsLink;
            }
            if ( !bIsLink )
            {
                Graphic aGraphic;
                aFileDlg.GetGraphic( aGraphic );

                Reference< graphic::XGraphicObject > xGrfObj = graphic::GraphicObject::create( m_xContext );
                xGrfObj->setGraphic( aGraphic.GetXGraphic() );


                _out_rNewValue <<= xGrfObj;

            }
            else
                _out_rNewValue <<= (OUString)aFileDlg.GetPath();
        }
        return bSuccess;
    }

    //------------------------------------------------------------------------
    bool FormComponentPropertyHandler::impl_browseForTargetURL_nothrow( Any& _out_rNewValue, ::osl::ClearableMutexGuard& _rClearBeforeDialog ) const
    {
        ::sfx2::FileDialogHelper aFileDlg(
                ui::dialogs::TemplateDescription::FILEOPEN_READONLY_VERSION, 0);

        OUString sURL;
        OSL_VERIFY( impl_getPropertyValue_throw( PROPERTY_TARGET_URL ) >>= sURL );
        INetURLObject aParser( sURL );
        if ( INET_PROT_FILE == aParser.GetProtocol() )
            // set the initial directory only for file-URLs. Everything else
            // is considered to be potentially expensive
            aFileDlg.SetDisplayDirectory( sURL );

        _rClearBeforeDialog.clear();
        bool bSuccess = ( 0 == aFileDlg.Execute() );
        if ( bSuccess )
            _out_rNewValue <<= (OUString)aFileDlg.GetPath();
        return bSuccess;
    }

    //------------------------------------------------------------------------
    bool FormComponentPropertyHandler::impl_executeFontDialog_nothrow( Any& _out_rNewValue, ::osl::ClearableMutexGuard& _rClearBeforeDialog ) const
    {
        bool bSuccess = false;

        // create an item set for use with the dialog
        SfxItemSet* pSet = NULL;
        SfxItemPool* pPool = NULL;
        SfxPoolItem** pDefaults = NULL;
        ControlCharacterDialog::createItemSet(pSet, pPool, pDefaults);
        ControlCharacterDialog::translatePropertiesToItems(m_xComponent, pSet);

        {   // do this in an own block. The dialog needs to be destroyed before we call
            // destroyItemSet
            ControlCharacterDialog aDlg( impl_getDefaultDialogParent_nothrow(), *pSet );
            _rClearBeforeDialog.clear();
            if ( RET_OK == aDlg.Execute() )
            {
                const SfxItemSet* pOut = aDlg.GetOutputItemSet();
                if ( pOut )
                {
                    Sequence< NamedValue > aFontPropertyValues;
                    ControlCharacterDialog::translateItemsToProperties( *pOut, aFontPropertyValues );
                    _out_rNewValue <<= aFontPropertyValues;
                    bSuccess = true;
                }
            }
        }

        ControlCharacterDialog::destroyItemSet(pSet, pPool, pDefaults);
        return bSuccess;
    }

    //------------------------------------------------------------------------
    bool FormComponentPropertyHandler::impl_browseForDatabaseDocument_throw( Any& _out_rNewValue, ::osl::ClearableMutexGuard& _rClearBeforeDialog ) const
    {
        ::sfx2::FileDialogHelper aFileDlg(
                ui::dialogs::TemplateDescription::FILEOPEN_READONLY_VERSION, 0,
                OUString("sdatabase"));

        OUString sDataSource;
        OSL_VERIFY( impl_getPropertyValue_throw( PROPERTY_DATASOURCE ) >>= sDataSource );
        INetURLObject aParser( sDataSource );
        if ( INET_PROT_FILE == aParser.GetProtocol() )
            // set the initial directory only for file-URLs. Everything else
            // is considered to be potentially expensive
            aFileDlg.SetDisplayDirectory( sDataSource );

        const SfxFilter* pFilter = SfxFilter::GetFilterByName(OUString("StarOffice XML (Base)"));
        OSL_ENSURE(pFilter,"Filter: StarOffice XML (Base) could not be found!");
        if ( pFilter )
        {
            aFileDlg.SetCurrentFilter(pFilter->GetUIName());
            //aFileDlg.AddFilter(pFilter->GetFilterName(),pFilter->GetDefaultExtension());
        }

        _rClearBeforeDialog.clear();
        bool bSuccess = ( 0 == aFileDlg.Execute() );
        if ( bSuccess )
            _out_rNewValue <<= (OUString)aFileDlg.GetPath();
        return bSuccess;
    }

    //------------------------------------------------------------------------
    bool FormComponentPropertyHandler::impl_dialogColorChooser_throw( sal_Int32 _nColorPropertyId, Any& _out_rNewValue, ::osl::ClearableMutexGuard& _rClearBeforeDialog ) const
    {
        sal_Int32 nColor = 0;
        OSL_VERIFY( impl_getPropertyValue_throw( impl_getPropertyNameFromId_nothrow( _nColorPropertyId ) ) >>= nColor );
        ::Color aColor( nColor );
        SvColorDialog aColorDlg( impl_getDefaultDialogParent_nothrow() );
        aColorDlg.SetColor( aColor );

        _rClearBeforeDialog.clear();
        if ( !aColorDlg.Execute() )
            return false;

        aColor = aColorDlg.GetColor();
        nColor = aColor.GetColor();
        _out_rNewValue <<= (sal_Int32)nColor;
        return true;
    }

    //------------------------------------------------------------------------
    bool FormComponentPropertyHandler::impl_dialogChooseLabelControl_nothrow( Any& _out_rNewValue, ::osl::ClearableMutexGuard& _rClearBeforeDialog ) const
    {
        OSelectLabelDialog dlgSelectLabel( impl_getDefaultDialogParent_nothrow(), m_xComponent );
        _rClearBeforeDialog.clear();
        bool bSuccess = ( RET_OK == dlgSelectLabel.Execute() );
        if ( bSuccess )
            _out_rNewValue <<= dlgSelectLabel.GetSelected();
        return bSuccess;
    }

    //------------------------------------------------------------------------
    Reference< XControlContainer > FormComponentPropertyHandler::impl_getContextControlContainer_nothrow() const
    {
        Reference< XControlContainer > xControlContext;
        Any any = m_xContext->getValueByName( "ControlContext" );
        any >>= xControlContext;
        return xControlContext;
    }

    //------------------------------------------------------------------------
    bool FormComponentPropertyHandler::impl_dialogChangeTabOrder_nothrow( ::osl::ClearableMutexGuard& _rClearBeforeDialog ) const
    {
        OSL_PRECOND( impl_getContextControlContainer_nothrow().is(), "FormComponentPropertyHandler::impl_dialogChangeTabOrder_nothrow: invalid control context!" );

        Reference< XTabControllerModel > xTabControllerModel( impl_getRowSet_nothrow(), UNO_QUERY );
        TabOrderDialog aDialog(
            impl_getDefaultDialogParent_nothrow(),
            xTabControllerModel,
            impl_getContextControlContainer_nothrow(),
            m_xContext
        );
        _rClearBeforeDialog.clear();
        return ( RET_OK == aDialog.Execute() );
    }

    //------------------------------------------------------------------------
    namespace
    {
        //--------------------------------------------------------------------
        //- ISQLCommandPropertyUI
        //--------------------------------------------------------------------
        class ISQLCommandPropertyUI : public ISQLCommandAdapter
        {
        public:
            /** returns the empty-string-terminated list of names of properties
                whose UI is to be disabled while the SQL command property is
                being edited.
            */
            virtual OUString*    getPropertiesToDisable() = 0;
        };

        //--------------------------------------------------------------------
        //- SQLCommandPropertyUI
        //--------------------------------------------------------------------
        class SQLCommandPropertyUI : public ISQLCommandPropertyUI
        {
        protected:
            SQLCommandPropertyUI( const Reference< XPropertySet >& _rxObject )
                :m_xObject( _rxObject )
            {
                if ( !m_xObject.is() )
                    throw NullPointerException();
            }

            virtual oslInterlockedCount SAL_CALL acquire()
            {
                return osl_atomic_increment( &m_refCount );
            }

            virtual oslInterlockedCount SAL_CALL release()
            {
                if ( 0 == osl_atomic_decrement( &m_refCount ) )
                {
                    delete this;
                    return 0;
                }
                return m_refCount;
            }

        protected:
            Reference< XPropertySet >   m_xObject;

        private:
            oslInterlockedCount         m_refCount;
        };

        //--------------------------------------------------------------------
        //- FormSQLCommandUI - declaration
        //--------------------------------------------------------------------
        class FormSQLCommandUI : public SQLCommandPropertyUI
        {
        public:
            FormSQLCommandUI( const Reference< XPropertySet >& _rxForm );

            // ISQLCommandAdapter
            virtual OUString getSQLCommand() const;
            virtual sal_Bool        getEscapeProcessing() const;
            virtual void            setSQLCommand( const OUString& _rCommand ) const;
            virtual void            setEscapeProcessing( const sal_Bool _bEscapeProcessing ) const;

            // ISQLCommandPropertyUI
            virtual OUString*    getPropertiesToDisable();
        };

        //--------------------------------------------------------------------
        //- FormSQLCommandUI - implementation
        //--------------------------------------------------------------------
        //....................................................................
        FormSQLCommandUI::FormSQLCommandUI( const Reference< XPropertySet >& _rxForm )
            :SQLCommandPropertyUI( _rxForm )
        {
        }

        //....................................................................
        OUString FormSQLCommandUI::getSQLCommand() const
        {
            OUString sCommand;
            OSL_VERIFY( m_xObject->getPropertyValue( PROPERTY_COMMAND ) >>= sCommand );
            return sCommand;
        }

        //....................................................................
        sal_Bool FormSQLCommandUI::getEscapeProcessing() const
        {
            sal_Bool bEscapeProcessing( sal_False );
            OSL_VERIFY( m_xObject->getPropertyValue( PROPERTY_ESCAPE_PROCESSING ) >>= bEscapeProcessing );
            return bEscapeProcessing;
        }

        //....................................................................
        void FormSQLCommandUI::setSQLCommand( const OUString& _rCommand ) const
        {
            m_xObject->setPropertyValue( PROPERTY_COMMAND, makeAny( _rCommand ) );
        }

        //....................................................................
        void FormSQLCommandUI::setEscapeProcessing( const sal_Bool _bEscapeProcessing ) const
        {
            m_xObject->setPropertyValue( PROPERTY_ESCAPE_PROCESSING, makeAny( _bEscapeProcessing ) );
        }

        //....................................................................
        OUString* FormSQLCommandUI::getPropertiesToDisable()
        {
            static OUString s_aCommandProps[] = {
                OUString(PROPERTY_DATASOURCE),
                OUString(PROPERTY_COMMAND),
                OUString(PROPERTY_COMMANDTYPE),
                OUString(PROPERTY_ESCAPE_PROCESSING),
                OUString()
            };
            return s_aCommandProps;
        }
        //--------------------------------------------------------------------
        //- ValueListCommandUI - declaration
        //--------------------------------------------------------------------
        class ValueListCommandUI : public SQLCommandPropertyUI
        {
        public:
            ValueListCommandUI( const Reference< XPropertySet >& _rxListOrCombo );

            // ISQLCommandAdapter
            virtual OUString getSQLCommand() const;
            virtual sal_Bool        getEscapeProcessing() const;
            virtual void            setSQLCommand( const OUString& _rCommand ) const;
            virtual void            setEscapeProcessing( const sal_Bool _bEscapeProcessing ) const;

            // ISQLCommandPropertyUI
            virtual OUString*    getPropertiesToDisable();
        private:
            mutable bool    m_bPropertyValueIsList;
        };

        //--------------------------------------------------------------------
        //- ValueListCommandUI - implementation
        //--------------------------------------------------------------------
        //....................................................................
        ValueListCommandUI::ValueListCommandUI( const Reference< XPropertySet >& _rxListOrCombo )
            :SQLCommandPropertyUI( _rxListOrCombo )
            ,m_bPropertyValueIsList( false )
        {
        }

        //....................................................................
        OUString ValueListCommandUI::getSQLCommand() const
        {
            OUString sValue;
            m_bPropertyValueIsList = false;

            // for combo boxes, the property is a mere string
            Any aValue( m_xObject->getPropertyValue( PROPERTY_LISTSOURCE ) );
            if ( aValue >>= sValue )
                return sValue;

            Sequence< OUString > aValueList;
            if ( aValue >>= aValueList )
            {
                m_bPropertyValueIsList = true;
                if ( aValueList.getLength() )
                    sValue = aValueList[0];
                return sValue;
            }

            OSL_FAIL( "ValueListCommandUI::getSQLCommand: unexpected property type!" );
            return sValue;
        }

        //....................................................................
        sal_Bool ValueListCommandUI::getEscapeProcessing() const
        {
            enum ListSourceType eType( ListSourceType_SQL );
            OSL_VERIFY( m_xObject->getPropertyValue( PROPERTY_LISTSOURCETYPE ) >>= eType );
            OSL_ENSURE( ( eType == ListSourceType_SQL ) || ( eType == ListSourceType_SQLPASSTHROUGH ),
                "ValueListCommandUI::getEscapeProcessing: unexpected list source type!" );
            return ( eType == ListSourceType_SQL );
        }

        //....................................................................
        void ValueListCommandUI::setSQLCommand( const OUString& _rCommand ) const
        {
            Any aValue;
            if ( m_bPropertyValueIsList )
                aValue <<= Sequence< OUString >( &_rCommand, 1 );
            else
                aValue <<= _rCommand;
            m_xObject->setPropertyValue( PROPERTY_LISTSOURCE, aValue );
        }

        //....................................................................
        void ValueListCommandUI::setEscapeProcessing( const sal_Bool _bEscapeProcessing ) const
        {
            m_xObject->setPropertyValue( PROPERTY_LISTSOURCETYPE, makeAny(
                _bEscapeProcessing ? ListSourceType_SQL : ListSourceType_SQLPASSTHROUGH ) );
        }

        //....................................................................
        OUString* ValueListCommandUI::getPropertiesToDisable()
        {
            static OUString s_aListSourceProps[] = {
                OUString(PROPERTY_LISTSOURCETYPE),
                OUString(PROPERTY_LISTSOURCE),
                OUString()
            };
            return s_aListSourceProps;
        }
    }

    //------------------------------------------------------------------------
    bool FormComponentPropertyHandler::impl_doDesignSQLCommand_nothrow( const Reference< XObjectInspectorUI >& _rxInspectorUI, PropertyId _nDesignForProperty )
    {
        try
        {
            if ( m_xCommandDesigner.is() )
            {
                if ( m_xCommandDesigner->isActive() )
                {
                    m_xCommandDesigner->raise();
                    return true;
                }
                m_xCommandDesigner->dispose();
                m_xCommandDesigner.set( NULL );
            }

            if ( !impl_ensureRowsetConnection_nothrow() )
                return false;

            Reference< XPropertySet > xComponentProperties( m_xComponent, UNO_QUERY_THROW );

            ::rtl::Reference< ISQLCommandPropertyUI > xCommandUI;
            switch ( _nDesignForProperty )
            {
            case PROPERTY_ID_COMMAND:
                xCommandUI = new FormSQLCommandUI( xComponentProperties );
                break;
            case PROPERTY_ID_LISTSOURCE:
                xCommandUI = new ValueListCommandUI( xComponentProperties );
                break;
            default:
                OSL_FAIL( "FormComponentPropertyHandler::OnDesignerClosed: invalid property id!" );
                return false;
            }

            m_xCommandDesigner.set( new SQLCommandDesigner( m_xContext, xCommandUI.get(), m_xRowSetConnection, LINK( this, FormComponentPropertyHandler, OnDesignerClosed ) ) );

            DBG_ASSERT( _rxInspectorUI.is(), "FormComponentPropertyHandler::OnDesignerClosed: no access to the property browser ui!" );
            if ( m_xCommandDesigner->isActive() && _rxInspectorUI.is() )
            {
                m_xBrowserUI = _rxInspectorUI;
                // disable everything which would affect this property
                const OUString* pToDisable = xCommandUI->getPropertiesToDisable();
                while ( !pToDisable->isEmpty() )
                {
                    m_xBrowserUI->enablePropertyUIElements( *pToDisable++, PropertyLineElement::All, sal_False );
                }

                // but enable the browse button for the property itself - so it can be used to raise the query designer
                OUString sPropertyName( impl_getPropertyNameFromId_nothrow( _nDesignForProperty ) );
                m_xBrowserUI->enablePropertyUIElements( sPropertyName, PropertyLineElement::PrimaryButton, sal_True );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return m_xCommandDesigner.is();
    }

    //------------------------------------------------------------------------
    IMPL_LINK( FormComponentPropertyHandler, OnDesignerClosed, void*, /*NOTINTERESTEDIN*/ )
    {
        OSL_ENSURE( m_xBrowserUI.is() && m_xCommandDesigner.is(), "FormComponentPropertyHandler::OnDesignerClosed: too many NULLs!" );
        if ( m_xBrowserUI.is() && m_xCommandDesigner.is() )
        {
            try
            {
                ::rtl::Reference< ISQLCommandPropertyUI > xCommandUI(
                    dynamic_cast< ISQLCommandPropertyUI* >( m_xCommandDesigner->getPropertyAdapter().get() ) );
                if ( !xCommandUI.is() )
                    throw NullPointerException();

                const OUString* pToEnable = xCommandUI->getPropertiesToDisable();
                while ( !pToEnable->isEmpty() )
                {
                    m_xBrowserUI->enablePropertyUIElements( *pToEnable++, PropertyLineElement::All, sal_True );
                }
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }

        return 0L;
    }

    //------------------------------------------------------------------------
    bool FormComponentPropertyHandler::impl_hasValidDataSourceSignature_nothrow( const Reference< XPropertySet >& _xFormProperties, bool _bAllowEmptyDataSourceName ) SAL_THROW(())
    {
        bool bHas = false;
        if ( _xFormProperties.is() )
        {
            try
            {
                OUString sPropertyValue;
                // first, we need the name of an existent data source
                if ( _xFormProperties->getPropertySetInfo()->hasPropertyByName(PROPERTY_DATASOURCE) )
                    _xFormProperties->getPropertyValue( PROPERTY_DATASOURCE ) >>= sPropertyValue;
                bHas = ( !sPropertyValue.isEmpty() ) || _bAllowEmptyDataSourceName;

                // then, the command should not be empty
                if ( bHas )
                {
                    if ( _xFormProperties->getPropertySetInfo()->hasPropertyByName(PROPERTY_COMMAND) )
                        _xFormProperties->getPropertyValue( PROPERTY_COMMAND ) >>= sPropertyValue;
                    bHas = !sPropertyValue.isEmpty();
                }
            }
            catch( const Exception& )
            {
                OSL_FAIL( "FormComponentPropertyHandler::impl_hasValidDataSourceSignature_nothrow: caught an exception!" );
                DBG_UNHANDLED_EXCEPTION();
            }
        }
        return bHas;
    }
    //------------------------------------------------------------------------
    OUString FormComponentPropertyHandler::impl_getDocumentURL_nothrow() const
    {
        OUString sURL;
        try
        {
            Reference< XModel > xDocument( impl_getContextDocument_nothrow() );
            if ( xDocument.is() )
                sURL = xDocument->getURL();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return sURL;
    }
    // -------------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper* FormComponentPropertyHandler::createArrayHelper( ) const
    {
        uno::Sequence< beans::Property > aProps;
        describeProperties(aProps);
        return new ::cppu::OPropertyArrayHelper(aProps);

    }
    // -------------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper & FormComponentPropertyHandler::getInfoHelper()
    {
        return *const_cast<FormComponentPropertyHandler*>(this)->getArrayHelper();
    }
    // -----------------------------------------------------------------------------
    uno::Reference< beans::XPropertySetInfo > SAL_CALL FormComponentPropertyHandler::getPropertySetInfo(  ) throw(uno::RuntimeException)
    {
        return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
    }

//........................................................................
} // namespace pcr
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
