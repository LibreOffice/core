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


#include <unotools/securityoptions.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <comphelper/sequenceasvector.hxx>
#include <tools/urlobj.hxx>
#include <tools/wldcrd.hxx>

#include <unotools/pathoptions.hxx>

#include <rtl/logfile.hxx>
#include "itemholder1.hxx"

//_________________________________________________________________________________________________________________
//  namespaces
//_________________________________________________________________________________________________________________

using namespace ::utl                   ;
using namespace ::rtl                   ;
using namespace ::osl                   ;
using namespace ::com::sun::star::uno   ;

#define ROOTNODE_SECURITY               OUString(RTL_CONSTASCII_USTRINGPARAM("Office.Common/Security/Scripting"))
#define DEFAULT_SECUREURL               Sequence< OUString >()
#define DEFAULT_TRUSTEDAUTHORS          Sequence< SvtSecurityOptions::Certificate >()

// xmlsec05 depricated
#define DEFAULT_STAROFFICEBASIC         eALWAYS_EXECUTE

#define CSTR_SECUREURL                  "SecureURL"
#define CSTR_DOCWARN_SAVEORSEND         "WarnSaveOrSendDoc"
#define CSTR_DOCWARN_SIGNING            "WarnSignDoc"
#define CSTR_DOCWARN_PRINT              "WarnPrintDoc"
#define CSTR_DOCWARN_CREATEPDF          "WarnCreatePDF"
#define CSTR_DOCWARN_REMOVEPERSONALINFO "RemovePersonalInfoOnSaving"
#define CSTR_DOCWARN_RECOMMENDPASSWORD  "RecommendPasswordProtection"
#define CSTR_CTRLCLICK_HYPERLINK        "HyperlinksWithCtrlClick"
#define CSTR_MACRO_SECLEVEL             "MacroSecurityLevel"
#define CSTR_MACRO_TRUSTEDAUTHORS       "TrustedAuthors"
#define CSTR_MACRO_DISABLE              "DisableMacrosExecution"
#define CSTR_TRUSTEDAUTHOR_SUBJECTNAME  "SubjectName"
#define CSTR_TRUSTEDAUTHOR_SERIALNUMBER "SerialNumber"
#define CSTR_TRUSTEDAUTHOR_RAWDATA      "RawData"

#define PROPERTYNAME_SECUREURL                  OUString(RTL_CONSTASCII_USTRINGPARAM(CSTR_SECUREURL                     ))
#define PROPERTYNAME_DOCWARN_SAVEORSEND         OUString(RTL_CONSTASCII_USTRINGPARAM(CSTR_DOCWARN_SAVEORSEND            ))
#define PROPERTYNAME_DOCWARN_SIGNING            OUString(RTL_CONSTASCII_USTRINGPARAM(CSTR_DOCWARN_SIGNING               ))
#define PROPERTYNAME_DOCWARN_PRINT              OUString(RTL_CONSTASCII_USTRINGPARAM(CSTR_DOCWARN_PRINT                 ))
#define PROPERTYNAME_DOCWARN_CREATEPDF          OUString(RTL_CONSTASCII_USTRINGPARAM(CSTR_DOCWARN_CREATEPDF             ))
#define PROPERTYNAME_DOCWARN_REMOVEPERSONALINFO OUString(RTL_CONSTASCII_USTRINGPARAM(CSTR_DOCWARN_REMOVEPERSONALINFO    ))
#define PROPERTYNAME_DOCWARN_RECOMMENDPASSWORD  OUString(RTL_CONSTASCII_USTRINGPARAM(CSTR_DOCWARN_RECOMMENDPASSWORD     ))
#define PROPERTYNAME_CTRLCLICK_HYPERLINK        OUString(RTL_CONSTASCII_USTRINGPARAM(CSTR_CTRLCLICK_HYPERLINK           ))
#define PROPERTYNAME_MACRO_SECLEVEL             OUString(RTL_CONSTASCII_USTRINGPARAM(CSTR_MACRO_SECLEVEL                ))
#define PROPERTYNAME_MACRO_TRUSTEDAUTHORS       OUString(RTL_CONSTASCII_USTRINGPARAM(CSTR_MACRO_TRUSTEDAUTHORS          ))
#define PROPERTYNAME_MACRO_DISABLE              OUString(RTL_CONSTASCII_USTRINGPARAM(CSTR_MACRO_DISABLE                 ))
#define PROPERTYNAME_TRUSTEDAUTHOR_SUBJECTNAME  OUString(RTL_CONSTASCII_USTRINGPARAM(CSTR_TRUSTEDAUTHOR_SUBJECTNAME))
#define PROPERTYNAME_TRUSTEDAUTHOR_SERIALNUMBER OUString(RTL_CONSTASCII_USTRINGPARAM(CSTR_TRUSTEDAUTHOR_SERIALNUMBER))
#define PROPERTYNAME_TRUSTEDAUTHOR_RAWDATA      OUString(RTL_CONSTASCII_USTRINGPARAM(CSTR_TRUSTEDAUTHOR_RAWDATA))

// xmlsec05 depricated
#define PROPERTYNAME_STAROFFICEBASIC    OUString(RTL_CONSTASCII_USTRINGPARAM("OfficeBasic"  ))
#define PROPERTYNAME_EXECUTEPLUGINS     OUString(RTL_CONSTASCII_USTRINGPARAM("ExecutePlugins"  ))
#define PROPERTYNAME_WARNINGENABLED     OUString(RTL_CONSTASCII_USTRINGPARAM("Warning"  ))
#define PROPERTYNAME_CONFIRMATIONENABLED OUString(RTL_CONSTASCII_USTRINGPARAM("Confirmation"  ))
// xmlsec05 depricated


#define PROPERTYHANDLE_SECUREURL                    0

// xmlsec05 depricated
#define PROPERTYHANDLE_STAROFFICEBASIC  1
#define PROPERTYHANDLE_EXECUTEPLUGINS   2
#define PROPERTYHANDLE_WARNINGENABLED   3
#define PROPERTYHANDLE_CONFIRMATIONENABLED 4
// xmlsec05 depricated

