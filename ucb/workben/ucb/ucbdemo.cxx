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


#include <stack>
#include <rtl/ustrbuf.hxx>
#include <osl/mutex.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/ucb/ContentAction.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/ContentResultSetCapability.hpp>
#include <com/sun/star/ucb/SearchCommandArgument.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/ucb/GlobalTransferCommandArgument.hpp>
#include <com/sun/star/ucb/XContentIdentifierFactory.hpp>
#include <com/sun/star/ucb/CommandInfo.hpp>
#include <com/sun/star/ucb/XContentProviderManager.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/ucb/CHAOSProgressStart.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/ucb/ResultSetException.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#include <com/sun/star/beans/XPropertiesChangeNotifier.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/ucb/XSortedDynamicResultSetFactory.hpp>
#include <com/sun/star/bridge/XUnoUrlResolver.hpp>
#include <comphelper/processfactory.hxx>
#include <ucbhelper/configurationkeys.hxx>
#include <ucbhelper/fileidentifierconverter.hxx>
#include <ucbhelper/contentbroker.hxx>
#include <tools/debug.hxx>

#include "tools/time.hxx"
#include <vcl/wrkwin.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/edit.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/help.hxx>
#include <srcharg.hxx>

using ucbhelper::getLocalFileURL;
using ucbhelper::getSystemPathFromFileURL;
using ucbhelper::getFileURLFromSystemPath;

using namespace com::sun::star;

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
    SolarMutexGuard aGuard;

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
    public io::XOutputStream
{
    rtl::OUString m_sStart;
    bool m_bMore;

public:
    TestOutputStream(): m_bMore(false) {}

    virtual uno::Any SAL_CALL queryInterface(const uno::Type & rType)
    throw(uno::RuntimeException);
    virtual void SAL_CALL acquire() throw ()
    { OWeakObject::acquire(); }

    virtual void SAL_CALL release() throw ()
    { OWeakObject::release(); }

    virtual void SAL_CALL writeBytes(const uno::Sequence< sal_Int8 > & rData)
        throw(uno::RuntimeException);

    virtual void SAL_CALL flush() throw() {}

    virtual void SAL_CALL closeOutput() throw() {};

    rtl::OUString getStart() const;
};

//============================================================================
// virtual
uno::Any SAL_CALL
TestOutputStream::queryInterface(const uno::Type & rType)
    throw(uno::RuntimeException)
{
    uno::Any aRet = cppu::queryInterface(rType,
                        static_cast< io::XOutputStream * >(this));
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface(rType);
}

//============================================================================
// virtual
void SAL_CALL TestOutputStream::writeBytes(
                                    const uno::Sequence< sal_Int8 > & rData)
    throw(uno::RuntimeException)
{
    sal_Int32 nLen = rData.getLength();
    if (m_sStart.getLength() + nLen > 500)
    {
        nLen = 500 - m_sStart.getLength();
        m_bMore = true;
    }
    m_sStart
        += rtl::OUString(reinterpret_cast< const sal_Char * >(rData.
                                                              getConstArray()),
                         nLen, RTL_TEXTENCODING_ISO_8859_1);
}

//============================================================================
rtl::OUString TestOutputStream::getStart() const
{
    rtl::OUString sResult = m_sStart;
    if (m_bMore)
        sResult += rtl::OUString("...");
    return sResult;
}

/*========================================================================
 *
 * ProgressHandler.
 *
 *=======================================================================*/

class ProgressHandler:
    public cppu::OWeakObject,
    public ucb::XProgressHandler
{
    MessagePrinter & m_rPrinter;

    rtl::OUString toString(const uno::Any & rStatus);

public:
    ProgressHandler(MessagePrinter & rThePrinter): m_rPrinter(rThePrinter) {}

    virtual uno::Any SAL_CALL queryInterface(
                                const uno::Type & rType)
        throw(uno::RuntimeException);

    virtual void SAL_CALL acquire() throw ()
    { OWeakObject::acquire(); }

    virtual void SAL_CALL release() throw ()
    { OWeakObject::release(); }

    virtual void SAL_CALL push(const uno::Any & rStatus)
        throw (uno::RuntimeException);

    virtual void SAL_CALL update(const uno::Any & rStatus)
        throw (uno::RuntimeException);

    virtual void SAL_CALL pop() throw (uno::RuntimeException);
};

rtl::OUString ProgressHandler::toString(const uno::Any & rStatus)
{
    ucb::CHAOSProgressStart aStart;
    if (rStatus >>= aStart)
    {
        rtl::OUString sResult;
        if (aStart.Text.getLength() > 0)
        {
            sResult = aStart.Text;
            sResult += rtl::OUString(" ");
        }
        sResult += rtl::OUString("[");
        sResult += rtl::OUString::valueOf(aStart.Minimum);
        sResult += rtl::OUString("..");
        sResult += rtl::OUString::valueOf(aStart.Maximum);
        sResult += rtl::OUString("]");
        return sResult;
    }

    rtl::OUString sText;
    if (rStatus >>= sText)
        return sText;

    sal_Int32 nValue;
    if (rStatus >>= nValue)
    {
        rtl::OUString sResult("..");
        sResult += rtl::OUString::valueOf(nValue);
        sResult += rtl::OUString("..");
        return rtl::OUString(sResult);
    }

    return rtl::OUString("(Unknown object"));
}

