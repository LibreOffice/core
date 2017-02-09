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
#include <unotools/ucbhelper.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <comphelper/sequence.hxx>
#include <tools/urlobj.hxx>

#include <unotools/pathoptions.hxx>

#include "itemholder1.hxx"

//  namespaces

using namespace ::utl;
using namespace ::osl;
using namespace ::com::sun::star::uno;

#define ROOTNODE_SECURITY               "Office.Common/Security/Scripting"
#define DEFAULT_SECUREURL               Sequence< OUString >()
#define DEFAULT_TRUSTEDAUTHORS          Sequence< SvtSecurityOptions::Certificate >()

// xmlsec05 deprecated
#define DEFAULT_STAROFFICEBASIC         eALWAYS_EXECUTE

#define PROPERTYNAME_SECUREURL                  "SecureURL"
#define PROPERTYNAME_DOCWARN_SAVEORSEND         "WarnSaveOrSendDoc"
#define PROPERTYNAME_DOCWARN_SIGNING            "WarnSignDoc"
#define PROPERTYNAME_DOCWARN_PRINT              "WarnPrintDoc"
#define PROPERTYNAME_DOCWARN_CREATEPDF          "WarnCreatePDF"
#define PROPERTYNAME_DOCWARN_REMOVEPERSONALINFO "RemovePersonalInfoOnSaving"
#define PROPERTYNAME_DOCWARN_RECOMMENDPASSWORD  "RecommendPasswordProtection"
#define PROPERTYNAME_CTRLCLICK_HYPERLINK        "HyperlinksWithCtrlClick"
#define PROPERTYNAME_BLOCKUNTRUSTEDREFERERLINKS "BlockUntrustedRefererLinks"
#define PROPERTYNAME_MACRO_SECLEVEL             "MacroSecurityLevel"
#define PROPERTYNAME_MACRO_TRUSTEDAUTHORS       "TrustedAuthors"
#define PROPERTYNAME_MACRO_DISABLE              "DisableMacrosExecution"
#define PROPERTYNAME_TRUSTEDAUTHOR_SUBJECTNAME  "SubjectName"
#define PROPERTYNAME_TRUSTEDAUTHOR_SERIALNUMBER "SerialNumber"
#define PROPERTYNAME_TRUSTEDAUTHOR_RAWDATA      "RawData"

// xmlsec05 deprecated
#define PROPERTYNAME_STAROFFICEBASIC    "OfficeBasic"
#define PROPERTYNAME_EXECUTEPLUGINS     "ExecutePlugins"
#define PROPERTYNAME_WARNINGENABLED     "Warning"
#define PROPERTYNAME_CONFIRMATIONENABLED "Confirmation"
// xmlsec05 deprecated

#define PROPERTYHANDLE_SECUREURL                    0

// xmlsec05 deprecated
#define PROPERTYHANDLE_STAROFFICEBASIC  1
#define PROPERTYHANDLE_EXECUTEPLUGINS   2
#define PROPERTYHANDLE_WARNINGENABLED   3
#define PROPERTYHANDLE_CONFIRMATIONENABLED 4
// xmlsec05 deprecated

#define PROPERTYHANDLE_DOCWARN_SAVEORSEND           5
#define PROPERTYHANDLE_DOCWARN_SIGNING              6
#define PROPERTYHANDLE_DOCWARN_PRINT                7
#define PROPERTYHANDLE_DOCWARN_CREATEPDF            8
#define PROPERTYHANDLE_DOCWARN_REMOVEPERSONALINFO   9
#define PROPERTYHANDLE_DOCWARN_RECOMMENDPASSWORD    10
#define PROPERTYHANDLE_CTRLCLICK_HYPERLINK          11
#define PROPERTYHANDLE_BLOCKUNTRUSTEDREFERERLINKS   12
#define PROPERTYHANDLE_MACRO_SECLEVEL               13
#define PROPERTYHANDLE_MACRO_TRUSTEDAUTHORS         14
#define PROPERTYHANDLE_MACRO_DISABLE                15

#define PROPERTYCOUNT                               16
#define PROPERTYHANDLE_INVALID                      -1

#define CFG_READONLY_DEFAULT                        false

//  private declarations!