#define PROPERTYHANDLE_DOCWARN_SAVEORSEND           5
#define PROPERTYHANDLE_DOCWARN_SIGNING              6
#define PROPERTYHANDLE_DOCWARN_PRINT                7
#define PROPERTYHANDLE_DOCWARN_CREATEPDF            8
#define PROPERTYHANDLE_DOCWARN_REMOVEPERSONALINFO   9
#define PROPERTYHANDLE_DOCWARN_RECOMMENDPASSWORD    10
#define PROPERTYHANDLE_CTRLCLICK_HYPERLINK          11
#define PROPERTYHANDLE_MACRO_SECLEVEL               12
#define PROPERTYHANDLE_MACRO_TRUSTEDAUTHORS         13
#define PROPERTYHANDLE_MACRO_DISABLE                14

#define PROPERTYCOUNT                               15
#define PROPERTYHANDLE_INVALID                      -1

#define CFG_READONLY_DEFAULT                        sal_False

//_________________________________________________________________________________________________________________
//  private declarations!
//_________________________________________________________________________________________________________________

class SvtSecurityOptions_Impl : public ConfigItem
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------

         SvtSecurityOptions_Impl();
        ~SvtSecurityOptions_Impl();

        //---------------------------------------------------------------------------------------------------------
        //  overloaded methods of baseclass
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      called for notify of configmanager
            @descr      These method is called from the ConfigManager before application ends or from the
                         PropertyChangeListener if the sub tree broadcasts changes. You must update your
                        internal values.

            @seealso    baseclass ConfigItem

            @param      "seqPropertyNames" is the list of properties which should be updated.
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void Notify( const Sequence< OUString >& seqPropertyNames );

        /*-****************************************************************************************************//**
            @short      write changes to configuration
            @descr      These method writes the changed values into the sub tree
                        and should always called in our destructor to guarantee consistency of config data.

            @seealso    baseclass ConfigItem

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void Commit();

        //---------------------------------------------------------------------------------------------------------
        //  public interface
        //---------------------------------------------------------------------------------------------------------

        sal_Bool                IsReadOnly      ( SvtSecurityOptions::EOption eOption                   ) const ;

        Sequence< OUString >    GetSecureURLs   (                                                       ) const ;
        void                    SetSecureURLs   (   const   Sequence< OUString >&   seqURLList          )       ;
        sal_Bool                IsSecureURL     (   const   OUString&               sURL,
                                                    const   OUString&               sReferer            ) const ;
        inline sal_Int32        GetMacroSecurityLevel   (                                               ) const ;
        void                    SetMacroSecurityLevel   ( sal_Int32 _nLevel                             )       ;

        inline sal_Bool         IsMacroDisabled         (                                               ) const ;

        Sequence< SvtSecurityOptions::Certificate > GetTrustedAuthors       (                                                                                       ) const ;
        void                                        SetTrustedAuthors       ( const Sequence< SvtSecurityOptions::Certificate >& rAuthors                           )       ;

        sal_Bool                IsOptionSet     ( SvtSecurityOptions::EOption eOption                   ) const ;
        sal_Bool                SetOption       ( SvtSecurityOptions::EOption eOption, sal_Bool bValue  )       ;
        sal_Bool                IsOptionEnabled ( SvtSecurityOptions::EOption eOption                   ) const ;
private:

        /*-****************************************************************************************************//**
            @short      return list of key names of ouer configuration management which represent our module tree
            @descr      These methods return a static const list of key names. We need it to get needed values from our
                        configuration management.

            @seealso    -

            @param      -
            @return     A list of needed configuration keys is returned.

            @onerror    -
        *//*-*****************************************************************************************************/

        void                    SetProperty( sal_Int32 nHandle, const Any& rValue, sal_Bool bReadOnly );
        void                    LoadAuthors( void );
        static sal_Int32        GetHandle( const OUString& rPropertyName );
        bool                    GetOption( SvtSecurityOptions::EOption eOption, sal_Bool*& rpValue, sal_Bool*& rpRO );

        static Sequence< OUString > GetPropertyNames();

        Sequence< OUString >                        m_seqSecureURLs;
        sal_Bool                                    m_bSaveOrSend;
        sal_Bool                                    m_bSigning;
        sal_Bool                                    m_bPrint;
        sal_Bool                                    m_bCreatePDF;
        sal_Bool                                    m_bRemoveInfo;
        sal_Bool                                    m_bRecommendPwd;
        sal_Bool                                    m_bCtrlClickHyperlink;
        sal_Int32                                   m_nSecLevel;
        Sequence< SvtSecurityOptions::Certificate > m_seqTrustedAuthors;
        sal_Bool                                    m_bDisableMacros;

        sal_Bool                                    m_bROSecureURLs;
        sal_Bool                                    m_bROSaveOrSend;
        sal_Bool                                    m_bROSigning;
        sal_Bool                                    m_bROPrint;
        sal_Bool                                    m_bROCreatePDF;
        sal_Bool                                    m_bRORemoveInfo;
        sal_Bool                                    m_bRORecommendPwd;
        sal_Bool                                    m_bROCtrlClickHyperlink;
        sal_Bool                                    m_bROSecLevel;
        sal_Bool                                    m_bROTrustedAuthors;
        sal_Bool                                    m_bRODisableMacros;


        // xmlsec05 depricated
        EBasicSecurityMode      m_eBasicMode;
        sal_Bool                m_bExecutePlugins;
        sal_Bool                m_bWarning;
        sal_Bool                m_bConfirmation;

        sal_Bool                m_bROConfirmation;
        sal_Bool                m_bROWarning;
        sal_Bool                m_bROExecutePlugins;
        sal_Bool                m_bROBasicMode;
        public:
        sal_Bool IsWarningEnabled() const;
        void SetWarningEnabled( sal_Bool bSet );
        sal_Bool IsConfirmationEnabled() const;
        void SetConfirmationEnabled( sal_Bool bSet );
        sal_Bool    IsExecutePlugins() const;
        void        SetExecutePlugins( sal_Bool bSet );
        EBasicSecurityMode      GetBasicMode    (                                               ) const ;
        void                    SetBasicMode    (           EBasicSecurityMode      eMode       )       ;
};

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
SvtSecurityOptions_Impl::SvtSecurityOptions_Impl()
    :ConfigItem             ( ROOTNODE_SECURITY         )
    ,m_seqSecureURLs        ( DEFAULT_SECUREURL         )
    ,m_bSaveOrSend          ( sal_True                  )
    ,m_bSigning             ( sal_True                  )
    ,m_bPrint               ( sal_True                  )
    ,m_bCreatePDF           ( sal_True                  )
    ,m_bRemoveInfo          ( sal_True                  )
    ,m_nSecLevel            ( sal_True                  )
    ,m_seqTrustedAuthors    ( DEFAULT_TRUSTEDAUTHORS    )
    ,m_bDisableMacros       ( sal_False                 )
    ,m_bROSecureURLs        ( CFG_READONLY_DEFAULT      )
    ,m_bROSaveOrSend        ( CFG_READONLY_DEFAULT      )
    ,m_bROSigning           ( CFG_READONLY_DEFAULT      )
    ,m_bROPrint             ( CFG_READONLY_DEFAULT      )
    ,m_bROCreatePDF         ( CFG_READONLY_DEFAULT      )
    ,m_bRORemoveInfo        ( CFG_READONLY_DEFAULT      )
    ,m_bROSecLevel          ( CFG_READONLY_DEFAULT      )
    ,m_bROTrustedAuthors    ( CFG_READONLY_DEFAULT      )
    ,m_bRODisableMacros     ( sal_True                  ) // currently is not intended to be changed

    // xmlsec05 depricated
    ,   m_eBasicMode        ( DEFAULT_STAROFFICEBASIC )
    ,   m_bExecutePlugins   ( sal_True                )
    ,   m_bWarning          ( sal_True                )
    ,   m_bConfirmation     ( sal_True                )
    ,   m_bROConfirmation   ( CFG_READONLY_DEFAULT    )
    ,   m_bROWarning        ( CFG_READONLY_DEFAULT    )
    ,   m_bROExecutePlugins ( CFG_READONLY_DEFAULT    )
    ,   m_bROBasicMode      ( CFG_READONLY_DEFAULT    )
    // xmlsec05 depricated

