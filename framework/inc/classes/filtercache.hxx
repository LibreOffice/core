/*************************************************************************
 *
 *  $RCSfile: filtercache.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: as $ $Date: 2001-03-20 14:44:46 $
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

#ifndef __FRAMEWORK_CLASSES_FILTERCACHE_HXX_
#define __FRAMEWORK_CLASSES_FILTERCACHE_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_CLASSES_CHECKEDITERATOR_HXX_
#include <classes/checkediterator.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef __SGI_STL_HASH_MAP
#include <hash_map>
#endif

#ifndef __SGI_STL_VECTOR
#include <vector>
#endif

#ifndef __SGI_STL_ITERATOR
#include <iterator>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

#define EXCEPTION                   ::com::sun::star::uno::Exception
#define MUTEX                       ::osl::Mutex
#define OUSTRING                    ::rtl::OUString
#define REFERENCE                   ::com::sun::star::uno::Reference
#define SEQUENCE                    ::com::sun::star::uno::Sequence
#define HASH_MAP                    ::std::hash_map
#define VECTOR                      ::std::vector
#define XREGISTRYKEY                ::com::sun::star::registry::XRegistryKey
#define XMULTISERVICEFACTORY        ::com::sun::star::lang::XMultiServiceFactory
#define PROPERTYVALUE               ::com::sun::star::beans::PropertyValue

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
// Hash code function for using in all hash maps of follow implementation.
struct TStringHashFunction
{
    size_t operator()(const OUSTRING& sString) const
    {
        return sString.hashCode();
    }
};

// A generic string list to hold different string informations with a fast access to it.
typedef VECTOR< OUSTRING > TStringList;

//*****************************************************************************************************************
// These struct define a type, which present the type of a file.
// He is used for easy filter detection without file stream detection!
// The internal name is the keyname of an item with these structure in our hash map or our configuration set!
struct TType
{
    OUSTRING                    sUIName             ;   // empty = ""
    SEQUENCE< PROPERTYVALUE >   lUINames            ;   // empty = {}
    OUSTRING                    sMediaType          ;   // empty = ""
    OUSTRING                    sClipboardFormat    ;   // empty = ""
    TStringList                 lURLPattern         ;   // empty = {}
    TStringList                 lExtensions         ;   // empty = {}
    sal_Int32                   nDocumentIconID     ;   // empty = 0

    inline void clear()
    {
        sUIName             = OUSTRING()                    ;
        lUINames            = SEQUENCE< PROPERTYVALUE >()   ;
        sMediaType          = OUSTRING()                    ;
        sClipboardFormat    = OUSTRING()                    ;
        nDocumentIconID     = 0                             ;
        lURLPattern.clear();
        lExtensions.clear();
    }
};

//*****************************************************************************************************************
// These struct describe a filter wich is registered for one type.
// He hold information about services which present the document himself (like a item) and a filter service which
// filter a file in these document.
// The internal name is the keyname of an item with these structure in our hash map or our configuration set!
struct TFilter
{
    OUSTRING                    sType               ;   // empty not allowed!
    OUSTRING                    sUIName             ;   // empty = ""
    SEQUENCE< PROPERTYVALUE >   lUINames            ;   // empty = {}
    OUSTRING                    sDocumentService    ;   // empty = ""
    OUSTRING                    sFilterService      ;   // empty = ""
    sal_Int32                   nFlags              ;   // empty = 0
    TStringList                 lUserData           ;   // empty = {}
    sal_Int32                   nFileFormatVersion  ;   // empty = 0            ... should be moved in UserData ...!?
    OUSTRING                    sTemplateName       ;   // empty = ""           ... should be moved in UserData ...!?

    inline void clear()
    {
        sType               = OUSTRING()                    ;
        sUIName             = OUSTRING()                    ;
        lUINames            = SEQUENCE< PROPERTYVALUE >()   ;
        sDocumentService    = OUSTRING()                    ;
        sFilterService      = OUSTRING()                    ;
        nFlags              = 0                             ;
        nFileFormatVersion  = 0                             ;
        sTemplateName       = OUSTRING()                    ;
        lUserData.clear();
    }
};

//*****************************************************************************************************************
// Programmer can register his own services for an content detection of different types.
// The implementation or service name of these is the keyname of an item with these structure
// in our hash map or our configuration set!
struct TDetector
{
    TStringList     lTypes          ;   // empty not allowed! min 1 item need!

    inline void clear()
    {
        lTypes.clear();
    }
};

//*****************************************************************************************************************
// Programmer can register his own services for loading documents in a frame.
// The implementation or service name of these is the keyname of an item with these structure
// in our hash map or our configuration set!
struct TLoader
{
    OUSTRING                    sUIName         ;   // empty = ""
    SEQUENCE< PROPERTYVALUE >   lUINames        ;   // empty = {}
    TStringList                 lTypes          ;   // empty not allowed! min 1 item need!

    inline void clear()
    {
        sUIName     = OUSTRING()                    ;
        lUINames    = SEQUENCE< PROPERTYVALUE >()   ;
        lTypes.clear();
    }
};

//*****************************************************************************************************************
// We need different hash maps for different tables of our configuration management.
typedef HASH_MAP<   OUSTRING                    ,                       // structure of hash:   key< internal name >{ value< TType > }
                    TType                       ,
                    TStringHashFunction         ,
                    ::std::equal_to< OUSTRING > >   TTypeHash;

typedef HASH_MAP<   OUSTRING                    ,                       // structure of hash:   key< internal name >{ value< TFilter > }
                    TFilter                     ,
                    TStringHashFunction         ,
                    ::std::equal_to< OUSTRING > >   TFilterHash;

typedef HASH_MAP<   OUSTRING                    ,                       // structure of hash:   key< implmentation name >{ value< TDetector > }
                    TDetector                   ,
                    TStringHashFunction         ,
                    ::std::equal_to< OUSTRING > >   TDetectorHash;

typedef HASH_MAP<   OUSTRING                    ,                       // structure of hash:   key< implementation name >{ value< TLoader > }
                    TLoader                     ,
                    TStringHashFunction         ,
                    ::std::equal_to< OUSTRING > >   TLoaderHash;

// Use these hash to implement different table which assign types to frame loader or detect services.
// The normaly used TLoaderHash or TDetectorHash structures assign loader/detectors to types!
// It's an optimism!
typedef HASH_MAP<   OUSTRING                    ,                       // structure of detector hash:  key< internal type name >{ value< list of detector names > }
                    TStringList                 ,                       // structure of loader hash:    key< internal type name >{ value< list of loader names > }
                    TStringHashFunction         ,
                    ::std::equal_to< OUSTRING > >   TPerformanceHash;

typedef HASH_MAP<   OUSTRING                    ,                       // structure of hash:   key< internal name >{ value< TType > }
                    OUSTRING                    ,
                    TStringHashFunction         ,
                    ::std::equal_to< OUSTRING > >   TPreferredHash;     // structure of hash:   key< extension >{ value< internal type name > }

//*****************************************************************************************************************
// Defines "pointers" to items of our hash maps.
typedef TStringList::const_iterator                                 TConstStringIterator        ;
typedef TTypeHash::const_iterator                                   TConstTypeIterator          ;
typedef TFilterHash::const_iterator                                 TConstFilterIterator        ;
typedef TDetectorHash::const_iterator                               TConstDetectorIterator      ;
typedef TLoaderHash::const_iterator                                 TConstLoaderIterator        ;
typedef TPerformanceHash::const_iterator                            TConstPerformanceIterator   ;
typedef TPreferredHash::const_iterator                              TConstPreferredIterator     ;
typedef CheckedIterator< TStringList >                              TCheckedStringListIterator  ;
typedef CheckedIterator< TTypeHash >                                TCheckedTypeIterator        ;

//*****************************************************************************************************************
// describe type of current running office
// used to create right configuration provider in impl_openConfiguration()!
enum EOfficeType
{
    E_FATOFFICE ,
    E_WEBTOP
};

/*-************************************************************************************************************//**
    @short          cache for all filter and type information
    @descr          Fframeloader- and filterfactory need some informations about our current registered filters and types.
                    These keys are not changed during runtime (I hope it ...). For better performance its neccessary to
                    cache all needed values.

    @implements     -
    @base           -

    @devstatus      ready to use
*//*-*************************************************************************************************************/

