/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _UTL_CONFIGITEM_HXX_
#define _UTL_CONFIGITEM_HXX_

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/uno/Reference.h>
#include "unotools/unotoolsdllapi.h"
#include "unotools/options.hxx"

//-----------------------------------------------------------------------------
namespace com{ namespace sun{ namespace star{
    namespace uno{
        class Any;
    }
    namespace beans{
        struct PropertyValue;
    }
    namespace container{
        class XHierarchicalNameAccess;
    }
    namespace util{
        class XChangesListener;
    }
}}}
//-----------------------------------------------------------------------------
namespace utl
{
#define CONFIG_MODE_IMMEDIATE_UPDATE    0x00
#define CONFIG_MODE_DELAYED_UPDATE      0x01
#define CONFIG_MODE_ALL_LOCALES         0x02
#define CONFIG_MODE_RELEASE_TREE        0x04
#define CONFIG_MODE_IGNORE_ERRORS       0x08 // prevent assertions, if creation fails
#define CONFIG_MODE_PROPAGATE_ERRORS    0x10 // throw exceptions, if creation fails

    enum  ConfigNameFormat
    {
        CONFIG_NAME_PLAINTEXT_NAME, // unescaped local node name, for user display etc.
        CONFIG_NAME_LOCAL_NAME,     // local node name, for use in XNameAccess etc. ("Item", "Q & A")
        CONFIG_NAME_LOCAL_PATH,     // one-level relative path, for use when building pathes etc.  ("Item", "Typ['Q &amp; A']")
        CONFIG_NAME_FULL_PATH,       // full absolute path. ("/org.openoffice.Sample/Group/Item", "/org.openoffice.Sample/Set/Typ['Q &amp; A']")

        CONFIG_NAME_DEFAULT = CONFIG_NAME_LOCAL_PATH // default format
    };

    class ConfigChangeListener_Impl;
    class ConfigManager;
    struct ConfigItem_Impl;

    class UNOTOOLS_DLLPUBLIC ConfigItem : public ConfigurationBroadcaster
    {
            friend class ConfigChangeListener_Impl;
            friend class ConfigManager;

            const rtl::OUString         sSubTree;
            com::sun::star::uno::Reference< com::sun::star::container::XHierarchicalNameAccess>
                                        m_xHierarchyAccess;
            com::sun::star::uno::Reference< com::sun::star::util::XChangesListener >
                                        xChangeLstnr;
            ConfigItem_Impl*            pImpl;

            ConfigItem();//
            void                    RemoveChangesListener();
            void                    CallNotify(
                                const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames);

            //***********************************************************************************************************************
            // In special mode ALL_LOCALES we must support reading/writing of localized cfg entries as Sequence< PropertyValue >.
            // These methods are helper to convert given lists of names and Any-values.
            // format:  PropertyValue.Name  = <locale as ISO string>
            //          PropertyValue.Value = <value; type depends from cfg entry!>
            // e.g.
            //          LOCALIZED NODE
            //          "UIName"
            //                      LOCALE      VALUE
            //                      "de"        "Mein Name"
            //                      "en-US"     "my name"
            void impl_packLocalizedProperties   (   const   com::sun::star::uno::Sequence< rtl::OUString >&             lInNames    ,
                                                    const   com::sun::star::uno::Sequence< com::sun::star::uno::Any >&  lInValues   ,
                                                            com::sun::star::uno::Sequence< com::sun::star::uno::Any >&  lOutValues  );
            void impl_unpackLocalizedProperties (   const   com::sun::star::uno::Sequence< rtl::OUString >&             lInNames    ,
                                                    const   com::sun::star::uno::Sequence< com::sun::star::uno::Any >&  lInValues   ,
                                                            com::sun::star::uno::Sequence< rtl::OUString >&             lOutNames   ,
                                                            com::sun::star::uno::Sequence< com::sun::star::uno::Any >&  lOutValues  );

            com::sun::star::uno::Reference< com::sun::star::container::XHierarchicalNameAccess>
                                        GetTree();

