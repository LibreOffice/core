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

#ifndef __FRAMEWORK_CLASSES_FILTERCACHEDATA_HXX_
#define __FRAMEWORK_CLASSES_FILTERCACHEDATA_HXX_

/** Attention: stl headers must(!) be included at first. Otherwhise it can make trouble
               with solaris headers ...
*/
#include <boost/unordered_map.hpp>
#include <vector>
#include <iterator>

#include <classes/checkediterator.hxx>
#include <classes/wildcard.hxx>
#include <classes/converter.hxx>
#include <threadhelp/threadhelpbase.hxx>
#include <macros/xinterface.hxx>
#include <macros/debug.hxx>
#include <general.h>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/XChangesListener.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>

#include <unotools/configitem.hxx>
#include <cppuhelper/weak.hxx>
#include <rtl/ustring.hxx>

namespace framework{

#define DEFAULT_FILTERCACHE_VERSION                 6                                                                   /// these implmentation of FilterCache support different version of TypeDetection.xml! This define the current set default one.

#define DEFAULT_FILTERCACHE_MODE                    CONFIG_MODE_DELAYED_UPDATE | CONFIG_MODE_ALL_LOCALES                                          /// ConfigItems could run in different modes: supported values are ... { CONFIG_MODE_IMMEDIATE_UPDATE, CONFIG_MODE_DELAYED_UPDATE, CONFIG_MODE_ALL_LOCALES }

enum EModifyState
{
    E_UNTOUCHED ,
    E_ADDED     ,
    E_CHANGED   ,
    E_REMOVED
};

enum EFilterPackage
{
    E_STANDARD  ,
    E_ADDITIONAL
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
            sName               = OUString() ;
            sMediaType          = OUString() ;
            sClipboardFormat    = OUString() ;
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
        OUString     sName               ;
        OUStringHashMap     lUINames            ;
        OUString     sMediaType          ;
        OUString     sClipboardFormat    ;
        sal_Int32           nDocumentIconID     ;
        OUStringList        lURLPattern         ;
        OUStringList        lExtensions         ;
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
            sName               = OUString();
            sType               = OUString();
            sDocumentService    = OUString();
            sFilterService      = OUString();
            sUIComponent        = OUString();
            nFlags              = 0                ;
            nFileFormatVersion  = 0                ;
            sTemplateName       = OUString();
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
            sUIComponent        = rCopy.sUIComponent        ;
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
        OUString     sName               ;
        OUString     sType               ;
        OUStringHashMap     lUINames            ;
        OUString     sDocumentService    ;
        OUString     sFilterService      ;
        OUString     sUIComponent        ;
        sal_Int32           nFlags              ;
        OUStringList        lUserData           ;
        sal_Int32           nFileFormatVersion  ;
        OUString     sTemplateName       ;
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
            sName = OUString();
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

        OUString     sName       ;
        OUStringList        lTypes      ;
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
            sName = OUString();
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

        OUString sName       ;
        OUStringHashMap lUINames    ;
        OUStringList    lTypes      ;
};

//*****************************************************************************************************************
// Programmer can register his own services to handle a FileType and intercept dispatches.
// Don't forget: It's not a FrameLoader - it's a ContentHandler! (normaly without any UI)
//*****************************************************************************************************************
struct ContentHandler
{
    //-------------------------------------------------------------------------------------------------------------
    // public methods
    //-------------------------------------------------------------------------------------------------------------
    public:

        inline                   ContentHandler(                              ) { impl_clear();               }
        inline                   ContentHandler( const ContentHandler& rCopy  ) { impl_copy( rCopy );         }
        inline                  ~ContentHandler(                              ) { impl_clear();               }
        inline ContentHandler&   operator=     ( const ContentHandler& rCopy  ) { return impl_copy( rCopy );  }
        inline void              free          (                              ) { impl_clear();               }

    //-------------------------------------------------------------------------------------------------------------
    // private methods
    //-------------------------------------------------------------------------------------------------------------
    private:

        inline void impl_clear()
        {
            sName = OUString();
            lTypes.free();
        }

        inline ContentHandler& impl_copy( const ContentHandler& rCopy )
        {
            sName  = rCopy.sName ;
            lTypes = rCopy.lTypes;
            return (*this);
        }

