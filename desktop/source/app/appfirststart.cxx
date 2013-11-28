/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/ustring.hxx>
#include <tools/datetime.hxx>
#include <unotools/configmgr.hxx>

#include <comphelper/processfactory.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>

#include "app.hxx"

using ::rtl::OUString;
using namespace ::desktop;
using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;

static const OUString sConfigSrvc( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.configuration.ConfigurationProvider" ) );
static const OUString sAccessSrvc( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.configuration.ConfigurationUpdateAccess" ) );

/* Path of the license. */
OUString Desktop::GetLicensePath()
{
    // license file name
    static const char *szLicensePath = "/share/readme";
#if defined(WNT) || defined(OS2)
    static const char *szWNTLicenseName = "/license";
    static const char *szWNTLicenseExt = ".txt";
#else
    static const char *szUNXLicenseName = "/LICENSE";
    static const char *szUNXLicenseExt = "";
#endif
    static OUString aLicensePath;

    if (aLicensePath.getLength() > 0)
        return aLicensePath;

    OUString aBaseInstallPath(RTL_CONSTASCII_USTRINGPARAM("$OOO_BASE_DIR"));
    rtl::Bootstrap::expandMacros(aBaseInstallPath);

    // determine the filename of the license to show
    OUString  aLangString;
    ::com::sun::star::lang::Locale aLocale;
    OString aMgrName = OString("dkt");

    AllSettings aSettings(Application::GetSettings());
    aLocale = aSettings.GetUILocale();
    ResMgr* pLocalResMgr = ResMgr::SearchCreateResMgr( aMgrName.getStr(), aLocale);

    aLangString = aLocale.Language;
    if ( aLocale.Country.getLength() != 0 )
    {
        aLangString += OUString::createFromAscii("-");
        aLangString += aLocale.Country;
        if ( aLocale.Variant.getLength() != 0 )
        {
            aLangString += OUString::createFromAscii("-");
            aLangString += aLocale.Variant;
        }
    }
#if defined(WNT) || defined(OS2)
    aLicensePath =
        aBaseInstallPath + OUString::createFromAscii(szLicensePath)
        + OUString::createFromAscii(szWNTLicenseName)
        + OUString::createFromAscii("_")
        + aLangString
        + OUString::createFromAscii(szWNTLicenseExt);
#else
    aLicensePath =
        aBaseInstallPath + OUString::createFromAscii(szLicensePath)
        + OUString::createFromAscii(szUNXLicenseName)
        + OUString::createFromAscii("_")
        + aLangString
        + OUString::createFromAscii(szUNXLicenseExt);
#endif
    delete pLocalResMgr;
    return aLicensePath;
}

/* Check if we need to accept license. */
sal_Bool Desktop::LicenseNeedsAcceptance()
{
    // permissive licences like ALv2 don't need end user license approval
    return sal_False;
}

/* Local function - get access to the configuration */
static Reference< XPropertySet > impl_getConfigurationAccess( const OUString& rPath )
{
    Reference < XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();

    // get configuration provider
    Reference< XMultiServiceFactory > xConfigProvider = Reference< XMultiServiceFactory >(
            xFactory->createInstance( sConfigSrvc ), UNO_QUERY_THROW );

    Sequence< Any > aArgs( 1 );
    NamedValue aValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "NodePath" ) ), makeAny( rPath ) );
    aArgs[0] <<= aValue;
    return Reference< XPropertySet >(
            xConfigProvider->createInstanceWithArguments( sAccessSrvc, aArgs ), UNO_QUERY_THROW );
}

