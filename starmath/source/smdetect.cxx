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
#include "precompiled_starmath.hxx"

#include "smdetect.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/ucb/InteractiveAppException.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#include <framework/interaction.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <ucbhelper/simpleinteractionrequest.hxx>
#include <rtl/ustring.h>
#include <rtl/logfile.hxx>
#include <svl/itemset.hxx>
#include <vcl/window.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <tools/urlobj.hxx>
#include <vos/mutex.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>
#include <sot/storinfo.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/app.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/request.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/brokenpackageint.hxx>

#include "document.hxx"
#include "eqnolefilehdr.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ucb;
using namespace ::rtl;

SmFilterDetect::SmFilterDetect( const REFERENCE < ::com::sun::star::lang::XMultiServiceFactory >& /*xFactory*/ )
{
}

SmFilterDetect::~SmFilterDetect()
{
}

::rtl::OUString SAL_CALL SmFilterDetect::detect( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lDescriptor ) throw( ::com::sun::star::uno::RuntimeException )
{
    REFERENCE< XInputStream > xStream;
    REFERENCE< XContent > xContent;
    REFERENCE< XInteractionHandler > xInteraction;
    String aURL;
    ::rtl::OUString sTemp;
    String aTypeName;            // a name describing the type (from MediaDescriptor, usually from flat detection)
    String aPreselectedFilterName;      // a name describing the filter to use (from MediaDescriptor, usually from UI action)

    ::rtl::OUString aDocumentTitle; // interesting only if set in this method

    // opening as template is done when a parameter tells to do so and a template filter can be detected
    // (otherwise no valid filter would be found) or if the detected filter is a template filter and
    // there is no parameter that forbids to open as template
    sal_Bool bOpenAsTemplate = sal_False;
    sal_Bool bWasReadOnly = sal_False, bReadOnly = sal_False;

    sal_Bool bRepairPackage = sal_False;
    sal_Bool bRepairAllowed = sal_False;

    // now some parameters that can already be in the array, but may be overwritten or new inserted here
    // remember their indices in the case new values must be added to the array
    sal_Int32 nPropertyCount = lDescriptor.getLength();
    sal_Int32 nIndexOfFilterName = -1;
    sal_Int32 nIndexOfInputStream = -1;
    sal_Int32 nIndexOfContent = -1;
    sal_Int32 nIndexOfReadOnlyFlag = -1;
    sal_Int32 nIndexOfTemplateFlag = -1;
    sal_Int32 nIndexOfDocumentTitle = -1;

    for( sal_Int32 nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        // extract properties
        if( lDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("URL")) )
        {
            lDescriptor[nProperty].Value >>= sTemp;
            aURL = sTemp;
        }
        else if( !aURL.Len() && lDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("FileName")) )
        {
            lDescriptor[nProperty].Value >>= sTemp;
            aURL = sTemp;
        }
        else if( lDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("TypeName")) )
        {
            lDescriptor[nProperty].Value >>= sTemp;
            aTypeName = sTemp;
        }
        else if( lDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("FilterName")) )
        {
            lDescriptor[nProperty].Value >>= sTemp;
            aPreselectedFilterName = sTemp;

            // if the preselected filter name is not correct, it must be erased after detection
            // remember index of property to get access to it later
            nIndexOfFilterName = nProperty;
        }
        else if( lDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("InputStream")) )
            nIndexOfInputStream = nProperty;
        else if( lDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("ReadOnly")) )
            nIndexOfReadOnlyFlag = nProperty;
        else if( lDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("UCBContent")) )
            nIndexOfContent = nProperty;
        else if( lDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("AsTemplate")) )
        {
            lDescriptor[nProperty].Value >>= bOpenAsTemplate;
            nIndexOfTemplateFlag = nProperty;
        }
        else if( lDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("InteractionHandler")) )
            lDescriptor[nProperty].Value >>= xInteraction;
        else if( lDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("RepairPackage")) )
            lDescriptor[nProperty].Value >>= bRepairPackage;
        else if( lDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("DocumentTitle")) )
            nIndexOfDocumentTitle = nProperty;
    }

    // can't check the type for external filters, so set the "dont" flag accordingly
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    //SfxFilterFlags nMust = SFX_FILTER_IMPORT, nDont = SFX_FILTER_NOTINSTALLED;

    SfxApplication* pApp = SFX_APP();
    SfxAllItemSet *pSet = new SfxAllItemSet( pApp->GetPool() );
    TransformParameters( SID_OPENDOC, lDescriptor, *pSet );
    SFX_ITEMSET_ARG( pSet, pItem, SfxBoolItem, SID_DOC_READONLY, sal_False );

    bWasReadOnly = pItem && pItem->GetValue();

    String aFilterName;
    String aPrefix = String::CreateFromAscii( "private:factory/" );
    if( aURL.Match( aPrefix ) == aPrefix.Len() )
    {
        const SfxFilter* pFilter = 0;
        String aPattern( aPrefix );
        aPattern += String::CreateFromAscii("smath");
        if ( aURL.Match( aPattern ) >= aPattern.Len() )
        {
            pFilter = SfxFilter::GetDefaultFilterFromFactory( aURL );
            aTypeName = pFilter->GetTypeName();
            aFilterName = pFilter->GetName();
        }
    }
    else
    {
        // ctor of SfxMedium uses owner transition of ItemSet
        SfxMedium aMedium( aURL, bWasReadOnly ? STREAM_STD_READ : STREAM_STD_READWRITE, sal_False, NULL, pSet );
        aMedium.UseInteractionHandler( sal_True );

        sal_Bool bIsStorage = aMedium.IsStorage();
        if ( aMedium.GetErrorCode() == ERRCODE_NONE )
        {
            // remember input stream and content and put them into the descriptor later
            // should be done here since later the medium can switch to a version
            xStream = aMedium.GetInputStream();
            xContent = aMedium.GetContent();
            bReadOnly = aMedium.IsReadOnly();

            if ( bIsStorage )
            {
                //TODO/LATER: factor this out!
                uno::Reference < embed::XStorage > xStorage = aMedium.GetStorage( sal_False );
                if ( aMedium.GetLastStorageCreationState() != ERRCODE_NONE )
                {
                    // error during storage creation means _here_ that the medium
                    // is broken, but we can not handle it in medium since unpossibility
                    // to create a storage does not _always_ means that the medium is broken
                    aMedium.SetError( aMedium.GetLastStorageCreationState(), ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
                    if ( xInteraction.is() )
                    {
                        OUString empty;
                        try
                        {
                            InteractiveAppException xException( empty,
                                                            REFERENCE< XInterface >(),
                                                            InteractionClassification_ERROR,
                                                            aMedium.GetError() );

                            REFERENCE< XInteractionRequest > xRequest(
                                new ucbhelper::SimpleInteractionRequest( makeAny( xException ),
                                                                      ucbhelper::CONTINUATION_APPROVE ) );
                            xInteraction->handle( xRequest );
                        }
                        catch ( Exception & ) {};
                    }
                }
                else
                {
                    aFilterName.Erase();

                    try
                    {
                        const SfxFilter* pFilter = aPreselectedFilterName.Len() ?
                                SfxFilterMatcher().GetFilter4FilterName( aPreselectedFilterName ) : aTypeName.Len() ?
                                SfxFilterMatcher(String::CreateFromAscii("smath")).GetFilter4EA( aTypeName ) : 0;
                        String aTmpFilterName;
                        if ( pFilter )
                            aTmpFilterName = pFilter->GetName();
                        aTypeName = SfxFilter::GetTypeFromStorage( xStorage, pFilter ? pFilter->IsAllowedAsTemplate() : sal_False, &aTmpFilterName );
                    }
                    catch( lang::WrappedTargetException& aWrap )
                    {
                        packages::zip::ZipIOException aZipException;

                        // repairing is done only if this type is requested from outside
                        if ( ( aWrap.TargetException >>= aZipException ) && aTypeName.Len() )
                        {
                            if ( xInteraction.is() )
                            {
                                // the package is broken one
                                   aDocumentTitle = aMedium.GetURLObject().getName(
                                                            INetURLObject::LAST_SEGMENT,
                                                            true,
                                                            INetURLObject::DECODE_WITH_CHARSET );

                                if ( !bRepairPackage )
                                {
                                    // ask the user whether he wants to try to repair
                                    RequestPackageReparation aRequest( aDocumentTitle );
                                    xInteraction->handle( aRequest.GetRequest() );
                                    bRepairAllowed = aRequest.isApproved();
                                }

                                if ( !bRepairAllowed )
                                {
                                    // repair either not allowed or not successful
                                    NotifyBrokenPackage aNotifyRequest( aDocumentTitle );
                                    xInteraction->handle( aNotifyRequest.GetRequest() );
                                }
                            }

                            if ( !bRepairAllowed )
                                aTypeName.Erase();
                        }
                    }
                    catch( uno::RuntimeException& )
                    {
                        throw;
                    }
                    catch( uno::Exception& )
                    {
                        aTypeName.Erase();
                    }

                       if ( aTypeName.Len() )
                    {
                           const SfxFilter* pFilter =
                                    SfxFilterMatcher( String::CreateFromAscii("smath") ).GetFilter4EA( aTypeName );
                        if ( pFilter )
                            aFilterName = pFilter->GetName();
                    }
                }
            }
            else
            {
                // DesignScience Equation Editor MathType 3.0 ?
                SvStream *pStrm = aMedium.GetInStream();
                aTypeName.Erase();
                if (pStrm && !pStrm->GetError())
                {
                    SotStorageRef aStorage = new SotStorage ( pStrm, sal_False );
                    if ( !aStorage->GetError() )
                    {
                        if ( aStorage->IsStream( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "Equation Native" ) ) ) )
                        {
                            sal_uInt8 nVersion;
                            if (GetMathTypeVersion( aStorage, nVersion ) && nVersion <=3)
                                aTypeName.AssignAscii( "math_MathType_3x" );
                        }
                    }
                    else
                    {
                        // MathML? The SAX parser expects the 'math' root element incl.
                        // the namespace URL. Neither '<?xml' prolog nor <!doctype is needed.
                        // If the math element has a prefix (e.g. <bla:math), the
                        // prefix has to be defined in the namespace attribut
                        // (e.g. xmlns:bla="http://www.w3.org/1998/Math/MathML")
                        // #124636 is fixed too.
                        pStrm->Seek( STREAM_SEEK_TO_BEGIN );
                        const size_t nBufSize=2048;
                        sal_uInt16 aBuffer[nBufSize]; // will be casted to an Unicode-Array below
                        sal_uInt8* pByte = reinterpret_cast<sal_uInt8*>(aBuffer);
                        const sal_uLong nBytesRead(pStrm->Read( pByte, nBufSize * 2 ) );
                        const sal_uLong nUnicodeCharsRead (nBytesRead / 2 );

                        // For backwards searching an OUString is used. The conversion needs an
                        // encoding information. Default encoding is UTF-8, UTF-16 is possible
                        // (e.g. from MS "Math Input Control"), others are unlikely.
                        // Looking for Byte Order Mark
                        rtl_TextEncoding aEncoding = RTL_TEXTENCODING_UTF8;
                        bool bIsUnicode = false;
                        if (nBytesRead >= 2 && (aBuffer[0]==0xfffe || aBuffer[0]==0xfeff) )
                        {
                            aEncoding = RTL_TEXTENCODING_UNICODE;
                            bIsUnicode = true;
                            if ( aBuffer[0] == 0xfffe)
                            { //swap bytes to make Big Endian
                              for (size_t i=0; i < nUnicodeCharsRead; ++i)
                              {
                                  aBuffer[i] = (aBuffer[i]>>8) | (aBuffer[i]<<8) ;
                              }
                            }
                        }

                        bool isMathFile(false);
                        if ( nBytesRead > 56) // minimal <math xmlns="http://www.w3.org/1998/Math/MathML"></math>
                        {
                            const sal_Char* pChar = reinterpret_cast<sal_Char*>(aBuffer);
                            sal_Unicode* pUnicode = (sal_Unicode*) aBuffer;

                            const OUString sFragment( (bIsUnicode)
                                   ? OUString( pUnicode , nUnicodeCharsRead )
                                   : OUString( pChar, nBytesRead, aEncoding) );
                            const sal_Int32 nFragmentLength(sFragment.getLength());

                            // look for MathML URL http://www.w3.org/1998/Math/MathML
                            // #i53509 A MathML URL can be value of a namespace attribute, but can be as well
                            // inside a doctype e.g. [<!ENTITY mathml 'http://www.w3.org/1998/Math/MathML'>]
                            // or inside a schema reference e.g. s:schemaLocation="http://www.w3.org/1998/Math/MathML"
                            // Use a loop to get the correct one.
                            const OUString sURL( OUString::createFromAscii("http://www.w3.org/1998/Math/MathML"));
                            const sal_Int32 nURLLength = sURL.getLength();
                            const OUString sEQ( OUString::createFromAscii("=") );
                            const OUString sXMLNS( OUString::createFromAscii("xmlns") );
                            sal_Int32 nPosURL = -1; // for index of first character of URL
                            sal_Int32 nPosURLSearchStart = 0;
                            sal_Int32 nPosEQ = -1; // for index of equal sign
                            sal_Int32 nPosXMLNS = -1; // for index of first character of string "xmlns"
                            do
                            {
                                nPosURL = sFragment.indexOf(sURL,nPosURLSearchStart);
                                if( nPosURL < 0 )
                                {
                                    break; // no MathML URL, cannot be parsed
                                }
                                // need 'xmlns:prefix =' or 'xmlns =', look backwards, first for equal sign
                                nPosEQ = sFragment.lastIndexOf(sEQ,nPosURL);
                                if (nPosEQ >= 0 && nPosEQ >= nPosURLSearchStart)
                                {
                                    nPosXMLNS = sFragment.lastIndexOf(sXMLNS,nPosEQ);
                                    if( nPosXMLNS >= nPosURLSearchStart )
                                    { // an xmlns attribute is found, but it might belong to a schema
                                        // get prefix if present
                                        const OUString sPrefix = (sFragment.copy(nPosXMLNS+5,nPosEQ-(nPosXMLNS+5))).trim();
                                        // such prefix definition must start with colon (will be removed below)
                                        bool bHasPrefix( (sPrefix.isEmpty()) ? false : sPrefix.toChar() == sal_Unicode(':') );
                                        // the math element starts either with '<prefix:math' or '<math'
                                        const OUString sMathStart( (bHasPrefix)
                                                ?   OUString::createFromAscii("<") + sPrefix.copy(1,sPrefix.getLength()-1) + OUString::createFromAscii(":math")
                                                :   OUString::createFromAscii("<math") );
                                        sal_Int32 nPosMath (sFragment.lastIndexOf(sMathStart,nPosXMLNS));
                                        if( nPosMath >= 0)
                                        {   // xmlns attribute belongs to math element
                                            isMathFile = true;
                                            break;
                                        }
                                    }
                                }
                                // MathML URL was wrong one, look for next
                                nPosURLSearchStart = nPosURL + nURLLength;
                            }
                            while ( nPosURLSearchStart + nURLLength <= nFragmentLength);

                            if(isMathFile)
                            {
                                static const sal_Char sFltrNm_2[] = MATHML_XML;
                                static const sal_Char sTypeNm_2[] = "math_MathML_XML_Math";

                                aFilterName.AssignAscii( sFltrNm_2 );
                                aTypeName.AssignAscii( sTypeNm_2 );
                            }
                        }
                    }

                    if ( aTypeName.Len() )
                    {
                        const SfxFilter* pFilt = SfxFilterMatcher( String::CreateFromAscii("smath") ).GetFilter4EA( aTypeName );
                        if ( pFilt )
                            aFilterName = pFilt->GetName();
                    }
                }
            }
        }
    }

    if ( nIndexOfInputStream == -1 && xStream.is() )
    {
        // if input stream wasn't part of the descriptor, now it should be, otherwise the content would be opend twice
        lDescriptor.realloc( nPropertyCount + 1 );
        lDescriptor[nPropertyCount].Name = ::rtl::OUString::createFromAscii("InputStream");
        lDescriptor[nPropertyCount].Value <<= xStream;
        nPropertyCount++;
    }

    if ( nIndexOfContent == -1 && xContent.is() )
    {
        // if input stream wasn't part of the descriptor, now it should be, otherwise the content would be opend twice
        lDescriptor.realloc( nPropertyCount + 1 );
        lDescriptor[nPropertyCount].Name = ::rtl::OUString::createFromAscii("UCBContent");
        lDescriptor[nPropertyCount].Value <<= xContent;
        nPropertyCount++;
    }

    if ( bReadOnly != bWasReadOnly )
    {
        if ( nIndexOfReadOnlyFlag == -1 )
        {
            lDescriptor.realloc( nPropertyCount + 1 );
            lDescriptor[nPropertyCount].Name = ::rtl::OUString::createFromAscii("ReadOnly");
            lDescriptor[nPropertyCount].Value <<= bReadOnly;
            nPropertyCount++;
        }
        else
            lDescriptor[nIndexOfReadOnlyFlag].Value <<= bReadOnly;
    }

    if ( !bRepairPackage && bRepairAllowed )
    {
        lDescriptor.realloc( nPropertyCount + 1 );
        lDescriptor[nPropertyCount].Name = ::rtl::OUString::createFromAscii("RepairPackage");
        lDescriptor[nPropertyCount].Value <<= bRepairAllowed;
        nPropertyCount++;

        bOpenAsTemplate = sal_True;

        // TODO/LATER: set progress bar that should be used
    }

    if ( bOpenAsTemplate )
    {
        if ( nIndexOfTemplateFlag == -1 )
        {
            lDescriptor.realloc( nPropertyCount + 1 );
            lDescriptor[nPropertyCount].Name = ::rtl::OUString::createFromAscii("AsTemplate");
            lDescriptor[nPropertyCount].Value <<= bOpenAsTemplate;
            nPropertyCount++;
        }
        else
            lDescriptor[nIndexOfTemplateFlag].Value <<= bOpenAsTemplate;
    }

    if ( aDocumentTitle.getLength() )
    {
        // the title was set here
        if ( nIndexOfDocumentTitle == -1 )
        {
            lDescriptor.realloc( nPropertyCount + 1 );
            lDescriptor[nPropertyCount].Name = ::rtl::OUString::createFromAscii("DocumentTitle");
            lDescriptor[nPropertyCount].Value <<= aDocumentTitle;
            nPropertyCount++;
        }
        else
            lDescriptor[nIndexOfDocumentTitle].Value <<= aDocumentTitle;
    }

    if ( !aFilterName.Len() )
        aTypeName.Erase();

    return aTypeName;
}

