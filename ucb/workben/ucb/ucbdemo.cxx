/*************************************************************************
 *
 *  $RCSfile: ucbdemo.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: sb $ $Date: 2000-11-16 11:15:29 $
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

#include <stack>

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif
#ifndef _VOS_SOCKET_HXX_
#include <vos/socket.hxx>
#endif
#ifndef _CPPUHELPER_SERVICEFACTORY_HXX_
#include <cppuhelper/servicefactory.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_UCB_CONTENTACTION_HPP_
#include <com/sun/star/ucb/ContentAction.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENCOMMANDARGUMENT2_HPP_
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_CONTENTRESULTSETCAPABILITY_HPP_
#include <com/sun/star/ucb/ContentResultSetCapability.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_SEARCHCOMMANDARGUMENT_HPP_
#include <com/sun/star/ucb/SearchCommandArgument.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_NAMECLASH_HPP_
#include <com/sun/star/ucb/NameClash.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_TRANSFERINFO_HPP_
#include <com/sun/star/ucb/TransferInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTIDENTIFIERFACTORY_HPP_
#include <com/sun/star/ucb/XContentIdentifierFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMANDINFO_HPP_
#include <com/sun/star/ucb/CommandInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDERMANAGER_HPP_
#include <com/sun/star/ucb/XContentProviderManager.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_CHAOSPROGRESSSTART_HPP_
#include <com/sun/star/ucb/CHAOSProgressStart.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENMODE_HPP_
#include <com/sun/star/ucb/OpenMode.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_RESULTSETEXCEPTION_HPP_
#include <com/sun/star/ucb/ResultSetException.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCONTAINER_HPP_
#include <com/sun/star/beans/XPropertyContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XPROGRESSHANDLER_HPP_
#include <com/sun/star/ucb/XProgressHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDENVIRONMENT_HPP_
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTIESCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTIESCHANGENOTIFIER_HPP_
#include <com/sun/star/beans/XPropertiesChangeNotifier.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDPROCESSOR_HPP_
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XDYNAMICRESULTSET_HPP_
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTACCESS_HPP_
#include <com/sun/star/ucb/XContentAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDINFO_HPP_
#include <com/sun/star/ucb/XCommandInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XSORTEDDYNAMICRESULTSETFACTORY_HPP_
#include <com/sun/star/ucb/XSortedDynamicResultSetFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BRIDGE_XUNOURLRESOLVER_HPP_
#include <com/sun/star/bridge/XUnoUrlResolver.hpp>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _UCBHELPER_CONFIGURATIONKEYS_HXX_
#include <ucbhelper/configurationkeys.hxx>
#endif
#ifndef _UCBHELPER_FILEIDENTIFIERCONVERTER_HXX_
#include <ucbhelper/fileidentifierconverter.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_HELP_HXX
#include <vcl/help.hxx>
#endif

#ifndef CHAOS_UCBDEMO_SRCHARG_HXX
#include <srcharg.hxx>
#endif

using namespace vos;
using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::ucb;
using namespace com::sun::star::task;
using namespace com::sun::star::io;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::container;
using namespace com::sun::star::bridge;

/*========================================================================
 *
 * MyOutWindow.
 *
 *======================================================================*/

#define MYOUTWINDOW_MAXLINES 4096

class MyOutWindow : public ListBox
{
public:
    MyOutWindow( Window *pParent, WinBits nWinStyle )
    : ListBox ( pParent, nWinStyle | WB_AUTOHSCROLL ) {}
    ~MyOutWindow() {}

    void Append( const String &rLine );
};

//-------------------------------------------------------------------------
void MyOutWindow::Append( const String &rLine )
{
    String aLine( rLine );

    xub_StrLen nPos = aLine.Search( '\n' );
    while ( nPos != STRING_NOTFOUND )
    {
        if ( GetEntryCount() >= MYOUTWINDOW_MAXLINES )
            RemoveEntry( 0 );

        InsertEntry( aLine.Copy( 0, nPos ) );

        aLine.Erase( 0, nPos + 1 );
        nPos = aLine.Search( '\n' );
    }

    if ( GetEntryCount() >= MYOUTWINDOW_MAXLINES )
        RemoveEntry( 0 );

    InsertEntry( aLine );

    SetTopEntry( MYOUTWINDOW_MAXLINES - 1 );
}

/*========================================================================
 *
 * MessagePrinter.
 *
 *=======================================================================*/

class MessagePrinter
{
protected:
    MyOutWindow* m_pOutEdit;

public:
    MessagePrinter( MyOutWindow* pOutEdit = NULL )
    : m_pOutEdit( pOutEdit ) {}
    void setOutEdit( MyOutWindow* pOutEdit )
    { m_pOutEdit = pOutEdit; }
    void print( const sal_Char* pText );
    void print( const UniString& rText );
};

//-------------------------------------------------------------------------
void MessagePrinter::print( const sal_Char* pText )
{
    print( UniString::CreateFromAscii( pText ) );
}

//-------------------------------------------------------------------------
void MessagePrinter::print( const UniString& rText )
{
    vos::OGuard aGuard( Application::GetSolarMutex() );

    if ( m_pOutEdit )
    {
        m_pOutEdit->Append( rText );
        m_pOutEdit->Update();
    }
}

//============================================================================
//
//  TestOutputStream
//
//============================================================================

class TestOutputStream:
    public cppu::OWeakObject,
    public XOutputStream
{
    OUString m_sStart;
    bool m_bMore;

public:
    TestOutputStream(): m_bMore(false) {}

    virtual com::sun::star::uno::Any SAL_CALL queryInterface(
                                const com::sun::star::uno::Type & rType)
    throw(RuntimeException);
    virtual void SAL_CALL acquire() throw (RuntimeException)
    { OWeakObject::acquire(); }

    virtual void SAL_CALL release() throw (RuntimeException)
    { OWeakObject::release(); }

    virtual void SAL_CALL writeBytes(const Sequence< sal_Int8 > & rData)
        throw(RuntimeException);

    virtual void SAL_CALL flush() throw() {}

    virtual void SAL_CALL closeOutput() throw() {};

    OUString getStart() const;
};

//============================================================================
// virtual
Any SAL_CALL
TestOutputStream::queryInterface(const com::sun::star::uno::Type & rType)
    throw(RuntimeException)
{
    Any aRet = cppu::queryInterface(rType,
                           static_cast< XOutputStream * >(this));
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface(rType);
}

//============================================================================
// virtual
void SAL_CALL TestOutputStream::writeBytes(const Sequence< sal_Int8 > & rData)
    throw(RuntimeException)
{
    sal_Int32 nLen = rData.getLength();
    if (m_sStart.getLength() + nLen > 500)
    {
        nLen = 500 - m_sStart.getLength();
        m_bMore = true;
    }
    m_sStart
        += OUString(reinterpret_cast< const sal_Char * >(rData.
                                                             getConstArray()),
                    nLen, RTL_TEXTENCODING_ISO_8859_1);
}

//============================================================================
OUString TestOutputStream::getStart() const
{
    OUString sResult = m_sStart;
    if (m_bMore)
        sResult += OUString::createFromAscii("...");
    return sResult;
}

/*========================================================================
 *
 * ProgressHandler.
 *
 *=======================================================================*/

class ProgressHandler:
    public cppu::OWeakObject,
    public XProgressHandler
{
    MessagePrinter & m_rPrinter;

    OUString toString(const Any & rStatus);

public:
    ProgressHandler(MessagePrinter & rThePrinter): m_rPrinter(rThePrinter) {}

    virtual com::sun::star::uno::Any SAL_CALL queryInterface(
                                const com::sun::star::uno::Type & rType)
        throw(RuntimeException);

    virtual void SAL_CALL acquire() throw (RuntimeException)
    { OWeakObject::acquire(); }

    virtual void SAL_CALL release() throw (RuntimeException)
    { OWeakObject::release(); }

    virtual void SAL_CALL push(const Any & rStatus) throw (RuntimeException);

    virtual void SAL_CALL update(const Any & rStatus)
        throw (RuntimeException);

    virtual void SAL_CALL pop() throw (RuntimeException);
};

OUString ProgressHandler::toString(const Any & rStatus)
{
    CHAOSProgressStart aStart;
    if (rStatus >>= aStart)
    {
        OUString sResult;
        if (aStart.Text.getLength() > 0)
        {
            sResult = aStart.Text;
            sResult += OUString::createFromAscii(" ");
        }
        sResult += OUString::createFromAscii("[");
        sResult += OUString::valueOf(aStart.Minimum);
        sResult += OUString::createFromAscii("..");
        sResult += OUString::valueOf(aStart.Maximum);
        sResult += OUString::createFromAscii("]");
        return sResult;
    }

    OUString sText;
    if (rStatus >>= sText)
        return sText;

    sal_Int32 nValue;
    if (rStatus >>= nValue)
    {
        OUString sResult = OUString::createFromAscii("..");
        sResult += OUString::valueOf(nValue);
        sResult += OUString::createFromAscii("..");
        return OUString(sResult);
    }

    return OUString::createFromAscii("(Unknown object)");
}

