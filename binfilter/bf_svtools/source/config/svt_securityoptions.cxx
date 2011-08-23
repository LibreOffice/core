/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove

//_________________________________________________________________________________________________________________
//	includes
//_________________________________________________________________________________________________________________

#include <bf_svtools/securityoptions.hxx>

#include <unotools/configmgr.hxx>

#include <unotools/configitem.hxx>

#include <tools/debug.hxx>

#include <com/sun/star/uno/Any.hxx>

#include <com/sun/star/uno/Sequence.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>

#include <tools/urlobj.hxx>

#include <tools/wldcrd.hxx>

#include <bf_svtools/pathoptions.hxx>

#include <rtl/logfile.hxx>
#include "itemholder1.hxx"

//_________________________________________________________________________________________________________________
//	namespaces
//_________________________________________________________________________________________________________________

using namespace ::utl					;
using namespace ::rtl					;
using namespace ::osl					;
using namespace ::com::sun::star::uno	;

namespace binfilter
{

//_________________________________________________________________________________________________________________
//	const
//_________________________________________________________________________________________________________________

#define	ROOTNODE_SECURITY				OUString(RTL_CONSTASCII_USTRINGPARAM("Office.Common/Security/Scripting"))
#define	DEFAULT_SECUREURL				Sequence< OUString >()
#define	DEFAULT_SECLEVEL				3
#define DEFAULT_TRUSTEDAUTHORS			Sequence< SvtSecurityOptions::Certificate >()

// xmlsec05 depricated
#define	DEFAULT_STAROFFICEBASIC			eALWAYS_EXECUTE

#define	CSTR_SECUREURL					"SecureURL"
#define CSTR_DOCWARN_SAVEORSEND			"WarnSaveOrSendDoc"
#define CSTR_DOCWARN_SIGNING			"WarnSignDoc"
#define CSTR_DOCWARN_PRINT				"WarnPrintDoc"
#define CSTR_DOCWARN_CREATEPDF			"WarnCreatePDF"
#define CSTR_DOCWARN_REMOVEPERSONALINFO	"RemovePersonalInfoOnSaving"
#define CSTR_DOCWARN_RECOMMENDPASSWORD	"RecommendPasswordProtection"
#define CSTR_CTRLCLICK_HYPERLINK        "HyperlinksWithCtrlClick"
#define CSTR_MACRO_SECLEVEL				"MacroSecurityLevel"
#define CSTR_MACRO_TRUSTEDAUTHORS		"TrustedAuthors"
#define CSTR_MACRO_DISABLE				"DisableMacrosExecution"
#define CSTR_TRUSTEDAUTHOR_SUBJECTNAME	"SubjectName"
#define CSTR_TRUSTEDAUTHOR_SERIALNUMBER	"SerialNumber"
#define CSTR_TRUSTEDAUTHOR_RAWDATA		"RawData"

#define	PROPERTYNAME_SECUREURL					OUString(RTL_CONSTASCII_USTRINGPARAM(CSTR_SECUREURL						))
#define PROPERTYNAME_DOCWARN_SAVEORSEND			OUString(RTL_CONSTASCII_USTRINGPARAM(CSTR_DOCWARN_SAVEORSEND			))
#define PROPERTYNAME_DOCWARN_SIGNING			OUString(RTL_CONSTASCII_USTRINGPARAM(CSTR_DOCWARN_SIGNING				))
#define PROPERTYNAME_DOCWARN_PRINT				OUString(RTL_CONSTASCII_USTRINGPARAM(CSTR_DOCWARN_PRINT					))
#define PROPERTYNAME_DOCWARN_CREATEPDF			OUString(RTL_CONSTASCII_USTRINGPARAM(CSTR_DOCWARN_CREATEPDF				))
#define PROPERTYNAME_DOCWARN_REMOVEPERSONALINFO	OUString(RTL_CONSTASCII_USTRINGPARAM(CSTR_DOCWARN_REMOVEPERSONALINFO	))
#define PROPERTYNAME_DOCWARN_RECOMMENDPASSWORD  OUString(RTL_CONSTASCII_USTRINGPARAM(CSTR_DOCWARN_RECOMMENDPASSWORD     ))
#define PROPERTYNAME_CTRLCLICK_HYPERLINK        OUString(RTL_CONSTASCII_USTRINGPARAM(CSTR_CTRLCLICK_HYPERLINK           ))
#define PROPERTYNAME_MACRO_SECLEVEL				OUString(RTL_CONSTASCII_USTRINGPARAM(CSTR_MACRO_SECLEVEL				))
#define PROPERTYNAME_MACRO_TRUSTEDAUTHORS		OUString(RTL_CONSTASCII_USTRINGPARAM(CSTR_MACRO_TRUSTEDAUTHORS			))
#define PROPERTYNAME_MACRO_DISABLE				OUString(RTL_CONSTASCII_USTRINGPARAM(CSTR_MACRO_DISABLE					))
#define PROPERTYNAME_TRUSTEDAUTHOR_SUBJECTNAME	OUString(RTL_CONSTASCII_USTRINGPARAM(CSTR_TRUSTEDAUTHOR_SUBJECTNAME))
#define PROPERTYNAME_TRUSTEDAUTHOR_SERIALNUMBER	OUString(RTL_CONSTASCII_USTRINGPARAM(CSTR_TRUSTEDAUTHOR_SERIALNUMBER))
#define PROPERTYNAME_TRUSTEDAUTHOR_RAWDATA		OUString(RTL_CONSTASCII_USTRINGPARAM(CSTR_TRUSTEDAUTHOR_RAWDATA))

// xmlsec05 depricated
#define	PROPERTYNAME_STAROFFICEBASIC	OUString(RTL_CONSTASCII_USTRINGPARAM("OfficeBasic"	))
#define PROPERTYNAME_EXECUTEPLUGINS     OUString(RTL_CONSTASCII_USTRINGPARAM("ExecutePlugins"  ))
#define PROPERTYNAME_WARNINGENABLED     OUString(RTL_CONSTASCII_USTRINGPARAM("Warning"  ))
#define PROPERTYNAME_CONFIRMATIONENABLED OUString(RTL_CONSTASCII_USTRINGPARAM("Confirmation"  ))
// xmlsec05 depricated


#define	PROPERTYHANDLE_SECUREURL					0

// xmlsec05 depricated
#define	PROPERTYHANDLE_STAROFFICEBASIC	1
#define PROPERTYHANDLE_EXECUTEPLUGINS   2
#define PROPERTYHANDLE_WARNINGENABLED   3
#define PROPERTYHANDLE_CONFIRMATIONENABLED 4
// xmlsec05 depricated

#define PROPERTYHANDLE_DOCWARN_SAVEORSEND			5
#define PROPERTYHANDLE_DOCWARN_SIGNING				6
#define PROPERTYHANDLE_DOCWARN_PRINT				7
#define PROPERTYHANDLE_DOCWARN_CREATEPDF			8
#define PROPERTYHANDLE_DOCWARN_REMOVEPERSONALINFO	9
#define PROPERTYHANDLE_DOCWARN_RECOMMENDPASSWORD    10
#define PROPERTYHANDLE_CTRLCLICK_HYPERLINK          11
#define PROPERTYHANDLE_MACRO_SECLEVEL               12
#define PROPERTYHANDLE_MACRO_TRUSTEDAUTHORS         13
#define PROPERTYHANDLE_MACRO_DISABLE                14

#define PROPERTYCOUNT                               15
#define PROPERTYHANDLE_INVALID						-1

#define CFG_READONLY_DEFAULT						sal_False

//_________________________________________________________________________________________________________________
//	private declarations!
//_________________________________________________________________________________________________________________

class SvtSecurityOptions_Impl : public ConfigItem
{
    //-------------------------------------------------------------------------------------------------------------
    //	public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //	constructor / destructor
        //---------------------------------------------------------------------------------------------------------

