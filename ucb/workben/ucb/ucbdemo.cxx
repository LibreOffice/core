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
    MyOutWindow( vcl::Window *pParent, WinBits nWinStyle )
    : ListBox ( pParent, nWinStyle | WB_AUTOHSCROLL ) {}
    ~MyOutWindow() {}

    void Append( const String &rLine );
};


void MyOutWindow::Append( const String &rLine )
{
    OUString aLine( rLine );

    sal_Int32 nPos = aLine.indexOf( '\n' );
    while ( nPos != -1 )
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
    void print( const OUString& rText );
};


void MessagePrinter::print( const sal_Char* pText )
{
    print( OUString::createFromAscii(pText) );
}


void MessagePrinter::print( const OUString& rText )
{
    SolarMutexGuard aGuard;

    if ( m_pOutEdit )
    {
        m_pOutEdit->Append( rText );
        m_pOutEdit->Update();
    }
}



//  TestOutputStream



class TestOutputStream:
    public cppu::OWeakObject,
    public io::XOutputStream
{
    OUString m_sStart;
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

    OUString getStart() const;
};


// virtual
uno::Any SAL_CALL
TestOutputStream::queryInterface(const uno::Type & rType)
    throw(uno::RuntimeException)
{
    uno::Any aRet = cppu::queryInterface(rType,
                        static_cast< io::XOutputStream * >(this));
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface(rType);
}


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
        += OUString(reinterpret_cast< const sal_Char * >(rData.getConstArray()),
                         nLen, RTL_TEXTENCODING_ISO_8859_1);
}


OUString TestOutputStream::getStart() const
{
    OUString sResult = m_sStart;
    if (m_bMore)
        sResult += "...";
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

    OUString toString(const uno::Any & rStatus);

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

OUString ProgressHandler::toString(const uno::Any & rStatus)
{
    ucb::CHAOSProgressStart aStart;
    if (rStatus >>= aStart)
    {
        OUString sResult;
        if (aStart.Text.getLength() > 0)
        {
            sResult = aStart.Text + " ";
        }
        sResult += "[";
        sResult += OUString::valueOf(aStart.Minimum);
        sResult += "..";
        sResult += OUString::valueOf(aStart.Maximum);
        sResult += "]";
        return sResult;
    }

    OUString sText;
    if (rStatus >>= sText)
        return sText;

    sal_Int32 nValue;
    if (rStatus >>= nValue)
    {
        OUString sResult = ".." + OUString::valueOf(nValue) + "..";
        return OUString(sResult);
    }

    return OUString("(Unknown object)");
}


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


// virtual
void SAL_CALL ProgressHandler::push(const uno::Any & rStatus)
    throw (uno::RuntimeException)
{
    OUString sMessage("Status push: ");
    sMessage += toString(rStatus);
    m_rPrinter.print(sMessage);
}


// virtual
void SAL_CALL ProgressHandler::update(const uno::Any & rStatus)
    throw (uno::RuntimeException)
{
    OUString sMessage("Status update: ");
    sMessage += toString(rStatus);
    m_rPrinter.print(sMessage);
}


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
    OUString m_aConfigurationKey1;
    OUString m_aConfigurationKey2;
    sal_Bool m_bInited : 1;

public:
    Ucb( uno::Reference< lang::XMultiServiceFactory >& rxFactory,
         OUString const & rConfigurationKey1,
         OUString const & rConfigurationKey2 );
    ~Ucb();

    sal_Bool init();

    uno::Reference< lang::XMultiServiceFactory > getServiceFactory() const
    { return m_xFac; }

    uno::Reference< ucb::XContentIdentifierFactory >
    getContentIdentifierFactory();
    uno::Reference< ucb::XContentProvider >
    getContentProvider();

    static OUString m_aProtocol;
};

// static
OUString Ucb::m_aProtocol;


Ucb::Ucb( uno::Reference< lang::XMultiServiceFactory >& rxFactory,
          OUString const & rConfigurationKey1,
          OUString const & rConfigurationKey2 )
: m_xFac( rxFactory ),
  m_aConfigurationKey1( rConfigurationKey1 ),
  m_aConfigurationKey2( rConfigurationKey2 ),
  m_bInited( sal_False )
{
}