    //-------------------------------------------------------------------------------------------------------------
    // public member
    //-------------------------------------------------------------------------------------------------------------
    public:

        OUString     sName   ;
        OUStringList        lTypes  ;
};

//*****************************************************************************************************************
// We need different hash maps for different tables of our configuration management.
// Follow maps convert <names> to <properties> of type, filter, detector, loader ...
// and could be used in a generic way
//*****************************************************************************************************************
template< class HashType >
class SetNodeHash : public ::boost::unordered_map< OUString                    ,
                                            HashType                           ,
                                            OUStringHash                  ,
                                            ::std::equal_to< OUString > >
{
    //-------------------------------------------------------------------------------------------------------------
    // interface
    //-------------------------------------------------------------------------------------------------------------
    public:
        //---------------------------------------------------------------------------------------------------------
        // The only way to free ALL memory realy!
        //---------------------------------------------------------------------------------------------------------
        inline void free()
        {
            SetNodeHash().swap( *this );
            lAddedItems.free  ();
            lChangedItems.free();
            lRemovedItems.free();
        }

        //---------------------------------------------------------------------------------------------------------
        // Append changed, added or removed items to special lists
        // Necessary for saving changes
        //---------------------------------------------------------------------------------------------------------
        void appendChange( const OUString& sName  ,
                                 EModifyState     eState );

    //-------------------------------------------------------------------------------------------------------------
    // member
    //-------------------------------------------------------------------------------------------------------------
    public:
        OUStringList  lAddedItems    ;
        OUStringList  lChangedItems  ;
        OUStringList  lRemovedItems  ;
};

//*****************************************************************************************************************
// Use these hashes to implement different tables which assign types to frame loader or detect services.
// It's an optimism to find registered services faster!
// The preferred hash maps file extensions to preferred types to find these ones faster.
//*****************************************************************************************************************
class PerformanceHash   :   public  ::boost::unordered_map<    OUString                     ,
                                                        OUStringList                        ,
                                                        OUStringHash                    ,
                                                        ::std::equal_to< OUString >  >
{
    public:
        //---------------------------------------------------------------------------------------------------------
        //  try to free all used memory REALY!
        //---------------------------------------------------------------------------------------------------------
        inline void free()
        {
            PerformanceHash().swap( *this );
        }

        //---------------------------------------------------------------------------------------------------------
        //  normaly a complete string must match our hash key values ...
        //  But sometimes we need a search by using these key values as pattern!
        //  The in/out parameter "pStepper" is used to return a pointer to found element in hash ...
        //  and could be used for further searches again, which should be started at next element!
        //  We stop search at the end of hash. You can start it again by setting it to the begin by himself.
        //---------------------------------------------------------------------------------------------------------
        inline sal_Bool findPatternKey( const OUString& sSearchValue ,
                                              const_iterator&  pStepper     )
        {
            sal_Bool bFound = sal_False;

            // If this is the forst call - start search on first element.
            // Otherwise start search on further elements!
            if( pStepper != begin() )
            {
                ++pStepper;
            }

            while(
                    ( pStepper != end()     )   &&
                    ( bFound   == sal_False )
                )
            {
                bFound = Wildcard::match( sSearchValue, pStepper->first );
                // If element was found - break loop by setting right return value
                // and don't change "pStepper". He must point to found element!
                // Otherwise step to next one.
                if( bFound == sal_False )
                    ++pStepper;
            }
            return bFound;
        }
};

//*****************************************************************************************************************
// Define easy usable types
//*****************************************************************************************************************
typedef SetNodeHash< FileType >                                     FileTypeHash                ;
typedef SetNodeHash< Filter >                                       FilterHash                  ;
typedef SetNodeHash< Detector >                                     DetectorHash                ;
typedef SetNodeHash< Loader >                                       LoaderHash                  ;
typedef SetNodeHash< ContentHandler >                               ContentHandlerHash          ;
typedef OUStringHashMap                                             PreferredHash               ;
typedef OUStringList                                                OrderList                   ;

typedef CheckedIterator< OUStringList >                             CheckedStringListIterator   ;
typedef CheckedIterator< FileTypeHash >                             CheckedTypeIterator         ;
typedef CheckedIterator< PerformanceHash >                          CheckedPerformanceIterator  ;

typedef ::std::vector< FilterHash::const_iterator >                 FilterQuery                 ;

//*****************************************************************************************************************
// Use private static data container to hold all values of configuration!
//*****************************************************************************************************************
class DataContainer : private ThreadHelpBase
{
    public:

    /** @short  identifies different sets of the TypeDetection configuration package.

        @descr  Most functions on top of this configuration package are the same ...
                but must be executed on different places inside ths configuration structures.
                These enum values can be used ate some interface methods to specify, which
                configuration set should be used.
                Further it must be possible to start the same action for more then one cfg type.
                That's why these values must be interpreted as flags. Means: it's values must be
                in range [2^n]!
      */
    enum ECFGType
    {
        E_TYPE           =  1,
        E_FILTER         =  2,
        E_DETECTSERVICE  =  4,
        E_FRAMELOADER    =  8,
        E_CONTENTHANDLER = 16,

        E_ALL            = E_TYPE | E_FILTER | E_DETECTSERVICE | E_FRAMELOADER | E_CONTENTHANDLER
    };

    public:

        DataContainer();

        void        startListener();
        void        stopListener ();

        sal_Bool    isModified();

        LockHelper& getSyncronizer();
        void        free();

        sal_Bool isValidOrRepairable       () const;
        sal_Bool validateAndRepair         ();
        sal_Bool validateAndRepairTypes    ();
        sal_Bool validateAndRepairFilter   ();
        sal_Bool validateAndRepairDetectors();
        sal_Bool validateAndRepairLoader   ();
        sal_Bool validateAndRepairHandler  ();

        sal_Bool existsType           ( const OUString& sName );
        sal_Bool existsFilter         ( const OUString& sName );
        sal_Bool existsDetector       ( const OUString& sName );
        sal_Bool existsLoader         ( const OUString& sName );
        sal_Bool existsContentHandler ( const OUString& sName );

        void addType              ( const FileType&        aType    , sal_Bool bSetModified );
        void addFilter            ( const Filter&          aFilter  , sal_Bool bSetModified );
        void addDetector          ( const Detector&        aDetector, sal_Bool bSetModified );
        void addLoader            ( const Loader&          aLoader  , sal_Bool bSetModified );
        void addContentHandler    ( const ContentHandler&  aHandler , sal_Bool bSetModified );

        void replaceType          ( const FileType&        aType    , sal_Bool bSetModified );
        void replaceFilter        ( const Filter&          aFilter  , sal_Bool bSetModified );
        void replaceDetector      ( const Detector&        aDetector, sal_Bool bSetModified );
        void replaceLoader        ( const Loader&          aLoader  , sal_Bool bSetModified );
        void replaceContentHandler( const ContentHandler&  aHandler , sal_Bool bSetModified );

        void removeType           ( const OUString& sName    , sal_Bool bSetModified );
        void removeFilter         ( const OUString& sName    , sal_Bool bSetModified );
        void removeDetector       ( const OUString& sName    , sal_Bool bSetModified );
        void removeLoader         ( const OUString& sName    , sal_Bool bSetModified );
        void removeContentHandler ( const OUString& sName    , sal_Bool bSetModified );

