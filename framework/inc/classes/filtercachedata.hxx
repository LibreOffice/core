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

#ifndef INCLUDED_FRAMEWORK_INC_CLASSES_FILTERCACHEDATA_HXX
#define INCLUDED_FRAMEWORK_INC_CLASSES_FILTERCACHEDATA_HXX

#include <classes/checkediterator.hxx>
#include <classes/wildcard.hxx>
#include <classes/converter.hxx>
#include <macros/xinterface.hxx>
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

#include <iterator>
#include <unordered_map>
#include <vector>

namespace framework{

#define DEFAULT_FILTERCACHE_VERSION                 6                                                                   /// these implmentation of FilterCache support different version of TypeDetection.xml! This define the current set default one.

#define DEFAULT_FILTERCACHE_MODE                    ConfigItemMode::DelayedUpdate | ConfigItemMode::AllLocales                                          /// ConfigItems could run in different modes: supported values are ... { ConfigItemMode::ImmediateUpdate, ConfigItemMode::DelayedUpdate, ConfigItemMode::AllLocales }

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

// These struct define a type, which present the type of a file.
// He is used for easy filter detection without file stream detection!
// The internal name is the keyname of an item with these structure in our hash map or our configuration set!

struct FileType
{

    // public methods

    public:

        inline               FileType   (                               ) { impl_clear();               }
        inline               FileType   (   const   FileType&   rCopy   ) { impl_copy( rCopy );         }
        inline              ~FileType   (                               ) { impl_clear();               }
        inline FileType&    operator=   (   const   FileType&   rCopy   ) { return impl_copy( rCopy );  }

    private:

        inline void impl_clear()
        {
            bPreferred          = false;
            sName.clear();
            sMediaType.clear();
            sClipboardFormat.clear();
            nDocumentIconID     = 0;
            lUINames.free   ();
            framework::free(lURLPattern);
            framework::free(lExtensions);
        }

        inline FileType& impl_copy( const FileType& rCopy )
        {
            bPreferred          = rCopy.bPreferred;
            sName               = rCopy.sName;
            lUINames            = rCopy.lUINames;
            sMediaType          = rCopy.sMediaType;
            sClipboardFormat    = rCopy.sClipboardFormat;
            nDocumentIconID     = rCopy.nDocumentIconID;
            lURLPattern         = rCopy.lURLPattern;
            lExtensions         = rCopy.lExtensions;
            return (*this);
        }

    // public member

    public:

        bool                bPreferred;
        OUString            sName;
        OUStringHashMap     lUINames;
        OUString            sMediaType;
        OUString            sClipboardFormat;
        sal_Int32           nDocumentIconID;
        OUStringList        lURLPattern;
        OUStringList        lExtensions;
};

// These struct describe a filter which is registered for one type.
// He hold information about services which present the document himself (like a item) and a filter service which
// filter a file in these document.
// The internal name is the keyname of an item with these structure in our hash map or our configuration set!

struct Filter
{

    // public methods

    public:

        inline           Filter     (                           ) { impl_clear();               }
        inline           Filter     (   const   Filter& rCopy   ) { impl_copy( rCopy );         }
        inline          ~Filter     (                           ) { impl_clear();               }
        inline Filter&  operator=   (   const   Filter& rCopy   ) { return impl_copy( rCopy );  }

    private:

        inline void impl_clear()
        {
            nOrder              = 0;
            sName.clear();
            sType.clear();
            sDocumentService.clear();
            sFilterService.clear();
            sUIComponent.clear();
            nFlags              = 0;
            nFileFormatVersion  = 0;
            sTemplateName.clear();
            lUINames.free();
            framework::free(lUserData);
        }

        inline Filter& impl_copy( const Filter& rCopy )
        {
            nOrder              = rCopy.nOrder;
            sName               = rCopy.sName;
            sType               = rCopy.sType;
            lUINames            = rCopy.lUINames;
            sDocumentService    = rCopy.sDocumentService;
            sFilterService      = rCopy.sFilterService;
            sUIComponent        = rCopy.sUIComponent;
            nFlags              = rCopy.nFlags;
            nFileFormatVersion  = rCopy.nFileFormatVersion;
            sTemplateName       = rCopy.sTemplateName;
            lUserData           = rCopy.lUserData;
            return (*this);
        }