         SvtSecurityOptions_Impl();
        ~SvtSecurityOptions_Impl();

        //---------------------------------------------------------------------------------------------------------
        //	overloaded methods of baseclass
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short		called for notify of configmanager
            @descr		These method is called from the ConfigManager before application ends or from the
                         PropertyChangeListener if the sub tree broadcasts changes. You must update your
                        internal values.

            @seealso	baseclass ConfigItem

            @param		"seqPropertyNames" is the list of properties which should be updated.
            @return		-

            @onerror	-
        *//*-*****************************************************************************************************/

        virtual void Notify( const Sequence< OUString >& seqPropertyNames );

        /*-****************************************************************************************************//**
            @short		write changes to configuration
            @descr		These method writes the changed values into the sub tree
                        and should always called in our destructor to guarantee consistency of config data.

            @seealso	baseclass ConfigItem

            @param		-
            @return		-

            @onerror	-
        *//*-*****************************************************************************************************/

        virtual void Commit();

        //---------------------------------------------------------------------------------------------------------
        //	public interface
        //---------------------------------------------------------------------------------------------------------

        inline sal_Int32		GetMacroSecurityLevel	(												) const	;

        inline sal_Bool			IsMacroDisabled			(												) const	;

private:

        /*-****************************************************************************************************//**
            @short		return list of key names of ouer configuration management which represent our module tree
            @descr		These methods return a static const list of key names. We need it to get needed values from our
                        configuration management.

            @seealso	-

            @param		-
            @return		A list of needed configuration keys is returned.

            @onerror	-
        *//*-*****************************************************************************************************/

