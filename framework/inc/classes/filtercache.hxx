/*************************************************************************
 *
 *  $RCSfile: filtercache.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: as $ $Date: 2001-04-05 13:26:56 $
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

#ifndef __FRAMEWORK_THREADHELP_RWLOCKBASE_HXX_
#include <threadhelp/rwlockbase.hxx>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
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

#ifndef _COM_SUN_STAR_CONTAINER_ELEMENTEXISTEXCEPTION_HPP_
#include <com/sun/star/container/ElementExistException.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_NOSUCHELEMENTEXCEPTION_HPP_
#include <com/sun/star/container/NoSuchElementException.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

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

#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
// Hash code function for using in all hash maps of follow implementation.
struct StringHashFunction
{
    size_t operator()(const ::rtl::OUString& sString) const
    {
        return sString.hashCode();
    }
};

//*****************************************************************************************************************
// A generic string list to hold different string informations with a fast access to it.
// Implment new free function to clear memory realy!
//*****************************************************************************************************************
class StringList : public ::std::vector< ::rtl::OUString >
{
    public:
        inline void push_front( const ::rtl::OUString& sElement )
        {
            insert( begin(), sElement );
        }

        inline void free()
        {
            erase( begin(), end() );
            clear();
        }
};

//*****************************************************************************************************************
// These struct define a type, which present the type of a file.
// He is used for easy filter detection without file stream detection!
// The internal name is the keyname of an item with these structure in our hash map or our configuration set!
//*****************************************************************************************************************
struct FileType
{
    //-------------------------------------------------------------------------------------------------------------
    // public methods
    //-------------------------------------------------------------------------------------------------------------
    public:

        inline               FileType   (                               ) { impl_clear();               }
        inline               FileType   (   const   FileType&   rCopy   ) { impl_copy( rCopy );         }
        inline              ~FileType   (                               ) { impl_clear();               }
        inline FileType&    operator=   (   const   FileType&   rCopy   ) { return impl_copy( rCopy );  }
        inline void         free        (                               ) { impl_clear();               }

    //-------------------------------------------------------------------------------------------------------------
    // private methods
    //-------------------------------------------------------------------------------------------------------------
    private:

        inline void impl_clear()
        {
            bPreferred          = sal_False                                         ;
            sName               = ::rtl::OUString()                                 ;
            sUIName             = ::rtl::OUString()                                 ;
            lUINames            = css::uno::Sequence< css::beans::PropertyValue >() ;
            sMediaType          = ::rtl::OUString()                                 ;
            sClipboardFormat    = ::rtl::OUString()                                 ;
            nDocumentIconID     = 0                                                 ;
            lURLPattern.free();
            lExtensions.free();
        }

        inline FileType& impl_copy( const FileType& rCopy )
        {
            bPreferred          = rCopy.bPreferred      ;
            sName               = rCopy.sName           ;
            sUIName             = rCopy.sUIName         ;
            lUINames            = rCopy.lUINames        ;
            sMediaType          = rCopy.sMediaType      ;
            sClipboardFormat    = rCopy.sClipboardFormat;
            nDocumentIconID     = rCopy.nDocumentIconID ;
            lURLPattern         = rCopy.lURLPattern     ;
            lExtensions         = rCopy.lExtensions     ;
            return (*this);
        }

    //-------------------------------------------------------------------------------------------------------------
    // public member
    //-------------------------------------------------------------------------------------------------------------
    public:

        sal_Bool                                        bPreferred          ;
        ::rtl::OUString                                 sName               ;
        ::rtl::OUString                                 sUIName             ;
        css::uno::Sequence< css::beans::PropertyValue > lUINames            ;
        ::rtl::OUString                                 sMediaType          ;
        ::rtl::OUString                                 sClipboardFormat    ;
        sal_Int32                                       nDocumentIconID     ;
        StringList                                      lURLPattern         ;
        StringList                                      lExtensions         ;
};

//*****************************************************************************************************************
// These struct describe a filter which is registered for one type.
// He hold information about services which present the document himself (like a item) and a filter service which
// filter a file in these document.
// The internal name is the keyname of an item with these structure in our hash map or our configuration set!
//*****************************************************************************************************************
struct Filter
{
    //-------------------------------------------------------------------------------------------------------------
    // public methods
    //-------------------------------------------------------------------------------------------------------------
    public:

        inline           Filter     (                           ) { impl_clear();               }
        inline           Filter     (   const   Filter& rCopy   ) { impl_copy( rCopy );         }
        inline          ~Filter     (                           ) { impl_clear();               }
        inline Filter&  operator=   (   const   Filter& rCopy   ) { return impl_copy( rCopy );  }
        inline void     free        (                           ) { impl_clear();               }

    //-------------------------------------------------------------------------------------------------------------
    // private methods
    //-------------------------------------------------------------------------------------------------------------
    private:

        inline void impl_clear()
        {
            sName               = ::rtl::OUString()                                 ;
            sType               = ::rtl::OUString()                                 ;
            sUIName             = ::rtl::OUString()                                 ;
            lUINames            = css::uno::Sequence< css::beans::PropertyValue >() ;
            sDocumentService    = ::rtl::OUString()                                 ;
            sFilterService      = ::rtl::OUString()                                 ;
            nFlags              = 0                                                 ;
            nFileFormatVersion  = 0                                                 ;
            sTemplateName       = ::rtl::OUString()                                 ;
            lUserData.free();
        }

        inline Filter& impl_copy( const Filter& rCopy )
        {
            sName               = rCopy.sName               ;
            sType               = rCopy.sType               ;
            sUIName             = rCopy.sUIName             ;
            lUINames            = rCopy.lUINames            ;
            sDocumentService    = rCopy.sDocumentService    ;
            sFilterService      = rCopy.sFilterService      ;
            nFlags              = rCopy.nFlags              ;
            nFileFormatVersion  = rCopy.nFileFormatVersion  ;
            sTemplateName       = rCopy.sTemplateName       ;
            lUserData           = rCopy.lUserData           ;
            return (*this);
        }

    //-------------------------------------------------------------------------------------------------------------
    // public member
    //-------------------------------------------------------------------------------------------------------------
    public:

        ::rtl::OUString                                     sName               ;
        ::rtl::OUString                                     sType               ;
        ::rtl::OUString                                     sUIName             ;
        css::uno::Sequence< css::beans::PropertyValue >     lUINames            ;
        ::rtl::OUString                                     sDocumentService    ;
        ::rtl::OUString                                     sFilterService      ;
        sal_Int32                                           nFlags              ;
        StringList                                          lUserData           ;
        sal_Int32                                           nFileFormatVersion  ;
        ::rtl::OUString                                     sTemplateName       ;
};

//*****************************************************************************************************************
// Programmer can register his own services for an content detection of different types.
// The implementation or service name of these is the keyname of an item with these structure
// in our hash map or our configuration set!
//*****************************************************************************************************************
struct Detector
{
    //-------------------------------------------------------------------------------------------------------------
    // public methods
    //-------------------------------------------------------------------------------------------------------------
    public:

        inline               Detector   (                               ) { impl_clear();               }
        inline               Detector   (   const   Detector&   rCopy   ) { impl_copy( rCopy );         }
        inline              ~Detector   (                               ) { impl_clear();               }
        inline Detector&    operator=   (   const   Detector&   rCopy   ) { return impl_copy( rCopy );  }
        inline void         free        (                               ) { impl_clear();               }

    //-------------------------------------------------------------------------------------------------------------
    // private methods
    //-------------------------------------------------------------------------------------------------------------
    private:

        inline void impl_clear()
        {
            sName = ::rtl::OUString();
            lTypes.free();
        }

        inline Detector& impl_copy( const Detector& rCopy )
        {
            sName   = rCopy.sName   ;
            lTypes  = rCopy.lTypes  ;
            return (*this);
        }

    //-------------------------------------------------------------------------------------------------------------
    // public member
    //-------------------------------------------------------------------------------------------------------------
    public:

        ::rtl::OUString     sName   ;
        StringList          lTypes  ;
};

//*****************************************************************************************************************
// Programmer can register his own services for loading documents in a frame.
// The implementation or service name of these is the keyname of an item with these structure
// in our hash map or our configuration set!
//*****************************************************************************************************************
struct Loader
{
    //-------------------------------------------------------------------------------------------------------------
    // public methods
    //-------------------------------------------------------------------------------------------------------------
    public:

        inline           Loader     (                           ) { impl_clear();               }
        inline           Loader     (   const   Loader& rCopy   ) { impl_copy( rCopy );         }
        inline          ~Loader     (                           ) { impl_clear();               }
        inline Loader&  operator=   (   const   Loader& rCopy   ) { return impl_copy( rCopy );  }
        inline void     free        (                           ) { impl_clear();               }

    //-------------------------------------------------------------------------------------------------------------
    // private methods
    //-------------------------------------------------------------------------------------------------------------
    private:

        inline void impl_clear()
        {
            sName       = ::rtl::OUString()                                 ;
            sUIName     = ::rtl::OUString()                                 ;
            lUINames    = css::uno::Sequence< css::beans::PropertyValue >() ;
            lTypes.free();
        }

        inline Loader& impl_copy( const Loader& rCopy )
        {
            sName       = rCopy.sName       ;
            sUIName     = rCopy.sUIName     ;
            lUINames    = rCopy.lUINames    ;
            lTypes      = rCopy.lTypes      ;
            return (*this);
        }

    //-------------------------------------------------------------------------------------------------------------
    // public member
    //-------------------------------------------------------------------------------------------------------------
    public:

        ::rtl::OUString                                 sName           ;
        ::rtl::OUString                                 sUIName         ;
        css::uno::Sequence< css::beans::PropertyValue > lUINames        ;
        StringList                                      lTypes          ;
};

//*****************************************************************************************************************
// We need different hash maps for different tables of our configuration management.
// Follow maps convert <names> to <properties> of type, filter, detector, loader
//*****************************************************************************************************************
class FileTypeHash  :   public  ::std::hash_map<    ::rtl::OUString                     ,
                                                    FileType                            ,
                                                    StringHashFunction                  ,
                                                    ::std::equal_to< ::rtl::OUString >  >
{
    public:
        void free()
        {
            erase( begin(), end() );
            clear();
        }
};

class FilterHash    :   public  ::std::hash_map<    ::rtl::OUString                     ,
                                                    Filter                              ,
                                                    StringHashFunction                  ,
                                                    ::std::equal_to< ::rtl::OUString >  >
{
    public:
        void free()
        {
            erase( begin(), end() );
            clear();
        }
};

class DetectorHash  :   public  ::std::hash_map<    ::rtl::OUString                     ,
                                                    Detector                            ,
                                                    StringHashFunction                  ,
                                                    ::std::equal_to< ::rtl::OUString >  >
{
    public:
        void free()
        {
            erase( begin(), end() );
            clear();
        }
};

class LoaderHash    :   public  ::std::hash_map<    ::rtl::OUString                     ,
                                                    Loader                              ,
                                                    StringHashFunction                  ,
                                                    ::std::equal_to< ::rtl::OUString >  >
{
    public:
        void free()
        {
            erase( begin(), end() );
            clear();
        }
};

//*****************************************************************************************************************
// Use these hashes to implement different tables which assign types to frame loader or detect services.
// It's an optimism to find registered services faster!
// The preferred hash maps file extensions to preferred types to find these ones faster.
//*****************************************************************************************************************
class PerformanceHash   :   public  ::std::hash_map<    ::rtl::OUString                     ,
                                                        StringList                          ,
                                                        StringHashFunction                  ,
                                                        ::std::equal_to< ::rtl::OUString >  >
{
    public:
        void free()
        {
            erase( begin(), end() );
            clear();
        }
};

class PreferredHash     :   public  ::std::hash_map<    ::rtl::OUString                     ,
                                                        ::rtl::OUString                     ,
                                                        StringHashFunction                  ,
                                                        ::std::equal_to< ::rtl::OUString >  >
{
    public:
        void free()
        {
            erase( begin(), end() );
            clear();
        }
};

//*****************************************************************************************************************
// Defines "pointers" to items of our hash maps.
//*****************************************************************************************************************
typedef StringList::iterator                                        StringIterator              ;
typedef StringList::const_iterator                                  ConstStringIterator         ;
typedef FileTypeHash::const_iterator                                ConstTypeIterator           ;
typedef FilterHash::const_iterator                                  ConstFilterIterator         ;
typedef DetectorHash::const_iterator                                ConstDetectorIterator       ;
typedef LoaderHash::const_iterator                                  ConstLoaderIterator         ;
typedef PerformanceHash::const_iterator                             ConstPerformanceIterator    ;
typedef PreferredHash::const_iterator                               ConstPreferredIterator      ;
typedef CheckedIterator< StringList >                               CheckedStringListIterator   ;
typedef CheckedIterator< FileTypeHash >                             CheckedTypeIterator         ;

/*-************************************************************************************************************//**
    @short          cache for all filter and type information
    @descr          Frameloader- and filterfactory need some informations about our current registered filters and types.
                    For better performance its neccessary to cache all needed values.

    @implements     -
    @base           FairRWLockBase
                    ConfigItem

    @devstatus      ready to use
*//*-*************************************************************************************************************/