        protected:
            ConfigItem(const rtl::OUString rSubTree,
                        sal_Int16 nMode = CONFIG_MODE_DELAYED_UPDATE);
            ConfigItem(utl::ConfigManager&  rManager, const rtl::OUString rSubTree);

            void                    SetModified  (); // mark item as modified
            void                    ClearModified(); // reset state after commit!

            com::sun::star::uno::Sequence< com::sun::star::uno::Any>
                                    GetProperties(const com::sun::star::uno::Sequence< rtl::OUString >& rNames);

            com::sun::star::uno::Sequence< sal_Bool >
                                    GetReadOnlyStates(const com::sun::star::uno::Sequence< rtl::OUString >& rNames);

            sal_Bool                PutProperties(
                                        const com::sun::star::uno::Sequence< rtl::OUString >& rNames,
                                        const com::sun::star::uno::Sequence< com::sun::star::uno::Any>& rValues);

            /** enables notifications about changes on selected sub nodes/values

                Before calling this method a second time for a possibly changed node/value set,
                you must disable the current notifications by calling DisableNotification.

                @see Notify
                @see DisableNotification
            */
            sal_Bool                EnableNotification(const com::sun::star::uno::Sequence< rtl::OUString >& rNames,
                                        sal_Bool bEnableInternalNotification = sal_False);
            /** disables notifications about changes on sub nodes/values, which previosly had
                been enabled with EnableNotification
                @see Notify
                @see EnableNotification
            */
            void                    DisableNotification();
            sal_Bool                IsInternalNotification()const {return IsInValueChange();}

            //returns all members of a node in a specific format
            com::sun::star::uno::Sequence< rtl::OUString >
                                    GetNodeNames(const rtl::OUString& rNode);
            //returns all members of a node in a specific format
            com::sun::star::uno::Sequence< rtl::OUString >
                                    GetNodeNames(const rtl::OUString& rNode, ConfigNameFormat eFormat);
            // remove all members of a set
            sal_Bool                ClearNodeSet(const rtl::OUString& rNode);
            // remove selected members of a set
            sal_Bool                ClearNodeElements(const rtl::OUString& rNode,
                                        com::sun::star::uno::Sequence< rtl::OUString >& rElements);
            // change or add members to a set
            sal_Bool                SetSetProperties(const rtl::OUString& rNode, com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > rValues);
            // remove, change or add members of a set
            sal_Bool                ReplaceSetProperties(const rtl::OUString& rNode, com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > rValues);
            // add a new node without setting any properties
            sal_Bool                AddNode(const rtl::OUString& rNode, const rtl::OUString& rNewNode);

            /** get a name for a new element of a set node
                @param _rSetNode
                    relative path to the set node
                @param _rBaseName
                    <ul><li><b>in</b>: the base to use when calculating a new name</li>
                        <li><b>out</b>: an unused element name</li>
                    </ul>
                @return <TRUE/> if a free name could be generated
            */
            sal_Bool                getUniqueSetElementName( const rtl::OUString& _rSetNode, rtl::OUString& _rBaseName);

        public:
            virtual ~ConfigItem();

            /** is called from the ConfigManager before application ends of from the
                PropertyChangeListener if the sub tree broadcasts changes. */
            virtual void            Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames)=0;

            /** is called from the ConfigManager if it is destroyed before the ConfigItem. */
            void                    ReleaseConfigMgr();

            /** enable locking of the XHierarchicalNameAccess if CONFIG_MODE_RELEASE_TREE is set to
             prevent multiple calls ConfigManager::AcquireTree() from a single Commit() operation*/
            void                    LockTree();
            void                    UnlockTree();

            const rtl::OUString&    GetSubTreeName() const {return sSubTree;}

            sal_Bool                IsModified() const;

            /** writes the changed values into the sub tree. Always called in the Dtor of the derived class.  */
            virtual void            Commit()=0;

            sal_Bool                IsInValueChange() const;

            sal_Int16               GetMode() const;

            /** checks if the configuration manager used by this item is valid.
            */
            sal_Bool                IsValidConfigMgr() const;
    };
}//namespace utl
#endif //_UTL_CONFIGITEM_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