//============================================================================
// virtual
Any SAL_CALL
ProgressHandler::queryInterface( const com::sun::star::uno::Type & rType )
    throw(RuntimeException)
{
    Any aRet = cppu::queryInterface(
                        rType,
                           static_cast< XProgressHandler* >(this));
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

//============================================================================
// virtual
void SAL_CALL ProgressHandler::push(const Any & rStatus)
    throw (RuntimeException)
{
    OUString sMessage = OUString::createFromAscii("Status push: ");
    sMessage += toString(rStatus);
    m_rPrinter.print(sMessage);
}

//============================================================================
// virtual
void SAL_CALL ProgressHandler::update(const Any & rStatus)
    throw (RuntimeException)
{
    OUString sMessage = OUString::createFromAscii("Status update: ");
    sMessage += toString(rStatus);
    m_rPrinter.print(sMessage);
}

//============================================================================
// virtual
void SAL_CALL ProgressHandler::pop() throw (RuntimeException)
{
    m_rPrinter.print("Status pop");
}

/*========================================================================
 *
 * Ucb.
 *
 *=======================================================================*/

#define UCB_MODULE_NAME  "ucb1"

class Ucb : public MessagePrinter
{
private:
    Reference< XMultiServiceFactory >      m_xFac;
    Reference< XContentProvider >          m_xProv;
    Reference< XContentIdentifierFactory > m_xIdFac;
    rtl::OUString m_aConfigurationKey1;
    rtl::OUString m_aConfigurationKey2;
    rtl::OUString m_aRapConnect;
    sal_Bool m_bInited : 1;

    static OUString getUnoURL();

public:
    Ucb( Reference< XMultiServiceFactory >& rxFactory,
         rtl::OUString const & rConfigurationKey1,
         rtl::OUString const & rConfigurationKey2,
         rtl::OUString const & rRapConnect );
    ~Ucb();

    sal_Bool init();

    Reference< XMultiServiceFactory > getServiceFactory() const
    { return m_xFac; }

    XContentIdentifierFactory* getContentIdentifierFactory();
    XContentProvider*          getContentProvider();

    static OUString m_aProtocol;
};

// static
OUString Ucb::m_aProtocol;

//-------------------------------------------------------------------------
// static
OUString Ucb::getUnoURL()
{
    OUString aUnoURL(OUString::createFromAscii(
                         "uno:socket,host=localhost,port=8121;"));
    if (m_aProtocol.getLength() == 0)
        aUnoURL += OUString::createFromAscii("urp");
    else
        aUnoURL += m_aProtocol;
    aUnoURL += OUString::createFromAscii(";UCB.Factory");
    return aUnoURL;
}

//-------------------------------------------------------------------------
Ucb::Ucb( Reference< XMultiServiceFactory >& rxFactory,
          rtl::OUString const & rConfigurationKey1,
          rtl::OUString const & rConfigurationKey2,
          rtl::OUString const & rRapConnect )
: m_xFac( rxFactory ),
  m_aConfigurationKey1( rConfigurationKey1 ),
  m_aConfigurationKey2( rConfigurationKey2 ),
  m_aRapConnect( rRapConnect ),
  m_bInited( sal_False )
{
}

//-------------------------------------------------------------------------
Ucb::~Ucb()
{
}

//-------------------------------------------------------------------------
sal_Bool Ucb::init()
{
    if ( m_bInited )
        return sal_True;

    // Create auto configured UCB:
    if (m_aRapConnect.getLength() == 0)
    {
        if (m_xFac.is())
            try
            {
                rtl::OUString aPipe;
                vos::OSecurity().getUserIdent(aPipe);
                Sequence< Any > aArgs(4);
                aArgs[0] <<= m_aConfigurationKey1;
                aArgs[1] <<= m_aConfigurationKey2;
                aArgs[2] <<= rtl::OUString::createFromAscii("PIPE");
                aArgs[3] <<= aPipe;
                m_xProv
                    = Reference< XContentProvider >(
                          m_xFac->
                              createInstanceWithArguments(
                                  rtl::OUString::createFromAscii(
                                      "com.sun.star.ucb."
                                          "UniversalContentBroker"),
                                  aArgs),
                          UNO_QUERY);
            }
            catch (Exception const &) {}

        if (m_xProv.is())
        {
            print("UCB initialized");
            Reference< XContentProviderManager > xProvMgr(m_xProv, UNO_QUERY);
            if (xProvMgr.is())
            {
                print("Registered schemes:");
                Sequence< ContentProviderInfo >
                    aInfos(xProvMgr->queryContentProviders());
                for (sal_Int32 i = 0; i < aInfos.getLength(); ++i)
                {
                    String aText(RTL_CONSTASCII_USTRINGPARAM("    "));
                    aText += UniString(aInfos[i].Scheme);
                    print(aText);
                }
            }
        }
        else
            print("Error initializing UCB");
    }
    else
    {
        if (m_xFac.is())
            try
            {
                Reference< XUnoUrlResolver >
                    xResolver(
                        m_xFac->
                            createInstance(
                                rtl::OUString::createFromAscii(
                                    "com.sun.star.bridge.UnoUrlResolver")),
                        UNO_QUERY);

                Reference< XMultiServiceFactory > xRemoteFactory;
                if (xResolver.is())
                {
                    rtl::OUStringBuffer aUrl;
                    aUrl.appendAscii(RTL_CONSTASCII_STRINGPARAM("uno:"));
                    aUrl.append(m_aRapConnect);
                    aUrl.appendAscii(RTL_CONSTASCII_STRINGPARAM(
                                         ";urp;UCB.Factory"));
                    xRemoteFactory
                        = Reference< XMultiServiceFactory >(
                              xResolver->resolve(aUrl.makeStringAndClear()),
                              UNO_QUERY);
                }

                if (xRemoteFactory.is())
                    xRemoteFactory
                        = Reference< XMultiServiceFactory >(
                              xRemoteFactory->
                                  createInstance(
                                      rtl::OUString::createFromAscii(
                                          "com.sun.star.lang."
                                              "ServiceManager")),
                              UNO_QUERY);

                if (xRemoteFactory.is())
                    m_xProv
                        = Reference< XContentProvider >(
                              xRemoteFactory->
                                  createInstance(
                                      rtl::OUString::createFromAscii(
                                          "com.sun.star.ucb."
                                              "RemoteAccessContentProvider")),
                              UNO_QUERY);
            }
            catch (Exception const &) {}

        print(m_xProv.is() ? "RAP initialized" : "Error initializing RAP");
    }

    m_bInited = m_xProv.is();
    return m_bInited;
}

//-------------------------------------------------------------------------
XContentIdentifierFactory* Ucb::getContentIdentifierFactory()
{
    if ( !m_xIdFac.is() )
    {
        if ( init() )
            m_xIdFac =
                Reference< XContentIdentifierFactory >( m_xProv, UNO_QUERY );
    }

    return m_xIdFac.get();
}

//-------------------------------------------------------------------------
XContentProvider* Ucb::getContentProvider()
{
    if ( !m_xProv.is() )
        init();

    return m_xProv.get();
}

/*========================================================================
 *
 * UcbTaskEnvironment.
 *
 *=======================================================================*/

class UcbTaskEnvironment : public cppu::OWeakObject,
                           public XCommandEnvironment
{
    Reference< XInteractionHandler > m_xInteractionHandler;
    Reference< XProgressHandler > m_xProgressHandler;

public:
    UcbTaskEnvironment( const Reference< XInteractionHandler>&
                         rxInteractionHandler,
                        const Reference< XProgressHandler>&
                         rxProgressHandler );
    virtual ~UcbTaskEnvironment();

    // Interface implementations...

    // XInterface

    virtual com::sun::star::uno::Any SAL_CALL queryInterface(
                                const com::sun::star::uno::Type & rType )
        throw( RuntimeException );
    virtual void SAL_CALL acquire()
        throw( RuntimeException );
    virtual void SAL_CALL release()
        throw( RuntimeException );

     // XCommandEnvironemnt

    virtual Reference<XInteractionHandler> SAL_CALL getInteractionHandler()
        throw (RuntimeException)
    { return m_xInteractionHandler; }

    virtual Reference<XProgressHandler> SAL_CALL getProgressHandler()
        throw (RuntimeException)
    { return m_xProgressHandler; }

 // !!! Das Interface ist noch nicht vollstaendig !!!
 //  -- application settings -> client language, etc.
 //  -- ???
};

//-------------------------------------------------------------------------
UcbTaskEnvironment::UcbTaskEnvironment(
                    const Reference< XInteractionHandler >&
                     rxInteractionHandler,
                    const Reference< XProgressHandler >&
                     rxProgressHandler )
: m_xInteractionHandler( rxInteractionHandler ),
  m_xProgressHandler( rxProgressHandler )
{
}

//-------------------------------------------------------------------------
// virtual
UcbTaskEnvironment::~UcbTaskEnvironment()
{
}

//----------------------------------------------------------------------------
//
// XInterface methods
//
//----------------------------------------------------------------------------

// virtual
Any SAL_CALL
UcbTaskEnvironment::queryInterface( const com::sun::star::uno::Type & rType )
    throw( RuntimeException )
{
    Any aRet = cppu::queryInterface(
                            rType,
                               static_cast< XCommandEnvironment* >( this ) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

//----------------------------------------------------------------------------
// virtual
void SAL_CALL UcbTaskEnvironment::acquire()
    throw( RuntimeException )
{
    OWeakObject::acquire();
}

//----------------------------------------------------------------------------
// virtual
void SAL_CALL UcbTaskEnvironment::release()
    throw( RuntimeException )
{
    OWeakObject::release();
}

/*========================================================================
 *
 * UcbCommandProcessor.
 *
 *=======================================================================*/

class UcbCommandProcessor : public MessagePrinter
{
private:
    Reference< XCommandProcessor > m_xProcessor;
    sal_Int32                      m_aCommandId;

protected:
    Ucb&                           m_rUCB;

public:
    UcbCommandProcessor( Ucb& rUCB,
                         Reference< XCommandProcessor >& rxProcessor,
                         MyOutWindow* pOutEdit );

    virtual ~UcbCommandProcessor();

    Any executeCommand( const OUString& rName, const Any& rArgument,
                        bool bPrint = true );
};

//-------------------------------------------------------------------------
UcbCommandProcessor::UcbCommandProcessor( Ucb& rUCB,
                                          Reference< XCommandProcessor >&
                                              rxProcessor,
                                          MyOutWindow* pOutEdit)
: MessagePrinter( pOutEdit ),
  m_rUCB( rUCB ),
  m_xProcessor( rxProcessor ),
  m_aCommandId( 0 )
{
    if ( m_xProcessor.is() )
    {
        // Generally, one command identifier per thread is enough. It
        // can be used for all commands executed by the processor which
        // created this id.
        m_aCommandId = m_xProcessor->createCommandIdentifier();
    }
}

//----------------------------------------------------------------------------
// virtual
UcbCommandProcessor::~UcbCommandProcessor()
{
}

//----------------------------------------------------------------------------
Any UcbCommandProcessor::executeCommand( const OUString& rName,
                                         const Any& rArgument,
                                         bool bPrint )
{
    if ( m_xProcessor.is() )
    {
        Command aCommand;
        aCommand.Name     = rName;
        aCommand.Handle   = -1; /* unknown */
        aCommand.Argument = rArgument;

        Reference< XInteractionHandler > xInteractionHandler;
        if (m_rUCB.getServiceFactory().is())
            xInteractionHandler
                = Reference< XInteractionHandler >(
                      m_rUCB.getServiceFactory()->
                          createInstance(
                              OUString::createFromAscii(
                                  "com.sun.star.uui.InteractionHandler")),
                      UNO_QUERY);
        Reference< XProgressHandler >
            xProgressHandler(new ProgressHandler(m_rUCB));
        Reference< XCommandEnvironment > xEnv(
            new UcbTaskEnvironment( xInteractionHandler, xProgressHandler ) );

        if ( bPrint )
        {
            UniString aText( UniString::CreateFromAscii(
                                RTL_CONSTASCII_STRINGPARAM(
                                    "Executing command: " ) ) );
            aText += UniString( rName );
            print( aText );
        }

        // Execute command
        Any aResult;
        bool bException = false;
        bool bAborted = false;
        try
        {
            aResult = m_xProcessor->execute( aCommand, m_aCommandId, xEnv );
        }
        catch ( CommandAbortedException )
        {
            bAborted = true;
        }
        catch ( Exception )
        {
            bException = true;
        }

        if ( bPrint )
        {
            if ( bException )
                print( "execute(...) threw an exception!" );

            if ( bAborted )
                print( "execute(...) aborted!" );

            if ( !bException && !bAborted )
                print( "execute() finished." );
        }

        return aResult;
    }

    print( "executeCommand failed!" );
    return Any();
}

/*========================================================================
 *
 * UcbContent.
 *
 *=======================================================================*/

class UcbContent : public UcbCommandProcessor,
                   public cppu::OWeakObject,
                   public XContentEventListener,
                   public XPropertiesChangeListener
{
    Reference< XContent > m_xContent;

    struct OpenStackEntry
    {
        Reference< XContentIdentifier > m_xIdentifier;
        Reference< XContent > m_xContent;
        sal_uInt32 m_nLevel;
        bool m_bUseIdentifier;

        OpenStackEntry(Reference< XContentIdentifier > const & rTheIdentifier,
                       sal_uInt32 nTheLevel):
            m_xIdentifier(rTheIdentifier), m_nLevel(nTheLevel),
            m_bUseIdentifier(true) {}

        OpenStackEntry(Reference< XContent > const & rTheContent,
                       sal_uInt32 nTheLevel):
            m_xContent(rTheContent), m_nLevel(nTheLevel),
            m_bUseIdentifier(false) {}
    };
    typedef std::stack< OpenStackEntry > OpenStack;

private:
    UcbContent( Ucb& rUCB, Reference< XContent >& rxContent, MyOutWindow* pOutEdit );

protected:
    virtual ~UcbContent();

public:
    static UcbContent* create(
            Ucb& rUCB, const UniString& rURL, MyOutWindow* pOutEdit );
    void dispose();

    const UniString getURL() const;
    const UniString getType() const;

    Sequence< CommandInfo > getCommands();
    Sequence< Property >    getProperties();

    Any  getPropertyValue( const OUString& rName );
    void setPropertyValue( const OUString& rName, const Any& rValue );
    void addProperty     ( const OUString& rName, const Any& rValue );
    void removeProperty  ( const OUString& rName );

    OUString getStringPropertyValue( const OUString& rName );
    void setStringPropertyValue( const OUString& rName, const OUString& rValue );
    void addStringProperty( const OUString& rName, const OUString& rValue );
    void open( const OUString & rName, const UniString& rInput,
               bool bPrint, bool bTiming, bool bSort,
               OpenStack * pStack = 0, sal_uInt32 nLevel = 0,
               sal_Int32 nFetchSize = 0 );
    void openAll( Ucb& rUCB, bool bPrint, bool bTiming, bool bSort,
                  sal_Int32 nFetchSize );
    void transfer( const OUString& rSourceURL, sal_Bool bMove );
    void destroy();

    // XInterface
    virtual com::sun::star::uno::Any SAL_CALL
    queryInterface( const com::sun::star::uno::Type & rType )
        throw( RuntimeException );
    virtual void SAL_CALL
    acquire()
        throw( RuntimeException );
    virtual void SAL_CALL
    release()
        throw( RuntimeException );

    // XEventListener
    // ( base interface of XContentEventListener, XPropertiesChangeListener )
    virtual void SAL_CALL
    disposing( const EventObject& Source )
        throw( RuntimeException );

    // XContentEventListener
    virtual void SAL_CALL
    contentEvent( const ContentEvent& evt )
        throw( RuntimeException );

    // XPropertiesChangeListener
    virtual void SAL_CALL
    propertiesChange( const Sequence< PropertyChangeEvent >& evt )
        throw( RuntimeException );
};

//-------------------------------------------------------------------------
UcbContent::UcbContent( Ucb& rUCB, Reference< XContent >& rxContent, MyOutWindow* pOutEdit)
: UcbCommandProcessor( rUCB,
                       Reference< XCommandProcessor >( rxContent, UNO_QUERY ),
                       pOutEdit ),
  m_xContent( rxContent )
{
}

//----------------------------------------------------------------------------
// virtual
UcbContent::~UcbContent()
{
}

//-------------------------------------------------------------------------
// static
UcbContent* UcbContent::create(
        Ucb& rUCB, const UniString& rURL, MyOutWindow* pOutEdit )
{
    if ( !rURL.Len() )
        return NULL;

    //////////////////////////////////////////////////////////////////////
    // Get XContentIdentifier interface from UCB and let it create an
    // identifer for the given URL.
    //////////////////////////////////////////////////////////////////////

    Reference< XContentIdentifierFactory > xIdFac =
                                        rUCB.getContentIdentifierFactory();
    if ( !xIdFac.is() )
        return NULL;

    Reference< XContentIdentifier > xId =
                            xIdFac->createContentIdentifier( rURL );
    if ( !xId.is() )
        return NULL;

    //////////////////////////////////////////////////////////////////////
    // Get XContentProvider interface from UCB and let it create a
    // content for the given identifier.
    //////////////////////////////////////////////////////////////////////

    Reference< XContentProvider > xProv = rUCB.getContentProvider();
    if ( !xProv.is() )
        return NULL;

    Reference< XContent > xContent;
    try
    {
        xContent = xProv->queryContent( xId );
    }
    catch (IllegalIdentifierException const &) {}
    if ( !xContent.is() )
        return NULL;

    UcbContent* pNew = new UcbContent( rUCB, xContent, pOutEdit );
    pNew->acquire();

    // Register listener(s).
    xContent->addContentEventListener( pNew );

    Reference< XPropertiesChangeNotifier > xNotifier( xContent, UNO_QUERY );
    if ( xNotifier.is() )
    {
        // Empty sequence -> interested in any property changes.
        xNotifier->addPropertiesChangeListener( Sequence< OUString >(), pNew );
    }

    return pNew;
}

//-------------------------------------------------------------------------
const UniString UcbContent::getURL() const
{
    Reference< XContentIdentifier > xId( m_xContent->getIdentifier() );
    if ( xId.is() )
        return UniString( xId->getContentIdentifier() );

    return UniString();
}

//-------------------------------------------------------------------------
const UniString UcbContent::getType() const
{
    const UniString aType( m_xContent->getContentType() );
    return aType;
}

//-------------------------------------------------------------------------
void UcbContent::dispose()
{
    Reference< XComponent > xComponent( m_xContent, UNO_QUERY );
    if ( xComponent.is() )
        xComponent->dispose();
}

//----------------------------------------------------------------------------
void UcbContent::open( const OUString & rName, const UniString& rInput,
                       bool bPrint, bool bTiming, bool bSort,
                       OpenStack * pStack, sal_uInt32 nLevel,
                       sal_Int32 nFetchSize )
{
    Any aArg;

    bool bDoSort = false;

    OpenCommandArgument2 aOpenArg;
    if (rName.compareToAscii("search") == 0)
    {
        SearchCommandArgument aArgument;
        if (!parseSearchArgument(rInput, aArgument.Info))
        {
            print("Can't parse search argument");
            return;
        }
        aArgument.Properties.realloc(5);
        aArgument.Properties[0].Name = OUString::createFromAscii("Title");
        aArgument.Properties[0].Handle = -1;
        aArgument.Properties[1].Name
            = OUString::createFromAscii("DateCreated");
        aArgument.Properties[1].Handle = -1;
        aArgument.Properties[2].Name = OUString::createFromAscii("Size");
        aArgument.Properties[2].Handle = -1;
        aArgument.Properties[3].Name = OUString::createFromAscii("IsFolder");
        aArgument.Properties[3].Handle = -1;
        aArgument.Properties[4].Name
            = OUString::createFromAscii("IsDocument");
        aArgument.Properties[4].Handle = -1;
        aArg <<= aArgument;
    }
    else
    {
        aOpenArg.Mode = OpenMode::ALL;
        aOpenArg.Priority = 32768;
//      if ( bFolder )
        {
            // Property values which shall be in the result set...
            Sequence< Property > aProps( 5 );
            Property* pProps = aProps.getArray();
            pProps[ 0 ].Name   = OUString::createFromAscii( "Title" );
            pProps[ 0 ].Handle = -1; // Important!
/**/        pProps[ 0 ].Type = getCppuType(static_cast< rtl::OUString * >(0));
                // HACK for sorting...
            pProps[ 1 ].Name   = OUString::createFromAscii( "DateCreated" );
            pProps[ 1 ].Handle = -1; // Important!
            pProps[ 2 ].Name   = OUString::createFromAscii( "Size" );
            pProps[ 2 ].Handle = -1; // Important!
            pProps[ 3 ].Name   = OUString::createFromAscii( "IsFolder" );
            pProps[ 3 ].Handle = -1; // Important!
/**/        pProps[ 3 ].Type = getCppuType(static_cast< sal_Bool * >(0));
                // HACK for sorting...
            pProps[ 4 ].Name   = OUString::createFromAscii( "IsDocument" );
            pProps[ 4 ].Handle = -1; // Important!
            aOpenArg.Properties = aProps;

            bDoSort = bSort;
            if (bDoSort)
            {
                // Sort criteria... Note that column numbering starts with 1!
                aOpenArg.SortingInfo.realloc(2);
                // primary sort criterium: column 4 --> IsFolder
                aOpenArg.SortingInfo[ 0 ].ColumnIndex = 4;
                aOpenArg.SortingInfo[ 0 ].Ascending   = sal_False;
                // secondary sort criterium: column 1 --> Title
                aOpenArg.SortingInfo[ 1 ].ColumnIndex = 1;
                aOpenArg.SortingInfo[ 1 ].Ascending   = sal_True;
            }
        }
//      else
            aOpenArg.Sink
                = static_cast< cppu::OWeakObject * >(new TestOutputStream);
        aArg <<= aOpenArg;
    }

//  putenv("PROT_REMOTE_ACTIVATE=1"); // to log remote uno traffic

    ULONG nTime;
    if ( bTiming )
        nTime = Time::GetSystemTicks();

    Any aResult = executeCommand( rName, aArg, bPrint );

    Reference< XDynamicResultSet > xDynamicResultSet;
    if ( ( aResult >>= xDynamicResultSet ) && xDynamicResultSet.is() )
    {
        if (bDoSort)
        {
            sal_Int16 nCaps = xDynamicResultSet->getCapabilities();
            if (!(nCaps & ContentResultSetCapability::SORTED))
            {
                if (bPrint)
                    print("Result set rows are not sorted"
                              "---using sorting cursor");

                Reference< XSortedDynamicResultSetFactory > xSortedFactory;
                if (m_rUCB.getServiceFactory().is())
                    xSortedFactory
                        = Reference< XSortedDynamicResultSetFactory >(
                              m_rUCB.
                                  getServiceFactory()->
                                      createInstance(
                                          OUString::createFromAscii(
                                              "com.sun.star.ucb.SortedDynamic"
                                                  "ResultSetFactory")),
                              UNO_QUERY);
                Reference< XDynamicResultSet > xSorted;
                if (xSortedFactory.is())
                    xSorted
                        = xSortedFactory->
                              createSortedDynamicResultSet(xDynamicResultSet,
                                                           aOpenArg.
                                                               SortingInfo,
                                                           0);
                if (xSorted.is())
                    xDynamicResultSet = xSorted;
                else
                    print("Sorting cursor not available!");
            }
        }

        Reference< XResultSet > xResultSet(
                                    xDynamicResultSet->getStaticResultSet() );
        if ( xResultSet.is() )
        {
            if ( bPrint )
            {
                print( "Folder object opened - iterating:" );
                print( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM(
                    "Content-ID : ContentType : Title : Size : IsFolder "
                    ": IsDocument\n"
                    "-------------------------------------------------" ) ) );
            }

            if (nFetchSize > 0)
            {
                bool bSet = false;
                Reference< XPropertySet > xProperties(xResultSet, UNO_QUERY);
                if (xProperties.is())
                    try
                    {
                        xProperties->
                            setPropertyValue(OUString::createFromAscii(
                                                 "FetchSize"),
                                             makeAny(nFetchSize));
                        bSet = true;
                    }
                    catch (UnknownPropertyException const &) {}
                    catch (PropertyVetoException const &) {}
                    catch (IllegalArgumentException const &) {}
                    catch (WrappedTargetException const &) {}
                if (!bSet)
                    print("Fetch size not set!");
            }

            try
            {
                ULONG n = 0;
                Reference< XContentAccess > xContentAccess(
                                                    xResultSet, UNO_QUERY );
                Reference< XRow > xRow( xResultSet, UNO_QUERY );

                while ( xResultSet->next() )
                {
                    UniString aText;

                    if ( bPrint )
                    {
                        OUString aId( xContentAccess->
                                          queryContentIdentifierString() );
                        aText += UniString::CreateFromInt32( ++n );
                        aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM(
                                               ") " ) );
                        aText += UniString( aId );
                        aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM(
                                               " : " ) );
                    }

                    // Title:
                    UniString aTitle( xRow->getString( 1 ) );
                    if ( bPrint )
                    {
                        if ( aTitle.Len() == 0 && xRow->wasNull() )
                            aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM(
                                                   "<null>" ) );
                        else
                            aText += aTitle;
                        aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM(
                                               " : " ) );
                    }

                    // Size:
                    sal_Int32 nSize = xRow->getInt( 3 );
                    if ( bPrint )
                    {
                        if ( nSize == 0 && xRow->wasNull() )
                            aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM(
                                                   "<null>" ) );
                        else
                            aText += UniString::CreateFromInt32( nSize );
                        aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM(
                                               " : " ) );
                    }

                    // IsFolder:
                    sal_Bool bFolder = xRow->getBoolean( 4 );
                    if ( bPrint )
                    {
                        if ( !bFolder && xRow->wasNull() )
                            aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM(
                                                   "<null>" ) );
                        else
                            aText
                                += bFolder ?
                                       UniString::CreateFromAscii(
                                           RTL_CONSTASCII_STRINGPARAM(
                                               "true" ) ) :
                                       UniString::CreateFromAscii(
                                           RTL_CONSTASCII_STRINGPARAM(
                                               "false" ) );
                        aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM(
                                               " : " ) );
                    }

                    // IsDocument:
                    sal_Bool bDocument = xRow->getBoolean( 5 );
                    if ( bPrint )
                    {
                        if ( !bFolder && xRow->wasNull() )
                            aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM(
                                "<null>" ) );
                        else
                            aText
                                += bDocument ?
                                       UniString::CreateFromAscii(
                                           RTL_CONSTASCII_STRINGPARAM(
                                               "true" ) ) :
                                       UniString::CreateFromAscii(
                                           RTL_CONSTASCII_STRINGPARAM(
                                               "false" ) ); //  IsDocument
                    }

                    if ( bPrint )
                        print( aText );

                    if ( pStack && bFolder )
                        pStack->push( OpenStackEntry(
#if 1
                                          xContentAccess->
                                              queryContentIdentifier(),
#else
                                          xContentAccess->queryContent(),
#endif
                                          nLevel + 1 ) );
                }
            }
            catch ( ResultSetException )
            {
                print( "ResultSetException caught!" );
            }

            if ( bPrint )
                print( "Iteration done." );
        }
    }