{
    Sequence< OUString >    seqNames    = GetPropertyNames  (           );
    Sequence< Any >         seqValues   = GetProperties     ( seqNames  );
    Sequence< sal_Bool >    seqRO       = GetReadOnlyStates ( seqNames  );

    // Safe impossible cases.
    // We need values from ALL configuration keys.
    // Follow assignment use order of values in relation to our list of key names!
    DBG_ASSERT( !(seqNames.getLength()!=seqValues.getLength()), "SvtSecurityOptions_Impl::SvtSecurityOptions_Impl()\nI miss some values of configuration keys!\n" );

    // Copy values from list in right order to our internal member.
    sal_Int32               nPropertyCount = seqValues.getLength();
    for( sal_Int32 nProperty = 0 ; nProperty < nPropertyCount ; ++nProperty )
        SetProperty( nProperty, seqValues[ nProperty ], seqRO[ nProperty ] );

    LoadAuthors();

    // Enable notification mechanism of our baseclass.
    // We need it to get information about changes outside these class on our used configuration keys!*/

    EnableNotification( seqNames );
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
SvtSecurityOptions_Impl::~SvtSecurityOptions_Impl()
{
    if( IsModified() )
        Commit();
}

void SvtSecurityOptions_Impl::SetProperty( sal_Int32 nProperty, const Any& rValue, sal_Bool bRO )
{
    switch( nProperty )
    {
        case PROPERTYHANDLE_SECUREURL:
        {
            m_seqSecureURLs.realloc( 0 );
            rValue >>= m_seqSecureURLs;
            SvtPathOptions  aOpt;
            sal_uInt32      nCount = m_seqSecureURLs.getLength();
            for( sal_uInt32 nItem = 0 ; nItem < nCount ; ++nItem )
                m_seqSecureURLs[ nItem ] = aOpt.SubstituteVariable( m_seqSecureURLs[ nItem ] );
            m_bROSecureURLs = bRO;
        }
        break;

        case PROPERTYHANDLE_DOCWARN_SAVEORSEND:
        {
            rValue >>= m_bSaveOrSend;
            m_bROSaveOrSend = bRO;
        }
        break;

        case PROPERTYHANDLE_DOCWARN_SIGNING:
        {
            rValue >>= m_bSigning;
            m_bROSigning = bRO;
        }
        break;

        case PROPERTYHANDLE_DOCWARN_PRINT:
        {
            rValue >>= m_bPrint;
            m_bROPrint = bRO;
        }
        break;

        case PROPERTYHANDLE_DOCWARN_CREATEPDF:
        {
            rValue >>= m_bCreatePDF;
            m_bROCreatePDF = bRO;
        }
        break;

        case PROPERTYHANDLE_DOCWARN_REMOVEPERSONALINFO:
        {
            rValue >>= m_bRemoveInfo;
            m_bRORemoveInfo = bRO;
        }
        break;

        case PROPERTYHANDLE_DOCWARN_RECOMMENDPASSWORD:
        {
            rValue >>= m_bRecommendPwd;
            m_bRORecommendPwd = bRO;
        }
        break;

        case PROPERTYHANDLE_CTRLCLICK_HYPERLINK:
        {
            rValue >>= m_bCtrlClickHyperlink;
            m_bROCtrlClickHyperlink = bRO;
        }
        break;

        case PROPERTYHANDLE_MACRO_SECLEVEL:
        {
            rValue >>= m_nSecLevel;
            m_bROSecLevel = bRO;
        }
        break;

        case PROPERTYHANDLE_MACRO_TRUSTEDAUTHORS:
        {
            // don't care about value here...
            m_bROTrustedAuthors = bRO;
        }
        break;

        case PROPERTYHANDLE_MACRO_DISABLE:
        {
            rValue >>= m_bDisableMacros;
            m_bRODisableMacros = bRO;
        }
        break;


        // xmlsec05 depricated
        case PROPERTYHANDLE_STAROFFICEBASIC:
        {
            sal_Int32 nMode = 0;
            rValue >>= nMode;
            m_eBasicMode = (EBasicSecurityMode)nMode;
            m_bROBasicMode = bRO;
        }
        break;
        case PROPERTYHANDLE_EXECUTEPLUGINS:
        {
            rValue >>= m_bExecutePlugins;
            m_bROExecutePlugins = bRO;
        }
        break;
        case PROPERTYHANDLE_WARNINGENABLED:
        {
            rValue >>= m_bWarning;
            m_bROWarning = bRO;
        }
        break;
        case PROPERTYHANDLE_CONFIRMATIONENABLED:
        {
            rValue >>= m_bConfirmation;
            m_bROConfirmation = bRO;
        }
        break;
        // xmlsec05 depricated


        #if OSL_DEBUG_LEVEL > 1
        default:
            DBG_ASSERT( false, "SvtSecurityOptions_Impl::SetProperty()\nUnkown property!\n" );
        #endif
    }
}

void SvtSecurityOptions_Impl::LoadAuthors( void )
{
    m_seqTrustedAuthors.realloc( 0 );       // first clear
    Sequence< OUString >    lAuthors = GetNodeNames( PROPERTYNAME_MACRO_TRUSTEDAUTHORS );
    sal_Int32               c1 = lAuthors.getLength();
    if( c1 )
    {
        sal_Int32               c2 = c1 * 3;                // 3 Properties inside Struct TrustedAuthor
        Sequence< OUString >    lAllAuthors( c2 );

        sal_Int32               i1;
        sal_Int32               i2;
        OUString                aSep( RTL_CONSTASCII_USTRINGPARAM( "/" ) );
        for( i1 = 0, i2 = 0 ; i1 < c1 ; ++i1 )
        {
            lAllAuthors[ i2 ] = PROPERTYNAME_MACRO_TRUSTEDAUTHORS + aSep + lAuthors[ i1 ] + aSep + PROPERTYNAME_TRUSTEDAUTHOR_SUBJECTNAME;
            ++i2;
            lAllAuthors[ i2 ] = PROPERTYNAME_MACRO_TRUSTEDAUTHORS + aSep + lAuthors[ i1 ] + aSep + PROPERTYNAME_TRUSTEDAUTHOR_SERIALNUMBER;
            ++i2;
            lAllAuthors[ i2 ] = PROPERTYNAME_MACRO_TRUSTEDAUTHORS + aSep + lAuthors[ i1 ] + aSep + PROPERTYNAME_TRUSTEDAUTHOR_RAWDATA;
            ++i2;
        }

        Sequence< Any >         lValues = GetProperties( lAllAuthors );
        if( lValues.getLength() == c2 )
        {
            comphelper::SequenceAsVector< SvtSecurityOptions::Certificate > v;
            SvtSecurityOptions::Certificate aCert( 3 );
            for( i1 = 0, i2 = 0 ; i1 < c1 ; ++i1 )
            {
                lValues[ i2 ] >>= aCert[ 0 ];
                ++i2;
                lValues[ i2 ] >>= aCert[ 1 ];
                ++i2;
                lValues[ i2 ] >>= aCert[ 2 ];
                ++i2;
                // Filter out TrustedAuthor entries with empty RawData, which
                // would cause an unexpected std::bad_alloc in
                // SecurityEnvironment_NssImpl::createCertificateFromAscii and
                // have been observed in the wild (fdo#55019):
                if( !aCert[ 2 ].isEmpty() )
                {
                    v.push_back( aCert );
                }
            }
            m_seqTrustedAuthors = v.getAsConstList();
        }
    }
}

sal_Int32 SvtSecurityOptions_Impl::GetHandle( const OUString& rName )
{
    sal_Int32   nHandle;

    if( rName.compareToAscii( CSTR_SECUREURL ) == 0 )
        nHandle = PROPERTYHANDLE_SECUREURL;
    else if( rName.compareToAscii( CSTR_DOCWARN_SAVEORSEND ) == 0 )
        nHandle = PROPERTYHANDLE_DOCWARN_SAVEORSEND;
    else if( rName.compareToAscii( CSTR_DOCWARN_SIGNING ) == 0 )
        nHandle = PROPERTYHANDLE_DOCWARN_SIGNING;
    else if( rName.compareToAscii( CSTR_DOCWARN_PRINT ) == 0 )
        nHandle = PROPERTYHANDLE_DOCWARN_PRINT;
    else if( rName.compareToAscii( CSTR_DOCWARN_CREATEPDF ) == 0 )
        nHandle = PROPERTYHANDLE_DOCWARN_CREATEPDF;
    else if( rName.compareToAscii( CSTR_DOCWARN_REMOVEPERSONALINFO ) == 0 )
        nHandle = PROPERTYHANDLE_DOCWARN_REMOVEPERSONALINFO;
    else if( rName.compareToAscii( CSTR_DOCWARN_RECOMMENDPASSWORD ) == 0 )
        nHandle = PROPERTYHANDLE_DOCWARN_RECOMMENDPASSWORD;
    else if( rName.compareToAscii( CSTR_CTRLCLICK_HYPERLINK ) == 0 )
        nHandle = PROPERTYHANDLE_CTRLCLICK_HYPERLINK;
    else if( rName.compareToAscii( CSTR_MACRO_SECLEVEL ) == 0 )
        nHandle = PROPERTYHANDLE_MACRO_SECLEVEL;
    else if( rName.compareToAscii( CSTR_MACRO_TRUSTEDAUTHORS ) == 0 )
        nHandle = PROPERTYHANDLE_MACRO_TRUSTEDAUTHORS;
    else if( rName.compareToAscii( CSTR_MACRO_DISABLE ) == 0 )
        nHandle = PROPERTYHANDLE_MACRO_DISABLE;

    // xmlsec05 depricated
    else if( rName == PROPERTYNAME_STAROFFICEBASIC )
        nHandle = PROPERTYHANDLE_STAROFFICEBASIC;
    else if( rName == PROPERTYNAME_EXECUTEPLUGINS )
        nHandle = PROPERTYHANDLE_EXECUTEPLUGINS;
    else if( rName == PROPERTYNAME_WARNINGENABLED )
        nHandle = PROPERTYHANDLE_WARNINGENABLED;
    else if( rName == PROPERTYNAME_CONFIRMATIONENABLED )
        nHandle = PROPERTYHANDLE_CONFIRMATIONENABLED;
    // xmlsec05 depricated

    else
        nHandle = PROPERTYHANDLE_INVALID;

    return nHandle;
}

bool SvtSecurityOptions_Impl::GetOption( SvtSecurityOptions::EOption eOption, sal_Bool*& rpValue, sal_Bool*& rpRO )
{
    switch( eOption )
    {
        case SvtSecurityOptions::E_DOCWARN_SAVEORSEND:
            rpValue = &m_bSaveOrSend;
            rpRO = &m_bROSaveOrSend;
            break;
        case SvtSecurityOptions::E_DOCWARN_SIGNING:
            rpValue = &m_bSigning;
            rpRO = &m_bROSigning;
            break;
        case SvtSecurityOptions::E_DOCWARN_PRINT:
            rpValue = &m_bPrint;
            rpRO = &m_bROPrint;
            break;
        case SvtSecurityOptions::E_DOCWARN_CREATEPDF:
            rpValue = &m_bCreatePDF;
            rpRO = &m_bROCreatePDF;
            break;
        case SvtSecurityOptions::E_DOCWARN_REMOVEPERSONALINFO:
            rpValue = &m_bRemoveInfo;
            rpRO = &m_bRORemoveInfo;
            break;
        case SvtSecurityOptions::E_DOCWARN_RECOMMENDPASSWORD:
            rpValue = &m_bRecommendPwd;
            rpRO = &m_bRORecommendPwd;
            break;
        case SvtSecurityOptions::E_CTRLCLICK_HYPERLINK:
            rpValue = &m_bCtrlClickHyperlink;
            rpRO = &m_bROCtrlClickHyperlink;
            break;
        default:
            rpValue = NULL;
            rpRO = NULL;
            break;
    }

    return rpValue != NULL;
}

void SvtSecurityOptions_Impl::Notify( const Sequence< OUString >& seqPropertyNames )
{
    // Use given list of updated properties to get his values from configuration directly!
    Sequence< Any >         seqValues = GetProperties( seqPropertyNames );
    Sequence< sal_Bool >    seqRO = GetReadOnlyStates( seqPropertyNames );
    // Safe impossible cases.
    // We need values from ALL notified configuration keys.
    DBG_ASSERT( !(seqPropertyNames.getLength()!=seqValues.getLength()), "SvtSecurityOptions_Impl::Notify()\nI miss some values of configuration keys!\n" );
    // Step over list of property names and get right value from coreesponding value list to set it on internal members!
    sal_Int32               nCount = seqPropertyNames.getLength();
    for( sal_Int32 nProperty = 0 ; nProperty < nCount ; ++nProperty )
        SetProperty( GetHandle( seqPropertyNames[ nProperty ] ), seqValues[ nProperty ], seqRO[ nProperty ] );

    // read set of trusted authors separately
    LoadAuthors();
}

void SvtSecurityOptions_Impl::Commit()
{
    // Get names of supported properties, create a list for values and copy current values to it.
    Sequence< OUString >    lOrgNames = GetPropertyNames();
    sal_Int32               nOrgCount = lOrgNames.getLength();

    Sequence< OUString >    lNames(nOrgCount);
    Sequence< Any >         lValues(nOrgCount);
    sal_Int32               nRealCount = 0;
    bool                    bDone;

    ClearNodeSet( PROPERTYNAME_MACRO_TRUSTEDAUTHORS );

    for( sal_Int32 nProperty = 0 ; nProperty < nOrgCount ; ++nProperty )
    {
        switch( nProperty )
        {
            case PROPERTYHANDLE_SECUREURL:
            {
                bDone = !m_bROSecureURLs;
                if( bDone )
                {
                    Sequence< OUString >    lURLs( m_seqSecureURLs );
                    SvtPathOptions          aOpt;
                    sal_Int32               nURLsCnt = lURLs.getLength();
                    for( sal_Int32 nItem = 0 ; nItem < nURLsCnt ; ++nItem )
                        lURLs[ nItem ] = aOpt.UseVariable( lURLs[ nItem ] );
                    lValues[ nRealCount ] <<= lURLs;
                }
            }
            break;

            case PROPERTYHANDLE_DOCWARN_SAVEORSEND:
            {
                bDone = !m_bROSaveOrSend;
                if( bDone )
                    lValues[ nRealCount ] <<= m_bSaveOrSend;
            }
            break;

            case PROPERTYHANDLE_DOCWARN_SIGNING:
            {
                bDone = !m_bROSigning;
                if( bDone )
                    lValues[ nRealCount ] <<= m_bSigning;
            }
            break;

            case PROPERTYHANDLE_DOCWARN_PRINT:
            {
                bDone = !m_bROPrint;
                if( bDone )
                    lValues[ nRealCount ] <<= m_bPrint;
            }
            break;

            case PROPERTYHANDLE_DOCWARN_CREATEPDF:
            {
                bDone = !m_bROCreatePDF;
                if( bDone )
                    lValues[ nRealCount ] <<= m_bCreatePDF;
            }
            break;

            case PROPERTYHANDLE_DOCWARN_REMOVEPERSONALINFO:
            {
                bDone = !m_bRORemoveInfo;
                if( bDone )
                    lValues[ nRealCount ] <<= m_bRemoveInfo;
            }
            break;

            case PROPERTYHANDLE_DOCWARN_RECOMMENDPASSWORD:
            {
                bDone = !m_bRORecommendPwd;
                if( bDone )
                    lValues[ nRealCount ] <<= m_bRecommendPwd;
            }
            break;

            case PROPERTYHANDLE_CTRLCLICK_HYPERLINK:
            {
                bDone = !m_bROCtrlClickHyperlink;
                if( bDone )
                    lValues[ nRealCount ] <<= m_bCtrlClickHyperlink;
            }
            break;

            case PROPERTYHANDLE_MACRO_SECLEVEL:
            {
                bDone = !m_bROSecLevel;
                if( bDone )
                    lValues[ nRealCount ] <<= m_nSecLevel;
            }
            break;

            case PROPERTYHANDLE_MACRO_TRUSTEDAUTHORS:
            {
                bDone = !m_bROTrustedAuthors;
                if( bDone )
                {
                    sal_Int32   nCnt = m_seqTrustedAuthors.getLength();
                    if( nCnt )
                    {
                        String  s;
                        s.AppendAscii( CSTR_MACRO_TRUSTEDAUTHORS );
                        s.AppendAscii( "/a" );

                        Sequence< Sequence< com::sun::star::beans::PropertyValue > > lPropertyValuesSeq( nCnt );
                        for( sal_Int32 i = 0 ; i < nCnt ; ++i )
                        {
                            rtl::OUString aPrefix = rtl::OUStringBuffer(s).append(i).append('/').makeStringAndClear();
                            Sequence< com::sun::star::beans::PropertyValue >    lPropertyValues( 3 );
                            lPropertyValues[ 0 ].Name = aPrefix + PROPERTYNAME_TRUSTEDAUTHOR_SUBJECTNAME;
                            lPropertyValues[ 0 ].Value <<= m_seqTrustedAuthors[ i ][0];
                            lPropertyValues[ 1 ].Name = aPrefix + PROPERTYNAME_TRUSTEDAUTHOR_SERIALNUMBER;
                            lPropertyValues[ 1 ].Value <<= m_seqTrustedAuthors[ i ][1];
                            lPropertyValues[ 2 ].Name = aPrefix + PROPERTYNAME_TRUSTEDAUTHOR_RAWDATA;
                            lPropertyValues[ 2 ].Value <<= m_seqTrustedAuthors[ i ][2];


                            SetSetProperties( PROPERTYNAME_MACRO_TRUSTEDAUTHORS, lPropertyValues );
                        }

                        bDone = false;      // because we save in loop above!
                    }
                    else
                        bDone = false;
                }
            }
            break;

            case PROPERTYHANDLE_MACRO_DISABLE:
            {
                bDone = !m_bRODisableMacros;
                if( bDone )
                    lValues[ nRealCount ] <<= (sal_Bool)m_bDisableMacros;
            }
            break;


            // xmlsec05 depricated
            case PROPERTYHANDLE_STAROFFICEBASIC:
            {
                bDone = !m_bROBasicMode;
                if( bDone )
                    lValues[ nRealCount ] <<= (sal_Int32)m_eBasicMode;
            }
            break;
            case PROPERTYHANDLE_EXECUTEPLUGINS:
            {
                bDone = !m_bROExecutePlugins;
                if( bDone )
                    lValues[ nRealCount ] <<= m_bExecutePlugins;
            }
            break;
            case PROPERTYHANDLE_WARNINGENABLED:
            {
                bDone = !m_bROWarning;
                if( bDone )
                    lValues[ nRealCount ] <<= m_bWarning;
            }
            break;
            case PROPERTYHANDLE_CONFIRMATIONENABLED:
            {
                bDone = !m_bROConfirmation;
                if( bDone )
                    lValues[ nRealCount ] <<= m_bConfirmation;
            }
            break;
            // xmlsec05 depricated


            default:
                bDone = false;
        }

        if( bDone )
        {
            lNames[ nRealCount ] = lOrgNames[ nProperty ];
            ++nRealCount;
        }
    }
    // Set properties in configuration.
    lNames.realloc(nRealCount);
    lValues.realloc(nRealCount);
    PutProperties( lNames, lValues );
}

sal_Bool SvtSecurityOptions_Impl::IsReadOnly( SvtSecurityOptions::EOption eOption ) const
{
    sal_Bool    bReadonly;
    switch(eOption)
    {
        case SvtSecurityOptions::E_SECUREURLS :
            bReadonly = m_bROSecureURLs;
            break;
        case SvtSecurityOptions::E_DOCWARN_SAVEORSEND:
            bReadonly = m_bROSaveOrSend;
            break;
        case SvtSecurityOptions::E_DOCWARN_SIGNING:
            bReadonly = m_bROSigning;
            break;
        case SvtSecurityOptions::E_DOCWARN_PRINT:
            bReadonly = m_bROPrint;
            break;
        case SvtSecurityOptions::E_DOCWARN_CREATEPDF:
            bReadonly = m_bROCreatePDF;
            break;
        case SvtSecurityOptions::E_DOCWARN_REMOVEPERSONALINFO:
            bReadonly = m_bRORemoveInfo;
            break;
        case SvtSecurityOptions::E_DOCWARN_RECOMMENDPASSWORD:
            bReadonly = m_bRORecommendPwd;
            break;
        case SvtSecurityOptions::E_MACRO_SECLEVEL:
            bReadonly = m_bROSecLevel;
            break;
        case SvtSecurityOptions::E_MACRO_TRUSTEDAUTHORS:
            bReadonly = m_bROTrustedAuthors;
            break;
        case SvtSecurityOptions::E_MACRO_DISABLE:
            bReadonly = m_bRODisableMacros;
            break;
        case SvtSecurityOptions::E_CTRLCLICK_HYPERLINK:
            bReadonly = m_bROCtrlClickHyperlink;
            break;


        // xmlsec05 depricated
        case SvtSecurityOptions::E_BASICMODE:
            bReadonly = m_bROBasicMode;
            break;
        case SvtSecurityOptions::E_EXECUTEPLUGINS:
            bReadonly = m_bROExecutePlugins;
            break;
        case SvtSecurityOptions::E_WARNING:
            bReadonly = m_bROWarning;
            break;
        case SvtSecurityOptions::E_CONFIRMATION:
            bReadonly = m_bROConfirmation;
            break;
        // xmlsec05 depricated


        default:
            bReadonly = sal_True;
    }

    return bReadonly;
}

Sequence< OUString > SvtSecurityOptions_Impl::GetSecureURLs() const
{
    return m_seqSecureURLs;
}

void SvtSecurityOptions_Impl::SetSecureURLs( const Sequence< OUString >& seqURLList )
{
    DBG_ASSERT(!m_bROSecureURLs, "SvtSecurityOptions_Impl::SetSecureURLs()\nYou tried to write on a readonly value!\n");
    if (!m_bROSecureURLs && m_seqSecureURLs!=seqURLList)
    {
        m_seqSecureURLs = seqURLList;
        SetModified();
    }
}

sal_Bool SvtSecurityOptions_Impl::IsSecureURL(  const   OUString&   sURL    ,
                                                const   OUString&   sReferer) const
{
    sal_Bool bState = sal_False;

    // Check for uncritical protocols first
    // All protocols different from "macro..." and "slot..." are secure per definition and must not be checked.
    // "macro://#..." means AppBasic macros that are considered safe
    INetURLObject   aURL        ( sURL );
    INetProtocol    aProtocol   = aURL.GetProtocol();

    // All other URLs must checked in combination with referer and internal information about security
    if ( (aProtocol != INET_PROT_MACRO && aProtocol !=  INET_PROT_SLOT) ||
         aURL.GetMainURL( INetURLObject::NO_DECODE ).matchIgnoreAsciiCaseAsciiL( "macro:///", 9 ) == 0)
    {
        // security check only for "macro" ( without app basic ) or "slot" protocols
        bState = sal_True;
    }
    else
    {
        //  check list of allowed URL patterns
        // Trusted referer given?
        // NO  => bState will be false per default
        // YES => search for it in our internal url list
        if( !sReferer.isEmpty() )
        {
            // Search in internal list
            ::rtl::OUString sRef = sReferer.toAsciiLowerCase();
            sal_uInt32 nCount = m_seqSecureURLs.getLength();
            for( sal_uInt32 nItem=0; nItem<nCount; ++nItem )
            {
                OUString sCheckURL = m_seqSecureURLs[nItem].toAsciiLowerCase();
                sCheckURL += OUString(RTL_CONSTASCII_USTRINGPARAM("*"));
                if( WildCard( sCheckURL ).Matches( sRef ) == sal_True )
                {
                    bState = sal_True;
                    break;
                }
            }

            if ( !bState )
                bState = sRef.compareToAscii("private:user") == COMPARE_EQUAL;
        }
    }

    // Return result of operation.
    return bState;
}

inline sal_Int32 SvtSecurityOptions_Impl::GetMacroSecurityLevel() const
{
    return m_nSecLevel;
}

inline sal_Bool SvtSecurityOptions_Impl::IsMacroDisabled() const
{
    return m_bDisableMacros;
}

void SvtSecurityOptions_Impl::SetMacroSecurityLevel( sal_Int32 _nLevel )
{
    if( !m_bROSecLevel )
    {
        if( _nLevel > 3 || _nLevel < 0 )
            _nLevel = 3;

        if( m_nSecLevel != _nLevel )
        {
            m_nSecLevel = _nLevel;
            SetModified();
        }
    }
}

Sequence< SvtSecurityOptions::Certificate > SvtSecurityOptions_Impl::GetTrustedAuthors() const
{
    return m_seqTrustedAuthors;
}

void SvtSecurityOptions_Impl::SetTrustedAuthors( const Sequence< SvtSecurityOptions::Certificate >& rAuthors )
{
    DBG_ASSERT(!m_bROTrustedAuthors, "SvtSecurityOptions_Impl::SetTrustedAuthors()\nYou tried to write on a readonly value!\n");
    if( !m_bROTrustedAuthors && rAuthors != m_seqTrustedAuthors )
    {
        m_seqTrustedAuthors = rAuthors;
        SetModified();
    }
}

sal_Bool SvtSecurityOptions_Impl::IsOptionSet( SvtSecurityOptions::EOption eOption ) const
{
    sal_Bool*   pValue;
    sal_Bool*   pRO;
    sal_Bool    bRet = sal_False;

    if( ( const_cast< SvtSecurityOptions_Impl* >( this ) )->GetOption( eOption, pValue, pRO ) )
        bRet = *pValue;

    return bRet;
}

sal_Bool SvtSecurityOptions_Impl::SetOption( SvtSecurityOptions::EOption eOption, sal_Bool bValue )
{
    sal_Bool*   pValue;
    sal_Bool*   pRO;
    sal_Bool    bRet = sal_False;

    if( GetOption( eOption, pValue, pRO ) )
    {
        if( !*pRO )
        {
            bRet = sal_True;
            if( *pValue != bValue )
            {
                *pValue = bValue;
                SetModified();
            }
        }
    }

    return bRet;
}

sal_Bool SvtSecurityOptions_Impl::IsOptionEnabled( SvtSecurityOptions::EOption eOption ) const
{
    sal_Bool*   pValue;
    sal_Bool*   pRO;
    sal_Bool    bRet = sal_False;

    if( ( const_cast< SvtSecurityOptions_Impl* >( this ) )->GetOption( eOption, pValue, pRO ) )
        bRet = !*pRO;

    return bRet;
}

Sequence< OUString > SvtSecurityOptions_Impl::GetPropertyNames()
{
    // Build static list of configuration key names.
    const OUString pProperties[] =
    {
        PROPERTYNAME_SECUREURL,
        PROPERTYNAME_STAROFFICEBASIC,
        PROPERTYNAME_EXECUTEPLUGINS,
        PROPERTYNAME_WARNINGENABLED,
        PROPERTYNAME_CONFIRMATIONENABLED,
        PROPERTYNAME_DOCWARN_SAVEORSEND,
        PROPERTYNAME_DOCWARN_SIGNING,
        PROPERTYNAME_DOCWARN_PRINT,
        PROPERTYNAME_DOCWARN_CREATEPDF,
        PROPERTYNAME_DOCWARN_REMOVEPERSONALINFO,
        PROPERTYNAME_DOCWARN_RECOMMENDPASSWORD,
        PROPERTYNAME_CTRLCLICK_HYPERLINK,
        PROPERTYNAME_MACRO_SECLEVEL,
        PROPERTYNAME_MACRO_TRUSTEDAUTHORS,
        PROPERTYNAME_MACRO_DISABLE
    };
    // Initialize return sequence with these list ...
    const Sequence< OUString > seqPropertyNames( pProperties, PROPERTYCOUNT );
    // ... and return it.
    return seqPropertyNames;
}

//*****************************************************************************************************************
//  initialize static member
//  DON'T DO IT IN YOUR HEADER!
//  see definition for further informations
//*****************************************************************************************************************
SvtSecurityOptions_Impl*    SvtSecurityOptions::m_pDataContainer    = NULL  ;
sal_Int32                   SvtSecurityOptions::m_nRefCount         = 0     ;

SvtSecurityOptions::SvtSecurityOptions()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetInitMutex() );
    // Increase ouer refcount ...
    ++m_nRefCount;
    // ... and initialize ouer data container only if it not already exist!
    if( m_pDataContainer == NULL )
    {
        RTL_LOGFILE_CONTEXT(aLog, "unotools ( ??? ) ::SvtSecurityOptions_Impl::ctor()");
        m_pDataContainer = new SvtSecurityOptions_Impl;

        ItemHolder1::holdConfigItem(E_SECURITYOPTIONS);
    }
}