//============================================================================
// virtual
uno::Any SAL_CALL
ProgressHandler::queryInterface( const uno::Type & rType )
    throw(uno::RuntimeException)
{
    uno::Any aRet = cppu::queryInterface(
                        rType,
                        static_cast< ucb::XProgressHandler* >(this));
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

//============================================================================
// virtual
void SAL_CALL ProgressHandler::push(const uno::Any & rStatus)
    throw (uno::RuntimeException)
{
    rtl::OUString sMessage("Status push: ");
    sMessage += toString(rStatus);
    m_rPrinter.print(sMessage);
}

//============================================================================
// virtual
void SAL_CALL ProgressHandler::update(const uno::Any & rStatus)
    throw (uno::RuntimeException)
{
    rtl::OUString sMessage("Status update: ");
    sMessage += toString(rStatus);
    m_rPrinter.print(sMessage);
}

//============================================================================
// virtual
void SAL_CALL ProgressHandler::pop() throw (uno::RuntimeException)
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
    uno::Reference< lang::XMultiServiceFactory >      m_xFac;
    uno::Reference< ucb::XContentProvider >          m_xProv;
    uno::Reference< ucb::XContentIdentifierFactory > m_xIdFac;
    rtl::OUString m_aConfigurationKey1;
    rtl::OUString m_aConfigurationKey2;
    sal_Bool m_bInited : 1;

    static rtl::OUString getUnoURL();

public:
    Ucb( uno::Reference< lang::XMultiServiceFactory >& rxFactory,
         rtl::OUString const & rConfigurationKey1,
         rtl::OUString const & rConfigurationKey2 );
    ~Ucb();

    sal_Bool init();

    uno::Reference< lang::XMultiServiceFactory > getServiceFactory() const
    { return m_xFac; }

    uno::Reference< ucb::XContentIdentifierFactory >
    getContentIdentifierFactory();
    uno::Reference< ucb::XContentProvider >
    getContentProvider();

    static rtl::OUString m_aProtocol;
};

// static
rtl::OUString Ucb::m_aProtocol;

//-------------------------------------------------------------------------
// static
rtl::OUString Ucb::getUnoURL()
{
    rtl::OUString aUnoURL(
                         "uno:socket,host=localhost,port=8121;");
    if (m_aProtocol.getLength() == 0)
        aUnoURL += rtl::OUString("urp");
    else
        aUnoURL += m_aProtocol;
    aUnoURL += rtl::OUString(";UCB.Factory");
    return aUnoURL;
}

//-------------------------------------------------------------------------
Ucb::Ucb( uno::Reference< lang::XMultiServiceFactory >& rxFactory,
          rtl::OUString const & rConfigurationKey1,
          rtl::OUString const & rConfigurationKey2 )
: m_xFac( rxFactory ),
  m_aConfigurationKey1( rConfigurationKey1 ),
  m_aConfigurationKey2( rConfigurationKey2 ),
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
    if (m_xFac.is())
        try
        {
            uno::Sequence< uno::Any > aArgs(2);
            aArgs[0] <<= m_aConfigurationKey1;
            aArgs[1] <<= m_aConfigurationKey2;

            ::ucbhelper::ContentBroker::initialize( m_xFac, aArgs );
            m_xProv
                = ::ucbhelper::ContentBroker::get()->getContentProviderInterface();

        }
        catch (uno::Exception const &) {}

    if (m_xProv.is())
    {
        print("UCB initialized");
        uno::Reference< ucb::XContentProviderManager > xProvMgr(
            m_xProv, uno::UNO_QUERY);
        if (xProvMgr.is())
        {
            print("Registered schemes:");
            uno::Sequence< ucb::ContentProviderInfo >
                aInfos(xProvMgr->queryContentProviders());
            for (sal_Int32 i = 0; i < aInfos.getLength(); ++i)
            {
                String aText("    ");
                aText += UniString(aInfos[i].Scheme);
                print(aText);
            }
        }
    }
    else
        print("Error initializing UCB");

    m_bInited = m_xProv.is();
    return m_bInited;
}

//-------------------------------------------------------------------------
uno::Reference< ucb::XContentIdentifierFactory >
Ucb::getContentIdentifierFactory()
{
    if ( !m_xIdFac.is() )
    {
        if ( init() )
            m_xIdFac = uno::Reference< ucb::XContentIdentifierFactory >(
                            m_xProv, uno::UNO_QUERY );
    }

    return m_xIdFac;
}

//-------------------------------------------------------------------------
uno::Reference< ucb::XContentProvider > Ucb::getContentProvider()
{
    if ( !m_xProv.is() )
        init();

    return m_xProv;
}

/*========================================================================
 *
 * UcbTaskEnvironment.
 *
 *=======================================================================*/

class UcbTaskEnvironment : public cppu::OWeakObject,
                           public ucb::XCommandEnvironment
{
    uno::Reference< task::XInteractionHandler > m_xInteractionHandler;
    uno::Reference< ucb::XProgressHandler > m_xProgressHandler;

public:
    UcbTaskEnvironment( const uno::Reference< task::XInteractionHandler>&
                         rxInteractionHandler,
                        const uno::Reference< ucb::XProgressHandler>&
                         rxProgressHandler );
    virtual ~UcbTaskEnvironment();

    // Interface implementations...

    // XInterface

    virtual uno::Any SAL_CALL queryInterface( const uno::Type & rType )
        throw( uno::RuntimeException );
    virtual void SAL_CALL acquire()
        throw();
    virtual void SAL_CALL release()
        throw();

     // XCommandEnvironemnt

    virtual uno::Reference<task::XInteractionHandler> SAL_CALL
    getInteractionHandler()
        throw (uno::RuntimeException)
    { return m_xInteractionHandler; }

    virtual uno::Reference<ucb::XProgressHandler> SAL_CALL
    getProgressHandler()
        throw (uno::RuntimeException)
    { return m_xProgressHandler; }
 };