class SvtSecurityOptions_Impl : public ConfigItem
{

    private:
        virtual void ImplCommit() override;

    //  public methods

    public:

        //  constructor / destructor

         SvtSecurityOptions_Impl();
        virtual ~SvtSecurityOptions_Impl() override;

        //  override methods of baseclass

        /*-****************************************************************************************************
            @short      called for notify of configmanager
            @descr      This method is called from the ConfigManager before application ends or from the
                        PropertyChangeListener if the sub tree broadcasts changes. You must update your
                        internal values.

            @seealso    baseclass ConfigItem

            @param      "seqPropertyNames" is the list of properties which should be updated.
        *//*-*****************************************************************************************************/

        virtual void Notify( const Sequence< OUString >& seqPropertyNames ) override;

        //  public interface

        bool                IsReadOnly      ( SvtSecurityOptions::EOption eOption                   ) const;

        const Sequence< OUString >& GetSecureURLs(                                                       ) const { return m_seqSecureURLs;}
        void                    SetSecureURLs    (   const   Sequence< OUString >&   seqURLList          );
        inline sal_Int32        GetMacroSecurityLevel   (                                               ) const;
        void                    SetMacroSecurityLevel   ( sal_Int32 _nLevel                             );

        inline bool         IsMacroDisabled         (                                               ) const;

        const Sequence< SvtSecurityOptions::Certificate >& GetTrustedAuthors(                                                                                       ) const { return m_seqTrustedAuthors;}
        void                                        SetTrustedAuthors       ( const Sequence< SvtSecurityOptions::Certificate >& rAuthors                           );

        bool                IsOptionSet     ( SvtSecurityOptions::EOption eOption                   ) const;
        void                SetOption       ( SvtSecurityOptions::EOption eOption, bool bValue  );
        bool                IsOptionEnabled ( SvtSecurityOptions::EOption eOption                   ) const;


        void                    SetProperty( sal_Int32 nHandle, const Any& rValue, bool bReadOnly );
        void                    LoadAuthors();
        static sal_Int32        GetHandle( const OUString& rPropertyName );
        bool                    GetOption( SvtSecurityOptions::EOption eOption, bool*& rpValue, bool*& rpRO );

        /*-****************************************************************************************************
            @short      return list of key names of our configuration management which represent our module tree
            @descr      This method returns a static const list of key names. We need it to get needed values from our
                        configuration management.
            @return     A list of needed configuration keys is returned.
        *//*-*****************************************************************************************************/
        static Sequence< OUString > GetPropertyNames();

        Sequence< OUString >                        m_seqSecureURLs;
        bool                                    m_bSaveOrSend;
        bool                                    m_bSigning;
        bool                                    m_bPrint;
        bool                                    m_bCreatePDF;
        bool                                    m_bRemoveInfo;
        bool                                    m_bRecommendPwd;
        bool                                    m_bCtrlClickHyperlink;
        bool                                    m_bBlockUntrustedRefererLinks;
        sal_Int32                                   m_nSecLevel;
        Sequence< SvtSecurityOptions::Certificate > m_seqTrustedAuthors;
        bool                                    m_bDisableMacros;

        bool                                    m_bROSecureURLs;
        bool                                    m_bROSaveOrSend;
        bool                                    m_bROSigning;
        bool                                    m_bROPrint;
        bool                                    m_bROCreatePDF;
        bool                                    m_bRORemoveInfo;
        bool                                    m_bRORecommendPwd;
        bool                                    m_bROCtrlClickHyperlink;
        bool                                    m_bROBlockUntrustedRefererLinks;
        bool                                    m_bROSecLevel;
        bool                                    m_bROTrustedAuthors;
        bool                                    m_bRODisableMacros;

        // xmlsec05 deprecated
        EBasicSecurityMode      m_eBasicMode;
        bool                m_bExecutePlugins;
        bool                m_bWarning;
        bool                m_bConfirmation;