Reference< XComponent > xComponent(xDynamicResultSet, UNO_QUERY);
if (xComponent.is())
    xComponent->dispose();

//  putenv("PROT_REMOTE_ACTIVATE="); // to log remote uno traffic

    if ( bTiming )
    {
        nTime = Time::GetSystemTicks() - nTime;
        UniString
            aText( UniString::CreateFromAscii(
                       RTL_CONSTASCII_STRINGPARAM( "Operation took " ) ) );
        aText += UniString::CreateFromInt64( nTime );
        aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " ms." ) );
        print( aText );
    }
}

//----------------------------------------------------------------------------
void UcbContent::openAll( Ucb& rUCB, bool bPrint, bool bTiming, bool bSort,
                          sal_Int32 nFetchSize )
{
    ULONG nTime;
    if ( bTiming )
        nTime = Time::GetSystemTicks();

    OpenStack aStack;
    aStack.push( OpenStackEntry( m_xContent, 0 ) );

    while ( !aStack.empty() )
    {
        OpenStackEntry aEntry( aStack.top() );
        aStack.pop();

        if ( bPrint )
        {
            UniString aText;
            for ( sal_uInt32 i = aEntry.m_nLevel; i != 0; --i )
                aText += '=';
            aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "LEVEL " ) );
            aText += UniString::CreateFromInt64( aEntry.m_nLevel );

            Reference< XContentIdentifier > xID;
            if ( aEntry.m_bUseIdentifier )
                xID = aEntry.m_xIdentifier;
            else if ( aEntry.m_xContent.is() )
                xID = aEntry.m_xContent->getIdentifier();
            if ( xID.is() )
            {
                aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );
                aText += UniString( xID->getContentIdentifier() );
            }

            print( aText );
        }

        Reference< XContent > xChild;
        if ( aEntry.m_bUseIdentifier )
        {
            Reference< XContentProvider > xProv = rUCB.getContentProvider();
            if ( !xProv.is() )
            {
                print( "No content provider" );
                return;
            }

            try
            {
                xChild = xProv->queryContent( aEntry.m_xIdentifier );
            }
            catch (IllegalIdentifierException const &) {}
        }
        else
            xChild = aEntry.m_xContent;
        if ( !xChild.is() )
        {
            print( "No content" );
            return;
        }

        UcbContent( m_rUCB, xChild, m_pOutEdit ).
            open( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM(
                                                  "open" ) ),
                  UniString(), bPrint, false, bSort, &aStack,
                  aEntry.m_nLevel, nFetchSize );
    }

    if ( bTiming )
    {
        nTime = Time::GetSystemTicks() - nTime;
        UniString
            aText( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM(
                                                   "Operation took " ) ) );
        aText += UniString::CreateFromInt64( nTime );
        aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " ms." ) );
        print( aText );
    }
}