SvtSecurityOptions::~SvtSecurityOptions()
{
    // Global access, must be guarded (multithreading!)
    MutexGuard aGuard( GetInitMutex() );
    // Decrease ouer refcount.
    --m_nRefCount;
    // If last instance was deleted ...
    // we must destroy ouer static data container!
    if( m_nRefCount <= 0 )
    {
        delete m_pDataContainer;
        m_pDataContainer = NULL;
    }
}

sal_Bool SvtSecurityOptions::IsReadOnly( EOption eOption ) const
{
    MutexGuard aGuard( GetInitMutex() );
    return m_pDataContainer->IsReadOnly(eOption);
}

Sequence< OUString > SvtSecurityOptions::GetSecureURLs() const
{
    MutexGuard aGuard( GetInitMutex() );
    return m_pDataContainer->GetSecureURLs();
}

void SvtSecurityOptions::SetSecureURLs( const Sequence< OUString >& seqURLList )
{
    MutexGuard aGuard( GetInitMutex() );
    m_pDataContainer->SetSecureURLs( seqURLList );
}

sal_Bool SvtSecurityOptions::IsSecureURL(   const   OUString&   sURL        ,
                                            const   OUString&   sReferer    ) const
{
    MutexGuard aGuard( GetInitMutex() );
    return m_pDataContainer->IsSecureURL( sURL, sReferer );
}