class FilterCache   :   private FairRWLockBase
                    ,   public  ::utl::ConfigItem
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
            @descr      This will initialize the cache automaticly ... but at first call only!
                        These class use a refcount mechanism to share cache between different ownern.

            @seealso    -

            @param      -
            @return     -

            @onerror    An assertion is thrown and the cache will be empty!
                        Method isValid() returns false then.
        *//*-*****************************************************************************************************/

         FilterCache();

        /*-****************************************************************************************************//**
            @short      standard destructor to delete instance
            @descr      This will clear the cache if last owner release it.

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual ~FilterCache();

        /*-****************************************************************************************************//**
            @short      called for notify of configmanager
            @descr      These method is called from the ConfigManager before application ends or from the
                         PropertyChangeListener if the sub tree broadcasts changes. You must update your
                        internal values.

            @seealso    baseclass ConfigItem

            @param      "lPropertyNames" is the list of properties which should be updated.
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void Notify( const css::uno::Sequence< ::rtl::OUString >& lPropertyNames );

        /*-****************************************************************************************************//**
            @short      write changes to configuration
            @descr      These method writes the changed values into the sub tree
                        and should always called in our destructor to guarantee consistency of config data.

            @seealso    baseclass ConfigItem

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void Commit();

        /*-****************************************************************************************************//**
            @short      get the current state of the cache
            @descr      Call this methods to get information about the state of the current cache.

            @seealso    -

            @param      -
            @return     -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        sal_Bool isValid        () const;
        sal_Bool hasTypes       () const;
        sal_Bool hasFilters     () const;
        sal_Bool hasDetectors   () const;
        sal_Bool hasLoaders     () const;

        /*-****************************************************************************************************//**
            @short      search routines to find items which match given parameter
            @descr      Mostly we search for a type first and get all informations about filter, detector and loader
                        services from the other configuration tables which are registered for this type.
                        These operations support a FindFirst/Next mechanism.
                        If you call search...( ... nStartEntry=0 ... ) we search for
                        the first entry. If these return a value different from <empty> you can work with these value.
                        If found value isn't the right one - you can call search method again.
                        DONT'T CHANGE THE VALUE OF "rStartEntry" between two search calls!
                        You can use returned value as parameter for getBy...Name() functions of this implementation too!

            @attention  returned type name is an internal name
                        returned filter name is an internal name
                        returned loader name is an implementation name of a service
                        returned detector name is an implementation name of a service

            @seealso    -

            @param      "sResult", name of found type, filter, ...
            @return     true, if search was successful,
                        false, otherwise.

            @onerror    We return false.
        *//*-*****************************************************************************************************/

        sal_Bool searchType             (   const   ::rtl::OUString&            sURL                ,
                                            const   ::rtl::OUString*            pMediaType          ,
                                            const   ::rtl::OUString*            pClipboardFormat    ,
                                                    CheckedTypeIterator&        aStartEntry         ,
                                                    ::rtl::OUString&            sResult             ) const;

        sal_Bool searchFilterForType    (   const   ::rtl::OUString&            sInternalTypeName   ,
                                                       CheckedStringListIterator&   aStartEntry         ,
                                                    ::rtl::OUString&            sResult             ) const;

        sal_Bool searchDetectorForType  (   const   ::rtl::OUString&            sInternalTypeName   ,
                                                       CheckedStringListIterator&   aStartEntry         ,
                                                    ::rtl::OUString&            sResult             ) const;

        sal_Bool searchLoaderForType    (   const   ::rtl::OUString&            sInternalTypeName   ,
                                                       CheckedStringListIterator&   aStartEntry         ,
                                                    ::rtl::OUString&            sResult             ) const;

        /*-****************************************************************************************************//**
            @short      get all properties of a cache entry by given name
            @descr      If you need additional informations about our internal cache values
                        you can use these methods to get a list of all cached config values
                        and subkeys of specified entry.

            @seealso    -

            @param      "sName", name of suspected entry in cache
            @return     A structure with valid information if item exists! An empty Any otherwise.

            @onerror    We return an empty Any.
        *//*-*****************************************************************************************************/

        css::uno::Sequence< ::rtl::OUString >               getAllTypeNames         (                                       ) const;
        css::uno::Sequence< ::rtl::OUString >               getAllFilterNames       (                                       ) const;
        css::uno::Sequence< ::rtl::OUString >               getAllDetectorNames     (                                       ) const;
        css::uno::Sequence< ::rtl::OUString >               getAllLoaderNames       (                                       ) const;

        css::uno::Sequence< css::beans::PropertyValue >     getTypeProperties       (   const   ::rtl::OUString&    sName   ) const;
        css::uno::Sequence< css::beans::PropertyValue >     getFilterProperties     (   const   ::rtl::OUString&    sName   ) const;
        css::uno::Sequence< css::beans::PropertyValue >     getDetectorProperties   (   const   ::rtl::OUString&    sName   ) const;
        css::uno::Sequence< css::beans::PropertyValue >     getLoaderProperties     (   const   ::rtl::OUString&    sName   ) const;

        FileType                                            getType                 (   const   ::rtl::OUString&    sName   ) const;
        Filter                                              getFilter               (   const   ::rtl::OUString&    sName   ) const;
        Detector                                            getDetector             (   const   ::rtl::OUString&    sName   ) const;
        Loader                                              getLoader               (   const   ::rtl::OUString&    sName   ) const;

        sal_Bool                                            existsType              (   const   ::rtl::OUString&    sName   ) const;
        sal_Bool                                            existsFilter            (   const   ::rtl::OUString&    sName   ) const;
        sal_Bool                                            existsDetector          (   const   ::rtl::OUString&    sName   ) const;
        sal_Bool                                            existsLoader            (   const   ::rtl::OUString&    sName   ) const;

        /*-****************************************************************************************************//**
            @short      support special query modes
            @descr      Our owner services need sometimes a special mode to query for subsets of our configuration!
                        They give us a special query string - we return right values.

            @seealso    file queries.h
            @seealso    class FilterFactory
            @seealso    class FrameLoaderFactory
            @seealso    class TypeDetection

            @param      "sName", name of query
            @return     A structure with valid information!

            @onerror    We return an empty result set.
        *//*-*****************************************************************************************************/

        css::uno::Any queryFilters( const ::rtl::OUString& sQuery ) const;

        /*-****************************************************************************************************//**
            @short      support registration of elements in current configuration
            @descr      Use this methods to add or remove items in our configuration files.
                        We use the globale configuration to do that ... in fat office "share/config/registry/..."!

                        *** structure of type properties **********************************************************

                            PropertyValue.Name                  PropertyValue.Value                 Description
                            ---------------------------------------------------------------------------------------
                            ...

                        *** structure of filter properties ********************************************************

                            PropertyValue.Name                  PropertyValue.Value                 Description
                            ---------------------------------------------------------------------------------------
                            "Name"                              [string]                            internal name
                            "Type"                              [string]                            registered for these type
                            "UIName"                            [string]                            localized name for UI (valid for current locale at runtime!)
                            "UINames"                           [stringlist]                        assignment of all supported localized names to right locales
                            "DocumentService"                   [string]                            uno servicename of document services
                            "FilterService"                     [string]                            uno servicename of filter implementation
                            "Flags"                             [long]                              describe filter
                            "UserData"                          [stringlist]                        additional user data (format not fixed!)
                            "FileFormatVersion"                 [long]                              version numbher of supported files
                            "TemplateName"                      [string]                            name of template

                        *** structure of detector properties ******************************************************

                            PropertyValue.Name                  PropertyValue.Value                 Description
                            ---------------------------------------------------------------------------------------
                            ...

                        *** structure of loader properties ********************************************************

                            PropertyValue.Name                  PropertyValue.Value                 Description
                            ---------------------------------------------------------------------------------------
                            ...

            @seealso    -

            @param      "sName"         , name of type, filter ...
            @param      "lProperties"   , values of new type, filter
            @return     state of operation as bool

            @onerror    We return false then.
        *//*-*****************************************************************************************************/

        void addFilter      (   const   ::rtl::OUString&                                    sName       ,
                                const   css::uno::Sequence< css::beans::PropertyValue >&    lProperties ) throw( css::container::ElementExistException );

        void replaceFilter  (   const   ::rtl::OUString&                                    sName       ,
                                const   css::uno::Sequence< css::beans::PropertyValue >&    lProperties ) throw( css::container::NoSuchElementException );

        void removeFilter   (   const   ::rtl::OUString&                                    sName       ) throw( css::container::NoSuchElementException );

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------

    protected:

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

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

        static void impl_convertStringSequenceToVector      (   const   css::uno::Sequence< ::rtl::OUString >&              lSource,    StringList&                                         lDestination    );
        static void impl_convertStringVectorToSequence      (   const   StringList&                                         lSource,    css::uno::Sequence< ::rtl::OUString >&              lDestination    );
        static void impl_convertFileTypeToPropertySequence  (   const   FileType&                                           aSource,    css::uno::Sequence< css::beans::PropertyValue >&    lDestination    );
        static void impl_convertFilterToPropertySequence    (   const   Filter&                                             aSource,    css::uno::Sequence< css::beans::PropertyValue >&    lDestination    );
        static void impl_convertLoaderToPropertySequence    (   const   Loader&                                             aSource,    css::uno::Sequence< css::beans::PropertyValue >&    lDestination    );
        static void impl_convertDetectorToPropertySequence  (   const   Detector&                                           aSource,    css::uno::Sequence< css::beans::PropertyValue >&    lDestination    );
        static void impl_convertPropertySequenceToFilter    (   const   css::uno::Sequence< css::beans::PropertyValue >&    lSource,    Filter&                                             aDestination    );

        /*-****************************************************************************************************//**
            @short      extract extension from given URL
            @descr      For our type detection we must search for matching registered extensions with given URL.

            @seealso    search methods

            @param      "sURL", URL to extract extension
            @return     The extension or an empty string if URL has no one.

            @onerror    No error should occure.
        *//*-*****************************************************************************************************/

        static ::rtl::OUString impl_extractURLExtension( const ::rtl::OUString& sURL );

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