//----------------------------------------------------------------------------
void UcbContent::transfer( const OUString& rSourceURL, sal_Bool bMove  )
{
/*
    TransferInfo:

    sal_Bool MoveData;
    ::rtl::OUString SourceURL;
    ::rtl::OUString NewTitle;
    sal_Int32 NameClash;
*/

    if ( bMove )
        print( "Moving content..." );
    else
        print( "Copying content..." );

    Any aArg;
    aArg <<= TransferInfo( bMove, rSourceURL, OUString(), NameClash::ERROR );
    executeCommand( OUString::createFromAscii( "transfer" ), aArg );
}

//----------------------------------------------------------------------------
void UcbContent::destroy()
{
    print( "Deleting content..." );

    Any aArg;
    aArg <<= sal_Bool( sal_True ); // delete physically, not only to trash.
    executeCommand( OUString::createFromAscii( "delete" ), aArg );
}

//-------------------------------------------------------------------------
Sequence< CommandInfo > UcbContent::getCommands()
{
    Any aResult = executeCommand(
                    OUString::createFromAscii( "getCommandInfo" ), Any() );

    Reference< XCommandInfo > xInfo;
    if ( aResult >>= xInfo )
    {
        Sequence< CommandInfo > aCommands( xInfo->getCommands() );
        const CommandInfo* pCommands = aCommands.getConstArray();

        String aText( UniString::CreateFromAscii(
                        RTL_CONSTASCII_STRINGPARAM( "Commands:\n" ) ) );
        sal_uInt32 nCount = aCommands.getLength();
        for ( sal_uInt32 n = 0; n < nCount; ++n )
        {
            aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "    " ) );
            aText += String( pCommands[ n ].Name );
            aText += '\n';
        }
        print( aText );

        return aCommands;
    }

    print( "getCommands failed!" );
    return Sequence< CommandInfo >();
}