Ucb::~Ucb()
{
}


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
                OUString aText = "    " + aInfos[i].Scheme;
                print(aText);
            }
        }
    }
    else
        print("Error initializing UCB");

    m_bInited = m_xProv.is();
    return m_bInited;
}


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


UcbTaskEnvironment::UcbTaskEnvironment(
                    const uno::Reference< task::XInteractionHandler >&
                     rxInteractionHandler,
                    const uno::Reference< ucb::XProgressHandler >&
                     rxProgressHandler )
: m_xInteractionHandler( rxInteractionHandler ),
  m_xProgressHandler( rxProgressHandler )
{
}


// virtual
UcbTaskEnvironment::~UcbTaskEnvironment()
{
}



// XInterface methods



// virtual
uno::Any SAL_CALL
UcbTaskEnvironment::queryInterface( const uno::Type & rType )
    throw( uno::RuntimeException )
{
    uno::Any aRet = cppu::queryInterface(
            rType, static_cast< ucb::XCommandEnvironment* >( this ) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


// virtual
void SAL_CALL UcbTaskEnvironment::acquire()
    throw()
{
    OWeakObject::acquire();
}


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

    uno::Any executeCommand( const OUString& rName,
                             const uno::Any& rArgument,
                             bool bPrint = true );
};


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


// virtual
UcbCommandProcessor::~UcbCommandProcessor()
{
}


uno::Any UcbCommandProcessor::executeCommand( const OUString& rName,
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
            OUString aText = "Executing command: " + rName;
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
            Ucb& rUCB, const OUString& rURL, MyOutWindow* pOutEdit );
    void dispose();

    const OUString getURL() const;
    const OUString getType() const;

    uno::Sequence< ucb::CommandInfo > getCommands();
    uno::Sequence< beans::Property >    getProperties();

    uno::Any  getPropertyValue( const OUString& rName );
    void setPropertyValue( const OUString& rName, const uno::Any& rValue );
    void addProperty     ( const OUString& rName, const uno::Any& rValue );
    void removeProperty  ( const OUString& rName );

    OUString getStringPropertyValue( const OUString& rName );
    void setStringPropertyValue( const OUString& rName,
                                 const OUString& rValue );
    void addStringProperty( const OUString& rName,
                            const OUString& rValue );
    void open( const OUString & rName, const OUString& rInput,
               bool bPrint, bool bTiming, bool bSort,
               OpenStack * pStack = 0, sal_uInt32 nLevel = 0,
               sal_Int32 nFetchSize = 0 );
    void openAll( Ucb& rUCB, bool bPrint, bool bTiming, bool bSort,
                  sal_Int32 nFetchSize );
    void transfer( const OUString& rSourceURL, sal_Bool bMove );
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


// virtual
UcbContent::~UcbContent()
{
}


// static
UcbContent* UcbContent::create(
        Ucb& rUCB, const OUString& rURL, MyOutWindow* pOutEdit )
{
    if ( !rURL.Len() )
        return NULL;


    // Get XContentIdentifier interface from UCB and let it create an
    // identifer for the given URL.


    uno::Reference< ucb::XContentIdentifierFactory > xIdFac =
                                        rUCB.getContentIdentifierFactory();
    if ( !xIdFac.is() )
        return NULL;

    uno::Reference< ucb::XContentIdentifier > xId =
                            xIdFac->createContentIdentifier( rURL );
    if ( !xId.is() )
        return NULL;


    // Get XContentProvider interface from UCB and let it create a
    // content for the given identifier.


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
            uno::Sequence< OUString >(), pNew );
    }

    return pNew;
}


const OUString UcbContent::getURL() const
{
    uno::Reference< ucb::XContentIdentifier > xId(
        m_xContent->getIdentifier() );
    if ( xId.is() )
        return OUString( xId->getContentIdentifier() );

    return OUString();
}


const OUString UcbContent::getType() const
{
    const OUString aType( m_xContent->getContentType() );
    return aType;
}


void UcbContent::dispose()
{
    uno::Reference< lang::XComponent > xComponent( m_xContent, uno::UNO_QUERY );
    if ( xComponent.is() )
        xComponent->dispose();
}