        static void             convertFileTypeToPropertySequence          ( const FileType&                                           aSource         ,
                                                                                   css::uno::Sequence< css::beans::PropertyValue >&    lDestination    ,
                                                                             const OUString&                                    sCurrentLocale  );
        static void             convertFilterToPropertySequence            ( const Filter&                                             aSource         ,
                                                                                   css::uno::Sequence< css::beans::PropertyValue >&    lDestination    ,
                                                                             const OUString&                                    sCurrentLocale  );
        static void             convertDetectorToPropertySequence          ( const Detector&                                           aSource         ,
                                                                                   css::uno::Sequence< css::beans::PropertyValue >&    lDestination    );
        static void             convertLoaderToPropertySequence            ( const Loader&                                             aSource         ,
                                                                                   css::uno::Sequence< css::beans::PropertyValue >&    lDestination    ,
                                                                             const OUString&                                    sCurrentLocale  );
        static void             convertContentHandlerToPropertySequence    ( const ContentHandler&                                     aSource         ,
                                                                                   css::uno::Sequence< css::beans::PropertyValue >&    lDestination    );
        static void             convertPropertySequenceToFilter            ( const css::uno::Sequence< css::beans::PropertyValue >&    lSource         ,
                                                                                   Filter&                                             aDestination    ,
                                                                             const OUString&                                    sCurrentLocale  );
        static void             convertPropertySequenceToFileType          ( const css::uno::Sequence< css::beans::PropertyValue >&    lSource         ,
                                                                                   FileType&                                           aDestination    ,
                                                                             const OUString&                                    sCurrentLocale  );
        static void             convertPropertySequenceToDetector          ( const css::uno::Sequence< css::beans::PropertyValue >&    lSource         ,
                                                                                   Detector&                                           aDestination    );
        static void             convertPropertySequenceToLoader            ( const css::uno::Sequence< css::beans::PropertyValue >&    lSource         ,
                                                                                   Loader&                                             aDestination    ,
                                                                             const OUString&                                    sCurrentLocale  );
        static void             convertPropertySequenceToContentHandler    ( const css::uno::Sequence< css::beans::PropertyValue >&    lSource         ,
                                                                                   ContentHandler&                                     aDestination    ,
                                                                             const OUString&                                    sCurrentLocale  );
        static void             extractLocalizedStrings                    ( const OUString&                                    sCurrentLocale  ,
                                                                             const css::uno::Any&                                      aCFGValue       ,
                                                                                   OUStringHashMap&                                       lLocales        );
        static void             packLocalizedStrings                       (       sal_Int16                                           nMode           ,
                                                                             const OUString&                                    sCurrentLocale  ,
                                                                                   css::uno::Any&                                      aCFGValue       ,
                                                                             const OUStringHashMap&                                       lLocales        );
        static OUString  getLocalelizedString                       ( const OUStringHashMap&                                       lLocales        ,
                                                                             const OUString&                                    sLocale         );
        static void             setLocalelizedString                       (       OUStringHashMap&                                       lLocales        ,
                                                                             const OUString&                                    sLocale         ,
                                                                             const OUString&                                    sValue          );
        static void             correctExtensions                          (       OUStringList&                                       lExtensions     );

    public:

        FileTypeHash            m_aTypeCache                ;     /// hold all information about registered file types
        FilterHash              m_aFilterCache              ;     /// hold all information about registered filters
        DetectorHash            m_aDetectorCache            ;     /// hold all information about registered detect services
        LoaderHash              m_aLoaderCache              ;     /// hold all information about registered loader services
        ContentHandlerHash      m_aContentHandlerCache      ;     /// hold all information about registered content handler services
        PerformanceHash         m_aFastFilterCache          ;     /// hold all registered filter for a special file type
        PerformanceHash         m_aFastDetectorCache        ;     /// hold all registered detect services for a special file type
        PerformanceHash         m_aFastLoaderCache          ;     /// hold all registered loader services for a special file type
        PerformanceHash         m_aFastContentHandlerCache  ;     /// hold all registered content handler services for a special file type
        PreferredHash           m_aPreferredTypesCache      ;     /// assignment of extensions to preferred types for it
        Loader                  m_aGenericLoader            ;     /// information about our default frame loader
        OUString         m_sLocale                   ;     /// current set locale of configuration to handle right UIName from set of all UINames!
        sal_Bool                m_bTypesModified            ;
        sal_Bool                m_bFiltersModified          ;
        sal_Bool                m_bDetectorsModified        ;
        sal_Bool                m_bLoadersModified          ;
        sal_Bool                m_bHandlersModified         ;
};

/*-************************************************************************************************************//**
    @short          capsulate configuration access for fiter configuration
    @descr          We use the ConfigItem mechanism to read/write values from/to configuration.
                    This implementation could be used to handle standard AND additional filter configurations in the same way.
                    We set a data container pointer for filling or reading ... this class use it temp.
                    After successfully calling of read(), we can use filled container directly or merge it with an existing one.
                    After successfully calling of write() all values of given data container are flushed to our configuration.

    @implements     -
    @base           ConfigItem

    @devstatus      ready to use
    @threadsafe     no
*//*-*************************************************************************************************************/
class FilterCFGAccess : public ::utl::ConfigItem
{
    //-------------------------------------------------------------------------------------------------------------
    //  interface
    //-------------------------------------------------------------------------------------------------------------
    public:
                                    FilterCFGAccess ( const OUString& sPath                                  ,
                                                            sal_Int32        nVersion = DEFAULT_FILTERCACHE_VERSION ,
                                                            sal_Int16        nMode    = DEFAULT_FILTERCACHE_MODE    ); // open configuration
        virtual                     ~FilterCFGAccess(                                                               );