SFX_IMPL_SINGLEFACTORY( SmFilterDetect )

/* XServiceInfo */
UNOOUSTRING SAL_CALL SmFilterDetect::getImplementationName() throw( UNORUNTIMEEXCEPTION )
{
    return impl_getStaticImplementationName();
}
                                                                                                                                \
/* XServiceInfo */
sal_Bool SAL_CALL SmFilterDetect::supportsService( const UNOOUSTRING& sServiceName ) throw( UNORUNTIMEEXCEPTION )
{
    UNOSEQUENCE< UNOOUSTRING >  seqServiceNames =   getSupportedServiceNames();
    const UNOOUSTRING*          pArray          =   seqServiceNames.getConstArray();
    for ( sal_Int32 nCounter=0; nCounter<seqServiceNames.getLength(); nCounter++ )
    {
        if ( pArray[nCounter] == sServiceName )
        {
            return sal_True ;
        }
    }
    return sal_False ;
}

/* XServiceInfo */
UNOSEQUENCE< UNOOUSTRING > SAL_CALL SmFilterDetect::getSupportedServiceNames() throw( UNORUNTIMEEXCEPTION )
{
    return impl_getStaticSupportedServiceNames();
}

/* Helper for XServiceInfo */
UNOSEQUENCE< UNOOUSTRING > SmFilterDetect::impl_getStaticSupportedServiceNames()
{
    UNOMUTEXGUARD aGuard( UNOMUTEX::getGlobalMutex() );
    UNOSEQUENCE< UNOOUSTRING > seqServiceNames( 1 );
    seqServiceNames.getArray() [0] = UNOOUSTRING::createFromAscii( "com.sun.star.frame.ExtendedTypeDetection"  );
    return seqServiceNames ;
}

/* Helper for XServiceInfo */
UNOOUSTRING SmFilterDetect::impl_getStaticImplementationName()
{
    return UNOOUSTRING::createFromAscii( "com.sun.star.comp.math.FormatDetector" );
}

/* Helper for registry */
UNOREFERENCE< UNOXINTERFACE > SAL_CALL SmFilterDetect::impl_createInstance( const UNOREFERENCE< UNOXMULTISERVICEFACTORY >& xServiceManager ) throw( UNOEXCEPTION )
{
    return UNOREFERENCE< UNOXINTERFACE >( *new SmFilterDetect( xServiceManager ) );
}