//-------------------------------------------------------------------------
UcbTaskEnvironment::UcbTaskEnvironment(
                    const uno::Reference< task::XInteractionHandler >&
                     rxInteractionHandler,
                    const uno::Reference< ucb::XProgressHandler >&
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
uno::Any SAL_CALL
UcbTaskEnvironment::queryInterface( const uno::Type & rType )
    throw( uno::RuntimeException )
{
    uno::Any aRet = cppu::queryInterface(
            rType, static_cast< ucb::XCommandEnvironment* >( this ) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

//----------------------------------------------------------------------------
// virtual
void SAL_CALL UcbTaskEnvironment::acquire()
    throw()
{
    OWeakObject::acquire();
}

//----------------------------------------------------------------------------
// virtual
void SAL_CALL UcbTaskEnvironment::release()
    throw()
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
protected:
    Ucb& m_rUCB;

private:
    uno::Reference< ucb::XCommandProcessor > m_xProcessor;
    sal_Int32 m_aCommandId;

public:
    UcbCommandProcessor( Ucb& rUCB,
                         const uno::Reference<
                            ucb::XCommandProcessor >& rxProcessor,
                         MyOutWindow* pOutEdit );

    virtual ~UcbCommandProcessor();

    uno::Any executeCommand( const rtl::OUString& rName,
                             const uno::Any& rArgument,
                             bool bPrint = true );
};

//-------------------------------------------------------------------------
UcbCommandProcessor::UcbCommandProcessor( Ucb& rUCB,
                                          const uno::Reference<
                                            ucb::XCommandProcessor >&
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
uno::Any UcbCommandProcessor::executeCommand( const rtl::OUString& rName,
                                              const uno::Any& rArgument,
                                              bool bPrint )
{
    if ( m_xProcessor.is() )
    {
        ucb::Command aCommand;
        aCommand.Name     = rName;
        aCommand.Handle   = -1; /* unknown */
        aCommand.Argument = rArgument;

        uno::Reference< task::XInteractionHandler > xInteractionHandler;
        if (m_rUCB.getServiceFactory().is())
            xInteractionHandler
                = uno::Reference< task::XInteractionHandler >(
                      task::InteractionHandler::create(m_rUCB.getServiceFactory()),
                      uno::UNO_QUERY_THROW);
        uno::Reference< ucb::XProgressHandler >
            xProgressHandler(new ProgressHandler(m_rUCB));
        uno::Reference< ucb::XCommandEnvironment > xEnv(
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
        uno::Any aResult;
        bool bException = false;
        bool bAborted = false;
        try
        {
            aResult = m_xProcessor->execute( aCommand, m_aCommandId, xEnv );
        }
        catch ( ucb::CommandAbortedException const & )
        {
            bAborted = true;
        }
        catch ( uno::Exception const & )
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
    return uno::Any();
}

/*========================================================================
 *
 * UcbContent.
 *
 *=======================================================================*/

class UcbContent : public UcbCommandProcessor,
                   public cppu::OWeakObject,
                   public ucb::XContentEventListener,
                   public beans::XPropertiesChangeListener
{
    uno::Reference< ucb::XContent > m_xContent;

    struct OpenStackEntry
    {
        uno::Reference< ucb::XContentIdentifier > m_xIdentifier;
        uno::Reference< ucb::XContent > m_xContent;
        sal_uInt32 m_nLevel;
        bool m_bUseIdentifier;

        OpenStackEntry(uno::Reference< ucb::XContentIdentifier > const &
                        rTheIdentifier,
                       sal_uInt32 nTheLevel):
            m_xIdentifier(rTheIdentifier), m_nLevel(nTheLevel),
            m_bUseIdentifier(true) {}

        OpenStackEntry(uno::Reference< ucb::XContent > const & rTheContent,
                       sal_uInt32 nTheLevel):
            m_xContent(rTheContent), m_nLevel(nTheLevel),
            m_bUseIdentifier(false) {}
    };
    typedef std::stack< OpenStackEntry > OpenStack;

private:
    UcbContent( Ucb& rUCB,
                uno::Reference< ucb::XContent >& rxContent,
                MyOutWindow* pOutEdit );

protected:
    virtual ~UcbContent();

public:
    static UcbContent* create(
            Ucb& rUCB, const UniString& rURL, MyOutWindow* pOutEdit );
    void dispose();

    const UniString getURL() const;
    const UniString getType() const;

    uno::Sequence< ucb::CommandInfo > getCommands();
    uno::Sequence< beans::Property >    getProperties();

    uno::Any  getPropertyValue( const rtl::OUString& rName );
    void setPropertyValue( const rtl::OUString& rName, const uno::Any& rValue );
    void addProperty     ( const rtl::OUString& rName, const uno::Any& rValue );
    void removeProperty  ( const rtl::OUString& rName );

    rtl::OUString getStringPropertyValue( const rtl::OUString& rName );
    void setStringPropertyValue( const rtl::OUString& rName,
                                 const rtl::OUString& rValue );
    void addStringProperty( const rtl::OUString& rName,
                            const rtl::OUString& rValue );
    void open( const rtl::OUString & rName, const UniString& rInput,
               bool bPrint, bool bTiming, bool bSort,
               OpenStack * pStack = 0, sal_uInt32 nLevel = 0,
               sal_Int32 nFetchSize = 0 );
    void openAll( Ucb& rUCB, bool bPrint, bool bTiming, bool bSort,
                  sal_Int32 nFetchSize );
    void transfer( const rtl::OUString& rSourceURL, sal_Bool bMove );
    void destroy();

    // XInterface
    virtual uno::Any SAL_CALL queryInterface( const uno::Type & rType )
        throw( uno::RuntimeException );
    virtual void SAL_CALL
    acquire()
        throw();
    virtual void SAL_CALL
    release()
        throw();

    // XEventListener
    // ( base interface of XContentEventListener, XPropertiesChangeListener )
    virtual void SAL_CALL
    disposing( const lang::EventObject& Source )
        throw( uno::RuntimeException );

    // XContentEventListener
    virtual void SAL_CALL
    contentEvent( const ucb::ContentEvent& evt )
        throw( uno::RuntimeException );

    // XPropertiesChangeListener
    virtual void SAL_CALL
    propertiesChange( const uno::Sequence< beans::PropertyChangeEvent >& evt )
        throw( uno::RuntimeException );
};

//-------------------------------------------------------------------------
UcbContent::UcbContent( Ucb& rUCB,
                        uno::Reference< ucb::XContent >& rxContent,
                        MyOutWindow* pOutEdit)
: UcbCommandProcessor( rUCB,
                       uno::Reference< ucb::XCommandProcessor >(
                                                rxContent, uno::UNO_QUERY ),
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

    uno::Reference< ucb::XContentIdentifierFactory > xIdFac =
                                        rUCB.getContentIdentifierFactory();
    if ( !xIdFac.is() )
        return NULL;

    uno::Reference< ucb::XContentIdentifier > xId =
                            xIdFac->createContentIdentifier( rURL );
    if ( !xId.is() )
        return NULL;

    //////////////////////////////////////////////////////////////////////
    // Get XContentProvider interface from UCB and let it create a
    // content for the given identifier.
    //////////////////////////////////////////////////////////////////////

    uno::Reference< ucb::XContentProvider > xProv
        = rUCB.getContentProvider();
    if ( !xProv.is() )
        return NULL;

    uno::Reference< ucb::XContent > xContent;
    try
    {
        xContent = xProv->queryContent( xId );
    }
    catch (ucb::IllegalIdentifierException const &) {}
    if ( !xContent.is() )
        return NULL;

    UcbContent* pNew = new UcbContent( rUCB, xContent, pOutEdit );
    pNew->acquire();

    // Register listener(s).
    xContent->addContentEventListener( pNew );

    uno::Reference< beans::XPropertiesChangeNotifier > xNotifier(
        xContent, uno::UNO_QUERY );
    if ( xNotifier.is() )
    {
        // Empty sequence -> interested in any property changes.
        xNotifier->addPropertiesChangeListener(
            uno::Sequence< rtl::OUString >(), pNew );
    }

    return pNew;
}

//-------------------------------------------------------------------------
const UniString UcbContent::getURL() const
{
    uno::Reference< ucb::XContentIdentifier > xId(
        m_xContent->getIdentifier() );
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
    uno::Reference< lang::XComponent > xComponent( m_xContent, uno::UNO_QUERY );
    if ( xComponent.is() )
        xComponent->dispose();
}

//----------------------------------------------------------------------------
void UcbContent::open( const rtl::OUString & rName, const UniString& rInput,
                       bool bPrint, bool bTiming, bool bSort,
                       OpenStack * pStack, sal_uInt32 nLevel,
                       sal_Int32 nFetchSize )
{
    uno::Any aArg;

    bool bDoSort = false;

    ucb::OpenCommandArgument2 aOpenArg;
    if (rName.compareToAscii("search") == 0)
    {
        ucb::SearchCommandArgument aArgument;
        if (!parseSearchArgument(rInput, aArgument.Info))
        {
            print("Can't parse search argument");
            return;
        }
        aArgument.Properties.realloc(5);
        aArgument.Properties[0].Name = rtl::OUString("Title");
        aArgument.Properties[0].Handle = -1;
        aArgument.Properties[1].Name
            = rtl::OUString("DateCreated");
        aArgument.Properties[1].Handle = -1;
        aArgument.Properties[2].Name = rtl::OUString("Size");
        aArgument.Properties[2].Handle = -1;
        aArgument.Properties[3].Name
            = rtl::OUString("IsFolder");
        aArgument.Properties[3].Handle = -1;
        aArgument.Properties[4].Name
            = rtl::OUString("IsDocument");
        aArgument.Properties[4].Handle = -1;
        aArg <<= aArgument;
    }
    else
    {
        aOpenArg.Mode = ucb::OpenMode::ALL;
        aOpenArg.Priority = 32768;
//      if ( bFolder )
        {
            // Property values which shall be in the result set...
            uno::Sequence< beans::Property > aProps( 5 );
            beans::Property* pProps = aProps.getArray();
            pProps[ 0 ].Name   = rtl::OUString("Title");
            pProps[ 0 ].Handle = -1; // Important!
/**/        pProps[ 0 ].Type = getCppuType(static_cast< rtl::OUString * >(0));
                // HACK for sorting...
            pProps[ 1 ].Name   = rtl::OUString("DateCreated");
            pProps[ 1 ].Handle = -1; // Important!
            pProps[ 2 ].Name   = rtl::OUString("Size");
            pProps[ 2 ].Handle = -1; // Important!
            pProps[ 3 ].Name   = rtl::OUString("IsFolder");
            pProps[ 3 ].Handle = -1; // Important!
/**/        pProps[ 3 ].Type = getCppuType(static_cast< sal_Bool * >(0));
                // HACK for sorting...
            pProps[ 4 ].Name   = rtl::OUString("IsDocument");
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

    ULONG nTime = 0;
    if ( bTiming )
        nTime = Time::GetSystemTicks();

    uno::Any aResult = executeCommand( rName, aArg, bPrint );

    uno::Reference< ucb::XDynamicResultSet > xDynamicResultSet;
    if ( ( aResult >>= xDynamicResultSet ) && xDynamicResultSet.is() )
    {
        if (bDoSort)
        {
            sal_Int16 nCaps = xDynamicResultSet->getCapabilities();
            if (!(nCaps & ucb::ContentResultSetCapability::SORTED))
            {
                if (bPrint)
                    print("Result set rows are not sorted"
                              "---using sorting cursor");

                uno::Reference< ucb::XSortedDynamicResultSetFactory >
                    xSortedFactory;
                if (m_rUCB.getServiceFactory().is())
                    xSortedFactory
                        = uno::Reference<
                            ucb::XSortedDynamicResultSetFactory >(
                              m_rUCB.
                                  getServiceFactory()->
                                      createInstance(
                                          rtl::OUString( "com.sun.star.ucb.SortedDynamic"
                                                  "ResultSetFactory")),
                              uno::UNO_QUERY);
                uno::Reference< ucb::XDynamicResultSet > xSorted;
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

        uno::Reference< sdbc::XResultSet > xResultSet(
                                    xDynamicResultSet->getStaticResultSet() );
        if ( xResultSet.is() )
        {
            if ( bPrint )
            {
                print( "Folder object opened - iterating:" );
                print( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM(
                    "Content-ID : Title : Size : IsFolder : IsDocument\n"
                    "-------------------------------------------------" ) ) );
            }

            if (nFetchSize > 0)
            {
                bool bSet = false;
                uno::Reference< beans::XPropertySet > xProperties(
                    xResultSet, uno::UNO_QUERY);
                if (xProperties.is())
                    try
                    {
                        xProperties->
                            setPropertyValue(rtl::OUString( "FetchSize"),
                                             uno::makeAny(nFetchSize));
                        bSet = true;
                    }
                    catch (beans::UnknownPropertyException const &) {}
                    catch (beans::PropertyVetoException const &) {}
                    catch (lang::IllegalArgumentException const &) {}
                    catch (lang::WrappedTargetException const &) {}
                if (!bSet)
                    print("Fetch size not set!");
            }

            try
            {
                ULONG n = 0;
                uno::Reference< ucb::XContentAccess > xContentAccess(
                                                xResultSet, uno::UNO_QUERY );
                uno::Reference< sdbc::XRow > xRow( xResultSet, uno::UNO_QUERY );

                while ( xResultSet->next() )
                {
                    UniString aText;

                    if ( bPrint )
                    {
                        rtl::OUString aId( xContentAccess->
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
            catch (const ucb::ResultSetException &)
            {
                print( "ResultSetException caught!" );
            }

            if ( bPrint )
                print( "Iteration done." );
        }
    }

    uno::Reference< lang::XComponent > xComponent(
        xDynamicResultSet, uno::UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose();

//  putenv("PROT_REMOTE_ACTIVATE="); // to log remote uno traffic

    if ( bTiming )
    {
        nTime = Time::GetSystemTicks() - nTime;
        UniString
            aText( UniString::CreateFromAscii(
                       RTL_CONSTASCII_STRINGPARAM( "Operation took " ) ) );
        aText += rtl::OUString::valueOf(static_cast<sal_Int64>(nTime));
        aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " ms." ) );
        print( aText );
    }
}

//----------------------------------------------------------------------------
void UcbContent::openAll( Ucb& rUCB, bool bPrint, bool bTiming, bool bSort,
                          sal_Int32 nFetchSize )
{
    ULONG nTime = 0;
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
            aText += rtl::OUString::valueOf(static_cast<sal_Int64>(aEntry.m_nLevel));

            uno::Reference< ucb::XContentIdentifier > xID;
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

        uno::Reference< ucb::XContent > xChild;
        if ( aEntry.m_bUseIdentifier )
        {
            uno::Reference< ucb::XContentProvider > xProv
                = rUCB.getContentProvider();
            if ( !xProv.is() )
            {
                print( "No content provider" );
                return;
            }

            try
            {
                xChild = xProv->queryContent( aEntry.m_xIdentifier );
            }
            catch (ucb::IllegalIdentifierException const &) {}
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
        aText += rtl::OUString::valueOf(static_cast<sal_Int64>(nTime));
        aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " ms." ) );
        print( aText );
    }
}

//----------------------------------------------------------------------------
void UcbContent::transfer( const rtl::OUString& rSourceURL, sal_Bool bMove  )
{
    if ( bMove )
        print( "Moving content..." );
    else
        print( "Copying content..." );

#if 1 /* globalTransfer */

    uno::Reference< ucb::XCommandProcessor > xCommandProcessor(
                                m_rUCB.getContentProvider(), uno::UNO_QUERY );
    if ( xCommandProcessor.is() )
    {
        ucb::GlobalTransferCommandArgument aArg(
                            bMove ? ucb::TransferCommandOperation_MOVE
                                  : ucb::TransferCommandOperation_COPY,
                            rSourceURL,
                            getURL(),
                            rtl::OUString(),
                            //rtl::OUString("NewTitle"),
                            ucb::NameClash::ERROR );

        ucb::Command aTransferCommand( rtl::OUString( "globalTransfer" ),
                                             -1,
                                             uno::makeAny( aArg ) );

        uno::Reference< task::XInteractionHandler > xInteractionHandler;
        if (m_rUCB.getServiceFactory().is())
            xInteractionHandler
                = uno::Reference< task::XInteractionHandler >(
                        task::InteractionHandler::createDefault(comphelper::getComponentContext(m_rUCB.getServiceFactory())),
                        uno::UNO_QUERY_THROW);
        uno::Reference< ucb::XProgressHandler > xProgressHandler(
            new ProgressHandler(m_rUCB));
        uno::Reference< ucb::XCommandEnvironment > xEnv(
            new UcbTaskEnvironment( xInteractionHandler, xProgressHandler ) );

        try
        {
            xCommandProcessor->execute( aTransferCommand, 0, xEnv );
        }
        catch ( uno::Exception const & )
        {
            print( "globalTransfer threw exception!" );
            return;
        }

        print( "globalTransfer finished successfully" );
    }

#else /* transfer */

    uno::Any aArg;
    aArg <<= ucb::TransferInfo(
            bMove, rSourceURL, rtl::OUString(), ucb::NameClash::ERROR );
    executeCommand( rtl::OUString("transfer"), aArg );

//  executeCommand( rtl::OUString("flush"), Any() );

#endif
}

//----------------------------------------------------------------------------
void UcbContent::destroy()
{
    print( "Deleting content..." );

    uno::Any aArg;
    aArg <<= sal_Bool( sal_True ); // delete physically, not only to trash.
    executeCommand( rtl::OUString("delete"), aArg );

//  executeCommand( rtl::OUString("flush"), Any() );
}

//-------------------------------------------------------------------------
uno::Sequence< ucb::CommandInfo > UcbContent::getCommands()
{
    uno::Any aResult = executeCommand(
            rtl::OUString("getCommandInfo"), uno::Any() );

    uno::Reference< ucb::XCommandInfo > xInfo;
    if ( aResult >>= xInfo )
    {
        uno::Sequence< ucb::CommandInfo > aCommands(
            xInfo->getCommands() );
        const ucb::CommandInfo* pCommands = aCommands.getConstArray();

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
    return uno::Sequence< ucb::CommandInfo >();
}

//-------------------------------------------------------------------------
uno::Sequence< beans::Property > UcbContent::getProperties()
{
    uno::Any aResult = executeCommand(
        rtl::OUString("getPropertySetInfo"), uno::Any() );

    uno::Reference< beans::XPropertySetInfo > xInfo;
    if ( aResult >>= xInfo )
    {
        uno::Sequence< beans::Property > aProps( xInfo->getProperties() );
        const beans::Property* pProps = aProps.getConstArray();

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
    return uno::Sequence< beans::Property >();
}

//----------------------------------------------------------------------------
uno::Any UcbContent::getPropertyValue( const rtl::OUString& rName )
{
    uno::Sequence< beans::Property > aProps( 1 );
    beans::Property& rProp = aProps.getArray()[ 0 ];

    rProp.Name       = rName;
    rProp.Handle     = -1; /* unknown */
//  rProp.Type       = ;
//  rProp.Attributes = ;

    uno::Any aArg;
    aArg <<= aProps;

    uno::Any aResult = executeCommand(
        rtl::OUString("getPropertyValues"), aArg );

    uno::Reference< sdbc::XRow > xValues;
    if ( aResult >>= xValues )
        return xValues->getObject(
            1, uno::Reference< container::XNameAccess>() );

    print( "getPropertyValue failed!" );
    return uno::Any();
}

//----------------------------------------------------------------------------
rtl::OUString UcbContent::getStringPropertyValue( const rtl::OUString& rName )
{
    uno::Any aAny = getPropertyValue( rName );
    if ( aAny.getValueType() == getCppuType( (const ::rtl::OUString *)0 ) )
    {
        const rtl::OUString aValue(
            * static_cast< const rtl::OUString * >( aAny.getValue() ) );

        UniString aText( rName );
        aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " value: '" ) );
        aText += UniString( aValue );
        aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "'" ) );
        print( aText );

        return aValue;
    }

    print( "getStringPropertyValue failed!" );
    return rtl::OUString();
}

//----------------------------------------------------------------------------
void UcbContent::setPropertyValue( const rtl::OUString& rName,
                                   const uno::Any& rValue )
{
    uno::Sequence< beans::PropertyValue > aProps( 1 );
    beans::PropertyValue& rProp = aProps.getArray()[ 0 ];

    rProp.Name       = rName;
    rProp.Handle     = -1; /* unknown */
    rProp.Value      = rValue;
//  rProp.State      = ;

    uno::Any aArg;
    aArg <<= aProps;

    executeCommand( rtl::OUString("setPropertyValues"),
                    aArg );

//  executeCommand( rtl::OUString("flush"), Any() );
}

//----------------------------------------------------------------------------
void UcbContent::setStringPropertyValue( const rtl::OUString& rName,
                                         const rtl::OUString& rValue )
{
    uno::Any aAny;
    aAny <<= rValue;
    setPropertyValue( rName, aAny );

    UniString aText( rName );
    aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " value set to: '" ) );
    aText += UniString( rValue );
    aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "'" ) );
    print( aText );
}

//----------------------------------------------------------------------------
void UcbContent::addProperty( const rtl::OUString& rName,
                              const uno::Any& rValue )
{
    uno::Reference< beans::XPropertyContainer > xContainer( m_xContent,
                                                            uno::UNO_QUERY );
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
        catch ( beans::PropertyExistException const & )
        {
            print( "Adding property failed. Already exists!" );
            return;
        }
        catch ( beans::IllegalTypeException const & )
        {
            print( "Adding property failed. Illegal Type!" );
            return;
        }
        catch ( lang::IllegalArgumentException const & )
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
                    const rtl::OUString& rName, const rtl::OUString& rValue )
{
    uno::Any aValue;
    aValue <<= rValue;
    addProperty( rName, aValue );
}

//----------------------------------------------------------------------------
void UcbContent::removeProperty( const rtl::OUString& rName )
{
    uno::Reference< beans::XPropertyContainer > xContainer( m_xContent,
                                                            uno::UNO_QUERY );
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
        catch ( beans::UnknownPropertyException const & )
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
uno::Any SAL_CALL UcbContent::queryInterface( const uno::Type & rType )
    throw(uno::RuntimeException)
{
    uno::Any aRet = cppu::queryInterface(
                rType,
                static_cast< lang::XEventListener* >(
                    static_cast< ucb::XContentEventListener* >( this ) ),
                static_cast< ucb::XContentEventListener* >( this ),
                static_cast< beans::XPropertiesChangeListener* >( this ) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

//----------------------------------------------------------------------------
// virtual
void SAL_CALL UcbContent::acquire()
    throw()
{
    OWeakObject::acquire();
}

//----------------------------------------------------------------------------
// virtual
void SAL_CALL UcbContent::release()
    throw()
{
    OWeakObject::release();
}

//----------------------------------------------------------------------------
//
// XEventListener methods.
//
//----------------------------------------------------------------------------

// virtual
void SAL_CALL UcbContent::disposing( const lang::EventObject& /*Source*/ )
    throw( uno::RuntimeException )
{
    print ( "Content: disposing..." );
}

//----------------------------------------------------------------------------
//
// XContentEventListener methods,
//
//----------------------------------------------------------------------------

// virtual
void SAL_CALL UcbContent::contentEvent( const ucb::ContentEvent& evt )
    throw( uno::RuntimeException )
{
    switch ( evt.Action )
    {
        case ucb::ContentAction::INSERTED:
        {
            UniString aText( UniString::CreateFromAscii(
                                RTL_CONSTASCII_STRINGPARAM(
                                    "contentEvent: INSERTED: " ) ) );
            if ( evt.Content.is() )
            {
                uno::Reference< ucb::XContentIdentifier > xId(
                                           evt.Content->getIdentifier() );
                aText += UniString( xId->getContentIdentifier() );
                aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " - " ) );
                aText += UniString( evt.Content->getContentType() );
            }

            print( aText );
            break;
        }
        case ucb::ContentAction::REMOVED:
            print( "contentEvent: REMOVED" );
            break;

        case ucb::ContentAction::DELETED:
            print( "contentEvent: DELETED" );
            break;

        case ucb::ContentAction::EXCHANGED:
            print( "contentEvent: EXCHANGED" );
            break;

        case ucb::ContentAction::SEARCH_MATCHED:
        {
            String aMatch(
                              "contentEvent: SEARCH MATCHED ");
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
                    const uno::Sequence< beans::PropertyChangeEvent >& evt )
    throw( uno::RuntimeException )
{
    print( "propertiesChange..." );

    sal_uInt32 nCount = evt.getLength();
    if ( nCount )
    {
        const beans::PropertyChangeEvent* pEvents = evt.getConstArray();
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
#define MYWIN_ITEMID_SYS2URI        21
#define MYWIN_ITEMID_URI2SYS        22
#define MYWIN_ITEMID_OFFLINE        23
#define MYWIN_ITEMID_ONLINE         24
#define MYWIN_ITEMID_REORGANIZE     25

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
           uno::Reference< lang::XMultiServiceFactory >& rxFactory,
           rtl::OUString const & rConfigurationKey1,
           rtl::OUString const & rConfigurationKey2 );
    virtual ~MyWin();

    void Resize( void );
    DECL_LINK ( ToolBarHandler, ToolBox* );

    void print( const UniString& rText );
    void print( const sal_Char* pText );
};

//-------------------------------------------------------------------------
MyWin::MyWin( Window *pParent, WinBits nWinStyle,
              uno::Reference< lang::XMultiServiceFactory >& rxFactory,
              rtl::OUString const & rConfigurationKey1,
              rtl::OUString const & rConfigurationKey2 )
: WorkWindow( pParent, nWinStyle ),
  m_pTool( NULL ),
  m_pOutEdit( NULL ),
  m_aUCB( rxFactory, rConfigurationKey1, rConfigurationKey2 ),
  m_pContent( NULL ),
  m_nFetchSize( 0 ),
  m_bTiming( false ),
  m_bSort( false )
{
    // ToolBox.
    m_pTool = new ToolBox( this, WB_3DLOOK | WB_BORDER  | WB_SCROLL );

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

    m_pTool->InsertItem ( MYWIN_ITEMID_REORGANIZE,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Reorganize" ) ) );
    m_pTool->SetHelpText( MYWIN_ITEMID_REORGANIZE,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Reorganize the content storage" ) ) );

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
    m_pTool->InsertItem ( MYWIN_ITEMID_SYS2URI,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "UNC>URI" ) ) );
    m_pTool->SetHelpText( MYWIN_ITEMID_SYS2URI,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Translate 'System File Path' to URI,"
                                    " if possible" ) ) );
    m_pTool->InsertItem ( MYWIN_ITEMID_URI2SYS,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "URI>UNC" ) ) );
    m_pTool->SetHelpText( MYWIN_ITEMID_URI2SYS,
                          UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM(
                                "Translate URI to 'System File Path',"
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
                            RTL_CONSTASCII_STRINGPARAM( "file:///" ) ) );
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
    SolarMutexGuard aGuard;

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
            SolarMutexGuard aGuard;

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
                        rtl::OUString("DefaultValue") );
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
                                rtl::OUString("NewValue") );
            else
                print( "No content!" );

            break;

        case MYWIN_ITEMID_OPEN:
            if ( m_pContent )
                m_pContent->open(rtl::OUString("open"),
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
                m_pContent->open(rtl::OUString("update"),
                                 aCmdLine, !m_bTiming, m_bTiming, m_bSort, 0,
                                 0, m_nFetchSize);
            else
                print( "No content!" );

            break;

        case MYWIN_ITEMID_SYNCHRONIZE:
            if ( m_pContent )
                m_pContent->open(rtl::OUString("synchronize"),
                                 aCmdLine, !m_bTiming, m_bTiming, m_bSort, 0,
                                 0, m_nFetchSize);
            else
                print( "No content!" );

            break;

        case MYWIN_ITEMID_SEARCH:
            if ( m_pContent )
                m_pContent->open(rtl::OUString("search"),
                                 aCmdLine, !m_bTiming, m_bTiming, m_bSort, 0,
                                 0, m_nFetchSize);
            else
                print( "No content!" );

            break;

        case MYWIN_ITEMID_REORGANIZE:
            if ( m_pContent )
                m_pContent->executeCommand (
                    rtl::OUString("reorganizeData"),
                    uno::Any());
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

        case MYWIN_ITEMID_SYS2URI:
        {
            uno::Reference< ucb::XContentProviderManager >
                xManager(m_aUCB.getContentProvider(), uno::UNO_QUERY);
            DBG_ASSERT(xManager.is(),
                       "MyWin::ToolBarHandler(): Service lacks interface");

            rtl::OUString aURL(getLocalFileURL());

            String aText("Local file URL: ");
            aText += String(aURL);
            aText.AppendAscii("\nConversion: ");
            aText += aCmdLine;
            aText.AppendAscii(" to ");
            aText += String(getFileURLFromSystemPath(xManager,
                                                          aURL,
                                                          aCmdLine));
            print(aText);
            break;
        }

        case MYWIN_ITEMID_URI2SYS:
        {
            uno::Reference< ucb::XContentProviderManager >
                xManager(m_aUCB.getContentProvider(), uno::UNO_QUERY);
            DBG_ASSERT(xManager.is(),
                       "MyWin::ToolBarHandler(): Service lacks interface");

            String aText("Conversion: ");
            aText += aCmdLine;
            aText.AppendAscii(" to ");
            aText += String(getSystemPathFromFileURL(xManager,
                                                          aCmdLine));
            print(aText);
            break;
        }

        case MYWIN_ITEMID_OFFLINE:
        case MYWIN_ITEMID_ONLINE:
        {
            uno::Reference< ucb::XContentProviderManager >
                xManager(m_aUCB.getContentProvider(), uno::UNO_QUERY);
            uno::Reference< ucb::XCommandProcessor > xProcessor;
            if (xManager.is())
                xProcessor
                    = uno::Reference< ucb::XCommandProcessor >(
                        xManager->queryContentProvider(aCmdLine),
                        uno::UNO_QUERY);
            if (!xProcessor.is())
            {
                String aText(
                                 "No offline support for URL ");
                aText += aCmdLine;
                print(aText);
                break;
            }

            rtl::OUString aName;
            uno::Any aArgument;
            if (nItemId == MYWIN_ITEMID_OFFLINE)
            {
                aName = rtl::OUString("goOffline");

                uno::Sequence<
                    uno::Reference< ucb::XContentIdentifier > >
                        aIdentifiers(1);
                aIdentifiers[0]
                    = m_aUCB.getContentIdentifierFactory()->
                                 createContentIdentifier(aCmdLine);
                aArgument <<= aIdentifiers;
            }
            else
                aName = rtl::OUString("goOnline");

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

    rtl::OUString aConfigurationKey1( UCB_CONFIGURATION_KEY1_LOCAL);
    rtl::OUString aConfigurationKey2( UCB_CONFIGURATION_KEY2_OFFICE);

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
    }

    //////////////////////////////////////////////////////////////////////
    // Initialize local Service Manager and basic services.
    //////////////////////////////////////////////////////////////////////

    uno::Reference< lang::XMultiServiceFactory > xFac;
    try
    {
        uno::Reference< uno::XComponentContext > xCtx(
            cppu::defaultBootstrap_InitialComponentContext() );
        if ( !xCtx.is() )
        {
            OSL_FAIL( "Error creating initial component context!" );
            return;
        }

        xFac = uno::Reference< lang::XMultiServiceFactory >(
            xCtx->getServiceManager(), uno::UNO_QUERY );

        if ( !xFac.is() )
        {
            OSL_FAIL( "No service manager!" );
            return;
        }
    }
    catch (const uno::Exception &)
    {
        OSL_FAIL( "Exception during creation of initial component context!" );
        return;
    }

    comphelper::setProcessServiceFactory( xFac );

    uno::Reference< lang::XComponent > xComponent( xFac, uno::UNO_QUERY );

    //////////////////////////////////////////////////////////////////////
    // Create Application Window...
    //////////////////////////////////////////////////////////////////////

    Help::EnableBalloonHelp();

    MyWin *pMyWin = new MyWin( NULL, WB_APP | WB_STDWORK, xFac,
                               aConfigurationKey1, aConfigurationKey2 );

    pMyWin->
        SetText(
            UniString::CreateFromAscii(
                RTL_CONSTASCII_STRINGPARAM( "UCB Demo/Test Application" ) ) );

    pMyWin->SetPosSizePixel( 0, 0, 1024, 768 );

    pMyWin->Show();

    //////////////////////////////////////////////////////////////////////
    // Go...
    //////////////////////////////////////////////////////////////////////

    Execute();

    //////////////////////////////////////////////////////////////////////
    // Destroy Application Window...
    //////////////////////////////////////////////////////////////////////

    delete pMyWin;

    //////////////////////////////////////////////////////////////////////
    // Cleanup.
    //////////////////////////////////////////////////////////////////////

    ::ucbhelper::ContentBroker::deinitialize();

    // Dispose local service manager.
    if ( xComponent.is() )
        xComponent->dispose();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
