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


#include <rtl/strbuf.hxx>
    // keep this include at the beginning. Some of the other includes seems to inject a symbol "l" into the
    // global namespace, which leads to a compiler warning in strbuf.hxx, about some parameters named "l"
    // hiding objects "in an outer scope".

#include "MLdapAttributeMap.hxx"
#include "MTypeConverter.hxx"
#include "MQueryHelper.hxx"

#include <tools/diagnose_ex.h>

#include <boost/unordered_map.hpp>

//........................................................................
namespace connectivity { namespace mozab {
//........................................................................

    //====================================================================
    //= helper
    //====================================================================
    namespace
    {
        typedef NS_STDCALL_FUNCPROTO(nsresult, CardPropertyGetter, nsIAbCard, GetFirstName, (PRUnichar**));
        typedef NS_STDCALL_FUNCPROTO(nsresult, CardPropertySetter, nsIAbCard, SetFirstName, (const PRUnichar*));
        struct CardPropertyData
        {
            const sal_Char*     pLDAPAttributeList;
            CardPropertyGetter  PropGetter;
            CardPropertySetter  PropSetter;

            CardPropertyData()
                :pLDAPAttributeList( NULL )
                ,PropGetter( NULL )
                ,PropSetter( NULL )
            {
            }
            CardPropertyData( const sal_Char* _pLDAPAttributeList, CardPropertyGetter _PropGetter, CardPropertySetter _PropSetter )
                :pLDAPAttributeList( _pLDAPAttributeList )
                ,PropGetter( _PropGetter )
                ,PropSetter( _PropSetter )
            {
            }
        };

        typedef ::boost::unordered_map< OString, CardPropertyData, OStringHash > MapPropertiesToAttributes;

        #define DEF_CARD_ACCESS( PropertyName ) \
            &nsIAbCard::Get##PropertyName, &nsIAbCard::Set##PropertyName

        static const MapPropertiesToAttributes& lcl_getPropertyMap()
        {
            static MapPropertiesToAttributes aMap;
            if ( aMap.empty() )
            {
                struct MapEntry
                {
                    const sal_Char*     pAsciiPropertyName;
                    const sal_Char*     pAsciiAttrributeList;
                    CardPropertyGetter  PropGetter;
                    CardPropertySetter  PropSetter;
                };
                const MapEntry aEntries[] = {
                    { "FirstName",          "givenname",                                    DEF_CARD_ACCESS( FirstName ) },
                    { "LastName",           "sn,surnname",                                  DEF_CARD_ACCESS( LastName ) },
                    { "DisplayName",        "cn,commonname,displayname",                    DEF_CARD_ACCESS( DisplayName ) },
                    { "NickName",           "xmozillanickname",                             DEF_CARD_ACCESS( NickName ) },
                    { "PrimaryEmail",       "mail",                                         DEF_CARD_ACCESS( PrimaryEmail ) },
                    { "SecondEmail",        "xmozillasecondemail",                          DEF_CARD_ACCESS( SecondEmail ) },
                    { "WorkPhone",          "telephonenumber",                              DEF_CARD_ACCESS( WorkPhone ) },
                    { "HomePhone",          "homephone",                                    DEF_CARD_ACCESS( HomePhone ) },
                    { "FaxNumber",          "fax,facsimiletelephonenumber",                 DEF_CARD_ACCESS( FaxNumber ) },
                    { "PagerNumber",        "pager,pagerphone",                             DEF_CARD_ACCESS( PagerNumber ) },
                    { "CellularNumber",     "mobile,cellphone,carphone",                    DEF_CARD_ACCESS( CellularNumber ) },
                    { "HomeAddress",        "homepostaladdress,mozillaHomeStreet",          DEF_CARD_ACCESS( HomeAddress ) },
                    { "HomeAddress2",       "mozillaHomeStreet2",                           DEF_CARD_ACCESS( HomeAddress2 ) },
                    { "HomeCity",           "homelocality,mozillaHomeLocalityName",         DEF_CARD_ACCESS( HomeCity ) },
                    { "HomeState",          "homeregion,mozillaHomeState",                  DEF_CARD_ACCESS( HomeState ) },
                    { "HomeZipCode",        "homepostalcode,mozillaHomePostalCode",         DEF_CARD_ACCESS( HomeZipCode ) },
                    { "HomeCountry",        "homecountryname,mozillaHomeCountryName",       DEF_CARD_ACCESS( HomeCountry ) },
                    { "WorkAddress",        "postofficebox,streetaddress,streetaddress1",   DEF_CARD_ACCESS( WorkAddress ) },
                    { "WorkAddress2",       "streetaddress2",                               DEF_CARD_ACCESS( WorkAddress2 ) },
                    { "WorkCity",           "l,locality",                                   DEF_CARD_ACCESS( WorkCity ) },
                    { "WorkState",          "st,region",                                    DEF_CARD_ACCESS( WorkState ) },
                    { "WorkZipCode",        "postalcode,zip",                               DEF_CARD_ACCESS( WorkZipCode ) },
                    { "WorkCountry",        "countryname",                                  DEF_CARD_ACCESS( WorkCountry ) },
                    { "JobTitle",           "title",                                        DEF_CARD_ACCESS( JobTitle ) },
                    { "Department",         "ou,orgunit,department,departmentnumber",       DEF_CARD_ACCESS( Department ) },
                    { "Company",            "o,company",                                    DEF_CARD_ACCESS( Company ) },
                    { "WebPage1",           "workurl",                                      DEF_CARD_ACCESS( WebPage1 ) },
                    { "WebPage2",           "homeurl",                                      DEF_CARD_ACCESS( WebPage2 ) },
                    { "BirthYear",          "birthyear",                                    DEF_CARD_ACCESS( BirthYear ) },
                    { "BirthMonth",         "birthmonth",                                   DEF_CARD_ACCESS( BirthMonth ) },
                    { "BirthDay",          "birthday",                                     DEF_CARD_ACCESS( BirthDay ) },
                    { "Custom1",            "custom1",                                      DEF_CARD_ACCESS( Custom1 ) },
                    { "Custom2",            "custom2",                                      DEF_CARD_ACCESS( Custom2 ) },
                    { "Custom3",            "custom3",                                      DEF_CARD_ACCESS( Custom3 ) },
                    { "Custom4",            "custom4",                                      DEF_CARD_ACCESS( Custom4 ) },
                    { "Notes",              "notes,description",                            DEF_CARD_ACCESS( Notes ) },
                    { "PreferMailFormat",   "xmozillausehtmlmail",                          NULL, NULL },
                    { NULL, NULL, NULL, NULL }
                };
                const MapEntry* loop = aEntries;
                while ( loop->pAsciiPropertyName )
                {
                    aMap[ OString( loop->pAsciiPropertyName ) ] =
                        CardPropertyData( loop->pAsciiAttrributeList, loop->PropGetter, loop->PropSetter );
                    ++loop;
                }
            }
            return aMap;
        }
    }