sal_Int32 SvtSecurityOptions::GetMacroSecurityLevel() const
{
    MutexGuard aGuard( GetInitMutex() );
    return m_pDataContainer->GetMacroSecurityLevel();
}

void SvtSecurityOptions::SetMacroSecurityLevel( sal_Int32 _nLevel )
{
    MutexGuard aGuard( GetInitMutex() );
    m_pDataContainer->SetMacroSecurityLevel( _nLevel );
}

sal_Bool SvtSecurityOptions::IsMacroDisabled() const
{
    MutexGuard aGuard( GetInitMutex() );
    return m_pDataContainer->IsMacroDisabled();
}

Sequence< SvtSecurityOptions::Certificate > SvtSecurityOptions::GetTrustedAuthors() const
{
    MutexGuard aGuard( GetInitMutex() );
    return m_pDataContainer->GetTrustedAuthors();
}

void SvtSecurityOptions::SetTrustedAuthors( const Sequence< Certificate >& rAuthors )
{
    MutexGuard aGuard( GetInitMutex() );
    m_pDataContainer->SetTrustedAuthors( rAuthors );
}

bool SvtSecurityOptions::IsOptionSet( EOption eOption ) const
{
    MutexGuard aGuard( GetInitMutex() );
    return m_pDataContainer->IsOptionSet( eOption );
}