class FilterCache
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      standard constructor
            @descr      This will initialize the cache automaticly!

            @seealso    -

            @param      -
            @return     -

            @onerror    An assertion is thrown and the cache will be empty!
        *//*-*****************************************************************************************************/

         FilterCache();

        /*-****************************************************************************************************//**
            @short      standard destructor to delete instance
            @descr      This will clear the cache.

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual ~FilterCache();

        /*-****************************************************************************************************//**
            @short      get the current state of the cache
            @descr      Call this method to get information about the state of the current cache.

            @seealso    -

            @param      -
            @return     sal_True  ,if cache is initialzed and work correct
            @return     sal_False ,otherwise

            @onerror    The return value is sal_False.
        *//*-*****************************************************************************************************/

        sal_Bool isValid() const;

        /*-****************************************************************************************************//**
            @short      search routines to find items which match given parameter
            @descr      Mostly we search for a type first and get all informations about filter, detector and loader
                        services from the other configuration tables which are registered for this type.
                        These operations support a FindFirst/Next mechanism.
                        If you call searchFirst...() we initialize "rStartEntry" with a right value and search for
                        the first entry. If these not return NULL you can work with these value.
                        If found value not the right one - you can use "rStartEntry" for search...() methods again.
                        DONT'T CHANGE THE VALUE OF "rStartEntry" between two search calls!

            @attention  returned type name is an internal name
                        returned filter name is an internal name
                        returned loader name is an implementation name of a service
                        returned detector name is an implementation name of a service

            @seealso    -

            @param      -
            @return     A pointer to valid information if search was ok, NULL otherwise.

            @onerror    NULL is returned.
        *//*-*****************************************************************************************************/

        const OUSTRING*     searchFirstType (   const   OUSTRING*               pURL                ,
                                                const   OUSTRING*               pMediaType          ,
                                                const   OUSTRING*               pClipboardFormat    ,
                                                        TCheckedTypeIterator&   rStartEntry         ) const;

        const OUSTRING*     searchType      (   const   OUSTRING*               pURL                ,
                                                const   OUSTRING*               pMediaType          ,
                                                const   OUSTRING*               pClipboardFormat    ,
                                                        TCheckedTypeIterator&   rFollowEntry        ) const;

        const OUSTRING*     searchFirstFilterForType    (   const OUSTRING& sInternalTypeName,  TCheckedStringListIterator& rStartEntry ) const ;
        const OUSTRING*     searchFilterForType         (                                       TCheckedStringListIterator& rFollowEntry) const ;

        const OUSTRING*     searchFirstDetectorForType  (   const OUSTRING& sInternalTypeName,  TCheckedStringListIterator& rStartEntry ) const ;
        const OUSTRING*     searchDetectorForType       (                                       TCheckedStringListIterator& rFollowEntry) const ;

        const OUSTRING*     searchFirstLoaderForType    (   const OUSTRING& sInternalTypeName,  TCheckedStringListIterator& rStartEntry ) const ;
        const OUSTRING*     searchLoaderForType         (                                       TCheckedStringListIterator& rFollowEntry) const ;

        /*-****************************************************************************************************//**
            @short      get all properties of a cache entry by given name
            @descr      If you need additional informations about our internal cache values
                        you can use these methods to get a list of all cached config values
                        and subkeys of specified entry.

            @seealso    -

            @param      "sName", name of suspected entry in cache
            @return     A structure with valid information if item exists, an null pointer otherwise!

            @onerror    A null pointer is returned.
        *//*-*****************************************************************************************************/

        const SEQUENCE< OUSTRING >  getAllTypeNames     () const;
        const SEQUENCE< OUSTRING >  getAllFilterNames   () const;
        const SEQUENCE< OUSTRING >  getAllDetectorNames () const;
        const SEQUENCE< OUSTRING >  getAllLoaderNames   () const;

        const TType*                getTypeByName       ( const OUSTRING& sName ) const;
        const TFilter*              getFilterByName     ( const OUSTRING& sName ) const;
        const TDetector*            getDetectorByName   ( const OUSTRING& sName ) const;
        const TLoader*              getLoaderByName     ( const OUSTRING& sName ) const;

        sal_Bool existsType     ( const OUSTRING& sName ) const;
        sal_Bool existsFilter   ( const OUSTRING& sName ) const;
        sal_Bool existsDetector ( const OUSTRING& sName ) const;
        sal_Bool existsLoader   ( const OUSTRING& sName ) const;

        /*-****************************************************************************************************//**
            @short      support registration of filter in current configuration
            @descr      Use this methods to add or remove filter in our configuration files.
                        We use the globale configuration to do that ... in fat office "share/config/registry/..."!

            @seealso    -

            @param      "sName"         , name of filter
            @param      "lProperties"   , values of new filter
            @return     state of operation as bool

            @onerror    We return false then.
        *//*-*****************************************************************************************************/

        sal_Bool addFilter      (   const   OUSTRING&                   sName       ,
                                    const   SEQUENCE< PROPERTYVALUE >&  lProperties );
        sal_Bool removeFilter   (   const   OUSTRING&                   sName       );

        /*-****************************************************************************************************//**
            @short      convert between internal and external structures
            @descr      We use some vector or self defined structures internal - but get sequences from configuration or must
                        return uno compatible values.
                        Thats the reason for these convert methods.

            @seealso    -

            @param      -
            @return     -

            @onerror    No error should occure.
        *//*-*****************************************************************************************************/

        static void convertStringSequenceToVector       ( const SEQUENCE< OUSTRING >&   seqSource   , TStringList&          rDestination    );
        static void convertStringVectorToSequence       ( const TStringList&            rSource     , SEQUENCE< OUSTRING >& seqDestination  );

        static void convertTTypeToPropertySequence      ( const TType&      rSource, SEQUENCE< PROPERTYVALUE >& seqDestination );
        static void convertTFilterToPropertySequence    ( const TFilter&    rSource, SEQUENCE< PROPERTYVALUE >& seqDestination );
        static void convertTLoaderToPropertySequence    ( const TLoader&    rSource, SEQUENCE< PROPERTYVALUE >& seqDestination );
        static void convertTDetectorToPropertySequence  ( const TDetector&  rSource, SEQUENCE< PROPERTYVALUE >& seqDestination );

        static void convertPropertySequenceToTFilter    ( const SEQUENCE< PROPERTYVALUE >& lSource, TFilter& rDestination );

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------

    protected:

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

        /*-****************************************************************************************************//**
            @short      create a static global mutex to protect our static member!
            @descr      For static member we need normal the Mutex::getGlobalMutex() ...
                        but we use it only one time and create our own static global mutex.
                        These is used to make these class threadsafe.

            @seealso    -

            @param      -
            @return     A reference to created static mutex.

            @onerror    No error should occure.
        *//*-*****************************************************************************************************/

        static MUTEX& impl_getOwnGlobalMutex();

        /*-****************************************************************************************************//**
            @short      extract extension from given URL
            @descr      For our type detection we must search for matching registered extensions with given URL.

            @seealso    search methods

            @param      "sURL", URL to extract extension
            @return     The extension or an empty string if URL has no one.

            @onerror    No error should occure.
        *//*-*****************************************************************************************************/

        OUSTRING impl_extractURLExtension( const OUSTRING& sURL ) const;

        /*-****************************************************************************************************//**
            @short      eliminate "*." from every extension
            @descr      Our configuration save extensions as "*.nnn" everytime. But this isn't a realy effective method
                        to search for mathcing with a given URL. That's why we eleminate this obsolete letters from every
                        saved extension in our internal cache. Now a normal compare is enough!

            @ATTENTION  impl_extractURLExtension() must return strings with same format!

            @seealso    method impl_extractURLExtensions()
            @seealso    search methods

            @param      "lExtension", string vector with extensions to convert
            @return     Parameter "lExtensions" is an in/out parameter!

            @onerror    No error should occure.
        *//*-*****************************************************************************************************/

        void impl_correctExtensions( TStringList& lExtensions );

        /*-****************************************************************************************************//**
            @short      fill our cache with values from configuration
            @descr      We cache the complete type and filter information from our configuration as readonly values.
                        These helper method read the values and fill our static member with it.
                        The different fill-methods are specialized for the different lists of right configuration package!

            @seealso    structure of package "org.openoffice.Office.TypeDetection.xml"

            @param      "xRootKey"  , start point if actual list
            @param      "rCache"    , reference to our static member to fill it values
            @param      "rFastCache", we hold some tables for faster search - fill it too!
            @return     -

            @onerror    If an configuration item couldn't read we ignore it and warn programmer with an assertion.
        *//*-*****************************************************************************************************/

        void impl_loadConfiguration (                                                                                                   );
        void impl_fillTypeCache     ( const REFERENCE< XREGISTRYKEY >& xRootKey, TTypeHash&     rCache                                  );
        void impl_fillFilterCache   ( const REFERENCE< XREGISTRYKEY >& xRootKey, TFilterHash&   rCache  , TPerformanceHash& rFastCache  );
        void impl_fillDetectorCache ( const REFERENCE< XREGISTRYKEY >& xRootKey, TDetectorHash& rCache  , TPerformanceHash& rFastCache  );
        void impl_fillLoaderCache   ( const REFERENCE< XREGISTRYKEY >& xRootKey, TLoaderHash&   rCache  , TPerformanceHash& rFastCache  );

        EOfficeType                         impl_detectOfficeType   ();
        REFERENCE< XMULTISERVICEFACTORY >   impl_openConfiguration  ();

        void impl_addFilterInternal     ( const OUSTRING& sName, const TFilter& aInfo );
        void impl_removeFilterInternal  ( const OUSTRING& sName );

    //-------------------------------------------------------------------------------------------------------------
    //  debug methods
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      debug-method to check incoming parameter of some other mehods of this class
            @descr      The following methods are used to check parameters for other methods
                        of this class. The return value is used directly for an ASSERT(...).

            @seealso    ASSERT in implementation!

            @param      references to checking variables
            @return     sal_False ,on invalid parameter
            @return     sal_True  ,otherwise

            @onerror    -
        *//*-*****************************************************************************************************/

    #ifdef ENABLE_ASSERTIONS

    private:

        static sal_Bool impldbg_checkParameter_searchFirstType                      (   const   OUSTRING*                   pURL                ,
                                                                                        const   OUSTRING*                   pMediaType          ,
                                                                                        const   OUSTRING*                   pClipboardFormat    ,
                                                                                        const   TCheckedTypeIterator&       rStartEntry         );
        static sal_Bool impldbg_checkParameter_searchType                           (   const   OUSTRING*                   pURL                ,
                                                                                        const   OUSTRING*                   pMediaType          ,
                                                                                        const   OUSTRING*                   pClipboardFormat    ,
                                                                                        const   TCheckedTypeIterator&       rFollowEntry        );
        static sal_Bool impldbg_checkParameter_searchFirstFilterForType             (   const   OUSTRING&                   sInternalTypeName   ,
                                                                                        const   TCheckedStringListIterator& rStartEntry         );
        static sal_Bool impldbg_checkParameter_searchFilterForType                  (   const   TCheckedStringListIterator& rFollowEntry        );
        static sal_Bool impldbg_checkParameter_searchFirstDetectorForType           (   const   OUSTRING&                   sInternalTypeName   ,
                                                                                        const   TCheckedStringListIterator& rStartEntry         );
        static sal_Bool impldbg_checkParameter_searchDetectorForType                (   const   TCheckedStringListIterator& rFollowEntry        );
        static sal_Bool impldbg_checkParameter_searchFirstLoaderForType             (   const   OUSTRING&                   sInternalTypeName   ,
                                                                                        const   TCheckedStringListIterator& rStartEntry         );
        static sal_Bool impldbg_checkParameter_searchLoaderForType                  (   const   TCheckedStringListIterator& rFollowEntry        );
        static sal_Bool impldbg_checkParameter_getTypeByName                        (   const   OUSTRING&                   sName               );
        static sal_Bool impldbg_checkParameter_getFilterByName                      (   const   OUSTRING&                   sName               );
        static sal_Bool impldbg_checkParameter_getDetectorByName                    (   const   OUSTRING&                   sName               );
        static sal_Bool impldbg_checkParameter_getLoaderByName                      (   const   OUSTRING&                   sName               );
        static sal_Bool impldbg_checkParameter_existsType                           (   const   OUSTRING&                   sName               );
        static sal_Bool impldbg_checkParameter_existsFilter                         (   const   OUSTRING&                   sName               );
        static sal_Bool impldbg_checkParameter_existsDetector                       (   const   OUSTRING&                   sName               );
        static sal_Bool impldbg_checkParameter_existsLoader                         (   const   OUSTRING&                   sName               );
        static sal_Bool impldbg_checkParameter_convertStringSequenceToVector        (   const   SEQUENCE< OUSTRING >&       seqSource           ,
                                                                                        const   TStringList&                rDestination        );
        static sal_Bool impldbg_checkParameter_convertStringVectorToSequence        (   const   TStringList&                rSource             ,
                                                                                        const   SEQUENCE< OUSTRING >&       seqDestination      );
        static sal_Bool impldbg_checkParameter_convertTTypeToPropertySequence       (   const   TType&                      rSource             ,
                                                                                        const   SEQUENCE< PROPERTYVALUE >&  seqDestination      );
        static sal_Bool impldbg_checkParameter_convertTFilterToPropertySequence     (   const   TFilter&                    rSource             ,
                                                                                        const   SEQUENCE< PROPERTYVALUE >&  seqDestination      );
        static sal_Bool impldbg_checkParameter_convertTLoaderToPropertySequence     (   const   TLoader&                    rSource             ,
                                                                                        const   SEQUENCE< PROPERTYVALUE >&  seqDestination      );
        static sal_Bool impldbg_checkParameter_convertTDetectorToPropertySequence   (   const   TDetector&                  rSource             ,
                                                                                        const   SEQUENCE< PROPERTYVALUE >&  seqDestination      );
        static sal_Bool impldbg_checkParameter_convertPropertySequenceToTFilter     (   const   SEQUENCE< PROPERTYVALUE >&  lSource             ,
                                                                                                TFilter&                    rDestination        );
        static sal_Bool impldbg_checkParameter_addFilter                            (   const   OUSTRING&                   sName               ,
                                                                                        const   SEQUENCE< PROPERTYVALUE >&  lProperties         );
        static sal_Bool impldbg_checkParameter_removeFilter                         (   const   OUSTRING&                   sName               );

    #endif  //  #ifdef ENABLE_ASSERTIONS

        /*-****************************************************************************************************//**
            @short      debug method to log current cache content
            @descr      The stl vector is not suitable enough to show informations about his current content.
                        To get a overview you can call this special debug method. It will log all important informations
                        to a file on disk.

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

    #ifdef ENABLE_FILTERCACHEDEBUG

    public:

        void impldbg_generateHTMLView           ();

        void impl_generateTypeListHTML          ();
        void impl_generateFilterListHTML        ();
        void impl_generateDetectorListHTML      ();
        void impl_generateLoaderListHTML        ();

        void impl_generateInvalidFiltersHTML    ();
        void impl_generateInvalidDetectorsHTML  ();
        void impl_generateInvalidLoadersHTML    ();

        void impl_generateFilterFlagsHTML       ();
        void impl_generateDefaultFiltersHTML    ();

    #endif  //  #ifdef ENABLE_FILTERCACHEDEBUG

    //-------------------------------------------------------------------------------------------------------------
    //  private variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

    private:

        static TTypeHash*           m_pTypeCache            ;
        static TFilterHash*         m_pFilterCache          ;
        static TDetectorHash*       m_pDetectorCache        ;
        static TLoaderHash*         m_pLoaderCache          ;
        static TPerformanceHash*    m_pFastFilterCache      ;
        static TPerformanceHash*    m_pFastDetectorCache    ;
        static TPerformanceHash*    m_pFastLoaderCache      ;
        static TPreferredHash*      m_pPreferredTypesCache  ;
        static sal_Int32            m_nRefCount             ;
        static OUSTRING*            m_pDefaultDetectorName  ;
        static OUSTRING*            m_pGenericLoaderName    ;
        static TDetector*           m_pDefaultDetector      ;
        static TLoader*             m_pGenericLoader        ;

};      //  class FilterCache

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_CLASSES_FILTERCACHE_HXX_
