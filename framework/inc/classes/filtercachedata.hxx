/*************************************************************************
 *
 *  $RCSfile: filtercachedata.hxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-19 07:49:56 $
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

/** Attention: stl headers must(!) be included at first. Otherwhise it can make trouble
               with solaris headers ...
*/
#include <hash_map>
#include <vector>
#include <iterator>

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_CLASSES_CHECKEDITERATOR_HXX_
#include <classes/checkediterator.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_WILDCARD_HXX_
#include <classes/wildcard.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_CONVERTER_HXX_
#include <classes/converter.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
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

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XCHANGESLISTENER_HPP_
#include <com/sun/star/util/XChangesListener.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XCHANGESNOTIFIER_HPP_
#include <com/sun/star/util/XChangesNotifier.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring.hxx>
#endif

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

#define PACKAGENAME_TYPEDETECTION_STANDARD          DECLARE_ASCII("Office.TypeDetection"                            )   /// Names of our configuration files.
#define PACKAGENAME_TYPEDETECTION_ADDITIONAL        DECLARE_ASCII("Office.TypeDetectionAdditional"                  )
#define CFG_PATH_SEPERATOR                          DECLARE_ASCII("/"                                               )   /// seperator for configuration pathes
#define PROPERTY_SEPERATOR                          sal_Unicode(',')                                                    /// seperator for own formated property strings of types and filters
#define LIST_SEPERATOR                              sal_Unicode(';')                                                    /// seperator for own formated lists as part of our own formated type- or filter-string
#define LOCALE_FALLBACK                             DECLARE_ASCII("en-US"                                           )   /// fallback, if configuration can't give us current set locale ...
#define DEFAULT_FILTERCACHE_VERSION                 6                                                                   /// these implmentation of FilterCache support different version of TypeDetection.xml! This define the current set default one.

#define DEFAULT_FILTERCACHE_MODE                    CONFIG_MODE_DELAYED_UPDATE | CONFIG_MODE_ALL_LOCALES                                          /// ConfigItems could run in different modes: supported values are ... { CONFIG_MODE_IMMEDIATE_UPDATE, CONFIG_MODE_DELAYED_UPDATE, CONFIG_MODE_ALL_LOCALES }
//#define DEFAULT_FILTERCACHE_MODE                    CONFIG_MODE_DELAYED_UPDATE                                          /// ConfigItems could run in different modes: supported values are ... { CONFIG_MODE_IMMEDIATE_UPDATE, CONFIG_MODE_DELAYED_UPDATE, CONFIG_MODE_ALL_LOCALES }

#define CFG_ENCODING_OPEN                           DECLARE_ASCII("[\'"                                             )   /// used to start encoding of set names
#define CFG_ENCODING_CLOSE                          DECLARE_ASCII("\']"                                             )   /// used to finish encoding of set names
#define PRODUCTNAME_VARIABLE                        DECLARE_ASCII("%productname%")
#define PRODUCTNAME_VARLENGTH                       13
#define PRODUCTNAME_FALLBACK                        DECLARE_ASCII("StarOffice"                                      )   /// fallback, if configuration can't give us current set product name ...
#define FORMATVERSION_VARIABLE                      DECLARE_ASCII("%formatversion%"                                 )
#define FORMATVERSION_VARLENGTH                     15
#define FORMATVERSION_FALLBACK                      DECLARE_ASCII("6.0/7"                                           )

//*****************************************************************************************************************
// We know some default values ...
//*****************************************************************************************************************
#define NAME_DEFAULTDETECTOR                        DECLARE_ASCII("com.sun.star.comp.office.FilterDetect"           )
#define NAME_GENERICLOADER                          DECLARE_ASCII("com.sun.star.comp.office.FrameLoader"            )
#define UINAME_GENERICLOADER                        DECLARE_ASCII("com.sun.star.comp.office.FrameLoader"            )
#define TYPELIST_DEFAULTDETECTOR                    DECLARE_ASCII("*"                                               )
#define TYPELIST_GENERICLOADER                      DECLARE_ASCII("*"                                               )

//*****************************************************************************************************************
// This are all supported set-names of our filter configuration.
//*****************************************************************************************************************
#define SUBLIST_TYPES                               DECLARE_ASCII("Types"                                           )
#define SUBLIST_FILTERS                             DECLARE_ASCII("Filters"                                         )
#define SUBLIST_DETECTSERVICES                      DECLARE_ASCII("DetectServices"                                  )
#define SUBLIST_FRAMELOADERS                        DECLARE_ASCII("FrameLoaders"                                    )
#define SUBLIST_CONTENTHANDLERS                     DECLARE_ASCII("ContentHandlers"                                 )
#define SUBLIST_DEFAULTS                            DECLARE_ASCII("Defaults"                                        )