        bool                m_bROConfirmation;
        bool                m_bROWarning;
        bool                m_bROExecutePlugins;
        bool                m_bROBasicMode;
        public:
        bool IsWarningEnabled() const { return m_bWarning;}
        void SetWarningEnabled( bool bSet );
        bool IsConfirmationEnabled() const { return m_bConfirmation;}
        void SetConfirmationEnabled( bool bSet );
        bool    IsExecutePlugins() const { return m_bExecutePlugins;}
        void        SetExecutePlugins( bool bSet );
        // xmlsec05 deprecated
        EBasicSecurityMode      GetBasicMode    (                                               ) const { return m_eBasicMode;}
        void                    SetBasicMode    (           EBasicSecurityMode      eMode       );
};

//  constructor

SvtSecurityOptions_Impl::SvtSecurityOptions_Impl()
    :ConfigItem             ( ROOTNODE_SECURITY         )
    ,m_seqSecureURLs        ( DEFAULT_SECUREURL         )
    ,m_bSaveOrSend          ( true                  )
    ,m_bSigning             ( true                  )
    ,m_bPrint               ( true                  )
    ,m_bCreatePDF           ( true                  )
    ,m_bRemoveInfo          ( true                  )
    ,m_bRecommendPwd(false)
    ,m_bCtrlClickHyperlink(false)
    ,m_bBlockUntrustedRefererLinks(false)
    ,m_nSecLevel            ( 1                     )
    ,m_seqTrustedAuthors    ( DEFAULT_TRUSTEDAUTHORS    )
    ,m_bDisableMacros       ( false                 )
    ,m_bROSecureURLs        ( CFG_READONLY_DEFAULT      )
    ,m_bROSaveOrSend        ( CFG_READONLY_DEFAULT      )
    ,m_bROSigning           ( CFG_READONLY_DEFAULT      )
    ,m_bROPrint             ( CFG_READONLY_DEFAULT      )
    ,m_bROCreatePDF         ( CFG_READONLY_DEFAULT      )
    ,m_bRORemoveInfo        ( CFG_READONLY_DEFAULT      )
    ,m_bRORecommendPwd(CFG_READONLY_DEFAULT)
    ,m_bROCtrlClickHyperlink(CFG_READONLY_DEFAULT)
    ,m_bROBlockUntrustedRefererLinks(CFG_READONLY_DEFAULT)
    ,m_bROSecLevel          ( CFG_READONLY_DEFAULT      )
    ,m_bROTrustedAuthors    ( CFG_READONLY_DEFAULT      )
    ,m_bRODisableMacros     ( true                  ) // currently is not intended to be changed

    // xmlsec05 deprecated
    ,   m_eBasicMode        ( DEFAULT_STAROFFICEBASIC )
    ,   m_bExecutePlugins   ( true                )
    ,   m_bWarning          ( true                )
    ,   m_bConfirmation     ( true                )
    ,   m_bROConfirmation   ( CFG_READONLY_DEFAULT    )
    ,   m_bROWarning        ( CFG_READONLY_DEFAULT    )
    ,   m_bROExecutePlugins ( CFG_READONLY_DEFAULT    )
    ,   m_bROBasicMode      ( CFG_READONLY_DEFAULT    )
    // xmlsec05 deprecated

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
    for( sal_Int32 nProperty = 0; nProperty < nPropertyCount; ++nProperty )
        SetProperty( nProperty, seqValues[ nProperty ], seqRO[ nProperty ] );

    LoadAuthors();

    // Enable notification mechanism of our baseclass.
    // We need it to get information about changes outside these class on our used configuration keys!*/

    EnableNotification( seqNames );
}

//  destructor

SvtSecurityOptions_Impl::~SvtSecurityOptions_Impl()
{
    assert(!IsModified()); // should have been committed
}