//-------------------------------------------------------------------------
Sequence< Property > UcbContent::getProperties()
{
    Any aResult = executeCommand(
                    OUString::createFromAscii( "getPropertySetInfo" ), Any() );

    Reference< XPropertySetInfo > xInfo;
    if ( aResult >>= xInfo )
    {
        Sequence< Property > aProps( xInfo->getProperties() );
        const Property* pProps = aProps.getConstArray();

        String aText( UniString::CreateFromAscii(
                        RTL_CONSTASCII_STRINGPARAM( "Properties:\n" ) ) );
        sal_uInt32 nCount = aProps.getLength();
        for ( sal_uInt32 n = 0; n < nCount; ++n )
        {
            aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "    " ) );
            aText += UniString( pProps[ n ].Name );
            aText += '\n';
        }
        print( aText );

        return aProps;
    }

    print( "getProperties failed!" );
    return Sequence< Property >();
}

//----------------------------------------------------------------------------
Any UcbContent::getPropertyValue( const OUString& rName )
{
    Sequence< Property > aProps( 1 );
    Property& rProp = aProps.getArray()[ 0 ];

    rProp.Name       = rName;
    rProp.Handle     = -1; /* unknown */
//  rProp.Type       = ;
//  rProp.Attributes = ;

    Any aArg;
    aArg <<= aProps;

    Any aResult = executeCommand(
                    OUString::createFromAscii( "getPropertyValues" ), aArg );

    Reference< XRow > xValues;
    if ( aResult >>= xValues )
        return xValues->getObject( 1, Reference< XNameAccess>() );

    print( "getPropertyValue failed!" );
    return Any();
}

//----------------------------------------------------------------------------
OUString UcbContent::getStringPropertyValue( const OUString& rName )
{
    Any aAny = getPropertyValue( rName );
    if ( aAny.getValueType() == getCppuType( (const ::rtl::OUString *)NULL ) )
    {
        const OUString aValue(
                    *SAL_STATIC_CAST( const OUString*, aAny.getValue() ) );

        UniString aText( rName );
        aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " value: '" ) );
        aText += UniString( aValue );
        aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "'" ) );
        print( aText );

        return aValue;
    }

    print( "getStringPropertyValue failed!" );
    return OUString();
}

//----------------------------------------------------------------------------
void UcbContent::setPropertyValue( const OUString& rName, const Any& rValue )
{
    Sequence< PropertyValue > aProps( 1 );
    PropertyValue& rProp = aProps.getArray()[ 0 ];

    rProp.Name       = rName;
    rProp.Handle     = -1; /* unknown */
    rProp.Value      = rValue;
//  rProp.State      = ;

    Any aArg;
    aArg <<= aProps;

    executeCommand( OUString::createFromAscii( "setPropertyValues" ), aArg );
}

//----------------------------------------------------------------------------
void UcbContent::setStringPropertyValue( const OUString& rName,
                                         const OUString& rValue )
{
    Any aAny;
    aAny.setValue( &rValue, getCppuType( (const OUString *)NULL ) );
    setPropertyValue( rName, aAny );

    const OUString aValue(
                    *SAL_STATIC_CAST( const OUString*, aAny.getValue() ) );

    UniString aText( rName );
    aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " value set to: '" ) );
    aText += UniString( aValue );
    aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "'" ) );
    print( aText );
}

//----------------------------------------------------------------------------
void UcbContent::addProperty( const OUString& rName, const Any& rValue )
{
    Reference< XPropertyContainer > xContainer( m_xContent, UNO_QUERY );
    if ( xContainer.is() )
    {
        UniString aText( UniString::CreateFromAscii(
                            RTL_CONSTASCII_STRINGPARAM(
                                "Adding property: " ) ) );
        aText += UniString( rName );
        print( aText );

        try
        {
            xContainer->addProperty( rName, 0, rValue );
        }
        catch ( PropertyExistException& )
        {
            print( "Adding property failed. Already exists!" );
            return;
        }
        catch ( IllegalTypeException& )
        {
            print( "Adding property failed. Illegal Type!" );
            return;
        }
        catch ( IllegalArgumentException& )
        {
            print( "Adding property failed. Illegal Argument!" );
            return;
        }

        print( "Adding property succeeded." );
        return;
    }

    print( "Adding property failed. No XPropertyContainer!" );
}

//----------------------------------------------------------------------------
void UcbContent::addStringProperty(
                        const OUString& rName, const OUString& rValue )
{
    Any aValue;
    aValue <<= rValue;
    addProperty( rName, aValue );
}

//----------------------------------------------------------------------------
void UcbContent::removeProperty( const OUString& rName )
{
    Reference< XPropertyContainer > xContainer( m_xContent, UNO_QUERY );
    if ( xContainer.is() )
    {
        UniString aText( UniString::CreateFromAscii(
                            RTL_CONSTASCII_STRINGPARAM(
                                "Removing property: " ) ) );
        aText += UniString( rName );
        print( aText );

        try
        {
            xContainer->removeProperty( rName );
        }
        catch ( UnknownPropertyException& )
        {
            print( "Adding property failed. Unknown!" );
            return;
        }

        print( "Removing property succeeded." );
        return;
    }

    print( "Removing property failed. No XPropertyContainer!" );
}

//----------------------------------------------------------------------------
//
// XInterface methods
//
//----------------------------------------------------------------------------