    //====================================================================
    //= AttributeMap_Data
    //====================================================================
    struct AttributeMap_Data
    {
    };

    //====================================================================
    //= MLdapAttributeMap
    //====================================================================
    // -------------------------------------------------------------------
    MLdapAttributeMap::MLdapAttributeMap()
        :m_pData( new AttributeMap_Data )
    {
    }

    // -------------------------------------------------------------------
    MLdapAttributeMap::~MLdapAttributeMap()
    {
    }

    // -------------------------------------------------------------------
    NS_IMPL_THREADSAFE_ISUPPORTS1( MLdapAttributeMap, nsIAbLDAPAttributeMap )

    // -------------------------------------------------------------------
    NS_IMETHODIMP MLdapAttributeMap::GetAttributeList(const nsACString & aProperty, nsACString & _retval)
    {
        OString sProperty( MTypeConverter::nsACStringToOString( aProperty ) );

        const MapPropertiesToAttributes& rPropertyMap( lcl_getPropertyMap() );
        MapPropertiesToAttributes::const_iterator pos = rPropertyMap.find( sProperty );

        if ( pos == rPropertyMap.end() )
        {
            _retval.SetIsVoid( PR_TRUE );
        }
        else
        {
            MTypeConverter::asciiToNsACString( pos->second.pLDAPAttributeList, _retval );
        }

        return NS_OK;
    }

    // -------------------------------------------------------------------
    NS_IMETHODIMP MLdapAttributeMap::GetAttributes(const nsACString & aProperty, PRUint32* aCount, char*** aAttrs)
    {
        OSL_FAIL( "MLdapAttributeMap::GetAttributes: not implemented!" );
        (void)aProperty;
        (void)aCount;
        (void)aAttrs;
        return NS_ERROR_NOT_IMPLEMENTED;
    }

