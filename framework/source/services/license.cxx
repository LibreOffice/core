/*************************************************************************
 *
 *  $RCSfile: license.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 11:08:53 $
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

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_SERVICES_LICENSE_HXX_
#include <services/license.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_H_
#include <services.h>
#endif

// local header for UI implementation
#include "services/licensedlg.hxx"
#include "classes/resource.hrc"

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/lang/XComponent.hpp>


//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustring.hxx>
#include <rtl/string.hxx>
#include <unotools/bootstrap.hxx>
#include <osl/file.hxx>
#include <svtools/xtextedt.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <tools/datetime.hxx>
#include <osl/file.hxx>
#include <osl/time.h>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{
using namespace utl;
using namespace ::osl                           ;
using namespace ::rtl                           ;
using namespace ::cppu                          ;
using namespace ::com::sun::star::uno           ;
using namespace ::com::sun::star::beans         ;
using namespace ::com::sun::star::lang          ;
using namespace ::com::sun::star::util          ;
using namespace ::com::sun::star::frame         ;

//_________________________________________________________________________________________________________________
//  non exported const
//_________________________________________________________________________________________________________________

// license file name
static const char *szLicensePath = "/share/readme";
static const char *szUNXLicenseName = "/LICENSE";
static const char *szUNXLicenseExt = "";
static const char *szWNTLicenseName = "/license";
static const char *szWNTLicenseExt = ".txt";

//_________________________________________________________________________________________________________________
//  non exported definitions
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

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



IMPL_STATIC_LINK( License, Terminate, void*, pvoid )
{
    /*
    Reference< XMultiServiceFactory > xFactory = comphelper::getProcessServiceFactory();
    Reference< XDesktop > xDesktop(xFactory->createInstance(
        OUString::createFromAscii("com.sun.star.frame.Desktop")), UNO_QUERY);
    if (xDesktop.is())
        xDesktop->terminate();
    */
    /*
    _exit(0);
    */
    return 0;
}

static DateTime _oslDateTimeToDateTime(const oslDateTime& aDateTime)
{
    return DateTime(
        Date(aDateTime.Day, aDateTime.Month, aDateTime.Year),
        Time(aDateTime.Hours, aDateTime.Minutes, aDateTime.Seconds));
}

static OUString _makeDateTimeString (const DateTime& aDateTime, sal_Bool bUTC = sal_False)
{
    OStringBuffer aDateTimeString;
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

static sal_Bool _parseDateTime(const OUString& aString, DateTime& aDateTime)
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

    Date tmpDate((USHORT)nDay, (USHORT)nMonth, (USHORT)nYear);
    Time tmpTime(nHour, nMinute, nSecond);
    DateTime tmpDateTime(tmpDate, tmpTime);
    if (aString.indexOf(aUTCString) < 0)
        tmpDateTime.ConvertToUTC();

    aDateTime = tmpDateTime;
    return sal_True;
}

static OUString _getCurrentDateString()
{
    OUString aString;
    return _makeDateTimeString(DateTime());
}

