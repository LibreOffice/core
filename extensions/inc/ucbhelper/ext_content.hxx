/*************************************************************************
 *
 *  $RCSfile: ext_content.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:39 $
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


#ifndef _UCBHELPER_CONTENT_HXX_
#define _UCBHELPER_CONTENT_HXX_

#include <rtl/string.hxx>
#include <rtl/ustring>
#include <osl/mutex.hxx>
#include <osl/thread.h>

#include <cppuhelper/weak.hxx>

#ifndef _COM_SUN_STAR_UCB_XCOMMANDTASKPROCESSOR_HPP_
#include <com/sun/star/ucb/XCommandTaskProcessor.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDINFO_HPP_
#include <com/sun/star/ucb/XCommandInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENT_HPP_
#include <com/sun/star/ucb/XContent.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XPROPERTYTASKPROCESSOR_HPP_
#include <com/sun/star/ucb/XPropertyTaskProcessor.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTIDENTIFIER_HPP_
#include <com/sun/star/ucb/XContentIdentifier.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTIESCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

#include <list>
#include <hash_map>

#if defined(TF_TYPE) || (SUPD >= 564)
using namespace cppu;
#endif
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
#if defined(TF_TYPE) || (SUPD >= 564)
    virtual Any         SAL_CALL queryInterface( const Type &type ) throw( RuntimeException );
#else
    virtual sal_Bool    SAL_CALL queryInterface( const Uik& type, Any& ifc ) throw( RuntimeException );
#endif
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
#if defined(TF_TYPE) || (SUPD >= 564)
    virtual Any         SAL_CALL queryInterface( const Type &type ) throw( RuntimeException );
#else
    virtual sal_Bool SAL_CALL queryInterface( const Uik& type, Any& ifc ) throw( RuntimeException );
#endif

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
#if defined(TF_TYPE) || (SUPD >= 564)
    virtual Any         SAL_CALL queryInterface( const Type &type ) throw( RuntimeException );
#else
    virtual sal_Bool SAL_CALL queryInterface( const Uik& type, Any& ifc ) throw( RuntimeException );
#endif

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
#if defined(TF_TYPE) || (SUPD >= 564)
    virtual Any         SAL_CALL queryInterface( const Type &type ) throw( RuntimeException );
#else
    virtual sal_Bool    SAL_CALL queryInterface( const Uik& type, Any& ifc ) throw( RuntimeException );
#endif
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
#if defined(TF_TYPE) || (SUPD >= 564)
    virtual Any         SAL_CALL queryInterface( const Type &type ) throw( RuntimeException );
#else
    virtual sal_Bool    SAL_CALL queryInterface( const Uik& type, Any& ifc ) throw( RuntimeException );
#endif
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