// virtual
Any SAL_CALL
UcbContent::queryInterface( const com::sun::star::uno::Type & rType )
    throw(RuntimeException)
{
    Any aRet = cppu::queryInterface(
                rType,
                static_cast< XEventListener* >(
                    static_cast< XContentEventListener* >( this ) ),
                static_cast< XContentEventListener* >( this ),
                static_cast< XPropertiesChangeListener* >( this ) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

//----------------------------------------------------------------------------
// virtual
void SAL_CALL UcbContent::acquire()
    throw( RuntimeException )
{
    OWeakObject::acquire();
}

//----------------------------------------------------------------------------
// virtual
void SAL_CALL UcbContent::release()
    throw( RuntimeException )
{
    OWeakObject::release();
}

//----------------------------------------------------------------------------
//
// XEventListener methods.
//
//----------------------------------------------------------------------------

// virtual
void SAL_CALL UcbContent::disposing( const EventObject& Source )
    throw( RuntimeException )
{
    print ( "Content: disposing..." );
}

//----------------------------------------------------------------------------
//
// XContentEventListener methods,
//
//----------------------------------------------------------------------------

// virtual
void SAL_CALL UcbContent::contentEvent( const ContentEvent& evt )
    throw( RuntimeException )
{
    switch ( evt.Action )
    {
        case  ContentAction::INSERTED:
        {
            UniString aText( UniString::CreateFromAscii(
                                RTL_CONSTASCII_STRINGPARAM(
                                    "contentEvent: INSERTED: " ) ) );
            if ( evt.Content.is() )
            {
                Reference< XContentIdentifier > xId(
                                           evt.Content->getIdentifier() );
                aText += UniString( xId->getContentIdentifier() );
                aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " - " ) );
                aText += UniString( evt.Content->getContentType() );
            }

            print( aText );
            break;
        }
        case  ContentAction::REMOVED:
            print( "contentEvent: REMOVED" );
            break;

        case  ContentAction::DELETED:
            print( "contentEvent: DELETED" );
            break;

        case  ContentAction::EXCHANGED:
            print( "contentEvent: EXCHANGED" );
            break;

        case  ContentAction::SEARCH_MATCHED:
        {
            String aMatch(RTL_CONSTASCII_USTRINGPARAM(
                              "contentEvent: SEARCH MATCHED "));
            if (evt.Id.is())
            {
                aMatch += String(evt.Id->getContentIdentifier());
                if (evt.Content.is())
                {
                    aMatch.AppendAscii(RTL_CONSTASCII_STRINGPARAM(" - "));
                    aMatch += String(evt.Content->getContentType());
                }
            }
            else
                aMatch.AppendAscii(RTL_CONSTASCII_STRINGPARAM("<no id>"));
            print(aMatch);
            break;
        }

        default:
            print( "contentEvent..." );
            break;
    }
}

//----------------------------------------------------------------------------
//
// XPropertiesChangeListener methods.
//
//----------------------------------------------------------------------------

// virtual
void SAL_CALL UcbContent::propertiesChange(
                    const Sequence< PropertyChangeEvent >& evt )
    throw( RuntimeException )
{
    print( "propertiesChange..." );

    sal_uInt32 nCount = evt.getLength();
    if ( nCount )
    {
        const PropertyChangeEvent* pEvents = evt.getConstArray();
        for ( sal_uInt32 n = 0; n < nCount; ++n )
        {
            UniString aText( UniString::CreateFromAscii(
                                RTL_CONSTASCII_STRINGPARAM( "    " ) ) );
            aText += UniString( pEvents[ n ].PropertyName );
            print( aText );
        }
    }
}

/*========================================================================
 *
 * MyWin.
 *
 *=======================================================================*/

#define MYWIN_ITEMID_CLEAR          1
#define MYWIN_ITEMID_CREATE         2
#define MYWIN_ITEMID_RELEASE        3
#define MYWIN_ITEMID_COMMANDS       4
#define MYWIN_ITEMID_PROPS          5
#define MYWIN_ITEMID_ADD_PROP       6
#define MYWIN_ITEMID_REMOVE_PROP    7
#define MYWIN_ITEMID_GET_PROP       8
#define MYWIN_ITEMID_SET_PROP       9
#define MYWIN_ITEMID_OPEN           10
#define MYWIN_ITEMID_OPEN_ALL       11
#define MYWIN_ITEMID_UPDATE         12
#define MYWIN_ITEMID_SYNCHRONIZE    13
#define MYWIN_ITEMID_COPY           14
#define MYWIN_ITEMID_MOVE           15
#define MYWIN_ITEMID_DELETE         16
#define MYWIN_ITEMID_SEARCH         17
#define MYWIN_ITEMID_TIMING         18
#define MYWIN_ITEMID_SORT           19
#define MYWIN_ITEMID_FETCHSIZE      20
#define MYWIN_ITEMID_UNC2URI        21
#define MYWIN_ITEMID_URI2UNC        22
#define MYWIN_ITEMID_OFFLINE        23
#define MYWIN_ITEMID_ONLINE         24

//-------------------------------------------------------------------------
class MyWin : public WorkWindow
{
private:
    ToolBox*            m_pTool;
    Edit*               m_pCmdEdit;
    MyOutWindow*        m_pOutEdit;

    Ucb         m_aUCB;
    UcbContent* m_pContent;

    sal_Int32 m_nFetchSize;
    bool m_bTiming;
    bool m_bSort;

public:
    MyWin( Window *pParent, WinBits nWinStyle,
           Reference< XMultiServiceFactory >& rxFactory,
           rtl::OUString const & rConfigurationKey1,
           rtl::OUString const & rConfigurationKey2,
           rtl::OUString const & rRapConnect );
    virtual ~MyWin();

    void Resize( void );
    DECL_LINK ( ToolBarHandler, ToolBox* );

    void print( const UniString& rText );
    void print( const sal_Char* pText );
};

//-------------------------------------------------------------------------
MyWin::MyWin( Window *pParent, WinBits nWinStyle,
                 Reference< XMultiServiceFactory >& rxFactory,
              rtl::OUString const & rConfigurationKey1,
              rtl::OUString const & rConfigurationKey2,
              rtl::OUString const & rRapConnect )