// execution of license check...
css::uno::Any SAL_CALL License::execute(const css::uno::Sequence< css::beans::NamedValue >& args)
    throw( css::lang::IllegalArgumentException, css::uno::Exception)
{
    // return value
    Any aRet; aRet <<= sal_False;

    try
    {
        OUString aBaseInstallPath;
        Bootstrap::PathStatus aBaseLocateResult =
            Bootstrap::locateBaseInstallation(aBaseInstallPath);
        if (aBaseLocateResult != Bootstrap::PATH_EXISTS)
        {
            // base install noit found
            // prepare termination
            // m_bTerminate = sal_True;
            // Application::PostUserEvent( STATIC_LINK( 0, License, Terminate ) );
            aRet <<= sal_False;
            return aRet;
        }
        // determine the filename of the license to show
        OUString  aLangString;
        ::com::sun::star::lang::Locale aLocale;
        OString aMgrName = OString("fwe") + OString::valueOf((sal_Int32)SUPD, 10);
        AllSettings aSettings(Application::GetSettings());
        aLocale = aSettings.GetUILocale();
        ResMgr* pResMgr = ResMgr::SearchCreateResMgr(aMgrName, aLocale);

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
#ifdef WNT
        OUString aLicensePath =
            aBaseInstallPath + OUString::createFromAscii(szLicensePath)
            + OUString::createFromAscii(szWNTLicenseName)
            + OUString::createFromAscii("_")
            + aLangString
            + OUString::createFromAscii(szWNTLicenseExt);
#else
        OUString aLicensePath =
            aBaseInstallPath + OUString::createFromAscii(szLicensePath)
            + OUString::createFromAscii(szUNXLicenseName)
            + OUString::createFromAscii("_")
            + aLangString
            + OUString::createFromAscii(szUNXLicenseExt);
#endif
        // check if we need to show the license at all
        // open org.openoffice.Setup/Office/ooLicenseAcceptDate
        OUString sConfigSrvc = OUString::createFromAscii("com.sun.star.configuration.ConfigurationProvider");
        OUString sAccessSrvc = OUString::createFromAscii("com.sun.star.configuration.ConfigurationUpdateAccess");
        OUString sReadSrvc   = OUString::createFromAscii("com.sun.star.configuration.ConfigurationAccess");

        // get configuration provider
        Reference< XMultiServiceFactory > theConfigProvider = Reference< XMultiServiceFactory >(
        m_xFactory->createInstance(sConfigSrvc), UNO_QUERY_THROW);
        Sequence< Any > theArgs(1);
        NamedValue v;
        v.Name = OUString::createFromAscii("NodePath");
        v.Value <<= OUString::createFromAscii("org.openoffice.Setup/Office");
        theArgs[0] <<= v;
        Reference< XPropertySet > pset = Reference< XPropertySet >(
            theConfigProvider->createInstanceWithArguments(sAccessSrvc, theArgs), UNO_QUERY_THROW);
        Any result = pset->getPropertyValue(OUString::createFromAscii("ooLicenseAcceptDate"));

        // if we find a date there, compare it to baseinstall license date
        OUString aAcceptDate;
        if (result >>= aAcceptDate)
        {
            // get LicenseFileDate from base install
            OUString aLicenseURL = aLicensePath;
            /*
            if (FileBase::getFileURLFromSystemPath(aLicensePath, aLicenseURL) != FileBase::E_None)
                return makeAny(sal_False);
                */
            DirectoryItem aDirItem;
            if (DirectoryItem::get(aLicenseURL, aDirItem) != FileBase::E_None)
                return makeAny(sal_False);
            FileStatus aStatus(FileStatusMask_All);
            if (aDirItem.getFileStatus(aStatus) != FileBase::E_None)
                return makeAny(sal_False);
            TimeValue aTimeVal = aStatus.getModifyTime();
            oslDateTime aDateTimeVal;
            if (!osl_getDateTimeFromTimeValue(&aTimeVal, &aDateTimeVal))
                return makeAny(sal_False);

            // compare dates
            DateTime aLicenseDateTime = _oslDateTimeToDateTime(aDateTimeVal);
            DateTime aAcceptDateTime;
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
            pset->setPropertyValue(OUString::createFromAscii("ooLicenseAcceptDate"), makeAny(aAcceptDate));
            Reference< XChangesBatch >(pset, UNO_QUERY_THROW)->commitChanges();

            // enable quickstarter
            sal_Bool bQuickstart( sal_True );
            sal_Bool bAutostart( sal_True );
            Sequence< Any > aSeq( 2 );
            aSeq[0] <<= bQuickstart;
            aSeq[1] <<= bAutostart;

            Reference < XInitialization > xQuickstart( ::comphelper::getProcessServiceFactory()->createInstance(
                OUString::createFromAscii( "com.sun.star.office.Quickstart" )),UNO_QUERY );
            if ( xQuickstart.is() )
                xQuickstart->initialize( aSeq );

            aRet <<= sal_True;
        }
        else
        {
            // license was not accepted
            // prepare termination
            // m_bTerminate = sal_True;
            // Application::PostUserEvent( STATIC_LINK( 0, License, Terminate ) );
            aRet <<= sal_False;
        }
    }
    catch (RuntimeException& e)
    {
        // license could not be verified
        aRet <<= sal_False;
    }
    return aRet;
}

void SAL_CALL License::close(sal_Bool bDeliverOwnership) throw (css::util::CloseVetoException)
{
    if (!m_bTerminate)
        throw CloseVetoException();
}
void SAL_CALL License::addCloseListener(const css::uno::Reference< css::util::XCloseListener >& aListener)
    throw (css::uno::RuntimeException)
{
}
void SAL_CALL License::removeCloseListener(const css::uno::Reference< css::util::XCloseListener >& aListener)
    throw (css::uno::RuntimeException)
{
}


//************************************************************************
//   License Dialog
//************************************************************************