#define TEMPLATENAME_TYPE                           DECLARE_ASCII("Type"                                            )
#define TEMPLATENAME_FILTER                         DECLARE_ASCII("Filter"                                          )
#define TEMPLATENAME_DETECTSERVICE                  DECLARE_ASCII("DetectService"                                   )
#define TEMPLATENAME_FRAMELOADER                    DECLARE_ASCII("FrameLoader"                                     )
#define TEMPLATENAME_CONTENTHANDLER                 DECLARE_ASCII("ContentHandler"                                  )

//*****************************************************************************************************************
// These defines declare all supported names of configuration key names.
// They are not sorted and could be shared by different base configuration items.
//*****************************************************************************************************************
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
#define SUBKEY_UICOMPONENT                          DECLARE_ASCII("UIComponent"                                     )

//*****************************************************************************************************************
// These defines declare all supported property names for our name container interface.
// They are not sorted by using ... and could be shared by different methods and access operations.
//*****************************************************************************************************************
#define PROPERTY_NAME                               DECLARE_ASCII("Name"                                            )
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
#define PROPERTY_ORDER                              DECLARE_ASCII("Order"                                           )
#define PROPERTY_UICOMPONENT                        DECLARE_ASCII("UIComponent"                                     )

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
        OUStringHash        lUINames            ;
        ::rtl::OUString     sMediaType          ;
        ::rtl::OUString     sClipboardFormat    ;
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
            sName               = ::rtl::OUString();
            sType               = ::rtl::OUString();
            sDocumentService    = ::rtl::OUString();
            sFilterService      = ::rtl::OUString();
            sUIComponent        = ::rtl::OUString();
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
        ::rtl::OUString     sName               ;
        ::rtl::OUString     sType               ;
        OUStringHash        lUINames            ;
        ::rtl::OUString     sDocumentService    ;
        ::rtl::OUString     sFilterService      ;
        ::rtl::OUString     sUIComponent        ;
        sal_Int32           nFlags              ;
        OUStringList        lUserData           ;
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
        OUStringHash    lUINames    ;
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
            sName = ::rtl::OUString();
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

        ::rtl::OUString     sName   ;
        OUStringList        lTypes  ;
};

