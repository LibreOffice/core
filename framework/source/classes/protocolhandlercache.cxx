/*************************************************************************
 *
 *  $RCSfile: protocolhandlercache.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-04 17:15:05 $
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

#include <classes/protocolhandlercache.hxx>
#include <classes/wildcard.hxx>
#include <classes/converter.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>
#include <threadhelp/lockhelper.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef UNOTOOLS_CONFIGPATHES_HXX_INCLUDED
#include <unotools/configpathes.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  non exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  non exported definitions
//_________________________________________________________________________________________________________________

/**
    @short      overloaded index operator of hash map to support pattern key search
    @descr      All keys inside this hash map are URL pattern which points to an uno
                implementation name of a protocol handler service which is registered
                for this pattern. This operator makes it easy to find such registered
                handler by using a full qualified URL and compare it with all pattern
                keys.

    @param      sURL
                the full qualified URL which should match to a registered pattern

    @return     An iterator which points to the found item inside the hash or PatternHash::end()
                if no pattern match this given <var>sURL</var>.

    @modified   30.04.2002 09:52, as96863
 */
PatternHash::iterator PatternHash::findPatternKey( const ::rtl::OUString& sURL )
{
    PatternHash::iterator pItem = this->begin();
    while( pItem!=this->end() )
    {
        if (Wildcard::match(sURL,pItem->first))
            break;
        ++pItem;
    }
    return pItem;
}

//_________________________________________________________________________________________________________________

/**
    @short      initialize static member of class HandlerCache
    @descr      We use a singleton pattern to implement this handler cache.
                That means it use two static member list to hold all neccessary informations
                and a ref count mechanism to create/destroy it on demand.

    @modified   30.04.2002 11:13, as96863
 */
HandlerHash* HandlerCache::m_pHandler  = NULL;
PatternHash* HandlerCache::m_pPattern  = NULL;
sal_Int32    HandlerCache::m_nRefCount = 0   ;

//_________________________________________________________________________________________________________________

/**
    @short      ctor of the cache of all registered protoco handler
    @descr      It tries to open the right configuration package automaticly
                and fill the internal structures. After that the cache can be
                used for read access on this data and perform some search
                operations on it.

    @modified   30.04.2002 10:02, as96863
 */
HandlerCache::HandlerCache()
{
    /* SAFE */{
        WriteGuard aGlobalLock( LockHelper::getGlobalLock() );

        if (m_nRefCount==0)
        {
            m_pHandler = new HandlerHash();
            m_pPattern = new PatternHash();

            HandlerCFGAccess aConfig(PACKAGENAME_PROTOCOLHANDLER);
            aConfig.read(&m_pHandler,&m_pPattern);
        }

        ++m_nRefCount;
    /* SAFE */}
}

//_________________________________________________________________________________________________________________

/**
    @short      dtor of the cache
    @descr      It frees all used memory. In further implementations (may if we support write access too)
                it's a good place to flush changes back to the configuration - but not needed yet.

    @modified   30.04.2002 09:54, as96863
 */
HandlerCache::~HandlerCache()
{
    /* SAFE */{
        WriteGuard aGlobalLock( LockHelper::getGlobalLock() );

        if( m_nRefCount==1)
        {
            m_pHandler->free();
            m_pPattern->free();

            delete m_pHandler;
            delete m_pPattern;
            m_pHandler=NULL;
            m_pPattern=NULL;
        }

        --m_nRefCount;
    /* SAFE */}
}

//_________________________________________________________________________________________________________________

/**
    @short      dtor of the cache
    @descr      It frees all used memory. In further implementations (may if we support write access too)
                it's a good place to flush changes back to the configuration - but not needed yet.

    @modified   30.04.2002 09:54, as96863
 */
sal_Bool HandlerCache::search( const ::rtl::OUString& sURL, ProtocolHandler* pReturn ) const
{
    sal_Bool bFound = sal_False;
    /* SAFE */{
        ReadGuard aReadLock( LockHelper::getGlobalLock() );
        PatternHash::const_iterator pItem = m_pPattern->findPatternKey(sURL);
        if (pItem!=m_pPattern->end())
        {
            *pReturn = (*m_pHandler)[pItem->second];
            bFound = sal_True;
        }
    /* SAFE */}
    return bFound;
}