void SvtSecurityOptions_Impl::SetProperty( sal_Int32 nProperty, const Any& rValue, bool bRO )
{
    switch( nProperty )
    {
        case PROPERTYHANDLE_SECUREURL:
        {
            m_seqSecureURLs.realloc( 0 );
            rValue >>= m_seqSecureURLs;
            SvtPathOptions  aOpt;
            sal_uInt32      nCount = m_seqSecureURLs.getLength();
            for( sal_uInt32 nItem = 0; nItem < nCount; ++nItem )
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

        case PROPERTYHANDLE_BLOCKUNTRUSTEDREFERERLINKS:
        {
            rValue >>= m_bBlockUntrustedRefererLinks;
            m_bROBlockUntrustedRefererLinks = bRO;
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

        // xmlsec05 deprecated
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
        // xmlsec05 deprecated

#if OSL_DEBUG_LEVEL > 0
        default:
            assert(false && "SvtSecurityOptions_Impl::SetProperty()\nUnknown property!\n");
#endif
        }
}

void SvtSecurityOptions_Impl::LoadAuthors()
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
        OUString                aSep( "/" );
        for( i1 = 0, i2 = 0; i1 < c1; ++i1 )
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
            std::vector< SvtSecurityOptions::Certificate > v;
            SvtSecurityOptions::Certificate aCert( 3 );
            for( i1 = 0, i2 = 0; i1 < c1; ++i1 )
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
            m_seqTrustedAuthors = comphelper::containerToSequence(v);
        }
    }
}

sal_Int32 SvtSecurityOptions_Impl::GetHandle( const OUString& rName )
{
    sal_Int32   nHandle;

    if( rName == PROPERTYNAME_SECUREURL )
        nHandle = PROPERTYHANDLE_SECUREURL;
    else if( rName == PROPERTYNAME_DOCWARN_SAVEORSEND )
        nHandle = PROPERTYHANDLE_DOCWARN_SAVEORSEND;
    else if( rName == PROPERTYNAME_DOCWARN_SIGNING )
        nHandle = PROPERTYHANDLE_DOCWARN_SIGNING;
    else if( rName == PROPERTYNAME_DOCWARN_PRINT )
        nHandle = PROPERTYHANDLE_DOCWARN_PRINT;
    else if( rName == PROPERTYNAME_DOCWARN_CREATEPDF )
        nHandle = PROPERTYHANDLE_DOCWARN_CREATEPDF;
    else if( rName == PROPERTYNAME_DOCWARN_REMOVEPERSONALINFO )
        nHandle = PROPERTYHANDLE_DOCWARN_REMOVEPERSONALINFO;
    else if( rName == PROPERTYNAME_DOCWARN_RECOMMENDPASSWORD )
        nHandle = PROPERTYHANDLE_DOCWARN_RECOMMENDPASSWORD;
    else if( rName == PROPERTYNAME_CTRLCLICK_HYPERLINK )
        nHandle = PROPERTYHANDLE_CTRLCLICK_HYPERLINK;
    else if( rName == PROPERTYNAME_BLOCKUNTRUSTEDREFERERLINKS )
        nHandle = PROPERTYHANDLE_BLOCKUNTRUSTEDREFERERLINKS;
    else if( rName == PROPERTYNAME_MACRO_SECLEVEL )
        nHandle = PROPERTYHANDLE_MACRO_SECLEVEL;
    else if( rName == PROPERTYNAME_MACRO_TRUSTEDAUTHORS )
        nHandle = PROPERTYHANDLE_MACRO_TRUSTEDAUTHORS;
    else if( rName == PROPERTYNAME_MACRO_DISABLE )
        nHandle = PROPERTYHANDLE_MACRO_DISABLE;

    // xmlsec05 deprecated
    else if( rName == PROPERTYNAME_STAROFFICEBASIC )
        nHandle = PROPERTYHANDLE_STAROFFICEBASIC;
    else if( rName == PROPERTYNAME_EXECUTEPLUGINS )
        nHandle = PROPERTYHANDLE_EXECUTEPLUGINS;
    else if( rName == PROPERTYNAME_WARNINGENABLED )
        nHandle = PROPERTYHANDLE_WARNINGENABLED;
    else if( rName == PROPERTYNAME_CONFIRMATIONENABLED )
        nHandle = PROPERTYHANDLE_CONFIRMATIONENABLED;
    // xmlsec05 deprecated

    else
        nHandle = PROPERTYHANDLE_INVALID;

    return nHandle;
}