    // -------------------------------------------------------------------
    NS_IMETHODIMP MLdapAttributeMap::GetFirstAttribute(const nsACString & aProperty, nsACString & _retval)
    {
        OString sProperty( MTypeConverter::nsACStringToOString( aProperty ) );

        const MapPropertiesToAttributes& rPropertyMap( lcl_getPropertyMap() );
        MapPropertiesToAttributes::const_iterator pos = rPropertyMap.find( sProperty );

        if ( pos == rPropertyMap.end() )
        {
            _retval.SetIsVoid( PR_TRUE );
        }
        else
        {
            sal_Int32 tokenPos(0);
            OString sAttributeList( pos->second.pLDAPAttributeList );
            MTypeConverter::asciiToNsACString( sAttributeList.getToken( 0, ',', tokenPos ).getStr(), _retval );
        }

        return NS_OK;
    }

    // -------------------------------------------------------------------
    NS_IMETHODIMP MLdapAttributeMap::SetAttributeList(const nsACString & aProperty, const nsACString & aAttributeList, PRBool allowInconsistencies)
    {
        OSL_FAIL( "MLdapAttributeMap::SetAttributeList: not implemented!" );
        (void)aProperty;
        (void)aAttributeList;
        (void)allowInconsistencies;
        return NS_ERROR_NOT_IMPLEMENTED;
    }

    // -------------------------------------------------------------------
    NS_IMETHODIMP MLdapAttributeMap::GetProperty(const nsACString & aAttribute, nsACString & _retval)
    {
        OSL_FAIL( "MLdapAttributeMap::GetProperty: not implemented!" );
        (void)aAttribute;
        (void)_retval;
        return NS_ERROR_NOT_IMPLEMENTED;
    }

    // -------------------------------------------------------------------
    NS_IMETHODIMP MLdapAttributeMap::GetAllCardAttributes(nsACString & _retval)
    {
        const MapPropertiesToAttributes& rPropertyMap( lcl_getPropertyMap() );

        OStringBuffer aAllAttributes;
        for (   MapPropertiesToAttributes::const_iterator loop = rPropertyMap.begin();
                loop != rPropertyMap.end();
                ++loop
            )
        {
            aAllAttributes.append( loop->second.pLDAPAttributeList );
            if ( loop != rPropertyMap.end() )
                aAllAttributes.append( ',' );
        }

        MTypeConverter::asciiToNsACString( aAllAttributes.getStr(), _retval );
        return NS_OK;
    }

    // -------------------------------------------------------------------
    NS_IMETHODIMP MLdapAttributeMap::CheckState(void)
    {
        // we do not allow modifying the map, so we're always in a valid state
        return NS_OK;
    }

    // -------------------------------------------------------------------
    NS_IMETHODIMP MLdapAttributeMap::SetFromPrefs(const nsACString & aPrefBranchName)
    {
        OSL_FAIL( "MLdapAttributeMap::SetFromPrefs: not implemented!" );
        (void)aPrefBranchName;
        return NS_ERROR_NOT_IMPLEMENTED;
    }

    // -------------------------------------------------------------------
    NS_IMETHODIMP MLdapAttributeMap::SetCardPropertiesFromLDAPMessage(nsILDAPMessage* aMessage, nsIAbCard* aCard)
    {
        NS_ENSURE_ARG_POINTER( aMessage );
        NS_ENSURE_ARG_POINTER( aCard );

        // in case that's not present in the LDAP message: set the "preferred mail format" to "none"
        aCard->SetPreferMailFormat( nsIAbPreferMailFormat::unknown );

        const MapPropertiesToAttributes& rPropertyMap( lcl_getPropertyMap() );
        for (   MapPropertiesToAttributes::const_iterator prop = rPropertyMap.begin();
                prop != rPropertyMap.end();
                ++prop
            )
        {
            // split the list of attributes for the current property
            OString sAttributeList( prop->second.pLDAPAttributeList );
            OString sAttribute;

            sal_Int32 tokenPos = 0;
            while ( tokenPos != -1 )
            {
                sAttribute = sAttributeList.getToken( 0, ',', tokenPos );

                // retrieve the values for the current attribute
                PRUint32 valueCount = 0;
                PRUnichar** values = NULL;
                nsresult rv = aMessage->GetValues( sAttribute.getStr(), &valueCount, &values );
                if ( NS_FAILED( rv ) )
                    // try the next attribute
                    continue;

                if ( valueCount )
                {
                    CardPropertySetter propSetter = prop->second.PropSetter;
                    OSL_ENSURE( propSetter,
                        "MLdapAttributeMap::SetCardPropertiesFromLDAPMessage: "
                        "unexpected: could retrieve an attribute value, but have no setter for it!" );
                    if ( propSetter )
                    {
                        (aCard->*propSetter)( values[0] );
                    }

                    // we're done with this property - no need to handle the remaining attributes which
                    // map to it
                    break;
                }
            }
        }
        return NS_OK;
    }