        void                        read            (       DataContainer&   rData                                  ,
                                                            DataContainer::ECFGType         eType                   ); // read values from configuration into given struct
        void                        write           (       DataContainer&   rData                                  ,
                                                            DataContainer::ECFGType         eType                   ); // write values from given struct to configuration

        static   OUString    encodeTypeData  ( const FileType&        aType                                  ); // build own formated string of type properties
        static   void               decodeTypeData  ( const OUString& sData                                  ,
                                                            FileType&        aType                                  );
        static   OUString    encodeFilterData( const Filter&          aFilter                                ); // build own formated string of filter properties
        static   void               decodeFilterData( const OUString& sData                                  ,
                                                            Filter&          aFilter                                );
        static   OUString    encodeStringList( const OUStringList&    lList                                  ); // build own formated string of OUStringList
        static   OUStringList       decodeStringList( const OUString& sValue                                 );

        void             setProductName                             (       OUStringHashMap&                                       lUINames        );
        void             resetProductName                           (       OUStringHashMap&                                       lUINames        );

    //-------------------------------------------------------------------------------------------------------------
    //  internal helper
    //-------------------------------------------------------------------------------------------------------------
    private:
        void impl_initKeyCounts        (                                            );    // set right key counts, which are used at reading/writing of set node properties
        void impl_removeNodes          (       OUStringList&        rChangesList    ,     // helper to remove list of set nodes
                                         const OUString&     sTemplateType   ,
                                         const OUString&     sSetName        );

        void impl_loadTypes            ( DataContainer&             rData           );    // helper to load configuration parts
        void impl_loadFilters          ( DataContainer&             rData           );
        void impl_loadDetectors        ( DataContainer&             rData           );
        void impl_loadLoaders          ( DataContainer&             rData           );
        void impl_loadContentHandlers  ( DataContainer&             rData           );
        void impl_loadDefaults         ( DataContainer&             rData           );

        void impl_saveTypes            ( DataContainer&             rData           );    // helper to save configuration parts
        void impl_saveFilters          ( DataContainer&             rData           );
        void impl_saveDetectors        ( DataContainer&             rData           );
        void impl_saveLoaders          ( DataContainer&             rData           );
        void impl_saveContentHandlers  ( DataContainer&             rData           );

    //-------------------------------------------------------------------------------------------------------------
    //  debug checks
    //-------------------------------------------------------------------------------------------------------------
    private:
        static sal_Bool implcp_ctor ( const OUString& sPath    ,     // methods to check incoming parameter on our interface methods!
                                            sal_Int32        nVersion ,
                                            sal_Int16        nMode    );
        static sal_Bool implcp_read ( const DataContainer&   rData    );
        static sal_Bool implcp_write( const DataContainer&   rData    );

    //-------------------------------------------------------------------------------------------------------------
    //  member
    //-------------------------------------------------------------------------------------------------------------
    private:
        EFilterPackage  m_ePackage                     ;   // ... not realy used yet! should split configuration in STANDARD and ADDITIONAL filter
        sal_Int32       m_nVersion                     ;   // file format version of configuration! (necessary for "xml2xcd" transformation!)
        sal_Int32       m_nKeyCountTypes               ;   // follow key counts present count of configuration properties for types/filters ... and depends from m_nVersion - must be set right!
        sal_Int32       m_nKeyCountFilters             ;
        sal_Int32       m_nKeyCountDetectors           ;
        sal_Int32       m_nKeyCountLoaders             ;
        sal_Int32       m_nKeyCountContentHandlers     ;
        OUString m_sProductName                 ;
        OUString m_sFormatVersion               ;
};

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_CLASSES_FILTERCACHEDATA_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