: WorkWindow( pParent, nWinStyle ),
  m_pTool( NULL ),
  m_pOutEdit( NULL ),
  m_aUCB( rxFactory, rConfigurationKey1, rConfigurationKey2, rRapConnect ),
  m_pContent( NULL ),
  m_nFetchSize( 0 ),
  m_bTiming( false ),
  m_bSort( false )
{
    // ToolBox.
    m_pTool = new ToolBox( this, WB_SVLOOK | WB_BORDER  | WB_SCROLL );

    m_pTool->InsertItem ( MYWIN_ITEMID_CLEAR,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                  "Clear" ) ) );
    m_pTool->SetHelpText( MYWIN_ITEMID_CLEAR,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                  "Clear the Output Window" ) ) );
    m_pTool->InsertSeparator();
    m_pTool->InsertItem ( MYWIN_ITEMID_CREATE,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Create" ) ) );
    m_pTool->SetHelpText( MYWIN_ITEMID_CREATE,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Create a content" ) ) );
    m_pTool->InsertItem ( MYWIN_ITEMID_RELEASE,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Release" ) ) );
    m_pTool->SetHelpText( MYWIN_ITEMID_RELEASE,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Release current content" ) ) );
    m_pTool->InsertSeparator();
    m_pTool->InsertItem ( MYWIN_ITEMID_COMMANDS,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Commands" ) ) );
    m_pTool->SetHelpText( MYWIN_ITEMID_COMMANDS,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                  "Get Commands supported by the content" ) ) );
    m_pTool->InsertItem ( MYWIN_ITEMID_PROPS,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Properties" ) ) );
    m_pTool->SetHelpText( MYWIN_ITEMID_PROPS,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Get Properties supported by the content" ) ) );
    m_pTool->InsertSeparator();
    m_pTool->InsertItem ( MYWIN_ITEMID_ADD_PROP,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "addProperty" ) ) );
    m_pTool->SetHelpText( MYWIN_ITEMID_ADD_PROP,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                  "Add a new string(!) property to the content. "
                                  "Type the property name in the entry field and "
                                  "push this button. The default value for the "
                                  "property will be set to the string 'DefaultValue'" ) ) );
    m_pTool->InsertItem ( MYWIN_ITEMID_REMOVE_PROP,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "removeProperty" ) ) );
    m_pTool->SetHelpText( MYWIN_ITEMID_REMOVE_PROP,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                  "Removes a property from the content. "
                                  "Type the property name in the entry field and "
                                  "push this button." ) ) );
    m_pTool->InsertItem ( MYWIN_ITEMID_GET_PROP,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "getPropertyValue" ) ) );
    m_pTool->SetHelpText( MYWIN_ITEMID_GET_PROP,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                  "Get a string(!) property value from the content. "
                                  "Type the property name in the entry field and "
                                  "push this button to obtain the value" ) ) );
    m_pTool->InsertItem ( MYWIN_ITEMID_SET_PROP,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "setPropertyValue" ) ) );
    m_pTool->SetHelpText( MYWIN_ITEMID_SET_PROP,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                  "Set a string(!) property value of the content."
                                  "Type the property name in the entry field and "
                                  "push this button to set the value to the string "
                                  "'NewValue'" ) ) );
    m_pTool->InsertSeparator();
    m_pTool->InsertItem ( MYWIN_ITEMID_OPEN,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Open" ) ) );
    m_pTool->SetHelpText( MYWIN_ITEMID_OPEN,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Open the content" ) ) );
    m_pTool->InsertItem ( MYWIN_ITEMID_OPEN_ALL,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Open All" ) ) );
    m_pTool->SetHelpText( MYWIN_ITEMID_OPEN_ALL,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Open the content and all of its"
                                    " children" ) ) );
    m_pTool->InsertItem ( MYWIN_ITEMID_UPDATE,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Update" ) ) );
    m_pTool->SetHelpText( MYWIN_ITEMID_UPDATE,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Update the content" ) ) );
    m_pTool->InsertItem ( MYWIN_ITEMID_SYNCHRONIZE,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Synchronize" ) ) );
    m_pTool->SetHelpText( MYWIN_ITEMID_SYNCHRONIZE,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Synchronize the content" ) ) );
    m_pTool->InsertItem ( MYWIN_ITEMID_SEARCH,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Search" ) ) );
    m_pTool->SetHelpText( MYWIN_ITEMID_SEARCH,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Search the content" ) ) );

    m_pTool->InsertSeparator();
    m_pTool->InsertItem ( MYWIN_ITEMID_COPY,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Copy" ) ) );
    m_pTool->SetHelpText( MYWIN_ITEMID_COPY,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Copy a content. Type the URL of the source "
                                "content into the entry field." ) ) );
    m_pTool->InsertItem ( MYWIN_ITEMID_MOVE,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Move" ) ) );
    m_pTool->SetHelpText( MYWIN_ITEMID_MOVE,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Move a content. Type the URL of the source "
                                "content into the entry field." ) ) );
    m_pTool->InsertItem ( MYWIN_ITEMID_DELETE,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Delete" ) ) );
    m_pTool->SetHelpText( MYWIN_ITEMID_DELETE,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Delete the content." ) ) );

    m_pTool->InsertSeparator();
    m_pTool->InsertItem ( MYWIN_ITEMID_TIMING,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Timing" ) ),
                          TIB_CHECKABLE | TIB_AUTOCHECK );
    m_pTool->SetHelpText( MYWIN_ITEMID_TIMING,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Display execution times instead of"
                                    " output" ) ) );
    m_pTool->InsertItem ( MYWIN_ITEMID_SORT,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Sort" ) ),
                          TIB_CHECKABLE | TIB_AUTOCHECK );
    m_pTool->SetHelpText( MYWIN_ITEMID_SORT,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Sort result sets" ) ) );
    m_pTool->InsertItem ( MYWIN_ITEMID_FETCHSIZE,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Fetch Size" ) ) );
    m_pTool->SetHelpText( MYWIN_ITEMID_FETCHSIZE,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Set cached cursor fetch size to positive value" ) ) );

    m_pTool->InsertSeparator();
    m_pTool->InsertItem ( MYWIN_ITEMID_UNC2URI,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "UNC>URI" ) ) );
    m_pTool->SetHelpText( MYWIN_ITEMID_UNC2URI,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Translate 'Normalized File Path' to URI,"
                                    " if possible" ) ) );
    m_pTool->InsertItem ( MYWIN_ITEMID_URI2UNC,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "URI>UNC" ) ) );
    m_pTool->SetHelpText( MYWIN_ITEMID_URI2UNC,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Translate URI to 'Normalized File Path',"
                                    " if possible" ) ) );

    m_pTool->InsertSeparator();
    m_pTool->InsertItem ( MYWIN_ITEMID_OFFLINE,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Offline" ) ) );
    m_pTool->SetHelpText( MYWIN_ITEMID_OFFLINE,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Go offline" ) ) );
    m_pTool->InsertItem ( MYWIN_ITEMID_ONLINE,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Online" ) ) );
    m_pTool->SetHelpText( MYWIN_ITEMID_ONLINE,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Go back online" ) ) );

    m_pTool->SetSelectHdl( LINK( this, MyWin, ToolBarHandler ) );
    m_pTool->Show();

    // Edit.
    m_pCmdEdit = new Edit( this );
    m_pCmdEdit->SetReadOnly( FALSE );
    m_pCmdEdit->SetText( UniString::CreateFromAscii(
                            RTL_CONSTASCII_STRINGPARAM("file:///c|/" ) ) );
    m_pCmdEdit->Show();

    // MyOutWindow.
    m_pOutEdit = new MyOutWindow( this, WB_HSCROLL | WB_VSCROLL | WB_BORDER );
    m_pOutEdit->SetReadOnly( TRUE );
    m_pOutEdit->Show();

    m_aUCB.setOutEdit( m_pOutEdit );
}

//-------------------------------------------------------------------------
// virtual
MyWin::~MyWin()
{
    if ( m_pContent )
    {
        m_pContent->dispose();
        m_pContent->release();
    }

    delete m_pTool;
    delete m_pCmdEdit;
    delete m_pOutEdit;
}

//-------------------------------------------------------------------------
void MyWin::Resize()
{
    Size aWinSize = GetOutputSizePixel();
    int nWinW = aWinSize.Width();
    int nWinH = aWinSize.Height();
    int nBoxH = m_pTool->CalcWindowSizePixel().Height();

    m_pTool->SetPosSizePixel   (
        Point( 0, 0 ), Size ( nWinW, nBoxH ) );
    m_pCmdEdit->SetPosSizePixel(
        Point( 0, nBoxH ), Size( nWinW, nBoxH ) );
    m_pOutEdit->SetPosSizePixel(
        Point( 0, nBoxH + nBoxH ), Size ( nWinW, nWinH - ( nBoxH + nBoxH ) ) );
}

//-------------------------------------------------------------------------
void MyWin::print( const sal_Char* pText )
{
    print( UniString::CreateFromAscii( pText ) );
}

//-------------------------------------------------------------------------
void MyWin::print( const UniString& rText )
{
    vos::OGuard aGuard( Application::GetSolarMutex() );

    if ( m_pOutEdit )
    {
        m_pOutEdit->Append( rText );
        m_pOutEdit->Update();
    }
}

//-------------------------------------------------------------------------
IMPL_LINK( MyWin, ToolBarHandler, ToolBox*, pToolBox )
{
    USHORT nItemId   = pToolBox->GetCurItemId();
    UniString aCmdLine = m_pCmdEdit->GetText();

    ULONG n = Application::ReleaseSolarMutex();

    switch( nItemId )
    {
        case MYWIN_ITEMID_CLEAR:
        {
            vos::OGuard aGuard( Application::GetSolarMutex() );

            m_pOutEdit->Clear();
            m_pOutEdit->Show();
            break;
        }

        case MYWIN_ITEMID_CREATE:
            if ( m_pContent )
            {
                UniString aText( UniString::CreateFromAscii(
                                    RTL_CONSTASCII_STRINGPARAM(
                                        "Content released: " ) ) );
                aText += m_pContent->getURL();

                m_pContent->dispose();
                m_pContent->release();
                m_pContent = NULL;

                print( aText );
            }

            m_pContent = UcbContent::create( m_aUCB, aCmdLine, m_pOutEdit );
            if ( m_pContent )
            {
                String aText( UniString::CreateFromAscii(
                                RTL_CONSTASCII_STRINGPARAM(
                                    "Created content: " ) ) );
                aText += String( m_pContent->getURL() );
                aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " - " ) );
                aText += String( m_pContent->getType() );
                print( aText );
            }
            else
            {
                String aText( UniString::CreateFromAscii(
                                RTL_CONSTASCII_STRINGPARAM(
                                    "Creation failed for content: " ) ) );
                aText += String( aCmdLine );
                print( aText );
            }
            break;

        case MYWIN_ITEMID_RELEASE:
            if ( m_pContent )
            {
                UniString aText( UniString::CreateFromAscii(
                                    RTL_CONSTASCII_STRINGPARAM(
                                        "Content released: " ) ) );
                aText += m_pContent->getURL();

                m_pContent->dispose();
                m_pContent->release();
                m_pContent = NULL;

                print( aText );
            }
            else
                print( "No content!" );

            break;

        case MYWIN_ITEMID_COMMANDS:
            if ( m_pContent )
                m_pContent->getCommands();
            else
                print( "No content!" );

            break;

        case MYWIN_ITEMID_PROPS:
            if ( m_pContent )
                m_pContent->getProperties();
            else
                print( "No content!" );

            break;

        case MYWIN_ITEMID_ADD_PROP:
            if ( m_pContent )
                m_pContent->addStringProperty(
                        aCmdLine,
                        OUString::createFromAscii( "DefaultValue" ) );
            else
                print( "No content!" );

            break;

        case MYWIN_ITEMID_REMOVE_PROP:
            if ( m_pContent )
                m_pContent->removeProperty( aCmdLine );
            else
                print( "No content!" );

            break;

        case MYWIN_ITEMID_GET_PROP:
            if ( m_pContent )
                m_pContent->getStringPropertyValue( aCmdLine );
            else
                print( "No content!" );

            break;

        case MYWIN_ITEMID_SET_PROP:
            if ( m_pContent )
                m_pContent->setStringPropertyValue(
                                aCmdLine,
                                OUString::createFromAscii( "NewValue" ) );
            else
                print( "No content!" );

            break;

        case MYWIN_ITEMID_OPEN:
            if ( m_pContent )
                m_pContent->open(OUString::createFromAscii("open"),
                                 aCmdLine, !m_bTiming, m_bTiming, m_bSort, 0,
                                 0, m_nFetchSize);
            else
                print( "No content!" );

            break;

        case MYWIN_ITEMID_OPEN_ALL:
            if ( m_pContent )
                m_pContent->openAll(m_aUCB, !m_bTiming, m_bTiming, m_bSort,
                                    m_nFetchSize);
            else
                print( "No content!" );

            break;

        case MYWIN_ITEMID_UPDATE:
            if ( m_pContent )
                m_pContent->open(OUString::createFromAscii("update"),
                                 aCmdLine, !m_bTiming, m_bTiming, m_bSort, 0,
                                 0, m_nFetchSize);
            else
                print( "No content!" );

            break;

        case MYWIN_ITEMID_SYNCHRONIZE:
            if ( m_pContent )
                m_pContent->open(OUString::createFromAscii("synchronize"),
                                 aCmdLine, !m_bTiming, m_bTiming, m_bSort, 0,
                                 0, m_nFetchSize);
            else
                print( "No content!" );

            break;

        case MYWIN_ITEMID_SEARCH:
            if ( m_pContent )
                m_pContent->open(OUString::createFromAscii("search"),
                                 aCmdLine, !m_bTiming, m_bTiming, m_bSort, 0,
                                 0, m_nFetchSize);
            else
                print( "No content!" );

            break;

        case MYWIN_ITEMID_COPY:
            if ( m_pContent )
                m_pContent->transfer( aCmdLine, sal_False );
            else
                print( "No content!" );

            break;

        case MYWIN_ITEMID_MOVE:
            if ( m_pContent )
                m_pContent->transfer( aCmdLine, sal_True );
            else
                print( "No content!" );

            break;

        case MYWIN_ITEMID_DELETE:
            if ( m_pContent )
                m_pContent->destroy();
            else
                print( "No content!" );

            break;

        case MYWIN_ITEMID_TIMING:
            m_bTiming = m_pTool->IsItemChecked(MYWIN_ITEMID_TIMING) != false;
            break;

        case MYWIN_ITEMID_SORT:
            m_bSort = m_pTool->IsItemChecked(MYWIN_ITEMID_SORT) != false;
            break;

        case MYWIN_ITEMID_FETCHSIZE:
        {
            m_nFetchSize = aCmdLine.ToInt32();
            String aText;
            if (m_nFetchSize > 0)
            {
                aText.AssignAscii("Fetch size set to ");
                aText += String::CreateFromInt32(m_nFetchSize);
            }
            else
                aText.AssignAscii("Fetch size reset to default");
            print(aText);
            break;
        }

        case MYWIN_ITEMID_UNC2URI:
        case MYWIN_ITEMID_URI2UNC:
            {
                Reference< XContentProviderManager >
                    xManager(m_aUCB.getContentProvider(), UNO_QUERY);
                DBG_ASSERT(
                    xManager.is(),
                    "MyWin::ToolBarHandler(): Service lacks interface");

                OUString aHostName;
                vos::OSocketAddr::getLocalHostname(aHostName);

                String aText(RTL_CONSTASCII_USTRINGPARAM("Hostname: "));
                aText += String(aHostName);
                aText.AppendAscii("\nConversion: ");
                aText += aCmdLine;
                aText.AppendAscii(" to ");
                aText += String(nItemId == MYWIN_ITEMID_UNC2URI ?
                                    ucb::getFileURLFromNormalizedPath(
                                        xManager, aHostName, aCmdLine) :
                                    ucb::getNormalizedPathFromFileURL(
                                        xManager, aHostName, aCmdLine));
                print(aText);
                break;
            }

        case MYWIN_ITEMID_OFFLINE:
        case MYWIN_ITEMID_ONLINE:
        {
            Reference< XContentProviderManager >
                xManager(m_aUCB.getContentProvider(), UNO_QUERY);
            Reference< XCommandProcessor > xProcessor;
            if (xManager.is())
                xProcessor
                    = Reference< XCommandProcessor >(xManager->
                                                         queryContentProvider(
                                                             aCmdLine),
                                                     UNO_QUERY);
            if (!xProcessor.is())
            {
                String aText(RTL_CONSTASCII_USTRINGPARAM(
                                 "No offline support for URL "));
                aText += aCmdLine;
                print(aText);
                break;
            }

            OUString aName;
            Any aArgument;
            if (nItemId == MYWIN_ITEMID_OFFLINE)
            {
                aName = OUString::createFromAscii("goOffline");

                Sequence< Reference< XContentIdentifier > > aIdentifiers(1);
                aIdentifiers[0]
                    = m_aUCB.getContentIdentifierFactory()->
                                 createContentIdentifier(aCmdLine);
                aArgument <<= aIdentifiers;
            }
            else
                aName = OUString::createFromAscii("goOnline");

            UcbCommandProcessor(m_aUCB, xProcessor, m_pOutEdit).
                executeCommand(aName, aArgument);
            break;
        }

        default: // Ignored.
            break;
    }

    Application::AcquireSolarMutex( n );
    return 0;
}

/*========================================================================
 *
 * MyApp.
 *
 *=======================================================================*/
class MyApp : public Application
{
public:
    virtual void Main();
};

MyApp aMyApp;

//-------------------------------------------------------------------------
// virtual
void MyApp::Main()
{
    //////////////////////////////////////////////////////////////////////
    // Read command line params.
    //////////////////////////////////////////////////////////////////////

    rtl::OUString aConfigurationKey1(rtl::OUString::createFromAscii(
                                         UCB_CONFIGURATION_KEY1_LOCAL));
    rtl::OUString aConfigurationKey2(rtl::OUString::createFromAscii(
                                         UCB_CONFIGURATION_KEY2_OFFICE));
    rtl::OUString aRapConnect;

    USHORT nParams = Application::GetCommandLineParamCount();
    for ( USHORT n = 0; n < nParams; ++n )
    {
        String aParam( Application::GetCommandLineParam( n ) );
        if (aParam.CompareIgnoreCaseToAscii("-key=",
                                            RTL_CONSTASCII_LENGTH("-key="))
                == COMPARE_EQUAL)
        {
            xub_StrLen nSlash
                = aParam.Search('/', RTL_CONSTASCII_LENGTH("-key="));
            if (nSlash == STRING_NOTFOUND)
            {
                aConfigurationKey1
                    = aParam.Copy(RTL_CONSTASCII_LENGTH("-key="));
                aConfigurationKey2 = rtl::OUString();
            }
            else
            {
                aConfigurationKey1
                    = aParam.Copy(RTL_CONSTASCII_LENGTH("-key="),
                                  nSlash - RTL_CONSTASCII_LENGTH("-key="));
                aConfigurationKey2
                    = aParam.Copy(nSlash + 1);
            }
        }
        else if (aParam.CompareIgnoreCaseToAscii("-rapconnect=",
                                                 RTL_CONSTASCII_LENGTH(
                                                     "-rapconnect="))
                     == COMPARE_EQUAL)
            aRapConnect = aParam.Copy(RTL_CONSTASCII_LENGTH("-rapconnect="));
    }

    //////////////////////////////////////////////////////////////////////
    // Initialize local Service Manager and basic services.
    //////////////////////////////////////////////////////////////////////

    OStartupInfo aInfo;
    OUString aExeName;
    if ( aInfo.getExecutableFile( aExeName ) != OStartupInfo::E_None )
    {
        DBG_ERROR( "Error getting Executable file name!" );
        return;
    }

    OUString aRegFile( aExeName.copy( 0, aExeName.lastIndexOf( '/' ) + 1 ) );
    aRegFile += OUString::createFromAscii( "applicat.rdb" );

    Reference< XMultiServiceFactory > xFac;
    try
    {
        xFac = cppu::createRegistryServiceFactory( aRegFile, aRegFile );
    }
    catch ( com::sun::star::uno::Exception )
    {
        DBG_ERROR( "Error creating RegistryServiceFactory!" );
        return;
    }

    comphelper::setProcessServiceFactory( xFac );

    Reference< XComponent > xComponent( xFac, UNO_QUERY );

    //////////////////////////////////////////////////////////////////////
    // Create Application Window...
    //////////////////////////////////////////////////////////////////////

    Help::EnableBalloonHelp();

    MyWin *pMyWin = new MyWin( NULL, WB_APP | WB_STDWORK, xFac,
                               aConfigurationKey1, aConfigurationKey2,
                               aRapConnect );

    pMyWin->
        SetText(
            UniString::CreateFromAscii(
                RTL_CONSTASCII_STRINGPARAM(
                    "UCB Demo/Test Application ( Local UCB Client )" ) ) );

    pMyWin->Show();

    //////////////////////////////////////////////////////////////////////
    // Go...
    //////////////////////////////////////////////////////////////////////

    EnterMultiThread( TRUE );
    Execute();
    EnterMultiThread( FALSE );

    //////////////////////////////////////////////////////////////////////
    // Destroy Application Window...
    //////////////////////////////////////////////////////////////////////

    delete pMyWin;

    //////////////////////////////////////////////////////////////////////
    // Cleanup.
    //////////////////////////////////////////////////////////////////////

    // Dispose local service manager.
    if ( xComponent.is() )
        xComponent->dispose();
}