LicenseDialog::LicenseDialog(const OUString & aLicensePath, ResMgr *pResMgr) :
    ModalDialog(NULL, ResId(DLG_LICENSE, pResMgr)),
    aLicenseML(this, ResId(ML_LICENSE, pResMgr)),
    aInfo1FT(this, ResId(FT_INFO1, pResMgr)),
    aInfo2FT(this, ResId(FT_INFO2, pResMgr)),
    aInfo3FT(this, ResId(FT_INFO3, pResMgr)),
    aInfo2_1FT(this, ResId(FT_INFO2_1, pResMgr)),
    aInfo3_1FT(this, ResId(FT_INFO3_1, pResMgr)),
    aFixedLine(this, ResId(FL_DIVIDE, pResMgr)),
    aPBPageDown(this, ResId(PB_PAGEDOWN, pResMgr)),
    aArrow(this, ResId(IMG_ARROW, pResMgr)),
    aStrAccept( ResId(LICENSE_ACCEPT, pResMgr) ),
    aStrNotAccept( ResId(LICENSE_NOTACCEPT, pResMgr) ),
    aPBAccept( this, ResId(PB_ACCEPT, pResMgr) ),
    aPBDecline( this, ResId(PB_DECLINE, pResMgr) ),
    bEndReached(FALSE)
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
    aText.SearchAndReplaceAll( UniString::CreateFromAscii("%PAGEDOWN"), aPBPageDown.GetText() );
    aInfo2FT.SetText( aText );

    aPBDecline.SetText( aStrNotAccept );
    aPBAccept.SetText( aStrAccept );

    aPBAccept.Disable();

    // load license text
    File aLicenseFile(aLicensePath);
    if ( aLicenseFile.open(OpenFlag_Read) == FileBase::E_None)
    {
        DirectoryItem d;
        DirectoryItem::get(aLicensePath, d);
        FileStatus fs(FileStatusMask_FileSize);
        d.getFileStatus(fs);
        sal_uInt64 nBytesRead = 0;
        sal_uInt64 nPosition = 0;
        sal_uInt32 nBytes = (sal_uInt32)fs.getFileSize();
        sal_Char *pBuffer = new sal_Char[nBytes];
        // FileBase RC r = FileBase::E_None;
        while (aLicenseFile.read(pBuffer+nPosition, nBytes-nPosition, nBytesRead) == FileBase::E_None
            && nPosition + nBytesRead < nBytes)
        {
            nPosition += nBytesRead;
        }
        OUString aLicenseString(pBuffer, nBytes, RTL_TEXTENCODING_UTF8,
                OSTRING_TO_OUSTRING_CVTFLAGS | RTL_TEXTTOUNICODE_FLAGS_GLOBAL_SIGNATURE);
        delete[] pBuffer;
        aLicenseML.SetText(aLicenseString);
    }

}

LicenseDialog::~LicenseDialog()
{
}

IMPL_LINK( LicenseDialog, PageDownHdl, PushButton *, EMPTYARG )
{
    aLicenseML.ScrollDown( SCROLL_PAGEDOWN );
    return 0;
}

IMPL_LINK( LicenseDialog, EndReachedHdl, LicenseView *, EMPTYARG )
{
    bEndReached = TRUE;

    EnableControls();

    return 0;
}

IMPL_LINK( LicenseDialog, ScrolledHdl, LicenseView *, EMPTYARG )
{
    EnableControls();

    return 0;
}

IMPL_LINK( LicenseDialog, DeclineBtnHdl, PushButton *, EMPTYARG )
{
    EndDialog(0);
    return 0;
}
IMPL_LINK( LicenseDialog, AcceptBtnHdl, PushButton *, EMPTYARG )
{
    EndDialog(1);
    return 0;
}


void LicenseDialog::EnableControls()
{
    if( !bEndReached &&
        ( aLicenseML.IsEndReached() || !aLicenseML.GetText().Len() ) )
        bEndReached = TRUE;

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

BOOL LicenseView::IsEndReached() const
{
    BOOL bEndReached;

    ExtTextView*    pView = GetTextView();
    ExtTextEngine*  pEdit = GetTextEngine();
    ULONG           nHeight = pEdit->GetTextHeight();
    Size            aOutSize = pView->GetWindow()->GetOutputSizePixel();
    Point           aBottom( 0, aOutSize.Height() );

    if ( (ULONG) pView->GetDocPos( aBottom ).Y() >= nHeight - 1 )
        bEndReached = TRUE;
    else
        bEndReached = FALSE;

    return bEndReached;
}

void LicenseView::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.IsA( TYPE(TextHint) ) )
    {
        BOOL    bLastVal = EndReached();
        ULONG   nId = ((const TextHint&)rHint).GetId();

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