        void					SetProperty( sal_Int32 nHandle, const Any& rValue, sal_Bool bReadOnly );
        void					LoadAuthors( void );
        static sal_Int32		GetHandle( const OUString& rPropertyName );

        static Sequence< OUString > GetPropertyNames();

        Sequence< OUString >					    m_seqSecureURLs;
        sal_Bool									m_bSaveOrSend;
        sal_Bool									m_bSigning;
        sal_Bool									m_bPrint;
        sal_Bool									m_bCreatePDF;
        sal_Bool									m_bRemoveInfo;
        sal_Bool                                    m_bRecommendPwd;
        sal_Bool                                    m_bCtrlClickHyperlink;
        sal_Int32									m_nSecLevel;
        Sequence< SvtSecurityOptions::Certificate >	m_seqTrustedAuthors;
        sal_Bool									m_bDisableMacros;

        sal_Bool                					m_bROSecureURLs;
        sal_Bool									m_bROSaveOrSend;
        sal_Bool									m_bROSigning;
        sal_Bool									m_bROPrint;
        sal_Bool									m_bROCreatePDF;
        sal_Bool									m_bRORemoveInfo;
        sal_Bool                                    m_bRORecommendPwd;
        sal_Bool                                    m_bROCtrlClickHyperlink;
        sal_Bool									m_bROSecLevel;
        sal_Bool									m_bROTrustedAuthors;
        sal_Bool									m_bRODisableMacros;


        // xmlsec05 depricated
        EBasicSecurityMode      m_eBasicMode;
        sal_Bool                m_bExecutePlugins;
        sal_Bool                m_bWarning;
        sal_Bool                m_bConfirmation;

        sal_Bool                m_bROConfirmation;
        sal_Bool                m_bROWarning;
        sal_Bool                m_bROExecutePlugins;
        sal_Bool                m_bROBasicMode;
};

//_________________________________________________________________________________________________________________
//	definitions
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//	constructor
//*****************************************************************************************************************
SvtSecurityOptions_Impl::SvtSecurityOptions_Impl()
    :ConfigItem				( ROOTNODE_SECURITY			)
    ,m_seqSecureURLs		( DEFAULT_SECUREURL			)
    ,m_bSaveOrSend			( sal_True					)
    ,m_bSigning				( sal_True					)
    ,m_bPrint				( sal_True					)
    ,m_bCreatePDF			( sal_True					)
    ,m_bRemoveInfo			( sal_True					)
    ,m_nSecLevel			( sal_True					)
    ,m_seqTrustedAuthors	( DEFAULT_TRUSTEDAUTHORS	)
    ,m_bDisableMacros		( sal_False					)
    ,m_bROSecureURLs		( CFG_READONLY_DEFAULT		)
    ,m_bROSaveOrSend		( CFG_READONLY_DEFAULT		)
    ,m_bROSigning			( CFG_READONLY_DEFAULT		)
    ,m_bROPrint				( CFG_READONLY_DEFAULT		)
    ,m_bROCreatePDF			( CFG_READONLY_DEFAULT		)
    ,m_bRORemoveInfo		( CFG_READONLY_DEFAULT		)
    ,m_bROSecLevel			( CFG_READONLY_DEFAULT		)
    ,m_bROTrustedAuthors	( CFG_READONLY_DEFAULT		)
    ,m_bRODisableMacros		( sal_True					) // currently is not intended to be changed

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
    Sequence< OUString >	seqNames	= GetPropertyNames	(			);
    Sequence< Any >			seqValues	= GetProperties		( seqNames	);
    Sequence< sal_Bool >	seqRO		= GetReadOnlyStates	( seqNames	);

