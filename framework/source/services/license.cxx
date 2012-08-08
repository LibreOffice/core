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

#include <services/license.hxx>
#include <threadhelp/resetableguard.hxx>
#include <macros/debug.hxx>
#include <services.h>

// local header for UI implementation
#include "services/licensedlg.hxx"
#include "classes/resource.hrc"

#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustring.hxx>
#include <rtl/string.hxx>
#include <unotools/bootstrap.hxx>
#include <osl/file.hxx>
#include <vcl/xtextedt.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <tools/datetime.hxx>
#include <osl/file.hxx>
#include <osl/time.h>

namespace framework{
using namespace utl;
using namespace ::osl                           ;
using namespace ::cppu                          ;
using namespace ::com::sun::star::uno           ;
using namespace ::com::sun::star::beans         ;
using namespace ::com::sun::star::lang          ;
using namespace ::com::sun::star::util          ;
using namespace ::com::sun::star::frame         ;

// license file name
static const char *szLicensePath = "/share/readme";
#ifdef UNX
static const char *szUNXLicenseName = "/LICENSE";
static const char *szUNXLicenseExt = "";
#elif defined(WNT)
static const char *szWNTLicenseName = "/license";
static const char *szWNTLicenseExt = ".txt";
#endif

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
License::License( const Reference< XMultiServiceFactory >& xFactory )
        //  Init baseclasses first
        //  Attention:
        //      Don't change order of initialization!
        //      ThreadHelpBase is a struct with a mutex as member. We can't use a mutex as member, while
        //      we must garant right initialization and a valid value of this! First initialize
        //      baseclasses and then members. And we need the mutex for other baseclasses !!!
        :   ThreadHelpBase  ( &Application::GetSolarMutex() )
        ,   OWeakObject     (                               )
        // Init member
        ,   m_xFactory      ( xFactory                      )
        ,   m_bTerminate    ( sal_False                     )
{
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
License::~License()
{
}

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************

DEFINE_XINTERFACE_4                 (   License                        ,
                                        OWeakObject                    ,
                                        DIRECT_INTERFACE(XTypeProvider ),
                                        DIRECT_INTERFACE(XServiceInfo  ),
                                        DIRECT_INTERFACE(XJob          ),
                                        DIRECT_INTERFACE(XCloseable    )
                                    )

DEFINE_XTYPEPROVIDER_4              (   License ,
                                        XTypeProvider   ,
                                        XServiceInfo    ,
                                        XJob            ,
                                        XCloseable
                                    )

DEFINE_XSERVICEINFO_MULTISERVICE    (   License,
                                        OWeakObject                 ,
                                        SERVICENAME_LICENSE         ,
                                        IMPLEMENTATIONNAME_LICENSE
                                    )

DEFINE_INIT_SERVICE                 (   License,
                                        {
                                        }
                                    )



static DateTime _oslDateTimeToDateTime(const oslDateTime& aDateTime)
{
    return DateTime(
        Date(aDateTime.Day, aDateTime.Month, aDateTime.Year),
        Time(aDateTime.Hours, aDateTime.Minutes, aDateTime.Seconds));
}

static ::rtl::OUString _makeDateTimeString (const DateTime& aDateTime, sal_Bool bUTC = sal_False)
{
    ::rtl::OStringBuffer aDateTimeString;
    aDateTimeString.append((sal_Int32)aDateTime.GetYear());
    aDateTimeString.append("-");
    if (aDateTime.GetMonth()<10) aDateTimeString.append("0");
    aDateTimeString.append((sal_Int32)aDateTime.GetMonth());
    aDateTimeString.append("-");
    if (aDateTime.GetDay()<10) aDateTimeString.append("0");
    aDateTimeString.append((sal_Int32)aDateTime.GetDay());
    aDateTimeString.append("T");
    if (aDateTime.GetHour()<10) aDateTimeString.append("0");
    aDateTimeString.append((sal_Int32)aDateTime.GetHour());
    aDateTimeString.append(":");
    if (aDateTime.GetMin()<10) aDateTimeString.append("0");
    aDateTimeString.append((sal_Int32)aDateTime.GetMin());
    aDateTimeString.append(":");
    if (aDateTime.GetSec()<10) aDateTimeString.append("0");
    aDateTimeString.append((sal_Int32)aDateTime.GetSec());
    if (bUTC) aDateTimeString.append("Z");

    return OStringToOUString(aDateTimeString.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US);
}

static sal_Bool _parseDateTime(const ::rtl::OUString& aString, DateTime& aDateTime)
{
    // take apart a canonical literal xsd:dateTime string
    //CCYY-MM-DDThh:mm:ss(Z)

    ::rtl::OUString aDateTimeString = aString.trim();

    // check length
    if (aDateTimeString.getLength() < 19 || aDateTimeString.getLength() > 20)
        return sal_False;

    sal_Int32 nDateLength = 10;
    sal_Int32 nTimeLength = 8;

    ::rtl::OUString aUTCString("Z");

    ::rtl::OUString aDateString = aDateTimeString.copy(0, nDateLength);
    ::rtl::OUString aTimeString = aDateTimeString.copy(nDateLength+1, nTimeLength);

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

static ::rtl::OUString _getCurrentDateString()
{
    ::rtl::OUString aString;
    return _makeDateTimeString(DateTime( DateTime::SYSTEM));
}

// execution of license check...
css::uno::Any SAL_CALL License::execute(const css::uno::Sequence< css::beans::NamedValue >& )
    throw( css::lang::IllegalArgumentException, css::uno::Exception)
{
    // return value
    Any aRet; aRet <<= sal_False;

    try
    {
        ::rtl::OUString aBaseInstallPath;
        Bootstrap::PathStatus aBaseLocateResult =
            Bootstrap::locateBaseInstallation(aBaseInstallPath);
        if (aBaseLocateResult != Bootstrap::PATH_EXISTS)
        {
            aRet <<= sal_False;
            return aRet;
        }
        // determine the filename of the license to show
        ::rtl::OUString  aLangString;
        ::com::sun::star::lang::Locale aLocale;
        AllSettings aSettings(Application::GetSettings());
        aLocale = aSettings.GetUILocale();
        ResMgr* pResMgr = ResMgr::SearchCreateResMgr("fwe", aLocale);

        aLangString = aLocale.Language;
        if ( !aLocale.Country.isEmpty() )
        {
            aLangString += ::rtl::OUString("-");
            aLangString += aLocale.Country;
            if ( !aLocale.Variant.isEmpty() )
            {
                aLangString += ::rtl::OUString("-");
                aLangString += aLocale.Variant;
            }
        }
#if defined(WNT)
        ::rtl::OUString aLicensePath =
            aBaseInstallPath + ::rtl::OUString::createFromAscii(szLicensePath)
            + ::rtl::OUString::createFromAscii(szWNTLicenseName)
            + ::rtl::OUString("_")
            + aLangString
            + ::rtl::OUString::createFromAscii(szWNTLicenseExt);
#else
        ::rtl::OUString aLicensePath =
            aBaseInstallPath + ::rtl::OUString::createFromAscii(szLicensePath)
            + ::rtl::OUString::createFromAscii(szUNXLicenseName)
            + ::rtl::OUString("_")
            + aLangString
            + ::rtl::OUString::createFromAscii(szUNXLicenseExt);
#endif
        // check if we need to show the license at all
        // open org.openoffice.Setup/Office/ooLicenseAcceptDate
        ::rtl::OUString sConfigSrvc = SERVICENAME_CFGPROVIDER;
        ::rtl::OUString sAccessSrvc("com.sun.star.configuration.ConfigurationUpdateAccess");

        // get configuration provider
        Reference< XMultiServiceFactory > theConfigProvider = Reference< XMultiServiceFactory >(
        m_xFactory->createInstance(sConfigSrvc), UNO_QUERY_THROW);
        Sequence< Any > theArgs(1);
        NamedValue v;
        v.Name = ::rtl::OUString("NodePath");
        v.Value <<= ::rtl::OUString("org.openoffice.Setup/Office");
        theArgs[0] <<= v;
        Reference< XPropertySet > pset = Reference< XPropertySet >(
            theConfigProvider->createInstanceWithArguments(sAccessSrvc, theArgs), UNO_QUERY_THROW);

        // if we find a date there, compare it to baseinstall license date
        ::rtl::OUString aAcceptDate;
        if (pset->getPropertyValue(::rtl::OUString("ooLicenseAcceptDate")) >>= aAcceptDate)
        {
            // get LicenseFileDate from base install
            ::rtl::OUString aLicenseURL = aLicensePath;
            DirectoryItem aDirItem;
            if (DirectoryItem::get(aLicenseURL, aDirItem) != FileBase::E_None)
                return makeAny(sal_False);
            FileStatus aStatus(osl_FileStatus_Mask_All);
            if (aDirItem.getFileStatus(aStatus) != FileBase::E_None)
                return makeAny(sal_False);
            TimeValue aTimeVal = aStatus.getModifyTime();
            oslDateTime aDateTimeVal;
            if (!osl_getDateTimeFromTimeValue(&aTimeVal, &aDateTimeVal))
                return makeAny(sal_False);

            // compare dates
            DateTime aLicenseDateTime = _oslDateTimeToDateTime(aDateTimeVal);
            DateTime aAcceptDateTime( DateTime::EMPTY);
            if (!_parseDateTime(aAcceptDate, aAcceptDateTime))
                return makeAny(sal_False);

            if ( aAcceptDateTime > aLicenseDateTime )
                return makeAny(sal_True);
        }
        // prepare to show
        // display license dialog
        LicenseDialog* pDialog = new LicenseDialog(aLicensePath, pResMgr);
        sal_Bool bAgreed = (pDialog->Execute() == 1);
        delete pDialog;

        if (bAgreed) {

            // write org.openoffice.Setup/ooLicenseAcceptDate
            aAcceptDate = _getCurrentDateString();
            pset->setPropertyValue(::rtl::OUString("ooLicenseAcceptDate"), makeAny(aAcceptDate));
            Reference< XChangesBatch >(pset, UNO_QUERY_THROW)->commitChanges();

            // enable quickstarter
            sal_Bool bQuickstart( sal_True );
            sal_Bool bAutostart( sal_True );
            Sequence< Any > aSeq( 2 );
            aSeq[0] <<= bQuickstart;
            aSeq[1] <<= bAutostart;

            Reference < XInitialization > xQuickstart( ::comphelper::getProcessServiceFactory()->createInstance(
                ::rtl::OUString("com.sun.star.office.Quickstart")),UNO_QUERY );
            if ( xQuickstart.is() )
                xQuickstart->initialize( aSeq );

            aRet <<= sal_True;
        }
        else
        {
            aRet <<= sal_False;
        }
    }
    catch (const RuntimeException&)
    {
        // license could not be verified
        aRet <<= sal_False;
    }
    return aRet;
}

void SAL_CALL License::close(sal_Bool /*bDeliverOwnership*/) throw (css::util::CloseVetoException)
{
    if (!m_bTerminate)
        throw CloseVetoException();
}
void SAL_CALL License::addCloseListener(const css::uno::Reference< css::util::XCloseListener >&)
    throw (css::uno::RuntimeException)
{
}
void SAL_CALL License::removeCloseListener(const css::uno::Reference< css::util::XCloseListener >&)
    throw (css::uno::RuntimeException)
{
}


//************************************************************************
//   License Dialog
//************************************************************************

LicenseDialog::LicenseDialog(const ::rtl::OUString & aLicensePath, ResMgr *pResMgr) :
    ModalDialog(NULL, ResId(DLG_LICENSE, *pResMgr)),
    aLicenseML(this, ResId(ML_LICENSE, *pResMgr)),
    aInfo1FT(this, ResId(FT_INFO1, *pResMgr)),
    aInfo2FT(this, ResId(FT_INFO2, *pResMgr)),
    aInfo3FT(this, ResId(FT_INFO3, *pResMgr)),
    aInfo2_1FT(this, ResId(FT_INFO2_1, *pResMgr)),
    aInfo3_1FT(this, ResId(FT_INFO3_1, *pResMgr)),
    aFixedLine(this, ResId(FL_DIVIDE, *pResMgr)),
    aPBPageDown(this, ResId(PB_PAGEDOWN, *pResMgr)),
    aPBDecline( this, ResId(PB_DECLINE, *pResMgr) ),
    aPBAccept( this, ResId(PB_ACCEPT, *pResMgr) ),
    aArrow(this, ResId(IMG_ARROW, *pResMgr)),
    aStrAccept( ResId(LICENSE_ACCEPT, *pResMgr) ),
    aStrNotAccept( ResId(LICENSE_NOTACCEPT, *pResMgr) ),
    bEndReached(sal_False)
{
    FreeResource();

    aLicenseML.SetEndReachedHdl( LINK(this, LicenseDialog, EndReachedHdl) );
    aLicenseML.SetScrolledHdl( LINK(this, LicenseDialog, ScrolledHdl) );

    aPBPageDown.SetClickHdl( LINK(this, LicenseDialog, PageDownHdl) );
    aPBDecline.SetClickHdl( LINK(this, LicenseDialog, DeclineBtnHdl) );
    aPBAccept.SetClickHdl( LINK(this, LicenseDialog, AcceptBtnHdl) );

    // We want a automatic repeating page down button
    WinBits aStyle = aPBPageDown.GetStyle();
    aStyle |= WB_REPEAT;
    aPBPageDown.SetStyle( aStyle );

    String aText = aInfo2FT.GetText();
    aText.SearchAndReplaceAll( rtl::OUString("%PAGEDOWN"), aPBPageDown.GetText() );
    aInfo2FT.SetText( aText );

    aPBDecline.SetText( aStrNotAccept );
    aPBAccept.SetText( aStrAccept );

    aPBAccept.Disable();

    // load license text
    File aLicenseFile(aLicensePath);
    if ( aLicenseFile.open(osl_File_OpenFlag_Read) == FileBase::E_None)
    {
        DirectoryItem d;
        DirectoryItem::get(aLicensePath, d);
        FileStatus fs(osl_FileStatus_Mask_FileSize);
        d.getFileStatus(fs);
        sal_uInt64 nBytesRead = 0;
        sal_uInt64 nPosition = 0;
        sal_uInt32 nBytes = (sal_uInt32)fs.getFileSize();
        sal_Char *pBuffer = new sal_Char[nBytes];
        while (aLicenseFile.read(pBuffer+nPosition, nBytes-nPosition, nBytesRead) == FileBase::E_None
            && nPosition + nBytesRead < nBytes)
        {
            nPosition += nBytesRead;
        }
        ::rtl::OUString aLicenseString(pBuffer, nBytes, RTL_TEXTENCODING_UTF8,
                OSTRING_TO_OUSTRING_CVTFLAGS | RTL_TEXTTOUNICODE_FLAGS_GLOBAL_SIGNATURE);
        delete[] pBuffer;
        aLicenseML.SetText(aLicenseString);
    }

}

LicenseDialog::~LicenseDialog()
{
}

IMPL_LINK_NOARG(LicenseDialog, PageDownHdl)
{
    aLicenseML.ScrollDown( SCROLL_PAGEDOWN );
    return 0;
}

IMPL_LINK_NOARG(LicenseDialog, EndReachedHdl)
{
    bEndReached = sal_True;

    EnableControls();

    return 0;
}

IMPL_LINK_NOARG(LicenseDialog, ScrolledHdl)
{
    EnableControls();

    return 0;
}

IMPL_LINK_NOARG(LicenseDialog, DeclineBtnHdl)
{
    EndDialog(0);
    return 0;
}
IMPL_LINK_NOARG(LicenseDialog, AcceptBtnHdl)
{
    EndDialog(1);
    return 0;
}


void LicenseDialog::EnableControls()
{
    if( !bEndReached &&
        ( aLicenseML.IsEndReached() || !aLicenseML.GetText().Len() ) )
        bEndReached = sal_True;

    if ( bEndReached )
    {
        Point aPos( aInfo1FT.GetPosPixel().X(),
                aInfo3_1FT.GetPosPixel().Y() );
        aArrow.SetPosPixel( aPos );
        aPBAccept.Enable();
    }
    else
    {
        Point aPos( aInfo1FT.GetPosPixel().X(),
                aInfo2_1FT.GetPosPixel().Y() );
        aArrow.SetPosPixel( aPos );
        aPBAccept.Disable();
    }

    if ( aLicenseML.IsEndReached() )
        aPBPageDown.Disable();
    else
        aPBPageDown.Enable();

}


LicenseView::LicenseView( Window* pParent, const ResId& rResId )
    : MultiLineEdit( pParent, rResId )
{
    SetLeftMargin( 5 );

    mbEndReached = IsEndReached();

    StartListening( *GetTextEngine() );
}

LicenseView::~LicenseView()
{
    maEndReachedHdl = Link();
    maScrolledHdl   = Link();

    EndListeningAll();
}

void LicenseView::ScrollDown( ScrollType eScroll )
{
    ScrollBar*  pScroll = GetVScrollBar();

    if ( pScroll )
        pScroll->DoScrollAction( eScroll );
}

sal_Bool LicenseView::IsEndReached() const
{
    sal_Bool bEndReached;

    ExtTextView*    pView = GetTextView();
    ExtTextEngine*  pEdit = GetTextEngine();
    sal_uLong           nHeight = pEdit->GetTextHeight();
    Size            aOutSize = pView->GetWindow()->GetOutputSizePixel();
    Point           aBottom( 0, aOutSize.Height() );

    if ( (sal_uLong) pView->GetDocPos( aBottom ).Y() >= nHeight - 1 )
        bEndReached = sal_True;
    else
        bEndReached = sal_False;

    return bEndReached;
}

void LicenseView::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.IsA( TYPE(TextHint) ) )
    {
        sal_Bool    bLastVal = EndReached();
        sal_uLong   nId = ((const TextHint&)rHint).GetId();

        if ( nId == TEXT_HINT_PARAINSERTED )
        {
            if ( bLastVal )
                mbEndReached = IsEndReached();
        }
        else if ( nId == TEXT_HINT_VIEWSCROLLED )
        {
            if ( ! mbEndReached )
                mbEndReached = IsEndReached();
            maScrolledHdl.Call( this );
        }

        if ( EndReached() && !bLastVal )
        {
            maEndReachedHdl.Call( this );
        }
    }
}

}       //  namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