bool SvtSecurityOptions::SetOption( EOption eOption, bool bValue )
{
    MutexGuard aGuard( GetInitMutex() );
    return m_pDataContainer->SetOption( eOption, bValue );
}

bool SvtSecurityOptions::IsOptionEnabled( EOption eOption ) const
{
    MutexGuard aGuard( GetInitMutex() );
    return m_pDataContainer->IsOptionEnabled( eOption );
}

namespace
{
    class theSecurityOptionsMutex : public rtl::Static<osl::Mutex, theSecurityOptionsMutex>{};
}

Mutex& SvtSecurityOptions::GetInitMutex()
{
    return theSecurityOptionsMutex::get();
}




// xmlsec05 depricated

EBasicSecurityMode SvtSecurityOptions_Impl::GetBasicMode() const
{
    return m_eBasicMode;
}

void SvtSecurityOptions_Impl::SetBasicMode( EBasicSecurityMode eMode )
{
    DBG_ASSERT(!m_bROBasicMode, "SvtSecurityOptions_Impl::SetBasicMode()\nYou tried to write on a readonly value!\n");
    if (!m_bROBasicMode && m_eBasicMode!=eMode)
    {
        m_eBasicMode = eMode;
        SetModified();
    }
}

sal_Bool SvtSecurityOptions_Impl::IsExecutePlugins() const
{
    return m_bExecutePlugins;
}