    // Safe impossible cases.
    // We need values from ALL configuration keys.
    // Follow assignment use order of values in relation to our list of key names!
    DBG_ASSERT( !(seqNames.getLength()!=seqValues.getLength()), "SvtSecurityOptions_Impl::SvtSecurityOptions_Impl()\nI miss some values of configuration keys!\n" );

    // Copy values from list in right order to our internal member.
    sal_Int32				nPropertyCount = seqValues.getLength();
    for( sal_Int32 nProperty = 0 ; nProperty < nPropertyCount ; ++nProperty )
        SetProperty( nProperty, seqValues[ nProperty ], seqRO[ nProperty ] );

    LoadAuthors();

    // Enable notification mechanism of our baseclass.
    // We need it to get information about changes outside these class on our used configuration keys!*/

    EnableNotification( seqNames );
}

//*****************************************************************************************************************
//	destructor
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
            SvtPathOptions	aOpt;
            sal_uInt32		nCount = m_seqSecureURLs.getLength();
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

        case PROPERTYHANDLE_DOCWARN_REMOVEPERSONALINFO:
        {
            rValue >>= m_bRemoveInfo;
            m_bRORemoveInfo = bRO;
        }

        case PROPERTYHANDLE_DOCWARN_RECOMMENDPASSWORD:
        {
            rValue >>= m_bRecommendPwd;
            m_bRORecommendPwd = bRO;
        }

        case PROPERTYHANDLE_CTRLCLICK_HYPERLINK:
        {
            rValue >>= m_bCtrlClickHyperlink;
            m_bROCtrlClickHyperlink = bRO;
        }

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
    m_seqTrustedAuthors.realloc( 0 );		// first clear
    Sequence< OUString >	lAuthors = GetNodeNames( PROPERTYNAME_MACRO_TRUSTEDAUTHORS );
    sal_Int32				c1 = lAuthors.getLength();
    if( c1 )
    {
        sal_Int32				c2 = c1 * 3;				// 3 Properties inside Struct TrustedAuthor
        Sequence< OUString >	lAllAuthors( c2 );

        sal_Int32				i1;
        sal_Int32				i2;
        OUString				aSep( RTL_CONSTASCII_USTRINGPARAM( "/" ) );
        for( i1 = 0, i2 = 0 ; i1 < c1 ; ++i1 )
        {
            lAllAuthors[ i2 ] = PROPERTYNAME_MACRO_TRUSTEDAUTHORS + aSep + lAuthors[ i1 ] + aSep + PROPERTYNAME_TRUSTEDAUTHOR_SUBJECTNAME;
            ++i2;
            lAllAuthors[ i2 ] = PROPERTYNAME_MACRO_TRUSTEDAUTHORS + aSep + lAuthors[ i1 ] + aSep + PROPERTYNAME_TRUSTEDAUTHOR_SERIALNUMBER;
            ++i2;
            lAllAuthors[ i2 ] = PROPERTYNAME_MACRO_TRUSTEDAUTHORS + aSep + lAuthors[ i1 ] + aSep + PROPERTYNAME_TRUSTEDAUTHOR_RAWDATA;
            ++i2;
        }

        Sequence< Any >			lValues = GetProperties( lAllAuthors );
        if( lValues.getLength() == c2 )
        {
            m_seqTrustedAuthors.realloc( c1 );
            SvtSecurityOptions::Certificate aCert( 3 );
            for( i1 = 0, i2 = 0 ; i1 < c1 ; ++i1 )
            {
                lValues[ i2 ] >>= aCert[ 0 ];
                ++i2;
                lValues[ i2 ] >>= aCert[ 1 ];
                ++i2;
                lValues[ i2 ] >>= aCert[ 2 ];
                ++i2;
                m_seqTrustedAuthors[ i1 ] = aCert;
            }
        }
    }
}

