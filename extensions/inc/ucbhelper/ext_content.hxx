/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ext_content.hxx,v $
 * $Revision: 1.4 $
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


#ifndef _UCBHELPER_CONTENT_HXX_
#define _UCBHELPER_CONTENT_HXX_

#include <rtl/string.hxx>
#include <rtl/ustring>
#include <osl/mutex.hxx>
#include <osl/thread.h>

#include <cppuhelper/weak.hxx>
#include <com/sun/star/ucb/XCommandTaskProcessor.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/ucb/XPropertyTaskProcessor.hpp>
#include <com/sun/star/ucb/XContentIdentifier.hpp>
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <list>

using namespace cppu;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace std;
using namespace rtl;
using namespace osl;



template <class Type> class safe_list : public osl::Mutex, public std::list< Type > {};

class OSimpleContentIdentifier :    public OWeakObject,
                                    public XContentIdentifier
{
private:
    OUString    Identifier;
    OUString    ProviderScheme;

public:
    OSimpleContentIdentifier( const OUString& rIdentifier, const OUString& rProviderScheme );

    // XInterface
    virtual Any         SAL_CALL queryInterface( const Type &type ) throw( RuntimeException );
    virtual void        SAL_CALL acquire() throw(RuntimeException);
    virtual void        SAL_CALL release() throw(RuntimeException);

    // XContentIdentifier
    virtual OUString    SAL_CALL getContentIdentifier() throw(RuntimeException);
    virtual OUString    SAL_CALL getContentProviderScheme() throw(RuntimeException);
};


//---------------------------------------------------------------------------
//
//  FileSystemContent
//
//---------------------------------------------------------------------------

class OContent :
public OWeakObject,
public XContent,
public XCommandTaskProcessor,
public XPropertyTaskProcessor,
public XCommandInfo,
public XPropertySetInfo,
public XComponent
{
public:
    struct PropertyChangeEventInfo
    {
        OUString    Name;
        long        Handle;

        PropertyChangeEventInfo() : Handle( -1 ) {}

        inline int operator ==( const PropertyChangeEventInfo& crInfo ) const
        { return Handle == crInfo.Handle && Handle > 0 || Name == crInfo.Name; }
    #ifdef __SUNPRO_CC
        inline int operator <( const PropertyChangeEventInfo& crInfo ) const
        { return Handle != crInfo.Handle ? Handle < crInfo.Handle : Name < crInfo.Name; }
    #endif
    };

    struct PropertyChangeListenerInfo
    {
        Reference< XPropertiesChangeListener >  xListener;
        list< PropertyChangeEventInfo >         aEventInfos;

        inline int operator ==( const PropertyChangeListenerInfo& crInfo ) const
        { return xListener == crInfo.xListener; }
    #ifdef __SUNPRO_CC
        inline int operator <( const PropertyChangeListenerInfo& crInfo ) const
        { return xListener < crInfo.xListener; }
    #endif
    };

protected:
    Sequence< PropertyChangeEvent > matchListenerEvents( const Sequence< PropertyChangeEvent >& crEvents, const PropertyChangeListenerInfo & crInfo );

    safe_list< Reference< XContentEventListener > > m_aContentListeners;
    safe_list< Reference< XEventListener > >        m_aComponentListeners;
    safe_list< PropertyChangeListenerInfo >         m_aPropertyChangeListeners;
public:
    virtual ~OContent() {}

    virtual void broadcastContentEvent( const ContentEvent & crEvent );
    virtual void broadcastPropertiesChangeEvents( const Sequence< PropertyChangeEvent >& crEvents );

    // To be implemented by inheritents
    virtual Any doCommand( const Command & crCommand ) = 0;

    // XInterface
    virtual Any         SAL_CALL queryInterface( const Type &type ) throw( RuntimeException );

    virtual void SAL_CALL acquire() throw(RuntimeException);
    virtual void SAL_CALL release() throw(RuntimeException);

    // XContent
    virtual void SAL_CALL addContentEventListener( const Reference< XContentEventListener >& rListener ) throw();
    virtual void SAL_CALL removeContentEventListener( const Reference< XContentEventListener >& rListener ) throw();

    // XComponent
    virtual void SAL_CALL dispose() throw();
    virtual void SAL_CALL addEventListener( const Reference< XEventListener >& xListener ) throw();
    virtual void SAL_CALL removeEventListener( const Reference< XEventListener >& xListener ) throw();

    // XCommmandTaskProcessor
    virtual Reference< XCommandInfo > SAL_CALL getCommandsInfo() throw();

    // XCommandInfo
    virtual CommandInfo SAL_CALL getCommandInfoByName( const OUString& rName ) throw( UnsupportedCommandException );
    virtual CommandInfo SAL_CALL getCommandInfoByHandle( long nHandle ) throw( UnsupportedCommandException );
    virtual sal_Bool SAL_CALL hasCommandByName( const OUString& rName ) throw();
    virtual sal_Bool SAL_CALL hasCommandByHandle( long nHandle ) throw();

    // XPropertyTaskProcessor
    virtual Reference< XPropertySetInfo > SAL_CALL getPropertySetInfo() throw();

    // XPropertySetInfo
    virtual Property SAL_CALL getPropertyByName( const OUString& Name ) throw( UnknownPropertyException );
    virtual sal_Bool SAL_CALL hasPropertyByName( const OUString& Name ) throw();
    virtual void SAL_CALL addPropertiesChangeListener( const Sequence< OUString >& Names, const Reference< XPropertiesChangeListener >& xListener ) throw();
    virtual void SAL_CALL removePropertiesChangeListener( const Sequence< OUString >& Names, const Reference< XPropertiesChangeListener >& xListener ) throw();
};