    // -------------------------------------------------------------------
    namespace
    {
        struct PreferMailFormatType
        {
            const sal_Char* description;
            PRUint32        formatType;

            PreferMailFormatType()
                :description( NULL )
                ,formatType( nsIAbPreferMailFormat::unknown )
            {
            }

            PreferMailFormatType( const sal_Char* _description, PRUint32 _formatType )
                :description( _description )
                ,formatType( _formatType )
            {
            }
        };

        static const PreferMailFormatType* lcl_getMailFormatTypes()
        {
            static const PreferMailFormatType aMailFormatTypes[] =
            {
                PreferMailFormatType( "text/plain", nsIAbPreferMailFormat::plaintext ),
                PreferMailFormatType( "text/html",  nsIAbPreferMailFormat::html      ),
                PreferMailFormatType( "unknown",    nsIAbPreferMailFormat::unknown   ),
                PreferMailFormatType()
            };
            return aMailFormatTypes;
        }
    }

    // -------------------------------------------------------------------
    void MLdapAttributeMap::fillCardFromResult( nsIAbCard& _card, const MQueryHelperResultEntry& _result )
    {
        _card.SetPreferMailFormat( nsIAbPreferMailFormat::unknown );

        OUString resultValue;

        const MapPropertiesToAttributes& rPropertyMap( lcl_getPropertyMap() );
        for (   MapPropertiesToAttributes::const_iterator prop = rPropertyMap.begin();
                prop != rPropertyMap.end();
                ++prop
            )
        {
            resultValue = _result.getValue( prop->first );

            CardPropertySetter propSetter = prop->second.PropSetter;
            if ( propSetter )
            {
        // PRUnichar != sal_Unicode in mingw
                (_card.*propSetter)( reinterpret_cast_mingw_only<const PRUnichar *>(resultValue.getStr()) );
            }
            else
            {
                if ( prop->first.equals( "PreferMailFormat" ) )
                {
                    unsigned int format = nsIAbPreferMailFormat::unknown;
                    const PreferMailFormatType* pMailFormatType = lcl_getMailFormatTypes();
                    while ( pMailFormatType->description )
                    {
                        if ( resultValue.equalsAscii( pMailFormatType->description ) )
                        {
                            format = pMailFormatType->formatType;
                            break;
                        }
                        ++pMailFormatType;
                    }
                    _card.SetPreferMailFormat(format);
                }
                else
                    OSL_FAIL( "MLdapAttributeMap::fillCardFromResult: unexpected property without default setters!" );
            }
        }
    }

    // -------------------------------------------------------------------
    void MLdapAttributeMap::fillResultFromCard( MQueryHelperResultEntry& _result, nsIAbCard& _card )
    {
        nsXPIDLString value;
        OUString resultValue;

        const MapPropertiesToAttributes& rPropertyMap( lcl_getPropertyMap() );
        for (   MapPropertiesToAttributes::const_iterator prop = rPropertyMap.begin();
                prop != rPropertyMap.end();
                ++prop
            )
        {
            CardPropertyGetter propGetter = prop->second.PropGetter;
            if ( propGetter )
            {
                (_card.*propGetter)( getter_Copies( value ) );

                nsAutoString temp( value );
                MTypeConverter::nsStringToOUString( temp, resultValue );
            }
            else
            {
                if ( prop->first.equals( "PreferMailFormat" ) )
                {
                    unsigned int format = nsIAbPreferMailFormat::unknown;
                    _card.GetPreferMailFormat( &format );
                    const PreferMailFormatType* pMailFormatType = lcl_getMailFormatTypes();
                    while ( pMailFormatType->description )
                    {
                        if ( format == pMailFormatType->formatType )
                        {
                            resultValue = OUString::createFromAscii( pMailFormatType->description );
                            break;
                        }
                        ++pMailFormatType;
                    }
                }
                else
                    OSL_FAIL( "MLdapAttributeMap::fillResultFromCard: unexpected property without default getters!" );
            }

            _result.insert( prop->first, resultValue );
        }
    }

//........................................................................
} } // namespace connectivity::mozab
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