//_________________________________________________________________________________________________________________

/**
    @short      search for a registered handler by using an URL struct
    @descr      We combine neccessary parts of this struct to a valid URL string
                and call our other search method ...
                It's a helper for outside code.

    @modified   30.04.2002 09:54, as96863
 */
sal_Bool HandlerCache::search( const css::util::URL& aURL, ProtocolHandler* pReturn ) const
{
    return search( aURL.Complete, pReturn );
}

//_________________________________________________________________________________________________________________

/**
    @short      dtor of the config access class
    @descr      It opens the configuration package automaticly by using base class mechanism.
                After that "read()" method of this class should be called to use it.

    @param      sPackage
                specifies the package name of the configuration data which should be used

    @modified   30.04.2002 10:06, as96863
 */
HandlerCFGAccess::HandlerCFGAccess( const ::rtl::OUString& sPackage )
    : ConfigItem( sPackage )
{
}

//_________________________________________________________________________________________________________________

/**
    @short      use base class mechanism to fill given structures
    @descr      User use us as a wrapper between configuration api and his internal structures.
                He give us some pointer to his member and we fill it.

    @param      pHandler
                pointer to a list of protocol handler infos

    @param      pPattern
                reverse map of handler pattern to her uno names

    @modified   30.04.2002 09:54, as96863
 */
void HandlerCFGAccess::read( HandlerHash** ppHandler ,
                             PatternHash** ppPattern )
{
    // list of all uno implementation names without encoding
    css::uno::Sequence< ::rtl::OUString > lNames = GetNodeNames( SETNAME_HANDLER, ::utl::CONFIG_NAME_LOCAL_PATH );
    sal_Int32 nSourceCount = lNames.getLength();
    sal_Int32 nTargetCount = nSourceCount;
    // list of all full qualified path names of configuration entries
    css::uno::Sequence< ::rtl::OUString > lFullNames ( nTargetCount );

    // expand names to full path names
    sal_Int32 nSource=0;
    sal_Int32 nTarget=0;
    for( nSource=0; nSource<nSourceCount; ++nSource )
    {
        ::rtl::OUString sPath;

        sPath  = SETNAME_HANDLER    ;
        sPath += CFG_PATH_SEPERATOR ;
        sPath += lNames[nSource]    ;
        sPath += CFG_PATH_SEPERATOR ;

        lFullNames[nTarget]  = sPath;
        lFullNames[nTarget] += PROPERTY_PROTOCOLS;
        ++nTarget;
    }

    // get values at all
    css::uno::Sequence< css::uno::Any > lValues = GetProperties( lFullNames );
    LOG_ASSERT2( lFullNames.getLength()!=lValues.getLength(), "HandlerCFGAccess::read()", "Miss some configuration values of handler set!" )

    // fill structures
    nSource = 0;
    for( nTarget=0; nTarget<nTargetCount; ++nTarget )
    {
        // create it new for every loop to guarantee a real empty object!
        ProtocolHandler aHandler;
        aHandler.m_sUNOName = ::utl::extractFirstFromConfigurationPath(lNames[nSource]);

        // unpack all values of this handler
        css::uno::Sequence< ::rtl::OUString > lTemp;
        lValues[nTarget] >>= lTemp;
        aHandler.m_lProtocols = Converter::convert_seqOUString2OUStringList(lTemp);

        // register his pattern into the performance search hash
        for (OUStringList::iterator pItem =aHandler.m_lProtocols.begin();
                                    pItem!=aHandler.m_lProtocols.end()  ;
                                    ++pItem                             )
        {
            (**ppPattern)[*pItem] = lNames[nSource];
        }

        // ínsert the handler info into the normal handler cache
        (**ppHandler)[lNames[nSource]] = aHandler;
        ++nSource;
    }
}

} // namespace framework