//obsolete      static void impl_correctExtensions( StringList& lExtensions );

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

        void impl_load              ();
        void impl_save              ();

        void impl_loadTypes         ();
        void impl_loadFilters       ();
        void impl_loadDetectors     ();
        void impl_loadLoaders       ();
        void impl_loadDefaults      ();

        void impl_saveTypes         ();
        void impl_saveFilters       ();
        void impl_saveDetectors     ();
        void impl_saveLoaders       ();
        void impl_saveDefaults      ();

        void impl_addType           (   const   FileType&           aType       );
        void impl_replaceType       (   const   FileType&           aType       );
        void impl_removeType        (   const   ::rtl::OUString&    sName       );

        void impl_addFilter         (   const   Filter&             aFilter     );
        void impl_replaceFilter     (   const   Filter&             aFilter     );
        void impl_removeFilter      (   const   ::rtl::OUString&    sName       );

        void impl_addDetector       (   const   Detector&           aDetector   );
        void impl_replaceDetector   (   const   Detector&           aDetector   );
        void impl_removeDetector    (   const   ::rtl::OUString&    sName       );

        void impl_addLoader         (   const   Loader&             aLoader     );
        void impl_replaceLoader     (   const   Loader&             aLoader     );
        void impl_removeLoader      (   const   ::rtl::OUString&    sName       );

        /*-****************************************************************************************************//**
            @short      support query mode
            @descr      These helper functions returns subsets of our internal cached values.

            @seealso    methods query...()

            @param      -
            @return     A result set if query was successful - empty structures if not.

            @onerror    We return empty structures.
        *//*-*****************************************************************************************************/

        css::uno::Sequence< ::rtl::OUString >   impl_queryFilter_ByDocumentService              (   const   ::rtl::OUString&    sService    ) const;
        css::uno::Sequence< ::rtl::OUString >   impl_queryFilter_ByDocumentService_WithDefault  (   const   ::rtl::OUString&    sService    ) const;
        css::uno::Sequence< ::rtl::OUString >   impl_queryFilter_ByFlags                        (           sal_Int32           nFlags      ) const;

    //-------------------------------------------------------------------------------------------------------------
    //  debug methods
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

        static sal_Bool implcp_searchType                           (   const   ::rtl::OUString&                                    sURL                ,
                                                                        const   ::rtl::OUString*                                    pMediaType          ,
                                                                        const   ::rtl::OUString*                                    pClipboardFormat    ,
                                                                        const   CheckedTypeIterator&                                aStartEntry         ,
                                                                        const   ::rtl::OUString&                                    sResult             );
        static sal_Bool implcp_searchFilterForType                  (   const   ::rtl::OUString&                                    sInternalTypeName   ,
                                                                        const   CheckedStringListIterator&                          aStartEntry         ,
                                                                        const   ::rtl::OUString&                                    sResult             );
        static sal_Bool implcp_searchDetectorForType                (   const   ::rtl::OUString&                                    sInternalTypeName   ,
                                                                        const   CheckedStringListIterator&                          aStartEntry         ,
                                                                        const   ::rtl::OUString&                                    sResult             );
        static sal_Bool implcp_searchLoaderForType                  (   const   ::rtl::OUString&                                    sInternalTypeName   ,
                                                                        const   CheckedStringListIterator&                          aStartEntry         ,
                                                                        const   ::rtl::OUString&                                    sResult             );
        static sal_Bool implcp_getTypeProperties                    (   const   ::rtl::OUString&                                    sName               );
        static sal_Bool implcp_getFilterProperties                  (   const   ::rtl::OUString&                                    sName               );
        static sal_Bool implcp_getDetectorProperties                (   const   ::rtl::OUString&                                    sName               );
        static sal_Bool implcp_getLoaderProperties                  (   const   ::rtl::OUString&                                    sName               );
        static sal_Bool implcp_getType                              (   const   ::rtl::OUString&                                    sName               );
        static sal_Bool implcp_getFilter                            (   const   ::rtl::OUString&                                    sName               );
        static sal_Bool implcp_getDetector                          (   const   ::rtl::OUString&                                    sName               );
        static sal_Bool implcp_getLoader                            (   const   ::rtl::OUString&                                    sName               );
        static sal_Bool implcp_existsType                           (   const   ::rtl::OUString&                                    sName               );
        static sal_Bool implcp_existsFilter                         (   const   ::rtl::OUString&                                    sName               );
        static sal_Bool implcp_existsDetector                       (   const   ::rtl::OUString&                                    sName               );
        static sal_Bool implcp_existsLoader                         (   const   ::rtl::OUString&                                    sName               );
        static sal_Bool implcp_addFilter                            (   const   ::rtl::OUString&                                    sName               ,
                                                                        const   css::uno::Sequence< css::beans::PropertyValue >&    lProperties         );
        static sal_Bool implcp_replaceFilter                        (   const   ::rtl::OUString&                                    sName               ,
                                                                        const   css::uno::Sequence< css::beans::PropertyValue >&    lProperties         );
        static sal_Bool implcp_removeFilter                         (   const   ::rtl::OUString&                                    sName               );
        static sal_Bool implcp_queryFilters                         (   const   ::rtl::OUString&                                    sQuery              );

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
    //-------------------------------------------------------------------------------------------------------------
    private:

        static sal_Int32                m_nRefCount             ;

        static FileTypeHash*            m_pTypeCache            ;
        static FilterHash*              m_pFilterCache          ;
        static DetectorHash*            m_pDetectorCache        ;
        static LoaderHash*              m_pLoaderCache          ;
        static PerformanceHash*         m_pFastFilterCache      ;
        static PerformanceHash*         m_pFastDetectorCache    ;
        static PerformanceHash*         m_pFastLoaderCache      ;
        static PreferredHash*           m_pPreferredTypesCache  ;
        static Detector*                m_pDefaultDetector      ;
        static Loader*                  m_pGenericLoader        ;

};      //  class FilterCache

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_CLASSES_FILTERCACHE_HXX_