    // public member

    public:

        sal_Int32           nOrder;
        OUString     sName;
        OUString     sType;
        OUStringHashMap     lUINames;
        OUString     sDocumentService;
        OUString     sFilterService;
        OUString     sUIComponent;
        sal_Int32           nFlags;
        OUStringList        lUserData;
        sal_Int32           nFileFormatVersion;
        OUString     sTemplateName;
};

// Programmer can register his own services for an content detection of different types.
// The implementation or service name of these is the keyname of an item with these structure
// in our hash map or our configuration set!

struct Detector
{

    // public methods

    public:

        inline               Detector   (                               ) { impl_clear();               }
        inline               Detector   (   const   Detector&   rCopy   ) { impl_copy( rCopy );         }
        inline              ~Detector   (                               ) { impl_clear();               }
        inline Detector&    operator=   (   const   Detector&   rCopy   ) { return impl_copy( rCopy );  }

    private:

        inline void impl_clear()
        {
            sName.clear();
            framework::free(lTypes);
        }

        inline Detector& impl_copy( const Detector& rCopy )
        {
            sName  = rCopy.sName;
            lTypes = rCopy.lTypes;
            return (*this);
        }

    // public member

    public:

        OUString     sName;
        OUStringList        lTypes;
};

// Programmer can register his own services for loading documents in a frame.
// The implementation or service name of these is the keyname of an item with these structure
// in our hash map or our configuration set!

struct Loader
{

    // public methods

    public:

        inline           Loader     (                           ) { impl_clear();               }
        inline           Loader     (   const   Loader& rCopy   ) { impl_copy( rCopy );         }
        inline          ~Loader     (                           ) { impl_clear();               }
        inline Loader&  operator=   (   const   Loader& rCopy   ) { return impl_copy( rCopy );  }

    private:

        inline void impl_clear()
        {
            sName.clear();
            lUINames.free();
            framework::free(lTypes);
        }

        inline Loader& impl_copy( const Loader& rCopy )
        {
            sName       = rCopy.sName;
            lUINames    = rCopy.lUINames;
            lTypes      = rCopy.lTypes;
            return (*this);
        }

    // public member

    public:

        OUString sName;
        OUStringHashMap lUINames;
        OUStringList    lTypes;
};

// Programmer can register his own services to handle a FileType and intercept dispatches.
// Don't forget: It's not a FrameLoader - it's a ContentHandler! (normally without any UI)

struct ContentHandler
{

    // public methods

    public:

        inline                   ContentHandler(                              ) { impl_clear();               }
        inline                   ContentHandler( const ContentHandler& rCopy  ) { impl_copy( rCopy );         }
        inline                  ~ContentHandler(                              ) { impl_clear();               }
        inline ContentHandler&   operator=     ( const ContentHandler& rCopy  ) { return impl_copy( rCopy );  }

    private:

        inline void impl_clear()
        {
            sName.clear();
            framework::free(lTypes);
        }

        inline ContentHandler& impl_copy( const ContentHandler& rCopy )
        {
            sName  = rCopy.sName;
            lTypes = rCopy.lTypes;
            return (*this);
        }

    // public member

    public:

        OUString     sName;
        OUStringList        lTypes;
};

// We need different hash maps for different tables of our configuration management.
// Follow maps convert <names> to <properties> of type, filter, detector, loader ...
// and could be used in a generic way

template< class HashType >
class SetNodeHash : public std::unordered_map< OUString                    ,
                                               HashType                           ,
                                               OUStringHash                  ,
                                               std::equal_to< OUString > >
{
    // member

    public:
        OUStringList  lAddedItems;
        OUStringList  lChangedItems;
        OUStringList  lRemovedItems;
};

// Use these hashes to implement different tables which assign types to frame loader or detect services.
// It's an optimism to find registered services faster!
// The preferred hash maps file extensions to preferred types to find these ones faster.

class PerformanceHash   :   public  std::unordered_map< OUString,
                                                        OUStringList,
                                                        OUStringHash,
                                                        std::equal_to< OUString >  >
{
};


// Use private static data container to hold all values of configuration!

class DataContainer
{
    public:

