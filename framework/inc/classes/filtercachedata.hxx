/*************************************************************************
 *
 *  $RCSfile: filtercachedata.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: as $ $Date: 2001-06-05 10:19:00 $
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

#ifndef __FRAMEWORK_CLASSES_FILTERCACHEDATA_HXX_
#define __FRAMEWORK_CLASSES_FILTERCACHEDATA_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_CLASSES_CHECKEDITERATOR_HXX_
#include <classes/checkediterator.hxx>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring.hxx>
#endif

#ifdef ENABLE_GENERATEFILTERCACHE
    #ifndef _RTL_USTRBUF_HXX_
    #include <rtl/ustrbuf.hxx>
    #endif
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

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

#define PACKAGENAME_TYPEDETECTION_STANDARD          DECLARE_ASCII("Office.TypeDetection"                            )
#define PACKAGENAME_TYPEDETECTION_ADDITIONAL        DECLARE_ASCII("Office.TypeDetection"                            )
#define PATHSEPERATOR                               DECLARE_ASCII("/"                                               )
#define PROPERTY_SEPERATOR                          sal_Unicode(',')
#define LIST_SEPERATOR                              sal_Unicode(';')
#define LOCALE_FALLBACK                             DECLARE_ASCII("en-US"                                           )
#define DEFAULT_FILTERCACHE_VERSION                 3

#define NAME_DEFAULTDETECTOR                        DECLARE_ASCII("com.sun.star.comp.office.FilterDetect"           )
#define NAME_GENERICLOADER                          DECLARE_ASCII("com.sun.star.comp.office.FrameLoader"            )
#define UINAME_GENERICLOADER                        DECLARE_ASCII("com.sun.star.comp.office.FrameLoader"            )
#define TYPELIST_DEFAULTDETECTOR                    DECLARE_ASCII("*"                                               )
#define TYPELIST_GENERICLOADER                      DECLARE_ASCII("*"                                               )

#define SUBLIST_TYPES                               DECLARE_ASCII("Types"                                           )
#define SUBLIST_FILTERS                             DECLARE_ASCII("Filters"                                         )
#define SUBLIST_DETECTSERVICES                      DECLARE_ASCII("DetectServices"                                  )
#define SUBLIST_FRAMELOADERS                        DECLARE_ASCII("FrameLoaders"                                    )
#define SUBLIST_DEFAULTS                            DECLARE_ASCII("Defaults"                                        )

#define SUBKEY_PREFERRED                            DECLARE_ASCII("Preferred"                                       )
#define SUBKEY_INSTALLED                            DECLARE_ASCII("Installed"                                       )
#define SUBKEY_NAME                                 DECLARE_ASCII("Name"                                            )
#define SUBKEY_UINAME                               DECLARE_ASCII("UIName"                                          )
#define SUBKEY_MEDIATYPE                            DECLARE_ASCII("MediaType"                                       )
#define SUBKEY_CLIPBOARDFORMAT                      DECLARE_ASCII("ClipboardFormat"                                 )
#define SUBKEY_URLPATTERN                           DECLARE_ASCII("URLPattern"                                      )
#define SUBKEY_EXTENSIONS                           DECLARE_ASCII("Extensions"                                      )
#define SUBKEY_DOCUMENTICONID                       DECLARE_ASCII("DocumentIconID"                                  )
#define SUBKEY_TYPE                                 DECLARE_ASCII("Type"                                            )
#define SUBKEY_DOCUMENTSERVICE                      DECLARE_ASCII("DocumentService"                                 )
#define SUBKEY_FILTERSERVICE                        DECLARE_ASCII("FilterService"                                   )
#define SUBKEY_FLAGS                                DECLARE_ASCII("Flags"                                           )
#define SUBKEY_USERDATA                             DECLARE_ASCII("UserData"                                        )
#define SUBKEY_FILEFORMATVERSION                    DECLARE_ASCII("FileFormatVersion"                               )
#define SUBKEY_TEMPLATENAME                         DECLARE_ASCII("TemplateName"                                    )
#define SUBKEY_TYPES                                DECLARE_ASCII("Types"                                           )
#define SUBKEY_ORDER                                DECLARE_ASCII("Order"                                           )
#define SUBKEY_DEFAULTDETECTOR                      DECLARE_ASCII("DetectService"                                   )
#define SUBKEY_GENERICLOADER                        DECLARE_ASCII("FrameLoader"                                     )
#define SUBKEY_DATA                                 DECLARE_ASCII("Data"                                            )

#define PROPERTY_PREFERRED                          DECLARE_ASCII("Preferred"                                       )
#define PROPERTY_INSTALLED                          DECLARE_ASCII("Installed"                                       )
#define PROPERTY_UINAME                             DECLARE_ASCII("UIName"                                          )
#define PROPERTY_UINAMES                            DECLARE_ASCII("UINames"                                         )
#define PROPERTY_MEDIATYPE                          DECLARE_ASCII("MediaType"                                       )
#define PROPERTY_CLIPBOARDFORMAT                    DECLARE_ASCII("ClipboardFormat"                                 )
#define PROPERTY_URLPATTERN                         DECLARE_ASCII("URLPattern"                                      )
#define PROPERTY_EXTENSIONS                         DECLARE_ASCII("Extensions"                                      )
#define PROPERTY_DOCUMENTICONID                     DECLARE_ASCII("DocumentIconID"                                  )
#define PROPERTY_TYPE                               DECLARE_ASCII("Type"                                            )
#define PROPERTY_DOCUMENTSERVICE                    DECLARE_ASCII("DocumentService"                                 )
#define PROPERTY_FILTERSERVICE                      DECLARE_ASCII("FilterService"                                   )
#define PROPERTY_FLAGS                              DECLARE_ASCII("Flags"                                           )
#define PROPERTY_USERDATA                           DECLARE_ASCII("UserData"                                        )
#define PROPERTY_FILEFORMATVERSION                  DECLARE_ASCII("FileFormatVersion"                               )
#define PROPERTY_TEMPLATENAME                       DECLARE_ASCII("TemplateName"                                    )
#define PROPERTY_TYPES                              DECLARE_ASCII("Types"                                           )

#define PROPCOUNT_TYPE                              8
#define PROPCOUNT_FILTER                            9
#define PROPCOUNT_DETECTOR                          1
#define PROPCOUNT_LOADER                            3

#define SUBKEYCOUNT_TYPE_VERSION_1                  7
#define SUBKEYCOUNT_TYPE_VERSION_2                  7
#define SUBKEYCOUNT_TYPE_VERSION_3                  2
#define SUBKEYCOUNT_FILTER_VERSION_1                9
#define SUBKEYCOUNT_FILTER_VERSION_2                10
#define SUBKEYCOUNT_FILTER_VERSION_3                3
#define SUBKEYCOUNT_DETECTOR                        1
#define SUBKEYCOUNT_LOADER                          2

#define CFGPROPERTY_NODEPATH                        DECLARE_ASCII("nodepath"                                        )   // describe path of cfg entry
#define CFGPROPERTY_LAZYWRITE                       DECLARE_ASCII("lazywrite"                                       )   // true->async. update; false->sync. update
#define CFGPROPERTY_DEPTH                           DECLARE_ASCII("depth"                                           )   // depth of view
#define CFGPROPERTY_NOCACHE                         DECLARE_ASCII("nocache"                                         )   // use cache or not
#define CFGPROPERTY_USER                            DECLARE_ASCII("user"                                            )   // specify user
#define CFGPROPERTY_LOCALE                          DECLARE_ASCII("locale"                                          )   // set locale of cfg entry
#define CFGPROPERTY_SERVERTYPE                      DECLARE_ASCII("servertype"                                      )   // specify type of used configuration (fatoffice, network, webtop)
#define CFGPROPERTY_SOURCEPATH                      DECLARE_ASCII("sourcepath"                                      )   // specify path to "share/config/registry" files
#define CFGPROPERTY_UPDATEPATH                      DECLARE_ASCII("updatepath"                                      )   // specify path to "user/config/registry" files

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

enum EModifyState
{
    E_UNTOUCHED ,
    E_ADDED     ,
    E_CHANGED   ,
    E_REMOVED
};

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
            StringList().swap( *this );
        }
};

class StringHash : public ::std::hash_map<  ::rtl::OUString                     ,
                                            ::rtl::OUString                     ,
                                            StringHashFunction                  ,
                                            ::std::equal_to< ::rtl::OUString >  >
{
    public:
        void free()
        {
            StringHash().swap( *this );
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
            bPreferred          = sal_False         ;
            sName               = ::rtl::OUString() ;
            sMediaType          = ::rtl::OUString() ;
            sClipboardFormat    = ::rtl::OUString() ;
            nDocumentIconID     = 0                 ;
            lUINames.free   ();
            lURLPattern.free();
            lExtensions.free();
        }

        inline FileType& impl_copy( const FileType& rCopy )
        {
            bPreferred          = rCopy.bPreferred      ;
            sName               = rCopy.sName           ;
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

        sal_Bool            bPreferred          ;
        ::rtl::OUString     sName               ;
        StringHash          lUINames            ;
        ::rtl::OUString     sMediaType          ;
        ::rtl::OUString     sClipboardFormat    ;
        sal_Int32           nDocumentIconID     ;
        StringList          lURLPattern         ;
        StringList          lExtensions         ;
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
            nOrder              = 0                ;
            sName               = ::rtl::OUString();
            sType               = ::rtl::OUString();
            sDocumentService    = ::rtl::OUString();
            sFilterService      = ::rtl::OUString();
            nFlags              = 0                ;
            nFileFormatVersion  = 0                ;
            sTemplateName       = ::rtl::OUString();
            lUINames.free   ();
            lUserData.free  ();
        }

        inline Filter& impl_copy( const Filter& rCopy )
        {
            nOrder              = rCopy.nOrder              ;
            sName               = rCopy.sName               ;
            sType               = rCopy.sType               ;
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

        sal_Int32           nOrder              ;
        ::rtl::OUString     sName               ;
        ::rtl::OUString     sType               ;
        StringHash          lUINames            ;
        ::rtl::OUString     sDocumentService    ;
        ::rtl::OUString     sFilterService      ;
        sal_Int32           nFlags              ;
        StringList          lUserData           ;
        sal_Int32           nFileFormatVersion  ;
        ::rtl::OUString     sTemplateName       ;
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
            sName  = rCopy.sName  ;
            lTypes = rCopy.lTypes ;
            return (*this);
        }

    //-------------------------------------------------------------------------------------------------------------
    // public member
    //-------------------------------------------------------------------------------------------------------------
    public:

        ::rtl::OUString     sName       ;
        StringList          lTypes      ;
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
            sName = ::rtl::OUString();
            lUINames.free   ();
            lTypes.free     ();
        }

        inline Loader& impl_copy( const Loader& rCopy )
        {
            sName       = rCopy.sName       ;
            lUINames    = rCopy.lUINames    ;
            lTypes      = rCopy.lTypes      ;
            return (*this);
        }

    //-------------------------------------------------------------------------------------------------------------
    // public member
    //-------------------------------------------------------------------------------------------------------------
    public:

        ::rtl::OUString sName       ;
        StringHash      lUINames    ;
        StringList      lTypes      ;
};

//*****************************************************************************************************************
// This struct is used to collect informations about added, changed or removed cache entries.
//*****************************************************************************************************************
class ModifiedList
{
    public:
        //---------------------------------------------------------------------------------------------------------
        inline void append( const ::rtl::OUString& sName, EModifyState eState )
        {
            switch( eState )
            {
                case E_ADDED   :  lAddedItems.push_back  ( sName );
                                  break;
                case E_CHANGED :  lChangedItems.push_back( sName );
                                  break;
                case E_REMOVED :  lRemovedItems.push_back( sName );
                                  break;
            }
        }

        //---------------------------------------------------------------------------------------------------------
        inline void free()
        {
            lAddedItems.free  ();
            lChangedItems.free();
            lRemovedItems.free();
        }

    public:
        StringList  lAddedItems    ;
        StringList  lChangedItems  ;
        StringList  lRemovedItems  ;
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
        inline void free()
        {
            FileTypeHash().swap( *this );
            lModifiedTypes.free();
        }

    public:
        ModifiedList    lModifiedTypes;
};

//*****************************************************************************************************************
class FilterHash    :   public  ::std::hash_map<    ::rtl::OUString                     ,
                                                    Filter                              ,
                                                    StringHashFunction                  ,
                                                    ::std::equal_to< ::rtl::OUString >  >
{
    public:
        inline void free()
        {
            FilterHash().swap( *this );
            lModifiedFilters.free();
        }

    public:
        ModifiedList    lModifiedFilters;
};

//*****************************************************************************************************************
class DetectorHash  :   public  ::std::hash_map<    ::rtl::OUString                     ,
                                                    Detector                            ,
                                                    StringHashFunction                  ,
                                                    ::std::equal_to< ::rtl::OUString >  >
{
    public:
        inline void free()
        {
            DetectorHash().swap( *this );
            lModifiedDetectors.free();
        }

    public:
        ModifiedList    lModifiedDetectors;
};

//*****************************************************************************************************************
class LoaderHash    :   public  ::std::hash_map<    ::rtl::OUString                     ,
                                                    Loader                              ,
                                                    StringHashFunction                  ,
                                                    ::std::equal_to< ::rtl::OUString >  >
{
    public:
        inline void free()
        {
            LoaderHash().swap( *this );
            lModifiedLoaders.free();
        }

    public:
        ModifiedList    lModifiedLoaders;
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
        inline void free()
        {
            PerformanceHash().swap( *this );
        }
};

//*****************************************************************************************************************
typedef StringHash  PreferredHash;
typedef StringList  OrderList    ;

//*****************************************************************************************************************
// Defines "pointers" to items of our hash maps.
//*****************************************************************************************************************
typedef StringList::iterator                                        StringListIterator          ;
typedef StringList::const_iterator                                  ConstStringListIterator     ;
typedef StringHash::const_iterator                                  ConstStringHashIterator     ;
typedef FileTypeHash::const_iterator                                ConstTypeIterator           ;
typedef FilterHash::const_iterator                                  ConstFilterIterator         ;
typedef DetectorHash::const_iterator                                ConstDetectorIterator       ;
typedef LoaderHash::const_iterator                                  ConstLoaderIterator         ;
typedef PerformanceHash::const_iterator                             ConstPerformanceIterator    ;
typedef PreferredHash::const_iterator                               ConstPreferredIterator      ;
typedef CheckedIterator< StringList >                               CheckedStringListIterator   ;
typedef CheckedIterator< FileTypeHash >                             CheckedTypeIterator         ;

//*****************************************************************************************************************
// Use private static data container to hold all values of configuration!
//*****************************************************************************************************************
class DataContainer
{
    public:
        void free();
        void mergeData( const DataContainer& rData );

        void addType                (   const   FileType&           aType       , sal_Bool bSetModified );
        void addFilter              (   const   Filter&             aFilter     , sal_Bool bSetModified );
        void addDetector            (   const   Detector&           aDetector   , sal_Bool bSetModified );
        void addLoader              (   const   Loader&             aLoader     , sal_Bool bSetModified );

        void replaceType            (   const   FileType&           aType       , sal_Bool bSetModified );
        void replaceFilter          (   const   Filter&             aFilter     , sal_Bool bSetModified );
        void replaceDetector        (   const   Detector&           aDetector   , sal_Bool bSetModified );
        void replaceLoader          (   const   Loader&             aLoader     , sal_Bool bSetModified );

        void removeType             (   const   ::rtl::OUString&    sName       , sal_Bool bSetModified );
        void removeFilter           (   const   ::rtl::OUString&    sName       , sal_Bool bSetModified );
        void removeDetector         (   const   ::rtl::OUString&    sName       , sal_Bool bSetModified );
        void removeLoader           (   const   ::rtl::OUString&    sName       , sal_Bool bSetModified );

        static void             convertStringSequenceToVector      ( const css::uno::Sequence< ::rtl::OUString >&              lSource,    StringList&                                         lDestination    );
        static void             convertStringVectorToSequence      ( const StringList&                                         lSource,    css::uno::Sequence< ::rtl::OUString >&              lDestination    );
        static void             convertFileTypeToPropertySequence  ( const FileType&                                           aSource,    css::uno::Sequence< css::beans::PropertyValue >&    lDestination, const ::rtl::OUString& sCurrentLocale );
        static void             convertFilterToPropertySequence    ( const Filter&                                             aSource,    css::uno::Sequence< css::beans::PropertyValue >&    lDestination, const ::rtl::OUString& sCurrentLocale );
        static void             convertLoaderToPropertySequence    ( const Loader&                                             aSource,    css::uno::Sequence< css::beans::PropertyValue >&    lDestination, const ::rtl::OUString& sCurrentLocale );
        static void             convertDetectorToPropertySequence  ( const Detector&                                           aSource,    css::uno::Sequence< css::beans::PropertyValue >&    lDestination    );
        static void             convertPropertySequenceToFilter    ( const css::uno::Sequence< css::beans::PropertyValue >&    lSource,    Filter&                                             aDestination, const ::rtl::OUString& sCurrentLocale );
        static void             convertStringHashToSequence        ( const StringHash&                                         lSource,    css::uno::Sequence< css::beans::PropertyValue >&    lDestination    );
        static void             convertSequenceToStringHash        ( const css::uno::Sequence< css::beans::PropertyValue >&    lSource,    StringHash&                                         lDestination    );
        static void             extractLocalizedStrings            ( const ::rtl::OUString& sCurrentLocale, const css::uno::Any&   aCFGValue,       StringHash&      lLocales );
        static void             packLocalizedStrings               ( const ::rtl::OUString& sCurrentLocale,       css::uno::Any&   aCFGValue, const StringHash&      lLocales );
        static ::rtl::OUString  getLocalelizedString               ( const StringHash&      lLocales      , const ::rtl::OUString& sLocale                                    );
        static void             setLocalelizedString               (       StringHash&      lLocales      , const ::rtl::OUString& sLocale  , const ::rtl::OUString& sValue   );
        static void             correctExtensions                  (       StringList&      lExtensions                                                                       );

    public:

        FileTypeHash            aTypeCache            ;
        FilterHash              aFilterCache          ;
        DetectorHash            aDetectorCache        ;
        LoaderHash              aLoaderCache          ;
        PerformanceHash         aFastFilterCache      ;
        PerformanceHash         aFastDetectorCache    ;
        PerformanceHash         aFastLoaderCache      ;
        PreferredHash           aPreferredTypesCache  ;
        Detector                aDefaultDetector      ;
        Loader                  aGenericLoader        ;
        ::rtl::OUString         sLocale               ;
        sal_Bool                bIsModified           ;
};

//*****************************************************************************************************************
// We use config item mechanism to read/write values from/to configuration.
// This implementation could be used to handle standard AND additional filter configurations in the same way.
// We set a data container pointer for filling or reading ... this class use it.
// After successfuly calling of read(), we can use filled container directly or merge it with an existing one.
// After successfuly calling of write() all values of given data container are flushed to our configuration.
//*****************************************************************************************************************
enum EConfigType
{
    E_STANDARD  ,
    E_ADDITIONAL
};

#ifdef ENABLE_TIMEMEASURE
//_________________________________________________________________________________________________________________
class FilterCFGAccess   :   private DBGTimeMeasureBase // We need some informations about calling of baseclass "ConfigItem"! :-)
                        ,   public  ::utl::ConfigItem
#else
//_________________________________________________________________________________________________________________
class FilterCFGAccess   :   public  ::utl::ConfigItem
#endif
{
    public:
                                    FilterCFGAccess ( const ::rtl::OUString& sPath                                  ,
                                                            sal_Int32        nVersion = DEFAULT_FILTERCACHE_VERSION ); // open configuration
        virtual                     ~FilterCFGAccess(                                                               );
        void                        read            (       DataContainer&   rData                                  ); // read values from configuration into given struct
        void                        write           (       DataContainer&   rData                                  ); // write values from given struct to configuration
        static   ::rtl::OUString    encodeFilterName( const ::rtl::OUString& sName                                  ); // encode "/" of filter names ... configuration couldn't handle it otherwise
        static   ::rtl::OUString    decodeFilterName( const ::rtl::OUString& sName                                  ); // decode "/" of filter names ...
        static   ::rtl::OUString    encodeTypeData  ( const FileType&        aType                                  ); // build own formated string of type properties
        static   void               decodeTypeData  ( const ::rtl::OUString& sData                                  ,
                                                            FileType&        aType                                  );
        static   ::rtl::OUString    encodeFilterData( const Filter&          aFilter                                ); // build own formated string of filter properties
        static   void               decodeFilterData( const ::rtl::OUString& sData                                  ,
                                                            Filter&          aFilter                                );
        static   ::rtl::OUString    encodeStringList( const StringList&      lList                                  ); // build own formated string of StringList
        static   StringList         decodeStringList( const ::rtl::OUString& sValue                                 );

    private:
        void     impl_loadTypes     ( DataContainer& rData );
        void     impl_loadFilters   ( DataContainer& rData );
        void     impl_loadDetectors ( DataContainer& rData );
        void     impl_loadLoaders   ( DataContainer& rData );
        void     impl_loadDefaults  ( DataContainer& rData );
        void     impl_saveTypes     ( DataContainer& rData );
        void     impl_saveFilters   ( DataContainer& rData );
        void     impl_saveDetectors ( DataContainer& rData );
        void     impl_saveLoaders   ( DataContainer& rData );

    private:
        EConfigType m_eConfigType ; // obsolete? ...
        sal_Int32   m_nVersion    ; // file format version of configuration! (neccessary for "xml2xcd" transformation!)
};

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_CLASSES_FILTERCACHEDATA_HXX_
