/*************************************************************************
 *
 *  $RCSfile: filtertracer.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-08-07 15:22:49 $
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

#ifndef _FILTERTRACER_HXX
#define _FILTERTRACER_HXX

#ifndef __RTL_USTRING_
#include <rtl/ustring>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _STACK_HXX
#include <tools/stack.hxx>
#endif

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/RuntimeException.hpp>
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_LOGGING_XLOGGER_HPP_
#include <com/sun/star/util/logging/XLogger.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_LOGGING_LOGLEVEL_HPP_
#include <com/sun/star/util/logging/LogLevel.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XTEXTSEARCH_HPP_
#include <com/sun/star/util/XTextSearch.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_SEARCHRESULT_HPP_
#include <com/sun/star/util/SearchResult.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif

// -----------------------------------------------------------------------------

#define NMSP_IO         com::sun::star::io
#define NMSP_UNO        com::sun::star::uno
#define NMSP_BEANS      com::sun::star::beans
#define NMSP_LANG       com::sun::star::lang
#define NMSP_UTIL       com::sun::star::util
#define NMSP_SAX        com::sun::star::xml::sax
#define NMSP_LOGGING    NMSP_UTIL::logging


#define REF( _def_Obj )         NMSP_UNO::Reference< _def_Obj >
#define SEQ( _def_Obj )         NMSP_UNO::Sequence< _def_Obj >
#define B2UCONST( _def_pChar )  (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(_def_pChar )))

// ----------------
// - FILTERTRACER -
// ----------------
//

/** Some options of the FilterTracer can be initialized
    via XInitialization interface.

    Therefore the first sequence of PropertyValues that
    is given in the argument list is used.

    Following Properties are supported:

    OutputStream    com.sun.star.io.XOutputStream   Defines the output stream. Optional it is possible to provide
                                                    the URL property, then the corresponding output stream will
                                                    be generated automatically.

    URL             string                          Defines the URL, which is used to create an output stream.
                                                    This property is used only, if there is no valid
                                                    OutputStream property available.

    DocumentHandler com.sun.star.xml.sax.XDocumentHandler   The output can also be written to a DocumentHandler,
                                                            then the "characters" method of the handler is used.

    LogLevel        long                            Defines the LogLevel for the FilterTracer.
                                                    Using logp with a LogLevel that is higher as the LogLevel
                                                    for the FilterTracer component will generate no output.
                                                    LogLevel constants are defined in sun::star::util::logging::LogLevel
                                                    The default LogLevel com::sun::star::logging::LogLevel::ALL

    ClassFilter     string                          This property defines a filter for the SourceClass string of logp.
                                                    The ClassFilter string can be separated into multiple tokens using
                                                    a semicolon. If one of the ClassFilter token is part of the
                                                    SourceClass string of the logp method then there will be no output.

    MethodFilter    string                          This property defines a filter for the SourceMethod string of logp.
                                                    The MethodFilter string can be separated into multiple tokens using
                                                    a semicolon. If one of the MethodFilter token is part of the
                                                    SourceMethod string of the logp method then there will be no output.

    MessageFilter   string                          This property defines a filter for the Message string of logp.
                                                    The MessageFilter string can be separated into multiple tokens using
                                                    a semicolon. If one of the MessageFilter token is part of the
                                                    Message string of the logp method then there will be no output.

*/

class FilterTracer : public cppu::WeakImplHelper4
<
    NMSP_LOGGING::XLogger,
    NMSP_LANG::XInitialization,
    NMSP_LANG::XServiceInfo,
    NMSP_UTIL::XTextSearch
>
{
    REF( NMSP_LANG::XMultiServiceFactory )  xFact;

    sal_Int32       mnLogLevel;
    rtl::OUString   msClassFilter;
    rtl::OUString   msMethodFilter;
    rtl::OUString   msMessageFilter;
    rtl::OUString   msURL;

    SvStream*       mpStream;
    REF( NMSP_IO::XOutputStream )       mxOutputStream;
    REF( NMSP_SAX::XDocumentHandler)    mxDocumentHandler;

    REF( NMSP_UTIL::XTextSearch )       mxTextSearch;
    NMSP_UTIL::SearchOptions            maSearchOptions;

    sal_Bool                FilterTracer::ImplFilter( const rtl::OUString& rFilter, const rtl::OUString& rString );

public:
                            FilterTracer( const REF( NMSP_LANG::XMultiServiceFactory )& rxMgr );
    virtual                 ~FilterTracer();

    // XInterface
    virtual void SAL_CALL   acquire() throw();
    virtual void SAL_CALL   release() throw();

    // XInitialization
    virtual void SAL_CALL initialize( const SEQ( NMSP_UNO::Any )& aArguments )
        throw ( NMSP_UNO::Exception, NMSP_UNO::RuntimeException );

    // XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName()
        throw ( NMSP_UNO::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const rtl::OUString& rServiceName )
        throw ( NMSP_UNO::RuntimeException );
    virtual SEQ( rtl::OUString ) SAL_CALL getSupportedServiceNames()
        throw ( NMSP_UNO::RuntimeException );

    // XLogger
    virtual REF( NMSP_LOGGING::XLogger ) SAL_CALL getLogger( const rtl::OUString& rName ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getLevel() throw (::com::sun::star::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getName() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isLoggable( sal_Int32 nLevel ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL logp( sal_Int32 nLevel, const rtl::OUString& rSourceClass,
                    const rtl::OUString& rSourceMethod, const rtl::OUString& rMessage ) throw (::com::sun::star::uno::RuntimeException);

    // XTextSearch
    virtual void SAL_CALL setOptions( const NMSP_UTIL::SearchOptions& ) throw (::com::sun::star::uno::RuntimeException);
    virtual NMSP_UTIL::SearchResult SAL_CALL searchForward( const rtl::OUString& rSearchStr,
        sal_Int32 nStartPos, sal_Int32 nEndPos ) throw (::com::sun::star::uno::RuntimeException);
    virtual NMSP_UTIL::SearchResult SAL_CALL searchBackward( const rtl::OUString& rSearchStr,
        sal_Int32 nStartPos, sal_Int32 nEndPos ) throw (::com::sun::star::uno::RuntimeException);
};

rtl::OUString FilterTracer_getImplementationName()
    throw ( NMSP_UNO::RuntimeException );
sal_Bool SAL_CALL FilterTracer_supportsService( const rtl::OUString& rServiceName )
    throw( NMSP_UNO::RuntimeException );
SEQ( rtl::OUString ) SAL_CALL FilterTracer_getSupportedServiceNames()
    throw( NMSP_UNO::RuntimeException );

#endif
