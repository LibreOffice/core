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


}       //  namespace framework

#endif // INCLUDED_FRAMEWORK_INC_CLASSES_FILTERCACHEDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
