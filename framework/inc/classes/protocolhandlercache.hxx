/*************************************************************************
 *
 *  $RCSfile: protocolhandlercache.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 18:19:34 $
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

#ifndef __FRAMEWORK_CLASSES_PROTOCOLHANDLERCACHE_HXX_
#define __FRAMEWORK_CLASSES_PROTOCOLHANDLERCACHE_HXX_

/** Attention: stl headers must(!) be included at first. Otherwhise it can make trouble
               with solaris headers ...
*/
#include <hash_map>
#include <vector>

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <general.h>
#include <stdtypes.h>
#include <macros/debug.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#include <unotools/configitem.hxx>
#include <rtl/ustring.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

#define PACKAGENAME_PROTOCOLHANDLER                 DECLARE_ASCII("Office.ProtocolHandler"                          )   /// name of our configuration package

#define CFG_PATH_SEPERATOR                          DECLARE_ASCII("/"                                               )   /// seperator for configuration pathes
#define CFG_ENCODING_OPEN                           DECLARE_ASCII("[\'"                                             )   /// used to start encoding of set names
#define CFG_ENCODING_CLOSE                          DECLARE_ASCII("\']"                                             )   /// used to finish encoding of set names

#define SETNAME_HANDLER                             DECLARE_ASCII("HandlerSet"                                      )   /// name of configuration set inside package
#define PROPERTY_PROTOCOLS                          DECLARE_ASCII("Protocols"                                       )   /// properties of a protocol handler

//_________________________________________________________________________________________________________________

/**
    Programmer can register his own services to handle different protocols.
    Don't forget: It doesn't mean "handling of documents" ... these services could handle protocols ...
    e.g. "mailto:*", "file://*", ".java:*
    This struct holds the information about one such registered protocol handler.
    A list of handler objects is defined as ProtocolHandlerHash. see below
*/
struct ProtocolHandler
{
    /* member */
    public:

        /// the uno implementation name of this handler
        ::rtl::OUString m_sUNOName;
        /// list of URL pattern which defines the protocols which this handler is registered for
        OUStringList m_lProtocols;
};

//_________________________________________________________________________________________________________________

/**
    This hash use registered pattern of all protocol handlers as keys and provide her
    uno implementation names as value. Overloading of the index operator makes it possible
    to search for a key by using a full qualified URL on list of all possible pattern keys.
*/
class PatternHash : public BaseHash< ::rtl::OUString >
{
    /* interface */
    public:

        PatternHash::iterator findPatternKey( const ::rtl::OUString& sURL );
};

//_________________________________________________________________________________________________________________

/**
    This hash holds protocol handler structs by her names.
*/
typedef BaseHash< ProtocolHandler > HandlerHash;

//_________________________________________________________________________________________________________________

/**
    @short          this hash makes it easy to find a protocol handler by using his uno implementation name.
    @descr          It holds two lists of informations:
                        - first holds all handler by her uno implementation names and
                          can be used to get her other properties
                        - another one maps her registered pattern to her uno names to
                          perform search on such data
                    But this lists a static for all instances of this class. So it's possible to
                    create new objects without opening configuration twice and free memory automaticly
                    if last object will gone.

    @attention      We implement a singleton concept - so we doesn't need any mutex member here.
                    Because to safe access on static member we must use a static global lock
                    here too.

    @devstatus      ready to use
    @threadsafe     yes

    @modified       30.04.2002 11:19, as96863
*/
class HandlerCache
{
    /* member */
    private:

        /// list of all registered handler registered by her uno implementation names
        static HandlerHash* m_pHandler;
        /// maps URL pattern to handler names
        static PatternHash* m_pPattern;
        /// ref count to construct/destruct internal member lists on demand by using singleton mechanism
        static sal_Int32 m_nRefCount;

    /* interface */
    public:

                 HandlerCache();
        virtual ~HandlerCache();

        sal_Bool search( const ::rtl::OUString& sURL, ProtocolHandler* pReturn ) const;
        sal_Bool search( const css::util::URL&  aURL, ProtocolHandler* pReturn ) const;
};

//_________________________________________________________________________________________________________________

/**
    @short          implements configuration access for handler configuration
    @descr          We use the ConfigItem mechanism to read/write values from/to configuration.
                    We set a data container pointer for filling or reading ... this class use it temp.
                    After successfuly calling of read(), we can use filled container directly or merge it with an existing one.
                    After successfuly calling of write() all values of given data container are flushed to our configuration -
                    but current implementation doesn't support writeing realy.

    @base           ::utl::ConfigItem
                    base mechanism for configuration access

    @devstatus      ready to use
    @threadsafe     no

    @modified       30.04.2002 09:58, as96863
*/
class HandlerCFGAccess : public ::utl::ConfigItem
{
    /* interface */
    public:
                 HandlerCFGAccess( const ::rtl::OUString& sPackage  );
        void     read            (       HandlerHash**    ppHandler ,
                                         PatternHash**    ppPattern );
};

} // namespace framework

#endif // #ifndef __FRAMEWORK_CLASSES_PROTOCOLHANDLERCACHE_HXX_