void SvtSecurityOptions_Impl::SetExecutePlugins( sal_Bool bSet )
{
    DBG_ASSERT(!m_bROExecutePlugins, "SvtSecurityOptions_Impl::SetExecutePlugins()\nYou tried to write on a readonly value!\n");
    if (!m_bROExecutePlugins && m_bExecutePlugins!=bSet)
    {
        m_bExecutePlugins = bSet;
        SetModified();
    }
}

sal_Bool SvtSecurityOptions_Impl::IsWarningEnabled() const
{
    return m_bWarning;
}

void SvtSecurityOptions_Impl::SetWarningEnabled( sal_Bool bSet )
{
    DBG_ASSERT(!m_bROWarning, "SvtSecurityOptions_Impl::SetWarningEnabled()\nYou tried to write on a readonly value!\n");
    if (!m_bROWarning && m_bWarning!=bSet)
    {
        m_bWarning = bSet;
        SetModified();
    }
}

sal_Bool SvtSecurityOptions_Impl::IsConfirmationEnabled() const
{
    return m_bConfirmation;
}

void SvtSecurityOptions_Impl::SetConfirmationEnabled( sal_Bool bSet )
{
    DBG_ASSERT(!m_bROConfirmation, "SvtSecurityOptions_Impl::SetConfirmationEnabled()\nYou tried to write on a readonly value!\n");
    if (!m_bROConfirmation && m_bConfirmation!=bSet)
    {
        m_bConfirmation = bSet;
        SetModified();
    }
}