bool SvtSecurityOptions_Impl::GetOption( SvtSecurityOptions::EOption eOption, bool*& rpValue, bool*& rpRO )
{
    switch( eOption )
    {
        case SvtSecurityOptions::EOption::DocWarnSaveOrSend:
            rpValue = &m_bSaveOrSend;
            rpRO = &m_bROSaveOrSend;
            break;
        case SvtSecurityOptions::EOption::DocWarnSigning:
            rpValue = &m_bSigning;
            rpRO = &m_bROSigning;
            break;
        case SvtSecurityOptions::EOption::DocWarnPrint:
            rpValue = &m_bPrint;
            rpRO = &m_bROPrint;
            break;
        case SvtSecurityOptions::EOption::DocWarnCreatePdf:
            rpValue = &m_bCreatePDF;
            rpRO = &m_bROCreatePDF;
            break;
        case SvtSecurityOptions::EOption::DocWarnRemovePersonalInfo:
            rpValue = &m_bRemoveInfo;
            rpRO = &m_bRORemoveInfo;
            break;
        case SvtSecurityOptions::EOption::DocWarnRecommendPassword:
            rpValue = &m_bRecommendPwd;
            rpRO = &m_bRORecommendPwd;
            break;
        case SvtSecurityOptions::EOption::CtrlClickHyperlink:
            rpValue = &m_bCtrlClickHyperlink;
            rpRO = &m_bROCtrlClickHyperlink;
            break;
        case SvtSecurityOptions::EOption::BlockUntrustedRefererLinks:
            rpValue = &m_bBlockUntrustedRefererLinks;
            rpRO = &m_bROBlockUntrustedRefererLinks;
            break;
        default:
            rpValue = nullptr;
            rpRO = nullptr;
            break;
    }

    return rpValue != nullptr;
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
    for( sal_Int32 nProperty = 0; nProperty < nCount; ++nProperty )
        SetProperty( GetHandle( seqPropertyNames[ nProperty ] ), seqValues[ nProperty ], seqRO[ nProperty ] );

    // read set of trusted authors separately
    LoadAuthors();
}