void UcbContent::open( const OUString & rName, const OUString& rInput,
                       bool bPrint, bool bTiming, bool bSort,
                       OpenStack * pStack, sal_uInt32 nLevel,
                       sal_Int32 nFetchSize )
{
    uno::Any aArg;

    bool bDoSort = false;

    ucb::OpenCommandArgument2 aOpenArg;
    if (rName.equalsAscii("search"))
    {
        ucb::SearchCommandArgument aArgument;
        if (!parseSearchArgument(rInput, aArgument.Info))
        {
            print("Can't parse search argument");
            return;
        }
        aArgument.Properties.realloc(5);
        aArgument.Properties[0].Name = "Title";
        aArgument.Properties[0].Handle = -1;
        aArgument.Properties[1].Name = "DateCreated";
        aArgument.Properties[1].Handle = -1;
        aArgument.Properties[2].Name = "Size";
        aArgument.Properties[2].Handle = -1;
        aArgument.Properties[3].Name = "IsFolder";
        aArgument.Properties[3].Handle = -1;
        aArgument.Properties[4].Name = "IsDocument";
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
            pProps[ 0 ].Name   = "Title";
            pProps[ 0 ].Handle = -1; // Important!
/**/        pProps[ 0 ].Type = cppu::UnoType<OUString>::get();
                // HACK for sorting...
            pProps[ 1 ].Name   = "DateCreated";
            pProps[ 1 ].Handle = -1; // Important!
            pProps[ 2 ].Name   = "Size";
            pProps[ 2 ].Handle = -1; // Important!
            pProps[ 3 ].Name   = "IsFolder";
            pProps[ 3 ].Handle = -1; // Important!
/**/        pProps[ 3 ].Type = cppu::UnoType<sal_Bool>::get();
                // HACK for sorting...
            pProps[ 4 ].Name   = "IsDocument";
            pProps[ 4 ].Handle = -1; // Important!
            aOpenArg.Properties = aProps;

            bDoSort = bSort;
            if (bDoSort)
            {
                // Sort criteria... Note that column numbering starts with 1!
                aOpenArg.SortingInfo.realloc(2);
                // primary sort criterion: column 4 --> IsFolder
                aOpenArg.SortingInfo[ 0 ].ColumnIndex = 4;
                aOpenArg.SortingInfo[ 0 ].Ascending   = sal_False;
                // secondary sort criterion: column 1 --> Title
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
        nTime = tools::Time::GetSystemTicks();

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
                                          OUString( "com.sun.star.ucb.SortedDynamic"
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
                print( OUString(
                    "Content-ID : Title : Size : IsFolder : IsDocument\n"
                    "-------------------------------------------------" ) );
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
                            setPropertyValue("FetchSize",
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
                    OUString aText;

                    if ( bPrint )
                    {
                        OUString aId( xContentAccess->
                                          queryContentIdentifierString() );
                        aText = OUString::number( ++n ) + ") " + aId + " : ";
                    }

                    // Title:
                    OUString aTitle( xRow->getString( 1 ) );
                    if ( bPrint )
                    {
                        if ( aTitle.Len() == 0 && xRow->wasNull() )
                            aText += "<null>";
                        else
                            aText += aTitle;
                        aText += " : ";
                    }

                    // Size:
                    sal_Int32 nSize = xRow->getInt( 3 );
                    if ( bPrint )
                    {
                        if ( nSize == 0 && xRow->wasNull() )
                            aText += "<null>";
                        else
                            aText += OUString::valueOf( nSize );
                        aText += " : ";
                    }

                    // IsFolder:
                    sal_Bool bFolder = xRow->getBoolean( 4 );
                    if ( bPrint )
                    {
                        if ( !bFolder && xRow->wasNull() )
                            aText.AppendAscii( "<null>" );
                        else
                            aText += bFolder ? OUString("true") : OUString("false");
                        aText.AppendAscii( " : " );
                    }

                    // IsDocument:
                    sal_Bool bDocument = xRow->getBoolean( 5 );
                    if ( bPrint )
                    {
                        if ( !bFolder && xRow->wasNull() )
                            aText.AppendAscii( "<null>" );
                        else
                            aText
                                += bDocument ? OUString("true") : OUString("false"); //  IsDocument
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
        nTime = tools::Time::GetSystemTicks() - nTime;
        OUString aText = "Operation took " + OUString::number(nTime) + " ms.";
        print( aText );
    }
}


void UcbContent::openAll( Ucb& rUCB, bool bPrint, bool bTiming, bool bSort,
                          sal_Int32 nFetchSize )
{
    ULONG nTime = 0;
    if ( bTiming )
        nTime = tools::Time::GetSystemTicks();

    OpenStack aStack;
    aStack.push( OpenStackEntry( m_xContent, 0 ) );

    while ( !aStack.empty() )
    {
        OpenStackEntry aEntry( aStack.top() );
        aStack.pop();

        if ( bPrint )
        {
            OUString aText;
            for ( sal_uInt32 i = aEntry.m_nLevel; i != 0; --i )
                aText += '=';
            aText = aText + "LEVEL " + OUString::number(aEntry.m_nLevel);

            uno::Reference< ucb::XContentIdentifier > xID;
            if ( aEntry.m_bUseIdentifier )
                xID = aEntry.m_xIdentifier;
            else if ( aEntry.m_xContent.is() )
                xID = aEntry.m_xContent->getIdentifier();
            if ( xID.is() )
            {
                aText = aText + ": " + xID->getContentIdentifier();
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
            open( "open",
                  OUString(), bPrint, false, bSort, &aStack,
                  aEntry.m_nLevel, nFetchSize );
    }

    if ( bTiming )
    {
        nTime = tools::Time::GetSystemTicks() - nTime;
        OUString aText = "Operation took "  + OUString::number(nTime) + " ms.";
        print( aText );
    }
}


void UcbContent::transfer( const OUString& rSourceURL, sal_Bool bMove  )
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
                            OUString(),
                            //OUString("NewTitle"),
                            ucb::NameClash::ERROR );

        ucb::Command aTransferCommand( OUString( "globalTransfer" ),
                                             -1,
                                             uno::makeAny( aArg ) );

        uno::Reference< task::XInteractionHandler > xInteractionHandler;
        if (m_rUCB.getServiceFactory().is())
            xInteractionHandler
                = uno::Reference< task::XInteractionHandler >(
                    task::InteractionHandler::createWithParent(comphelper::getComponentContext(m_rUCB.getServiceFactory()), 0),
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
            bMove, rSourceURL, OUString(), ucb::NameClash::ERROR );
    executeCommand( OUString("transfer"), aArg );

//  executeCommand( OUString("flush"), Any() );

#endif
}


void UcbContent::destroy()
{
    print( "Deleting content..." );

    uno::Any aArg;
    aArg <<= sal_Bool( sal_True ); // delete physically, not only to trash.
    executeCommand( OUString("delete"), aArg );

//  executeCommand( OUString("flush"), Any() );
}


uno::Sequence< ucb::CommandInfo > UcbContent::getCommands()
{
    uno::Any aResult = executeCommand(
            OUString("getCommandInfo"), uno::Any() );

    uno::Reference< ucb::XCommandInfo > xInfo;
    if ( aResult >>= xInfo )
    {
        uno::Sequence< ucb::CommandInfo > aCommands(
            xInfo->getCommands() );
        const ucb::CommandInfo* pCommands = aCommands.getConstArray();

        OUString aText("Commands:\n");
        sal_uInt32 nCount = aCommands.getLength();
        for ( sal_uInt32 n = 0; n < nCount; ++n )
        {
            aText = aText + "    " + String( pCommands[ n ].Name ) + "\n";
        }
        print( aText );

        return aCommands;
    }

    print( "getCommands failed!" );
    return uno::Sequence< ucb::CommandInfo >();
}


uno::Sequence< beans::Property > UcbContent::getProperties()
{
    uno::Any aResult = executeCommand(
        OUString("getPropertySetInfo"), uno::Any() );

    uno::Reference< beans::XPropertySetInfo > xInfo;
    if ( aResult >>= xInfo )
    {
        uno::Sequence< beans::Property > aProps( xInfo->getProperties() );
        const beans::Property* pProps = aProps.getConstArray();

        OUString aText("Properties:\n");
        sal_uInt32 nCount = aProps.getLength();
        for ( sal_uInt32 n = 0; n < nCount; ++n )
        {
            aText = aText + "    " + pProps[ n ].Name + "\n";
        }
        print( aText );

        return aProps;
    }

    print( "getProperties failed!" );
    return uno::Sequence< beans::Property >();
}


uno::Any UcbContent::getPropertyValue( const OUString& rName )
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
        OUString("getPropertyValues"), aArg );

    uno::Reference< sdbc::XRow > xValues;
    if ( aResult >>= xValues )
        return xValues->getObject(
            1, uno::Reference< container::XNameAccess>() );

    print( "getPropertyValue failed!" );
    return uno::Any();
}


OUString UcbContent::getStringPropertyValue( const OUString& rName )
{
    uno::Any aAny = getPropertyValue( rName );
    if ( aAny.getValueType() == cppu::UnoType<OUString>::get() )
    {
        const OUString aValue(
            * static_cast< const OUString * >( aAny.getValue() ) );

        OUString aText = rName + " value: '" + aValue + "'";
        print( aText );

        return aValue;
    }

    print( "getStringPropertyValue failed!" );
    return OUString();
}


void UcbContent::setPropertyValue( const OUString& rName,
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

    executeCommand( OUString("setPropertyValues"),
                    aArg );

//  executeCommand( OUString("flush"), Any() );
}


void UcbContent::setStringPropertyValue( const OUString& rName,
                                         const OUString& rValue )
{
    uno::Any aAny;
    aAny <<= rValue;
    setPropertyValue( rName, aAny );

    OUString aText = rName + " value set to: '" + rValue + "'";
    print( aText );
}


void UcbContent::addProperty( const OUString& rName,
                              const uno::Any& rValue )
{
    uno::Reference< beans::XPropertyContainer > xContainer( m_xContent,
                                                            uno::UNO_QUERY );
    if ( xContainer.is() )
    {
        OUString aText = "Adding property: " + rName;
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


void UcbContent::addStringProperty(
                    const OUString& rName, const OUString& rValue )
{
    uno::Any aValue;
    aValue <<= rValue;
    addProperty( rName, aValue );
}


void UcbContent::removeProperty( const OUString& rName )
{
    uno::Reference< beans::XPropertyContainer > xContainer( m_xContent,
                                                            uno::UNO_QUERY );
    if ( xContainer.is() )
    {
        OUString aText = "Removing property: " + rName;
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



// XInterface methods



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


// virtual
void SAL_CALL UcbContent::acquire()
    throw()
{
    OWeakObject::acquire();
}


// virtual
void SAL_CALL UcbContent::release()
    throw()
{
    OWeakObject::release();
}



// XEventListener methods.



// virtual
void SAL_CALL UcbContent::disposing( const lang::EventObject& /*Source*/ )
    throw( uno::RuntimeException )
{
    print ( "Content: disposing..." );
}



// XContentEventListener methods,



// virtual
void SAL_CALL UcbContent::contentEvent( const ucb::ContentEvent& evt )
    throw( uno::RuntimeException )
{
    switch ( evt.Action )
    {
        case ucb::ContentAction::INSERTED:
        {
            OUString aText = "contentEvent: INSERTED: ";
            if ( evt.Content.is() )
            {
                uno::Reference< ucb::XContentIdentifier > xId(
                                           evt.Content->getIdentifier() );
                aText = aText + xId->getContentIdentifier() + " - " + evt.Content->getContentType();
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
                    aMatch.AppendAscii(" - ");
                    aMatch += String(evt.Content->getContentType());
                }
            }
            else
                aMatch.AppendAscii("<no id>");
            print(aMatch);
            break;
        }

        default:
            print( "contentEvent..." );
            break;
    }
}



// XPropertiesChangeListener methods.



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
            OUString aText = "    " + pEvents[ n ].PropertyName;
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
    MyWin( vcl::Window *pParent, WinBits nWinStyle,
           uno::Reference< lang::XMultiServiceFactory >& rxFactory,
           OUString const & rConfigurationKey1,
           OUString const & rConfigurationKey2 );
    virtual ~MyWin();

    void Resize( void );
    DECL_LINK ( ToolBarHandler, ToolBox* );

    void print( const OUString& rText );
    void print( const sal_Char* pText );
};


MyWin::MyWin( vcl::Window *pParent, WinBits nWinStyle,
              uno::Reference< lang::XMultiServiceFactory >& rxFactory,
              OUString const & rConfigurationKey1,
              OUString const & rConfigurationKey2 )
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
                          OUString("Clear"));
    m_pTool->SetHelpText( MYWIN_ITEMID_CLEAR,
                          OUString("Clear the Output Window"));
    m_pTool->InsertSeparator();
    m_pTool->InsertItem ( MYWIN_ITEMID_CREATE,
                          OUString("Create") );
    m_pTool->SetHelpText( MYWIN_ITEMID_CREATE,
                          OUString("Create a content") );
    m_pTool->InsertItem ( MYWIN_ITEMID_RELEASE,
                          OUString("Release") );
    m_pTool->SetHelpText( MYWIN_ITEMID_RELEASE,
                          OUString("Release current content") );
    m_pTool->InsertSeparator();
    m_pTool->InsertItem ( MYWIN_ITEMID_COMMANDS,
                          OUString("Commands") );
    m_pTool->SetHelpText( MYWIN_ITEMID_COMMANDS,
                          OUString("Get Commands supported by the content") );
    m_pTool->InsertItem ( MYWIN_ITEMID_PROPS,
                          OUString("Properties") );
    m_pTool->SetHelpText( MYWIN_ITEMID_PROPS,
                          OUString("Get Properties supported by the content") );
    m_pTool->InsertSeparator();
    m_pTool->InsertItem ( MYWIN_ITEMID_ADD_PROP,
                          OUString("addProperty") );
    m_pTool->SetHelpText( MYWIN_ITEMID_ADD_PROP,
                          OUString(
                                  "Add a new string(!) property to the content. "
                                  "Type the property name in the entry field and "
                                  "push this button. The default value for the "
                                  "property will be set to the string 'DefaultValue'" ) );
    m_pTool->InsertItem ( MYWIN_ITEMID_REMOVE_PROP,
                          OUString("removeProperty") );
    m_pTool->SetHelpText( MYWIN_ITEMID_REMOVE_PROP,
                          OUString(
                                  "Removes a property from the content. "
                                  "Type the property name in the entry field and "
                                  "push this button." ) );
    m_pTool->InsertItem ( MYWIN_ITEMID_GET_PROP,
                          OUString("getPropertyValue") );
    m_pTool->SetHelpText( MYWIN_ITEMID_GET_PROP,
                          OUString(
                                  "Get a string(!) property value from the content. "
                                  "Type the property name in the entry field and "
                                  "push this button to obtain the value" ) );
    m_pTool->InsertItem ( MYWIN_ITEMID_SET_PROP,
                          OUString("setPropertyValue") );
    m_pTool->SetHelpText( MYWIN_ITEMID_SET_PROP,
                          OUString(
                                  "Set a string(!) property value of the content."
                                  "Type the property name in the entry field and "
                                  "push this button to set the value to the string "
                                  "'NewValue'" ) );
    m_pTool->InsertSeparator();
    m_pTool->InsertItem ( MYWIN_ITEMID_OPEN,
                          OUString("Open") );
    m_pTool->SetHelpText( MYWIN_ITEMID_OPEN,
                          OUString("Open the content") );
    m_pTool->InsertItem ( MYWIN_ITEMID_OPEN_ALL,
                          OUString("Open All") );
    m_pTool->SetHelpText( MYWIN_ITEMID_OPEN_ALL,
                          OUString(
                                "Open the content and all of its"
                                    " children" ) );
    m_pTool->InsertItem ( MYWIN_ITEMID_UPDATE,
                          OUString("Update") );
    m_pTool->SetHelpText( MYWIN_ITEMID_UPDATE,
                          OUString("Update the content") );
    m_pTool->InsertItem ( MYWIN_ITEMID_SYNCHRONIZE,
                          OUString("Synchronize") );
    m_pTool->SetHelpText( MYWIN_ITEMID_SYNCHRONIZE,
                          OUString("Synchronize the content") );
    m_pTool->InsertItem ( MYWIN_ITEMID_SEARCH,
                          OUString("Search") );
    m_pTool->SetHelpText( MYWIN_ITEMID_SEARCH,
                          OUString("Search the content") );

    m_pTool->InsertItem ( MYWIN_ITEMID_REORGANIZE,
                          OUString("Reorganize") );
    m_pTool->SetHelpText( MYWIN_ITEMID_REORGANIZE,
                          OUString("Reorganize the content storage") );

    m_pTool->InsertSeparator();
    m_pTool->InsertItem ( MYWIN_ITEMID_COPY,
                          OUString("Copy") );
    m_pTool->SetHelpText( MYWIN_ITEMID_COPY,
                          OUString(
                                "Copy a content. Type the URL of the source "
                                "content into the entry field." ) );
    m_pTool->InsertItem ( MYWIN_ITEMID_MOVE,
                          OUString("Move") );
    m_pTool->SetHelpText( MYWIN_ITEMID_MOVE,
                          OUString(
                                "Move a content. Type the URL of the source "
                                "content into the entry field." ) );
    m_pTool->InsertItem ( MYWIN_ITEMID_DELETE,
                          OUString("Delete") );
    m_pTool->SetHelpText( MYWIN_ITEMID_DELETE,
                          OUString("Delete the content.") );

    m_pTool->InsertSeparator();
    m_pTool->InsertItem ( MYWIN_ITEMID_TIMING,
                          OUString("Timing"),
                          TIB_CHECKABLE | TIB_AUTOCHECK );
    m_pTool->SetHelpText( MYWIN_ITEMID_TIMING,
                          OUString(
                                "Display execution times instead of"
                                    " output" ) );
    m_pTool->InsertItem ( MYWIN_ITEMID_SORT,
                          OUString("Sort"),
                          TIB_CHECKABLE | TIB_AUTOCHECK );
    m_pTool->SetHelpText( MYWIN_ITEMID_SORT,
                          OUString("Sort result sets") );
    m_pTool->InsertItem ( MYWIN_ITEMID_FETCHSIZE,
                          OUString("Fetch Size") );
    m_pTool->SetHelpText( MYWIN_ITEMID_FETCHSIZE,
                          OUString("Set cached cursor fetch size to positive value") );

    m_pTool->InsertSeparator();
    m_pTool->InsertItem ( MYWIN_ITEMID_SYS2URI,
                          OUString("UNC>URI") );
    m_pTool->SetHelpText( MYWIN_ITEMID_SYS2URI,
                          OUString(
                                "Translate 'System File Path' to URI,"
                                    " if possible" ) );
    m_pTool->InsertItem ( MYWIN_ITEMID_URI2SYS,
                          OUString("URI>UNC") );
    m_pTool->SetHelpText( MYWIN_ITEMID_URI2SYS,
                          OUString(
                                "Translate URI to 'System File Path',"
                                    " if possible" ) );

    m_pTool->InsertSeparator();
    m_pTool->InsertItem ( MYWIN_ITEMID_OFFLINE,
                          OUString("Offline") );
    m_pTool->SetHelpText( MYWIN_ITEMID_OFFLINE,
                          OUString("Go offline") );
    m_pTool->InsertItem ( MYWIN_ITEMID_ONLINE,
                          OUString("Online") );
    m_pTool->SetHelpText( MYWIN_ITEMID_ONLINE,
                          OUString("Go back online") );

    m_pTool->SetSelectHdl( LINK( this, MyWin, ToolBarHandler ) );
    m_pTool->Show();

    // Edit.
    m_pCmdEdit = new Edit( this );
    m_pCmdEdit->SetReadOnly( FALSE );
    m_pCmdEdit->SetText( OUString( "file:///" ) );
    m_pCmdEdit->Show();

    // MyOutWindow.
    m_pOutEdit = new MyOutWindow( this, WB_HSCROLL | WB_VSCROLL | WB_BORDER );
    m_pOutEdit->SetReadOnly( TRUE );
    m_pOutEdit->Show();

    m_aUCB.setOutEdit( m_pOutEdit );
}


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


void MyWin::print( const sal_Char* pText )
{
    print( OUString.createFromAscii( pText ) );
}


void MyWin::print( const OUString& rText )
{
    SolarMutexGuard aGuard;

    if ( m_pOutEdit )
    {
        m_pOutEdit->Append( rText );
        m_pOutEdit->Update();
    }
}


IMPL_LINK( MyWin, ToolBarHandler, ToolBox*, pToolBox )
{
    USHORT nItemId   = pToolBox->GetCurItemId();
    OUString aCmdLine = m_pCmdEdit->GetText();

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
                OUString aText = "Content released: " + m_pContent->getURL();

                m_pContent->dispose();
                m_pContent->release();
                m_pContent = NULL;

                print( aText );
            }

            m_pContent = UcbContent::create( m_aUCB, aCmdLine, m_pOutEdit );
            if ( m_pContent )
            {
                OUString aText = "Created content: " + m_pContent->getURL() + " - " + m_pContent->getType();
                print( aText );
            }
            else
            {
                OUString aText = "Creation failed for content: " + aCmdLine;
                print( aText );
            }
            break;

        case MYWIN_ITEMID_RELEASE:
            if ( m_pContent )
            {
                OUString aText = "Content released: " + m_pContent->getURL();

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
                        OUString("DefaultValue") );
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
                                OUString("NewValue") );
            else
                print( "No content!" );

            break;

        case MYWIN_ITEMID_OPEN:
            if ( m_pContent )
                m_pContent->open(OUString("open"),
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
                m_pContent->open(OUString("update"),
                                 aCmdLine, !m_bTiming, m_bTiming, m_bSort, 0,
                                 0, m_nFetchSize);
            else
                print( "No content!" );

            break;

        case MYWIN_ITEMID_SYNCHRONIZE:
            if ( m_pContent )
                m_pContent->open(OUString("synchronize"),
                                 aCmdLine, !m_bTiming, m_bTiming, m_bSort, 0,
                                 0, m_nFetchSize);
            else
                print( "No content!" );

            break;

        case MYWIN_ITEMID_SEARCH:
            if ( m_pContent )
                m_pContent->open(OUString("search"),
                                 aCmdLine, !m_bTiming, m_bTiming, m_bSort, 0,
                                 0, m_nFetchSize);
            else
                print( "No content!" );

            break;

        case MYWIN_ITEMID_REORGANIZE:
            if ( m_pContent )
                m_pContent->executeCommand (
                    OUString("reorganizeData"),
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
            m_bTiming = m_pTool->IsItemChecked(MYWIN_ITEMID_TIMING);
            break;

        case MYWIN_ITEMID_SORT:
            m_bSort = m_pTool->IsItemChecked(MYWIN_ITEMID_SORT);
            break;

        case MYWIN_ITEMID_FETCHSIZE:
        {
            m_nFetchSize = aCmdLine.ToInt32();
            OUString aText;
            if (m_nFetchSize > 0)
            {
                aText = "Fetch size set to ";
                aText += OUString::number(m_nFetchSize);
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

            OUString aURL(getLocalFileURL());

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

            OUString aName;
            uno::Any aArgument;
            if (nItemId == MYWIN_ITEMID_OFFLINE)
            {
                aName = "goOffline";

                uno::Sequence<
                    uno::Reference< ucb::XContentIdentifier > >
                        aIdentifiers(1);
                aIdentifiers[0]
                    = m_aUCB.getContentIdentifierFactory()->
                                 createContentIdentifier(aCmdLine);
                aArgument <<= aIdentifiers;
            }
            else
                aName = "goOnline";

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


// virtual
void MyApp::Main()
{

    // Read command line params.


    OUString aConfigurationKey1( UCB_CONFIGURATION_KEY1_LOCAL);
    OUString aConfigurationKey2( UCB_CONFIGURATION_KEY2_OFFICE);

    USHORT nParams = Application::GetCommandLineParamCount();
    for ( USHORT n = 0; n < nParams; ++n )
    {
        OUString aParam( Application::GetCommandLineParam( n ) );
        if (aParam.CompareIgnoreCaseToAscii("-key=",
                                            RTL_CONSTASCII_LENGTH("-key="))
                == COMPARE_EQUAL)
        {
            sal_Int32 nSlash = aParam.indexOf('/', RTL_CONSTASCII_LENGTH("-key="));
            if (nSlash == -1)
            {
                aConfigurationKey1
                    = aParam.copy(RTL_CONSTASCII_LENGTH("-key="));
                aConfigurationKey2 = "";
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


    // Initialize local Service Manager and basic services.


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


    // Create Application Window...


    Help::EnableBalloonHelp();

    MyWin *pMyWin = new MyWin( NULL, WB_APP | WB_STDWORK, xFac,
                               aConfigurationKey1, aConfigurationKey2 );

    pMyWin->SetText( OUString( "UCB Demo/Test Application" ) );

    pMyWin->SetPosSizePixel( 0, 0, 1024, 768 );

    pMyWin->Show();


    // Go...


    Execute();


    // Destroy Application Window...


    delete pMyWin;


    // Cleanup.


    ::ucbhelper::ContentBroker::deinitialize();

    // Dispose local service manager.
    if ( xComponent.is() )
        xComponent->dispose();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
