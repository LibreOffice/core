/*************************************************************************
 *
 *  $RCSfile: content.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: abi $ $Date: 2001-05-16 07:36:23 $
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

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYACCESS_HPP_
#include <com/sun/star/beans/XPropertyAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENCOMMANDARGUMENT2_HPP_
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENMODE_HPP_
#include <com/sun/star/ucb/OpenMode.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDINFO_HPP_
#include <com/sun/star/ucb/XCommandInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XPERSISTENTPROPERTYSET_HPP_
#include <com/sun/star/ucb/XPersistentPropertySet.hpp>
#endif
#ifndef _VOS_DIAGNOSE_HXX_
#include <vos/diagnose.hxx>
#endif
#ifndef _UCBHELPER_CONTENTIDENTIFIER_HXX
#include <ucbhelper/contentidentifier.hxx>
#endif
#ifndef _UCBHELPER_PROPERTYVALUESET_HXX
#include <ucbhelper/propertyvalueset.hxx>
#endif

#ifndef _CONTENT_HXX
#include <provider/content.hxx>
#endif
#ifndef _PROVIDER_HXX
#include <provider/provider.hxx>
#endif
#ifndef _RESULTSET_HXX
#include <provider/resultset.hxx>
#endif
#ifndef _DATABASES_HXX_
#include <provider/databases.hxx>
#endif
#ifndef _RESULTSETFACTORY_HXX
#include <provider/resultsetfactory.hxx>
#endif
#ifndef _RESULTSETBASE_HXX
#include <provider/resultsetbase.hxx>
#endif
#ifndef _RESULTSETFORROOT_HXX
#include <provider/resultsetforroot.hxx>
#endif


using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace rtl;

using namespace chelp;

//=========================================================================
//=========================================================================
//
// Content Implementation.
//
//=========================================================================
//=========================================================================

Content::Content( const Reference< XMultiServiceFactory >& rxSMgr,
                  ::ucb::ContentProviderImplHelper* pProvider,
                  const Reference< XContentIdentifier >& Identifier )
    : ContentImplHelper( rxSMgr, pProvider, Identifier ),
      m_aURLParameter( Identifier->getContentIdentifier() )
{
    // m_aProps.aTitle       =
    // m_aprops.aContentType =
    // m_aProps.bIsDocument  =
    // m_aProps.bIsFolder    =
}

//=========================================================================
// virtual
Content::~Content()
{
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

// virtual
void SAL_CALL Content::acquire()
    throw( RuntimeException )
{
    ContentImplHelper::acquire();
}

//=========================================================================
// virtual
void SAL_CALL Content::release()
    throw( RuntimeException )
{
    ContentImplHelper::release();
}

//=========================================================================
// virtual
Any SAL_CALL Content::queryInterface( const Type & rType )
    throw ( RuntimeException )
{
    Any aRet;
     return aRet.hasValue() ? aRet : ContentImplHelper::queryInterface( rType );
}

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_COMMON_IMPL( Content );

//=========================================================================
// virtual
Sequence< Type > SAL_CALL Content::getTypes()
    throw( RuntimeException )
{
    static OTypeCollection* pCollection = NULL;

    if ( !pCollection )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
          if ( !pCollection )
          {
              static OTypeCollection aCollection(
                CPPU_TYPE_REF( XTypeProvider ),
                   CPPU_TYPE_REF( XServiceInfo ),
                   CPPU_TYPE_REF( XComponent ),
                   CPPU_TYPE_REF( XContent ),
                   CPPU_TYPE_REF( XCommandProcessor ),
                   CPPU_TYPE_REF( XPropertiesChangeNotifier ),
                   CPPU_TYPE_REF( XCommandInfoChangeNotifier ),
                   CPPU_TYPE_REF( XPropertyContainer ),
                   CPPU_TYPE_REF( XPropertySetInfoChangeNotifier ),
                   CPPU_TYPE_REF( XChild ) );
              pCollection = &aCollection;
        }
    }

    return (*pCollection).getTypes();
}

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

// virtual
OUString SAL_CALL Content::getImplementationName()
    throw( RuntimeException )
{
    return OUString::createFromAscii( "CHelpContent" );
}

//=========================================================================
// virtual
Sequence< OUString > SAL_CALL Content::getSupportedServiceNames()
    throw( RuntimeException )
{
    Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[ 0 ]
            = OUString::createFromAscii( MYUCP_CONTENT_SERVICE_NAME );
    return aSNS;
}

//=========================================================================
//
// XContent methods.
//
//=========================================================================

// virtual
OUString SAL_CALL Content::getContentType()
    throw( RuntimeException )
{
    return OUString::createFromAscii( MYUCP_CONTENT_TYPE );
}

//=========================================================================
//
// XCommandProcessor methods.
//
//=========================================================================

//virtual
void SAL_CALL Content::abort( sal_Int32 CommandId )
    throw( RuntimeException )
{
}



class ResultSetForRootFactory
    : public ResultSetFactory
{
private:

    Reference< XMultiServiceFactory >               m_xSMgr;
    Reference< XContentProvider >                   m_xProvider;
    sal_Int32                                       m_nOpenMode;
    Sequence< Property >                            m_seq;
    Sequence< NumberedSortingInfo >                 m_seqSort;
    URLParameter                                    m_aURLParameter;


public:

    ResultSetForRootFactory( const Reference< XMultiServiceFactory >& xSMgr,
                             const Reference< XContentProvider >&  xProvider,
                             sal_Int32 nOpenMode,
                             const Sequence< Property >& seq,
                             const Sequence< NumberedSortingInfo >& seqSort,
                             URLParameter aURLParameter )
        : m_xSMgr( xSMgr ),
          m_xProvider( xProvider ),
          m_nOpenMode( nOpenMode ),
          m_seq( seq ),
          m_seqSort( seqSort ),
          m_aURLParameter( aURLParameter )
    {
    }

    ResultSetBase* createResultSet()
    {
        return new ResultSetForRoot( m_xSMgr,
                                     m_xProvider,
                                     m_nOpenMode,
                                     m_seq,
                                     m_seqSort,
                                     m_aURLParameter );
    }
};



// virtual
Any SAL_CALL Content::execute( const Command& aCommand,
                                sal_Int32 CommandId,
                                const Reference<
                               XCommandEnvironment >& Environment )
    throw( Exception, CommandAbortedException, RuntimeException )
{
    Any aRet;

    if ( aCommand.Name.compareToAscii( "getPropertyValues" ) == 0 )
    {
        Sequence< Property > Properties;
        if ( !( aCommand.Argument >>= Properties ) )
        {
            VOS_ENSURE( sal_False, "Wrong argument type!" );
            return aRet;
        }

        aRet <<= getPropertyValues( Properties );
    }
    else if ( aCommand.Name.compareToAscii( "setPropertyValues" ) == 0 )
    {
        // No properties can be set
    }
    else if ( aCommand.Name.compareToAscii( "getPropertySetInfo" ) == 0 )
    {
        // Note: Implemented by base class.
        aRet <<= getPropertySetInfo( Environment );
    }
    else if ( aCommand.Name.compareToAscii( "getCommandInfo" ) == 0 )
    {
        // Note: Implemented by base class.
        aRet <<= getCommandInfo( Environment );
    }
    else if ( aCommand.Name.compareToAscii( "open" ) == 0 )
    {
          OpenCommandArgument2 aOpenCommand;
          if ( !( aCommand.Argument >>= aOpenCommand ) )
        {
            VOS_ENSURE( sal_False,
                        "Content::execute - invalid parameter!" );
            throw CommandAbortedException();
        }

        if( m_aURLParameter.isRoot() )
        {
            Reference< XDynamicResultSet > xSet
                = new DynamicResultSet( m_xSMgr,
                                        this,
                                        aOpenCommand,
                                        Environment,
                                        new ResultSetForRootFactory( m_xSMgr,
                                                                     m_xProvider.getBodyPtr(),
                                                                     aOpenCommand.Mode,
                                                                     aOpenCommand.Properties,
                                                                     aOpenCommand.SortingInfo,
                                                                     m_aURLParameter ) );
            aRet <<= xSet;
        }

        if( m_aURLParameter.isQuery() )
        {

        }
    }
    else
    {
        //////////////////////////////////////////////////////////////////
        // Unsupported command
        //////////////////////////////////////////////////////////////////

        VOS_ENSURE( sal_False, "Content::execute - unsupported command!" );
        throw CommandAbortedException();
    }

    return aRet;
}




//=========================================================================
Reference< XRow > Content::getPropertyValues(
                                const Sequence< Property >& rProperties )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    return Reference< XRow >(0);
}