//*****************************************************************************************************************
// We need different hash maps for different tables of our configuration management.
// Follow maps convert <names> to <properties> of type, filter, detector, loader ...
// and could be used in a generic way
//*****************************************************************************************************************
template< class HashType >
class SetNodeHash : public ::std::hash_map< ::rtl::OUString                    ,
                                            HashType                           ,
                                            OUStringHashCode                   ,
                                            ::std::equal_to< ::rtl::OUString > >
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
        // Neccessary for saving changes
        //---------------------------------------------------------------------------------------------------------
        void appendChange( const ::rtl::OUString& sName  ,
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
class PerformanceHash   :   public  ::std::hash_map<    ::rtl::OUString                     ,
                                                        OUStringList                        ,
                                                        OUStringHashCode                    ,
                                                        ::std::equal_to< ::rtl::OUString >  >
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
        inline sal_Bool findPatternKey( const ::rtl::OUString& sSearchValue ,
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
typedef OUStringHash                                                PreferredHash               ;
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

        sal_Bool existsType           ( const ::rtl::OUString& sName );
        sal_Bool existsFilter         ( const ::rtl::OUString& sName );
        sal_Bool existsDetector       ( const ::rtl::OUString& sName );
        sal_Bool existsLoader         ( const ::rtl::OUString& sName );
        sal_Bool existsContentHandler ( const ::rtl::OUString& sName );

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

        void removeType           ( const ::rtl::OUString& sName    , sal_Bool bSetModified );
        void removeFilter         ( const ::rtl::OUString& sName    , sal_Bool bSetModified );
        void removeDetector       ( const ::rtl::OUString& sName    , sal_Bool bSetModified );
        void removeLoader         ( const ::rtl::OUString& sName    , sal_Bool bSetModified );
        void removeContentHandler ( const ::rtl::OUString& sName    , sal_Bool bSetModified );

        static void             convertFileTypeToPropertySequence          ( const FileType&                                           aSource         ,
                                                                                   css::uno::Sequence< css::beans::PropertyValue >&    lDestination    ,
                                                                             const ::rtl::OUString&                                    sCurrentLocale  );
        static void             convertFilterToPropertySequence            ( const Filter&                                             aSource         ,
                                                                                   css::uno::Sequence< css::beans::PropertyValue >&    lDestination    ,
                                                                             const ::rtl::OUString&                                    sCurrentLocale  );
        static void             convertDetectorToPropertySequence          ( const Detector&                                           aSource         ,
                                                                                   css::uno::Sequence< css::beans::PropertyValue >&    lDestination    );
        static void             convertLoaderToPropertySequence            ( const Loader&                                             aSource         ,
                                                                                   css::uno::Sequence< css::beans::PropertyValue >&    lDestination    ,
                                                                             const ::rtl::OUString&                                    sCurrentLocale  );
        static void             convertContentHandlerToPropertySequence    ( const ContentHandler&                                     aSource         ,
                                                                                   css::uno::Sequence< css::beans::PropertyValue >&    lDestination    );
        static void             convertPropertySequenceToFilter            ( const css::uno::Sequence< css::beans::PropertyValue >&    lSource         ,
                                                                                   Filter&                                             aDestination    ,
                                                                             const ::rtl::OUString&                                    sCurrentLocale  );
        static void             convertPropertySequenceToFileType          ( const css::uno::Sequence< css::beans::PropertyValue >&    lSource         ,
                                                                                   FileType&                                           aDestination    ,
                                                                             const ::rtl::OUString&                                    sCurrentLocale  );
        static void             convertPropertySequenceToDetector          ( const css::uno::Sequence< css::beans::PropertyValue >&    lSource         ,
                                                                                   Detector&                                           aDestination    );
        static void             convertPropertySequenceToLoader            ( const css::uno::Sequence< css::beans::PropertyValue >&    lSource         ,
                                                                                   Loader&                                             aDestination    ,
                                                                             const ::rtl::OUString&                                    sCurrentLocale  );
        static void             convertPropertySequenceToContentHandler    ( const css::uno::Sequence< css::beans::PropertyValue >&    lSource         ,
                                                                                   ContentHandler&                                     aDestination    ,
                                                                             const ::rtl::OUString&                                    sCurrentLocale  );
        static void             extractLocalizedStrings                    ( const ::rtl::OUString&                                    sCurrentLocale  ,
                                                                             const css::uno::Any&                                      aCFGValue       ,
                                                                                   OUStringHash&                                       lLocales        );
        static void             packLocalizedStrings                       (       sal_Int16                                           nMode           ,
                                                                             const ::rtl::OUString&                                    sCurrentLocale  ,
                                                                                   css::uno::Any&                                      aCFGValue       ,
                                                                             const OUStringHash&                                       lLocales        );
        static ::rtl::OUString  getLocalelizedString                       ( const OUStringHash&                                       lLocales        ,
                                                                             const ::rtl::OUString&                                    sLocale         );
        static void             setLocalelizedString                       (       OUStringHash&                                       lLocales        ,
                                                                             const ::rtl::OUString&                                    sLocale         ,
                                                                             const ::rtl::OUString&                                    sValue          );
        static void             correctExtensions                          (       OUStringList&                                       lExtensions     );

    public:

        FileTypeHash            m_aTypeCache                ;     /// hold all informations about registered file types
        FilterHash              m_aFilterCache              ;     /// hold all informations about registered filters
        DetectorHash            m_aDetectorCache            ;     /// hold all informations about registered detect services
        LoaderHash              m_aLoaderCache              ;     /// hold all informations about registered loader services
        ContentHandlerHash      m_aContentHandlerCache      ;     /// hold all informations about registered content handler services
        PerformanceHash         m_aFastFilterCache          ;     /// hold all registered filter for a special file type
        PerformanceHash         m_aFastDetectorCache        ;     /// hold all registered detect services for a special file type
        PerformanceHash         m_aFastLoaderCache          ;     /// hold all registered loader services for a special file type
        PerformanceHash         m_aFastContentHandlerCache  ;     /// hold all registered content handler services for a special file type
        PreferredHash           m_aPreferredTypesCache      ;     /// assignment of extensions to preferred types for it
        Loader                  m_aGenericLoader            ;     /// informations about our default frame loader
        ::rtl::OUString         m_sLocale                   ;     /// current set locale of configuration to handle right UIName from set of all UINames!
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
                    After successfuly calling of read(), we can use filled container directly or merge it with an existing one.
                    After successfuly calling of write() all values of given data container are flushed to our configuration.

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
                                    FilterCFGAccess ( const ::rtl::OUString& sPath                                  ,
                                                            sal_Int32        nVersion = DEFAULT_FILTERCACHE_VERSION ,
                                                            sal_Int16        nMode    = DEFAULT_FILTERCACHE_MODE    ); // open configuration
        virtual                     ~FilterCFGAccess(                                                               );

        void                        read            (       DataContainer&   rData                                  ,
                                                            DataContainer::ECFGType         eType                   ); // read values from configuration into given struct
        void                        write           (       DataContainer&   rData                                  ,
                                                            DataContainer::ECFGType         eType                   ); // write values from given struct to configuration

        static   ::rtl::OUString    encodeTypeData  ( const FileType&        aType                                  ); // build own formated string of type properties
        static   void               decodeTypeData  ( const ::rtl::OUString& sData                                  ,
                                                            FileType&        aType                                  );
        static   ::rtl::OUString    encodeFilterData( const Filter&          aFilter                                ); // build own formated string of filter properties
        static   void               decodeFilterData( const ::rtl::OUString& sData                                  ,
                                                            Filter&          aFilter                                );
        static   ::rtl::OUString    encodeStringList( const OUStringList&    lList                                  ); // build own formated string of OUStringList
        static   OUStringList       decodeStringList( const ::rtl::OUString& sValue                                 );

        void             setProductName                             (       OUStringHash&                                       lUINames        );
        void             resetProductName                           (       OUStringHash&                                       lUINames        );

    //-------------------------------------------------------------------------------------------------------------
    //  internal helper
    //-------------------------------------------------------------------------------------------------------------
    private:
        void impl_initKeyCounts        (                                            );    // set right key counts, which are used at reading/writing of set node properties
        void impl_removeNodes          (       OUStringList&        rChangesList    ,     // helper to remove list of set nodes
                                         const ::rtl::OUString&     sTemplateType   ,
                                         const ::rtl::OUString&     sSetName        );

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
        static sal_Bool implcp_ctor ( const ::rtl::OUString& sPath    ,     // methods to check incoming parameter on our interface methods!
                                            sal_Int32        nVersion ,
                                            sal_Int16        nMode    );
        static sal_Bool implcp_read ( const DataContainer&   rData    );
        static sal_Bool implcp_write( const DataContainer&   rData    );

    //-------------------------------------------------------------------------------------------------------------
    //  member
    //-------------------------------------------------------------------------------------------------------------
    private:
        EFilterPackage  m_ePackage                     ;   // ... not realy used yet! should split configuration in STANDARD and ADDITIONAL filter
        sal_Int32       m_nVersion                     ;   // file format version of configuration! (neccessary for "xml2xcd" transformation!)
        sal_Int32       m_nKeyCountTypes               ;   // follow key counts present count of configuration properties for types/filters ... and depends from m_nVersion - must be set right!
        sal_Int32       m_nKeyCountFilters             ;
        sal_Int32       m_nKeyCountDetectors           ;
        sal_Int32       m_nKeyCountLoaders             ;
        sal_Int32       m_nKeyCountContentHandlers     ;
        ::rtl::OUString m_sProductName                 ;
        ::rtl::OUString m_sFormatVersion               ;
};
/*DRAFT
class FilterCFGListener : public  css::util::XChangesListener
                        , private ThreadHelpBase
                        , public  ::cppu::OWeakObject
{
    public:

        enum ECFGType
        {
            E_TYPE          ,
            E_FILTER        ,
            E_LOADER        ,
            E_DETECTOR      ,
            E_CONTENTHANDLER
        };

    private:

        // read only access to the configuration, where we are regsieterd as changes listener.
        css::uno::Reference< css::uno::XInterface > m_xCFG;

        // indicates, for which type of configuration item we listen.
        ECFGType m_eType;

        DataContainer* m_pData;

        // we must know, if we are already registered as listener or not.
        //    That can be usefull to supress double registration calls ...
        //    which may will force double call backs in our disposing method!
        //    Such superflous calls can be dangerous.
        sal_Bool m_bListening;

    public:

        DECLARE_XINTERFACE

        FilterCFGListener( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR ,
                                 ECFGType                                                eType ,
                                 DataContainer*                                          pData );

        void startListening();
        void stopListening ();

        virtual void SAL_CALL changesOccurred( const css::util::ChangesEvent& aEvent ) throw(css::uno::RuntimeException);
        virtual void SAL_CALL disposing      ( const css::lang::EventObject&  aEvent ) throw(css::uno::RuntimeException);

    private:

        FileType       impl_readType    ( const css::uno::Reference< css::uno::XInterface >& xNode );
        Filter         impl_readFilter  ( const css::uno::Reference< css::uno::XInterface >& xNode );
        Detector       impl_readDetector( const css::uno::Reference< css::uno::XInterface >& xNode );
        Loader         impl_readLoader  ( const css::uno::Reference< css::uno::XInterface >& xNode );
        ContentHandler impl_readHandler ( const css::uno::Reference< css::uno::XInterface >& xNode );
};
*/

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_CLASSES_FILTERCACHEDATA_HXX_
