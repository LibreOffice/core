/*************************************************************************
 *
 *  $RCSfile: mediadescriptor.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: svesik $ $Date: 2004-04-21 11:54:18 $
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
#ifndef _COMPHELPER_MEDIADESCRIPTOR_HXX_
#include <comphelper/mediadescriptor.hxx>
#endif

//_______________________________________________
// includes

#ifndef __COM_SUN_STAR_UCB_XCONTENT_HPP__
#include <com/sun/star/ucb/XContent.hpp>
#endif

#ifndef __COM_SUN_STAR_UCB_XCOMMANDENVIRONMENT_HPP__
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#endif

#ifndef __COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP__
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif

#ifndef __COM_SUN_STAR_IO_XSTREAM_HPP__
#include <com/sun/star/io/XStream.hpp>
#endif

#ifndef __COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP__
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef __COM_SUN_STAR_UCB_INTERACTIVEIOEXCEPTION_HPP__
#include <com/sun/star/ucb/InteractiveIOException.hpp>
#endif

#ifndef __COM_SUN_STAR_UCB_UNSUPPORTEDDATASINKEXCEPTION_HPP__
#include <com/sun/star/ucb/UnsupportedDataSinkException.hpp>
#endif

#ifndef __COM_SUN_STAR_UCB_COMMANDFAILEDEXCEPTION_HPP__
#include <com/sun/star/ucb/CommandFailedException.hpp>
#endif

#ifndef __COM_SUN_STAR_TASK_XINTERACTIONABORT_HPP__
#include <com/sun/star/task/XInteractionAbort.hpp>
#endif

#ifndef _UCBHELPER_INTERCEPTEDINTERACTION_HXX_
#include <ucbhelper/interceptedinteraction.hxx>
#endif

#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif

#ifndef _UCBHELPER_COMMANDENVIRONMENT_HXX
#include <ucbhelper/commandenvironment.hxx>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#if OSL_DEBUG_LEVEL>0
    #ifndef _RTL_USTRBUF_HXX_
    #include <rtl/ustrbuf.hxx>
    #endif
#endif

//_______________________________________________
// namespace

namespace comphelper{

namespace css = ::com::sun::star;

//_______________________________________________
// definitions

/*-----------------------------------------------
    10.03.2004 07:35
-----------------------------------------------*/
const ::rtl::OUString& MediaDescriptor::PROP_ASTEMPLATE()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("AsTemplate"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_CHARACTERSET()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("CharacterSet"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_DEEPDETECTION()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("DeepDetection"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_DETECTSERVICE()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("DetectService"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_DOCUMENTSERVICE()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("DocumentService"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_EXTENSION()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("Extension"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_FILENAME()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("FileName"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_FILTERNAME()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("FilterName"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_FILTEROPTIONS()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("FilterOptions"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_FORMAT()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("Format"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_FRAMENAME()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("FrameName"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_HIDDEN()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("Hidden"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_INPUTSTREAM()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("InputStream"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_INTERACTIONHANDLER()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("InteractionHandler"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_JUMPMARK()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("JumpMark"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_MACROEXECUTIONMODE()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("MacroExecutionMode"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_MEDIATYPE()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("MediaType"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_MINIMIZED()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("Minimized"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_OPENNEWVIEW()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("OpenNewView"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_OUTPUTSTREAM()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("OutputStream"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_PATTERN()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("Pattern"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_POSSIZE()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("PosSize"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_POSTDATA()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("PostData"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_POSTSTRING()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("PostString"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_PREVIEW()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("Preview"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_READONLY()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("ReadOnly"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_REFERRER()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("Referer"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_SILENT()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("Silent"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_STATUSINDICATOR()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("StatusIndicator"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_STREAM()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("Stream"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_TEMPLATENAME()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("TemplateName"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_TEMPLATEREGIONNAME()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("TemplateRegionName"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_TYPENAME()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("TypeName"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_UCBCONTENT()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("UCBContent"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_UPDATEDOCMODE()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("UpdateDocMode"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_URL()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("URL"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_VERSION()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("Version"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_VIEWID()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("ViewId"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_REPAIRPACKAGE()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("RepairPackage"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_DOCUMENTTITLE()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("DocumentTitle"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_MODEL()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("Model"));
    return sProp;
}

/*-----------------------------------------------
    10.03.2004 08:09
-----------------------------------------------*/
MediaDescriptor::MediaDescriptor()
    : SequenceAsHashMap()
{
}

/*-----------------------------------------------
    10.03.2004 08:09
-----------------------------------------------*/
MediaDescriptor::MediaDescriptor(const css::uno::Any& aSource)
    : SequenceAsHashMap(aSource)
{
}

/*-----------------------------------------------
    10.03.2004 08:09
-----------------------------------------------*/
MediaDescriptor::MediaDescriptor(const css::uno::Sequence< css::beans::PropertyValue >& lSource)
    : SequenceAsHashMap(lSource)
{
}

/*-----------------------------------------------
    10.03.2004 08:09
-----------------------------------------------*/
MediaDescriptor::MediaDescriptor(const css::uno::Sequence< css::beans::NamedValue >& lSource)
    : SequenceAsHashMap(lSource)
{
}

/*-----------------------------------------------
    10.03.2004 09:02
-----------------------------------------------*/
sal_Bool MediaDescriptor::addInputStream()
{
    // check for an already existing stream item first
    const_iterator pIt = find(MediaDescriptor::PROP_INPUTSTREAM());
    if (pIt != end())
        return sal_True;

    try
    {
        // No stream available - create a new one
        // a) data cames as PostData ...
        pIt = find(MediaDescriptor::PROP_POSTDATA());
        if (pIt != end())
        {
            const css::uno::Any& rPostData = pIt->second;
            css::uno::Reference< css::io::XInputStream > xPostData;
            rPostData >>= xPostData;
            if (!xPostData.is())
                throw css::uno::Exception(
                        ::rtl::OUString::createFromAscii("Found invalid PostData."),
                        css::uno::Reference< css::uno::XInterface >());

            return impl_openStreamWithPostData(xPostData);
        }

        // b) ... or we must get it from the given URL
        ::rtl::OUString sURL = getUnpackedValueOrDefault(MediaDescriptor::PROP_URL(), ::rtl::OUString());
        if (!sURL.getLength())
            throw css::uno::Exception(
                    ::rtl::OUString::createFromAscii("Found no URL."),
                    css::uno::Reference< css::uno::XInterface >());

        return impl_openStreamWithURL(sURL);
    }
#if OSL_DEBUG_LEVEL>0
    catch(const css::uno::Exception& ex)
    {
        ::rtl::OUStringBuffer sMsg(256);
        sMsg.appendAscii("Invalid MediaDescriptor detected:\n");
        sMsg.append     (ex.Message                           );
        OSL_ENSURE(sal_False, ::rtl::OUStringToOString(sMsg.makeStringAndClear(), RTL_TEXTENCODING_UTF8).getStr());
    }
#else
    catch(const css::uno::Exception&)
        {}
#endif

    return sal_False;
}

/*-----------------------------------------------
    25.03.2004 12:38
-----------------------------------------------*/
sal_Bool MediaDescriptor::impl_openStreamWithPostData(const css::uno::Reference< css::io::XInputStream >& xPostData)
    throw(::com::sun::star::uno::RuntimeException)
{
    // PostData cant be used in read/write mode!
    (*this)[MediaDescriptor::PROP_READONLY()] <<= sal_True;

    ::rtl::OUString sMediaType = getUnpackedValueOrDefault(MediaDescriptor::PROP_MEDIATYPE(), ::rtl::OUString());
    if (!sMediaType.getLength())
    {
        sMediaType = ::rtl::OUString::createFromAscii("application/x-www-form-urlencoded");
        (*this)[MediaDescriptor::PROP_MEDIATYPE()] <<= sMediaType;
    }

    css::uno::Reference< css::io::XInputStream > xInputStream = getUnpackedValueOrDefault(MediaDescriptor::PROP_POSTDATA(), css::uno::Reference< css::io::XInputStream >());
    if (xInputStream.is())
    {
        (*this)[MediaDescriptor::PROP_INPUTSTREAM()] <<= xInputStream;
        return sal_True;
    }

    return sal_False;
}

/*-----------------------------------------------*/
class StillReadWriteInteraction : public ::ucbhelper::InterceptedInteraction
{
    private:
        static const sal_Int32 HANDLE_INTERACTIVEIOEXCEPTION       = 0;
        static const sal_Int32 HANDLE_UNSUPPORTEDDATASINKEXCEPTION = 1;

        sal_Bool m_bUsed;
        sal_Bool m_bHandledByMySelf;
        sal_Bool m_bHandledByInternalHandler;

    public:
        StillReadWriteInteraction(const css::uno::Reference< css::task::XInteractionHandler >& xHandler)
            : m_bUsed                    (sal_False)
            , m_bHandledByMySelf         (sal_False)
            , m_bHandledByInternalHandler(sal_False)
        {
            ::std::vector< ::ucbhelper::InterceptedInteraction::InterceptedRequest > lInterceptions;
            ::ucbhelper::InterceptedInteraction::InterceptedRequest                  aInterceptedRequest;

            aInterceptedRequest.Handle               = HANDLE_INTERACTIVEIOEXCEPTION;
            aInterceptedRequest.Request            <<= css::ucb::InteractiveIOException();
            aInterceptedRequest.Continuation         = ::getCppuType(static_cast< css::uno::Reference< css::task::XInteractionAbort >* >(0));
            aInterceptedRequest.MatchExact           = sal_False;
            lInterceptions.push_back(aInterceptedRequest);

            aInterceptedRequest.Handle               = HANDLE_UNSUPPORTEDDATASINKEXCEPTION;
            aInterceptedRequest.Request            <<= css::ucb::UnsupportedDataSinkException();
            aInterceptedRequest.Continuation         = ::getCppuType(static_cast< css::uno::Reference< css::task::XInteractionAbort >* >(0));
            aInterceptedRequest.MatchExact           = sal_False;
            lInterceptions.push_back(aInterceptedRequest);

            setInterceptedHandler(xHandler);
            setInterceptions(lInterceptions);
        }

        void resetInterceptions()
        {
            setInterceptions(::std::vector< ::ucbhelper::InterceptedInteraction::InterceptedRequest >());
        }

        void resetErrorStates()
        {
            m_bUsed                     = sal_False;
            m_bHandledByMySelf          = sal_False;
            m_bHandledByInternalHandler = sal_False;
        }

        sal_Bool wasWriteError()
        {
            return (m_bUsed && m_bHandledByMySelf);
        }

    private:
        virtual ucbhelper::InterceptedInteraction::EInterceptionState intercepted(const ::ucbhelper::InterceptedInteraction::InterceptedRequest&                         aRequest,
                                                                                  const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionRequest >& xRequest)
        {
            // we are used!
            m_bUsed = sal_True;

            // check if its a real interception - might some parameters are not the right ones ...
            sal_Bool bAbort = sal_False;
            switch(aRequest.Handle)
            {
                case HANDLE_INTERACTIVEIOEXCEPTION:
                {
                    css::ucb::InteractiveIOException exIO;
                    xRequest->getRequest() >>= exIO;
                    bAbort = (
                                (exIO.Code == css::ucb::IOErrorCode_ACCESS_DENIED     ) ||
                                (exIO.Code == css::ucb::IOErrorCode_LOCKING_VIOLATION )
                            );
                }
                break;

                case HANDLE_UNSUPPORTEDDATASINKEXCEPTION:
                {
                    bAbort = sal_True;
                }
                break;
            }

            // handle interaction by ourself
            if (bAbort)
            {
                m_bHandledByMySelf = sal_True;
                css::uno::Reference< css::task::XInteractionContinuation > xAbort = ::ucbhelper::InterceptedInteraction::extractContinuation(
                    xRequest->getContinuations(),
                    ::getCppuType(static_cast< css::uno::Reference< css::task::XInteractionAbort >* >(0)));
                if (!xAbort.is())
                    return ::ucbhelper::InterceptedInteraction::E_NO_CONTINUATION_FOUND;
                xAbort->select();
                return ::ucbhelper::InterceptedInteraction::E_INTERCEPTED;
            }

            // Otherwhise use internal handler.
            if (m_xInterceptedHandler.is())
            {
                m_bHandledByInternalHandler = sal_True;
                m_xInterceptedHandler->handle(xRequest);
            }
            return ::ucbhelper::InterceptedInteraction::E_INTERCEPTED;
        }
};

/*-----------------------------------------------
    25.03.2004 12:29
-----------------------------------------------*/
sal_Bool MediaDescriptor::impl_openStreamWithURL(const ::rtl::OUString& sURL)
    throw(::com::sun::star::uno::RuntimeException)
{
    // prepare the environment
    css::uno::Reference< css::task::XInteractionHandler > xOrgInteraction = getUnpackedValueOrDefault(
        MediaDescriptor::PROP_INTERACTIONHANDLER(),
        css::uno::Reference< css::task::XInteractionHandler >());

    StillReadWriteInteraction* pInteraction = new StillReadWriteInteraction(xOrgInteraction);
    css::uno::Reference< css::task::XInteractionHandler > xInteraction(static_cast< css::task::XInteractionHandler* >(pInteraction), css::uno::UNO_QUERY);

    css::uno::Reference< css::ucb::XProgressHandler > xProgress;
    ::ucb::CommandEnvironment* pCommandEnv = new ::ucb::CommandEnvironment(xInteraction, xProgress);
    css::uno::Reference< css::ucb::XCommandEnvironment > xCommandEnv(static_cast< css::ucb::XCommandEnvironment* >(pCommandEnv), css::uno::UNO_QUERY);

    // try to create the content
    // no content -> no stream => return immediatly with FALSE
    ::ucb::Content                            aContent;
    css::uno::Reference< css::ucb::XContent > xContent;
    try
    {
        aContent = ::ucb::Content(sURL, xCommandEnv);
        xContent = aContent.get();
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::ucb::ContentCreationException&)
        { return sal_False; } // TODO error handling
    catch(const css::uno::Exception&)
        { return sal_False; } // TODO error handling

    // try to open the file in read/write mode
    // (if its allowed to do so).
    // But handle errors in a "hidden mode". Because
    // we try it readonly later - if read/write isnt an option.
    css::uno::Reference< css::io::XStream >      xStream     ;
    css::uno::Reference< css::io::XInputStream > xInputStream;
    sal_Bool bReadOnly = getUnpackedValueOrDefault(MediaDescriptor::PROP_READONLY(), sal_False);
    if (!bReadOnly)
    {
        try
        {
            // TODO: use "special" still interaction to supress error messages
            xStream = aContent.openWriteableStream();
            if (xStream.is())
                xInputStream = xStream->getInputStream();
        }
        catch(const css::uno::RuntimeException&)
            { throw; }
        catch(const css::uno::Exception&)
            {
                // ignore exception, if reason was problem reasoned on
                // open it in WRITEABLE mode! Then we try it READONLY
                // later a second time.
                // All other errors must be handled as real error an
                // break this method.
                if (!pInteraction->wasWriteError())
                    return sal_False;
                xStream.clear();
                xInputStream.clear();
            }
    }

    // If opening of the stream in read/write mode wasnt allowed
    // or failed by an error - we must try it in readonly mode.
    if (!xInputStream.is())
    {
        bReadOnly = sal_True;
        (*this)[MediaDescriptor::PROP_READONLY()] <<= bReadOnly;
        pInteraction->resetInterceptions();
        pInteraction->resetErrorStates();
        try
        {
            xInputStream = aContent.openStream();
        }
        catch(const css::uno::RuntimeException&)
            { throw; }
        catch(const css::uno::Exception&)
            { return sal_False; }
    }

    // add streams to the descriptor
    if (xContent.is())
        (*this)[MediaDescriptor::PROP_UCBCONTENT()] <<= xContent;
    if (xStream.is())
        (*this)[MediaDescriptor::PROP_STREAM()] <<= xStream;
    if (xInputStream.is())
        (*this)[MediaDescriptor::PROP_INPUTSTREAM()] <<= xInputStream;

    // At least we need an input stream. The r/w stream is optional ...
    return xInputStream.is();
}

} // namespace comphelper