    /** @short  identifies different sets of the TypeDetection configuration package.

        @descr  Most functions on top of this configuration package are the same ...
                but must be executed on different places inside this configuration structures.
                These enum values can be used ate some interface methods to specify, which
                configuration set should be used.
                Further it must be possible to start the same action for more than one cfg type.
                That's why these values must be interpreted as flags. Means: its values must be
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

        SetNodeHash< FileType > m_aTypeCache;               /// hold all information about registered file types
        SetNodeHash< Filter >   m_aFilterCache;             /// hold all information about registered filters
        SetNodeHash< Detector > m_aDetectorCache;           /// hold all information about registered detect services
        SetNodeHash< Loader >   m_aLoaderCache;             /// hold all information about registered loader services
        SetNodeHash< ContentHandler > m_aContentHandlerCache;     /// hold all information about registered content handler services
        PerformanceHash         m_aFastFilterCache;         /// hold all registered filter for a special file type
        PerformanceHash         m_aFastDetectorCache;       /// hold all registered detect services for a special file type
        PerformanceHash         m_aFastLoaderCache;         /// hold all registered loader services for a special file type
        PerformanceHash         m_aFastContentHandlerCache; /// hold all registered content handler services for a special file type
        OUStringHashMap         m_aPreferredTypesCache;     /// assignment of extensions to preferred types for it
        Loader                  m_aGenericLoader;           /// information about our default frame loader
        OUString                m_sLocale;                  /// current set locale of configuration to handle right UIName from set of all UINames!
        bool                    m_bTypesModified;
        bool                    m_bFiltersModified;
        bool                    m_bDetectorsModified;
        bool                    m_bLoadersModified;
        bool                    m_bHandlersModified;
};

/*-************************************************************************************************************
    @short          capsulate configuration access for filter configuration
    @descr          We use the ConfigItem mechanism to read/write values from/to configuration.
                    This implementation could be used to handle standard AND additional filter configurations in the same way.
                    We set a data container pointer for filling or reading ... this class use it temp.
                    After successfully calling of read(), we can use filled container directly or merge it with an existing one.
                    After successfully calling of write() all values of given data container are flushed to our configuration.
    @base           ConfigItem

    @devstatus      ready to use
    @threadsafe     no
*//*-*************************************************************************************************************/
class FilterCFGAccess : public ::utl::ConfigItem
{

    //  interface

    public:
                                    FilterCFGAccess ( const OUString& sPath                                  ,
                                                            sal_Int32        nVersion = DEFAULT_FILTERCACHE_VERSION ,
                                                            ConfigItemMode   nMode    = DEFAULT_FILTERCACHE_MODE    ); // open configuration
        virtual                     ~FilterCFGAccess(                                                               );

    //  member

    private:
        EFilterPackage  m_ePackage;   // ... not really used yet! should split configuration in STANDARD and ADDITIONAL filter
        sal_Int32       m_nVersion;   // file format version of configuration! (necessary for "xml2xcd" transformation!)
        sal_Int32       m_nKeyCountTypes;   // follow key counts present count of configuration properties for types/filters ... and depends from m_nVersion - must be set right!
        sal_Int32       m_nKeyCountFilters;
        sal_Int32       m_nKeyCountDetectors;
        sal_Int32       m_nKeyCountLoaders;
        sal_Int32       m_nKeyCountContentHandlers;
        OUString m_sProductName;
        OUString m_sFormatVersion;
};

}       //  namespace framework

#endif // INCLUDED_FRAMEWORK_INC_CLASSES_FILTERCACHEDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