sal_Int32 SvtSecurityOptions_Impl::GetHandle( const OUString& rName )
{
    sal_Int32	nHandle;

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

void SvtSecurityOptions_Impl::Notify( const Sequence< OUString >& seqPropertyNames )
{
    // Use given list of updated properties to get his values from configuration directly!
    Sequence< Any >			seqValues = GetProperties( seqPropertyNames );
    Sequence< sal_Bool >	seqRO = GetReadOnlyStates( seqPropertyNames );
    // Safe impossible cases.
    // We need values from ALL notified configuration keys.
    DBG_ASSERT( !(seqPropertyNames.getLength()!=seqValues.getLength()), "SvtSecurityOptions_Impl::Notify()\nI miss some values of configuration keys!\n" );
    // Step over list of property names and get right value from coreesponding value list to set it on internal members!
    sal_Int32				nCount = seqPropertyNames.getLength();
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
    bool					bDone;

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
                    Sequence< OUString >	lURLs( m_seqSecureURLs );
                    SvtPathOptions			aOpt;
                    sal_Int32				nURLsCnt = lURLs.getLength();
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
                    sal_Int32	nCnt = m_seqTrustedAuthors.getLength();
                    if( nCnt )
                    {
                        String	s;
                        s.AppendAscii( CSTR_MACRO_TRUSTEDAUTHORS );
                        s.AppendAscii( "/a" );

                        Sequence< Sequence< com::sun::star::beans::PropertyValue > > lPropertyValuesSeq( nCnt );
                        for( sal_Int32 i = 0 ; i < nCnt ; ++i )
                        {
                            String	aPrefix( s );
                            aPrefix += String::CreateFromInt32( i );
                            aPrefix.AppendAscii( "/" );
                            Sequence< com::sun::star::beans::PropertyValue >	lPropertyValues( 3 );
                            lPropertyValues[ 0 ].Name = aPrefix + PROPERTYNAME_TRUSTEDAUTHOR_SUBJECTNAME;
                            lPropertyValues[ 0 ].Value <<= m_seqTrustedAuthors[ i ][0];
                            lPropertyValues[ 1 ].Name = aPrefix + PROPERTYNAME_TRUSTEDAUTHOR_SERIALNUMBER;
                            lPropertyValues[ 1 ].Value <<= m_seqTrustedAuthors[ i ][1];
                            lPropertyValues[ 2 ].Name = aPrefix + PROPERTYNAME_TRUSTEDAUTHOR_RAWDATA;
                            lPropertyValues[ 2 ].Value <<= m_seqTrustedAuthors[ i ][2];


                            SetSetProperties( PROPERTYNAME_MACRO_TRUSTEDAUTHORS, lPropertyValues );
                        }

                        bDone = false;		// because we save in loop above!
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

inline sal_Int32 SvtSecurityOptions_Impl::GetMacroSecurityLevel() const
{
    return m_nSecLevel;
}

inline sal_Bool SvtSecurityOptions_Impl::IsMacroDisabled() const
{
    return m_bDisableMacros;
}

Sequence< OUString > SvtSecurityOptions_Impl::GetPropertyNames()
{
    // Build static list of configuration key names.
    static const OUString pProperties[] =
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
    static const Sequence< OUString > seqPropertyNames( pProperties, PROPERTYCOUNT );
    // ... and return it.
    return seqPropertyNames;
}

//*****************************************************************************************************************
//	initialize static member
//	DON'T DO IT IN YOUR HEADER!
//	see definition for further informations
//*****************************************************************************************************************
SvtSecurityOptions_Impl*	SvtSecurityOptions::m_pDataContainer	= NULL	;
sal_Int32					SvtSecurityOptions::m_nRefCount			= 0		;

SvtSecurityOptions::SvtSecurityOptions()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetInitMutex() );
    // Increase ouer refcount ...
    ++m_nRefCount;
    // ... and initialize ouer data container only if it not already exist!
    if( m_pDataContainer == NULL )
    {
        RTL_LOGFILE_CONTEXT(aLog, "svtools ( ??? ) ::SvtSecurityOptions_Impl::ctor()");
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

Mutex& SvtSecurityOptions::GetInitMutex()
{
    // Initialize static mutex only for one time!
    static Mutex* pMutex = NULL;
    // If these method first called (Mutex not already exist!) ...
    if( pMutex == NULL )
    {
        // ... we must create a new one. Protect follow code with the global mutex -
        // It must be - we create a static variable!
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        // We must check our pointer again - because it can be that another instance of ouer class will be faster then these!
        if( pMutex == NULL )
        {
            // Create the new mutex and set it for return on static variable.
            static Mutex aMutex;
            pMutex = &aMutex;
        }
    }
    // Return new created or already existing mutex object.
    return *pMutex;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