sal_Bool SvtSecurityOptions::IsExecutePlugins() const
{
    MutexGuard aGuard( GetInitMutex() );
    return m_pDataContainer->IsExecutePlugins();
}

void SvtSecurityOptions::SetExecutePlugins( sal_Bool bSet )
{
    MutexGuard aGuard( GetInitMutex() );
    m_pDataContainer->SetExecutePlugins( bSet );
}

sal_Bool SvtSecurityOptions::IsWarningEnabled() const
{
    MutexGuard aGuard( GetInitMutex() );
    return m_pDataContainer->IsWarningEnabled();
}

void SvtSecurityOptions::SetWarningEnabled( sal_Bool bSet )
{
    MutexGuard aGuard( GetInitMutex() );
    m_pDataContainer->SetWarningEnabled( bSet );
}

sal_Bool SvtSecurityOptions::IsConfirmationEnabled() const
{
    MutexGuard aGuard( GetInitMutex() );
    return m_pDataContainer->IsConfirmationEnabled();
}

void SvtSecurityOptions::SetConfirmationEnabled( sal_Bool bSet )
{
    MutexGuard aGuard( GetInitMutex() );
    m_pDataContainer->SetConfirmationEnabled( bSet );
}

void SvtSecurityOptions::SetBasicMode( EBasicSecurityMode eMode )
{
    MutexGuard aGuard( GetInitMutex() );
    m_pDataContainer->SetBasicMode( eMode );
}

EBasicSecurityMode SvtSecurityOptions::GetBasicMode() const
{
    MutexGuard aGuard( GetInitMutex() );
    return m_pDataContainer->GetBasicMode();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