void SvtSecurityOptions_Impl::ImplCommit()
{
    // Get names of supported properties, create a list for values and copy current values to it.
    Sequence< OUString >    lOrgNames = GetPropertyNames();
    sal_Int32               nOrgCount = lOrgNames.getLength();

    Sequence< OUString >    lNames(nOrgCount);
    Sequence< Any >         lValues(nOrgCount);
    sal_Int32               nRealCount = 0;
    bool                    bDone;

    ClearNodeSet( PROPERTYNAME_MACRO_TRUSTEDAUTHORS );

    for( sal_Int32 nProperty = 0; nProperty < nOrgCount; ++nProperty )
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
                    for( sal_Int32 nItem = 0; nItem < nURLsCnt; ++nItem )
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

            case PROPERTYHANDLE_BLOCKUNTRUSTEDREFERERLINKS:
            {
                bDone = !m_bROBlockUntrustedRefererLinks;
                if( bDone )
                    lValues[ nRealCount ] <<= m_bBlockUntrustedRefererLinks;
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
                        for( sal_Int32 i = 0; i < nCnt; ++i )
                        {
                            OUString aPrefix(
                                PROPERTYNAME_MACRO_TRUSTEDAUTHORS "/a"
                                + OUString::number(i) + "/");
                            Sequence< css::beans::PropertyValue >    lPropertyValues( 3 );
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
                    lValues[ nRealCount ] <<= m_bDisableMacros;
            }
            break;

            // xmlsec05 deprecated
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
            // xmlsec05 deprecated

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

bool SvtSecurityOptions_Impl::IsReadOnly( SvtSecurityOptions::EOption eOption ) const
{
    bool    bReadonly;
    switch(eOption)
    {
        case SvtSecurityOptions::EOption::SecureUrls :
            bReadonly = m_bROSecureURLs;
            break;
        case SvtSecurityOptions::EOption::DocWarnSaveOrSend:
            bReadonly = m_bROSaveOrSend;
            break;
        case SvtSecurityOptions::EOption::DocWarnSigning:
            bReadonly = m_bROSigning;
            break;
        case SvtSecurityOptions::EOption::DocWarnPrint:
            bReadonly = m_bROPrint;
            break;
        case SvtSecurityOptions::EOption::DocWarnCreatePdf:
            bReadonly = m_bROCreatePDF;
            break;
        case SvtSecurityOptions::EOption::DocWarnRemovePersonalInfo:
            bReadonly = m_bRORemoveInfo;
            break;
        case SvtSecurityOptions::EOption::DocWarnRecommendPassword:
            bReadonly = m_bRORecommendPwd;
            break;
        case SvtSecurityOptions::EOption::MacroSecLevel:
            bReadonly = m_bROSecLevel;
            break;
        case SvtSecurityOptions::EOption::MacroTrustedAuthors:
            bReadonly = m_bROTrustedAuthors;
            break;
        case SvtSecurityOptions::EOption::CtrlClickHyperlink:
            bReadonly = m_bROCtrlClickHyperlink;
            break;
        case SvtSecurityOptions::EOption::BlockUntrustedRefererLinks:
            bReadonly = m_bROBlockUntrustedRefererLinks;
            break;

        // xmlsec05 deprecated
        case SvtSecurityOptions::EOption::BasicMode:
            bReadonly = m_bROBasicMode;
            break;
        case SvtSecurityOptions::EOption::ExecutePlugins:
            bReadonly = m_bROExecutePlugins;
            break;
        case SvtSecurityOptions::EOption::Warning:
            bReadonly = m_bROWarning;
            break;
        case SvtSecurityOptions::EOption::Confirmation:
            bReadonly = m_bROConfirmation;
            break;
        // xmlsec05 deprecated

        default:
            bReadonly = true;
    }

    return bReadonly;
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

inline sal_Int32 SvtSecurityOptions_Impl::GetMacroSecurityLevel() const
{
    return m_nSecLevel;
}

inline bool SvtSecurityOptions_Impl::IsMacroDisabled() const
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


void SvtSecurityOptions_Impl::SetTrustedAuthors( const Sequence< SvtSecurityOptions::Certificate >& rAuthors )
{
    DBG_ASSERT(!m_bROTrustedAuthors, "SvtSecurityOptions_Impl::SetTrustedAuthors()\nYou tried to write on a readonly value!\n");
    if( !m_bROTrustedAuthors && rAuthors != m_seqTrustedAuthors )
    {
        m_seqTrustedAuthors = rAuthors;
        SetModified();
    }
}

bool SvtSecurityOptions_Impl::IsOptionSet( SvtSecurityOptions::EOption eOption ) const
{
    bool*   pValue;
    bool*   pRO;
    bool    bRet = false;

    if( ( const_cast< SvtSecurityOptions_Impl* >( this ) )->GetOption( eOption, pValue, pRO ) )
        bRet = *pValue;

    return bRet;
}

void SvtSecurityOptions_Impl::SetOption( SvtSecurityOptions::EOption eOption, bool bValue )
{
    bool*   pValue;
    bool*   pRO;

    if( GetOption( eOption, pValue, pRO ) && !*pRO && *pValue != bValue)
    {
        *pValue = bValue;
        SetModified();
    }
}

bool SvtSecurityOptions_Impl::IsOptionEnabled( SvtSecurityOptions::EOption eOption ) const
{
    bool*   pValue;
    bool*   pRO;
    bool    bRet = false;

    if( ( const_cast< SvtSecurityOptions_Impl* >( this ) )->GetOption( eOption, pValue, pRO ) )
        bRet = !*pRO;

    return bRet;
}

Sequence< OUString > SvtSecurityOptions_Impl::GetPropertyNames()
{
    // Build static list of configuration key names.
    const OUString pProperties[] =
    {
        OUString(PROPERTYNAME_SECUREURL),
        OUString(PROPERTYNAME_STAROFFICEBASIC),
        OUString(PROPERTYNAME_EXECUTEPLUGINS),
        OUString(PROPERTYNAME_WARNINGENABLED),
        OUString(PROPERTYNAME_CONFIRMATIONENABLED),
        OUString(PROPERTYNAME_DOCWARN_SAVEORSEND),
        OUString(PROPERTYNAME_DOCWARN_SIGNING),
        OUString(PROPERTYNAME_DOCWARN_PRINT),
        OUString(PROPERTYNAME_DOCWARN_CREATEPDF),
        OUString(PROPERTYNAME_DOCWARN_REMOVEPERSONALINFO),
        OUString(PROPERTYNAME_DOCWARN_RECOMMENDPASSWORD),
        OUString(PROPERTYNAME_CTRLCLICK_HYPERLINK),
        OUString(PROPERTYNAME_BLOCKUNTRUSTEDREFERERLINKS),
        OUString(PROPERTYNAME_MACRO_SECLEVEL),
        OUString(PROPERTYNAME_MACRO_TRUSTEDAUTHORS),
        OUString(PROPERTYNAME_MACRO_DISABLE)
    };
    // Initialize return sequence with these list ...
    const Sequence< OUString > seqPropertyNames( pProperties, PROPERTYCOUNT );
    // ... and return it.
    return seqPropertyNames;
}

namespace {

std::weak_ptr<SvtSecurityOptions_Impl> g_pSecurityOptions;

}

SvtSecurityOptions::SvtSecurityOptions()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetInitMutex() );

    m_pImpl = g_pSecurityOptions.lock();
    if( !m_pImpl )
    {
        m_pImpl = std::make_shared<SvtSecurityOptions_Impl>();
        g_pSecurityOptions = m_pImpl;

        ItemHolder1::holdConfigItem(E_SECURITYOPTIONS);
    }
}

SvtSecurityOptions::~SvtSecurityOptions()
{
    // Global access, must be guarded (multithreading!)
    MutexGuard aGuard( GetInitMutex() );

    m_pImpl.reset();
}

bool SvtSecurityOptions::IsReadOnly( EOption eOption ) const
{
    MutexGuard aGuard( GetInitMutex() );
    return m_pImpl->IsReadOnly(eOption);
}

Sequence< OUString > SvtSecurityOptions::GetSecureURLs() const
{
    MutexGuard aGuard( GetInitMutex() );
    return m_pImpl->GetSecureURLs();
}

void SvtSecurityOptions::SetSecureURLs( const Sequence< OUString >& seqURLList )
{
    MutexGuard aGuard( GetInitMutex() );
    m_pImpl->SetSecureURLs( seqURLList );
}

bool SvtSecurityOptions::isSecureMacroUri(
    OUString const & uri, OUString const & referer) const
{
    switch (INetURLObject(uri).GetProtocol()) {
    case INetProtocol::Macro:
        if (uri.startsWithIgnoreAsciiCase("macro:///")) {
            // Denotes an App-BASIC macro (see SfxMacroLoader::loadMacro), which
            // is considered safe:
            return true;
        }
        SAL_FALLTHROUGH;
    case INetProtocol::Slot:
        return referer.equalsIgnoreAsciiCase("private:user")
            || isTrustedLocationUri(referer);
    default:
        return true;
    }
}

bool SvtSecurityOptions::isUntrustedReferer(OUString const & referer) const {
    MutexGuard g(GetInitMutex());
    return m_pImpl->IsOptionSet(EOption::BlockUntrustedRefererLinks)
        && !(referer.isEmpty() || referer.startsWithIgnoreAsciiCase("private:")
             || isTrustedLocationUri(referer));
}

bool SvtSecurityOptions::isTrustedLocationUri(OUString const & uri) const {
    MutexGuard g(GetInitMutex());
    for (sal_Int32 i = 0; i != m_pImpl->m_seqSecureURLs.getLength();
         ++i)
    {
        if (UCBContentHelper::IsSubPath(
                m_pImpl->m_seqSecureURLs[i], uri))
        {
            return true;
        }
    }
    return false;
}

bool SvtSecurityOptions::isTrustedLocationUriForUpdatingLinks(
    OUString const & uri) const
{
    return GetMacroSecurityLevel() == 0 || uri.isEmpty()
        || uri.startsWithIgnoreAsciiCase("private:")
        || isTrustedLocationUri(uri);
}

sal_Int32 SvtSecurityOptions::GetMacroSecurityLevel() const
{
    MutexGuard aGuard( GetInitMutex() );
    return m_pImpl->GetMacroSecurityLevel();
}

void SvtSecurityOptions::SetMacroSecurityLevel( sal_Int32 _nLevel )
{
    MutexGuard aGuard( GetInitMutex() );
    m_pImpl->SetMacroSecurityLevel( _nLevel );
}

bool SvtSecurityOptions::IsMacroDisabled() const
{
    MutexGuard aGuard( GetInitMutex() );
    return m_pImpl->IsMacroDisabled();
}

Sequence< SvtSecurityOptions::Certificate > SvtSecurityOptions::GetTrustedAuthors() const
{
    MutexGuard aGuard( GetInitMutex() );
    return m_pImpl->GetTrustedAuthors();
}

void SvtSecurityOptions::SetTrustedAuthors( const Sequence< Certificate >& rAuthors )
{
    MutexGuard aGuard( GetInitMutex() );
    m_pImpl->SetTrustedAuthors( rAuthors );
}

bool SvtSecurityOptions::IsOptionSet( EOption eOption ) const
{
    MutexGuard aGuard( GetInitMutex() );
    return m_pImpl->IsOptionSet( eOption );
}

void SvtSecurityOptions::SetOption( EOption eOption, bool bValue )
{
    MutexGuard aGuard( GetInitMutex() );
    m_pImpl->SetOption( eOption, bValue );
}

bool SvtSecurityOptions::IsOptionEnabled( EOption eOption ) const
{
    MutexGuard aGuard( GetInitMutex() );
    return m_pImpl->IsOptionEnabled( eOption );
}

namespace
{
    class theSecurityOptionsMutex : public rtl::Static<osl::Mutex, theSecurityOptionsMutex>{};
}

Mutex& SvtSecurityOptions::GetInitMutex()
{
    return theSecurityOptionsMutex::get();
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


void SvtSecurityOptions_Impl::SetExecutePlugins( bool bSet )
{
    DBG_ASSERT(!m_bROExecutePlugins, "SvtSecurityOptions_Impl::SetExecutePlugins()\nYou tried to write on a readonly value!\n");
    if (!m_bROExecutePlugins && m_bExecutePlugins!=bSet)
    {
        m_bExecutePlugins = bSet;
        SetModified();
    }
}


void SvtSecurityOptions_Impl::SetWarningEnabled( bool bSet )
{
    DBG_ASSERT(!m_bROWarning, "SvtSecurityOptions_Impl::SetWarningEnabled()\nYou tried to write on a readonly value!\n");
    if (!m_bROWarning && m_bWarning!=bSet)
    {
        m_bWarning = bSet;
        SetModified();
    }
}


void SvtSecurityOptions_Impl::SetConfirmationEnabled( bool bSet )
{
    DBG_ASSERT(!m_bROConfirmation, "SvtSecurityOptions_Impl::SetConfirmationEnabled()\nYou tried to write on a readonly value!\n");
    if (!m_bROConfirmation && m_bConfirmation!=bSet)
    {
        m_bConfirmation = bSet;
        SetModified();
    }
}

bool SvtSecurityOptions::IsExecutePlugins() const
{
    MutexGuard aGuard( GetInitMutex() );
    return m_pImpl->IsExecutePlugins();
}

void SvtSecurityOptions::SetExecutePlugins( bool bSet )
{
    MutexGuard aGuard( GetInitMutex() );
    m_pImpl->SetExecutePlugins( bSet );
}

bool SvtSecurityOptions::IsWarningEnabled() const
{
    MutexGuard aGuard( GetInitMutex() );
    return m_pImpl->IsWarningEnabled();
}

void SvtSecurityOptions::SetWarningEnabled( bool bSet )
{
    MutexGuard aGuard( GetInitMutex() );
    m_pImpl->SetWarningEnabled( bSet );
}

bool SvtSecurityOptions::IsConfirmationEnabled() const
{
    MutexGuard aGuard( GetInitMutex() );
    return m_pImpl->IsConfirmationEnabled();
}

void SvtSecurityOptions::SetConfirmationEnabled( bool bSet )
{
    MutexGuard aGuard( GetInitMutex() );
    m_pImpl->SetConfirmationEnabled( bSet );
}

void SvtSecurityOptions::SetBasicMode( EBasicSecurityMode eMode )
{
    MutexGuard aGuard( GetInitMutex() );
    m_pImpl->SetBasicMode( eMode );
}

EBasicSecurityMode SvtSecurityOptions::GetBasicMode() const
{
    MutexGuard aGuard( GetInitMutex() );
    return m_pImpl->GetBasicMode();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