//---------------------------------------------------------------------------
//
//  FolderContent
//
//---------------------------------------------------------------------------

// supported Commands
static const sal_Int32 OPEN     = 0;
static const sal_Int32 CLOSE    = 1;

class OFolderContent : public OContent
{
protected:
    // Already provided children
    safe_list< XContent >   m_aChildList;

    // OContent
    virtual Any doCommand( const Command & crCommand );

    // new methods, can be overloaded
    virtual Any doOpenCommand();
    virtual Any doCloseCommand();

    // To be implemented by inheritants
    virtual Sequence< XContent > getAllChildren() = 0;

public:

    // XCommmandTaskProcessor
    virtual Reference< XCommandTask > SAL_CALL createCommandTask(const Command& rCommand, const Reference< XContentTaskEnvironment >& rEnvironment ) throw();

    // XCommandInfo
    virtual Sequence< CommandInfo > SAL_CALL getCommands() throw();
};

//---------------------------------------------------------------------------
//
//  OContentTask
//
//---------------------------------------------------------------------------

class OContentTask :
public OWeakObject,
public XContentTask
{
    Guard< OContent >                       m_aContentGuard;
protected:
    OContent                                *m_pContent;
    Reference< XContentTaskEnvironment >    m_xEnvironment;
    ContentTaskStatus                       m_eStatus;
    oslThread                               m_aThread;

    static void executeWorker( void * );
    virtual ContentTaskStatus setStatus( ContentTaskStatus eStatus );

    // To be implemented by inheritants
    virtual void doExecute() = 0;
public:
    OContentTask(  const Reference< XContentTaskEnvironment >& xEnv, OContent *pContent );
    virtual ~OContentTask();

    // XInterface
    virtual Any         SAL_CALL queryInterface( const Type &type ) throw( RuntimeException );

    virtual void SAL_CALL acquire() throw(RuntimeException);
    virtual void SAL_CALL release() throw(RuntimeException);

    // XContentTask
    virtual void SAL_CALL start() throw();
    virtual void SAL_CALL execute() throw( Exception );
    virtual void SAL_CALL abort() throw();
    virtual ContentTaskStatus SAL_CALL getStatus() throw();
    virtual Reference< XContentTaskEnvironment > SAL_CALL getEnvironment() throw();
};

//---------------------------------------------------------------------------
//
//  OCommandTask
//
//---------------------------------------------------------------------------

class OCommandTask :
public OContentTask,
public XCommandTask
{
protected:
    Command             m_aCommand;
    Any                 m_aResult;

public:
    OCommandTask( const Reference< XContentTaskEnvironment >& xEnv, OContent *pContent, const Command& rCommand );
    virtual ~OCommandTask();

    virtual void doExecute();

    // XInterface
    virtual Any         SAL_CALL queryInterface( const Type &type ) throw( RuntimeException );
    virtual void        SAL_CALL acquire() throw(RuntimeException);
    virtual void        SAL_CALL release() throw(RuntimeException);

    // XContentTask
    virtual void SAL_CALL start() throw();
    virtual void SAL_CALL execute() throw( Exception );
    virtual void SAL_CALL abort() throw();
    virtual ContentTaskStatus SAL_CALL getStatus() throw();
    virtual Reference< XContentTaskEnvironment > SAL_CALL getEnvironment() throw();

    // XCommandTask
    virtual Command SAL_CALL getCommand() throw();
    virtual Any SAL_CALL getResult() throw();
};

//---------------------------------------------------------------------------
//
//  OPropertyTask
//
//---------------------------------------------------------------------------

class OPropertyTask :
public OContentTask,
public XPropertyTask
{
protected:
    Sequence< PropertyValueInfo >   m_aProperties;
    PropertyTaskType                m_eType;
public:
    OPropertyTask(const Reference< XContentTaskEnvironment >& Environment, OContent *pContent, const Sequence< PropertyValue >& Properties, PropertyTaskType Type );
    virtual ~OPropertyTask();

    virtual void doExecute();

    // To be implemented by inheritants
    virtual Any setPropertyValue( PropertyValueInfo & rProperty ) = 0;
    virtual void getPropertyValue( PropertyValueInfo & rProperty ) = 0;

    // XInterface
    virtual Any         SAL_CALL queryInterface( const Type &type ) throw( RuntimeException );
    virtual void        SAL_CALL acquire() throw(RuntimeException);
    virtual void        SAL_CALL release() throw(RuntimeException);

    // XContentTask
    virtual void SAL_CALL start() throw();
    virtual void SAL_CALL execute() throw( Exception );
    virtual void SAL_CALL abort() throw();
    virtual ContentTaskStatus SAL_CALL getStatus() throw();
    virtual Reference< XContentTaskEnvironment > SAL_CALL getEnvironment() throw();

    // XPropertyTask
    PropertyTaskType SAL_CALL getType() throw();
    Sequence< PropertyValueInfo > SAL_CALL getProperties() throw();
};

#endif // _UCBHELPER_CONTENT_HXX_