/* Local function - was the wizard completed already? */
static sal_Bool impl_isFirstStart()
{
    try {
        Reference< XPropertySet > xPSet = impl_getConfigurationAccess( OUString( RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.Setup/Office" ) ) );

        Any result = xPSet->getPropertyValue(OUString::createFromAscii("FirstStartWizardCompleted"));
        sal_Bool bCompleted = sal_False;
        if ((result >>= bCompleted) && bCompleted)
            return sal_False;  // wizard was already completed
        else
            return sal_True;
    } catch (const Exception&)
    {
        return sal_True;
    }
}

/* Local function - convert oslDateTime to tools DateTime */
static DateTime impl_oslDateTimeToDateTime(const oslDateTime& aDateTime)
{
    return DateTime(
        Date(aDateTime.Day, aDateTime.Month, aDateTime.Year),
        Time(aDateTime.Hours, aDateTime.Minutes, aDateTime.Seconds));
}

/* Local function - get DateTime from a string */
static sal_Bool impl_parseDateTime(const OUString& aString, DateTime& aDateTime)
{
    // take apart a canonical literal xsd:dateTime string
    //CCYY-MM-DDThh:mm:ss(Z)

    OUString aDateTimeString = aString.trim();

    // check length
    if (aDateTimeString.getLength() < 19 || aDateTimeString.getLength() > 20)
        return sal_False;

    sal_Int32 nDateLength = 10;
    sal_Int32 nTimeLength = 8;

    OUString aDateTimeSep = OUString::createFromAscii("T");
    OUString aDateSep = OUString::createFromAscii("-");
    OUString aTimeSep = OUString::createFromAscii(":");
    OUString aUTCString = OUString::createFromAscii("Z");

    OUString aDateString = aDateTimeString.copy(0, nDateLength);
    OUString aTimeString = aDateTimeString.copy(nDateLength+1, nTimeLength);

    sal_Int32 nIndex = 0;
    sal_Int32 nYear = aDateString.getToken(0, '-', nIndex).toInt32();
    sal_Int32 nMonth = aDateString.getToken(0, '-', nIndex).toInt32();
    sal_Int32 nDay = aDateString.getToken(0, '-', nIndex).toInt32();
    nIndex = 0;
    sal_Int32 nHour = aTimeString.getToken(0, ':', nIndex).toInt32();
    sal_Int32 nMinute = aTimeString.getToken(0, ':', nIndex).toInt32();
    sal_Int32 nSecond = aTimeString.getToken(0, ':', nIndex).toInt32();

    Date tmpDate((sal_uInt16)nDay, (sal_uInt16)nMonth, (sal_uInt16)nYear);
    Time tmpTime(nHour, nMinute, nSecond);
    DateTime tmpDateTime(tmpDate, tmpTime);
    if (aString.indexOf(aUTCString) < 0)
        tmpDateTime.ConvertToUTC();

    aDateTime = tmpDateTime;
    return sal_True;
}

/* Local function - was the license accepted already? */
static sal_Bool impl_isLicenseAccepted()
{
    // If no license will be shown ... it must not be accepted.
    // So it was accepted "hardly" by the outside installer.
    // But if the configuration entry "HideEula" will be removed afterwards ..
    // we have to show the licese page again and user has to accept it here .-)
    if ( ! Desktop::LicenseNeedsAcceptance() )
        return sal_True;

    try
    {
        Reference< XPropertySet > xPSet = impl_getConfigurationAccess( OUString( RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.Setup/Office" ) ) );

        Any result = xPSet->getPropertyValue(OUString::createFromAscii("LicenseAcceptDate"));

        OUString aAcceptDate;
        if (result >>= aAcceptDate)
        {
            // compare to date of license file
            OUString aLicenseURL = Desktop::GetLicensePath();
            osl::DirectoryItem aDirItem;
            if (osl::DirectoryItem::get(aLicenseURL, aDirItem) != osl::FileBase::E_None)
                return sal_False;
            osl::FileStatus aStatus(FileStatusMask_All);
            if (aDirItem.getFileStatus(aStatus) != osl::FileBase::E_None)
                return sal_False;
            TimeValue aTimeVal = aStatus.getModifyTime();
            oslDateTime aDateTimeVal;
            if (!osl_getDateTimeFromTimeValue(&aTimeVal, &aDateTimeVal))
                return sal_False;

            // compare dates
            DateTime aLicenseDateTime = impl_oslDateTimeToDateTime(aDateTimeVal);
            DateTime aAcceptDateTime;
            if (!impl_parseDateTime(aAcceptDate, aAcceptDateTime))
                return sal_False;

            if ( aAcceptDateTime > aLicenseDateTime )
                return sal_True;
        }
        return sal_False;
    } catch (const Exception&)
    {
        return sal_False;
    }
}

/* Check if we need the first start wizard. */
sal_Bool Desktop::IsFirstStartWizardNeeded()
{
    return impl_isFirstStart() || !impl_isLicenseAccepted();
}

void Desktop::FinishFirstStart()
{
#if 0 // most platforms no longer benefit from the quickstarter, TODO: are the other benefits worth it?
#ifndef OS2 // cannot enable quickstart on first startup, see shutdownicon.cxx comments.
        EnableQuickstart();
#endif
#endif

    try {
        Reference < XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
        // get configuration provider
        Reference< XMultiServiceFactory > theConfigProvider = Reference< XMultiServiceFactory >(
        xFactory->createInstance(sConfigSrvc), UNO_QUERY_THROW);
        Sequence< Any > theArgs(1);
        NamedValue v(OUString::createFromAscii("NodePath"),
            makeAny(OUString::createFromAscii("org.openoffice.Setup/Office")));
        theArgs[0] <<= v;
        Reference< XPropertySet > pset = Reference< XPropertySet >(
            theConfigProvider->createInstanceWithArguments(sAccessSrvc, theArgs), UNO_QUERY_THROW);
        pset->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("FirstStartWizardCompleted")), makeAny(sal_True));
        Reference< util::XChangesBatch >(pset, UNO_QUERY_THROW)->commitChanges();
    } catch (const uno::Exception&) {
        // ignore the exception as it is not critical enough to prevent office from starting
    }
}

void Desktop::EnableQuickstart()
{
    sal_Bool bQuickstart( sal_True );
    sal_Bool bAutostart( sal_True );
    Sequence< Any > aSeq( 2 );
    aSeq[0] <<= bQuickstart;
    aSeq[1] <<= bAutostart;

    Reference < XInitialization > xQuickstart( ::comphelper::getProcessServiceFactory()->createInstance(
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.office.Quickstart"))), UNO_QUERY );
    if ( xQuickstart.is() )
        xQuickstart->initialize( aSeq );
}

void Desktop::DoRestartActionsIfNecessary( sal_Bool bQuickStart )
{
    if ( bQuickStart )
    {
        try
        {
            Reference< XPropertySet > xPSet = impl_getConfigurationAccess( OUString( RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.Setup/Office" ) ) );

            OUString sPropName( RTL_CONSTASCII_USTRINGPARAM( "OfficeRestartInProgress" ) );
            Any aRestart = xPSet->getPropertyValue( sPropName );
            sal_Bool bRestart = sal_False;
            if ( ( aRestart >>= bRestart ) && bRestart )
            {
                xPSet->setPropertyValue( sPropName, makeAny( sal_False ) );
                Reference< util::XChangesBatch >( xPSet, UNO_QUERY_THROW )->commitChanges();

                Sequence< Any > aSeq( 2 );
                aSeq[0] <<= sal_True;
                aSeq[1] <<= sal_True;

                Reference < XInitialization > xQuickstart( ::comphelper::getProcessServiceFactory()->createInstance(
                    OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.office.Quickstart" ) ) ),UNO_QUERY_THROW );
                xQuickstart->initialize( aSeq );
            }
        }
        catch( uno::Exception& )
        {
            // this is no critical operation so it should not prevent office from starting
        }
    }
}

void Desktop::SetRestartState()
{
    try
    {
        Reference< XPropertySet > xPSet = impl_getConfigurationAccess( OUString( RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.Setup/Office" ) ) );
        OUString sPropName( RTL_CONSTASCII_USTRINGPARAM( "OfficeRestartInProgress" ) );
        xPSet->setPropertyValue( sPropName, makeAny( sal_True ) );
        Reference< util::XChangesBatch >( xPSet, UNO_QUERY_THROW )->commitChanges();
    }
    catch( uno::Exception& )
    {
        // this is no critical operation, ignore the exception
    }

}
