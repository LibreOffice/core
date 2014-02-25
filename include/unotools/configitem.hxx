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

#ifndef INCLUDED_UNOTOOLS_CONFIGITEM_HXX
#define INCLUDED_UNOTOOLS_CONFIGITEM_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/uno/Reference.h>
#include <unotools/unotoolsdllapi.h>
#include <unotools/options.hxx>


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

namespace utl
{
#define CONFIG_MODE_IMMEDIATE_UPDATE    0x00
#define CONFIG_MODE_DELAYED_UPDATE      0x01
#define CONFIG_MODE_ALL_LOCALES         0x02
#define CONFIG_MODE_RELEASE_TREE        0x04

    enum  ConfigNameFormat
    {
        CONFIG_NAME_PLAINTEXT_NAME, // unescaped local node name, for user display etc.
        CONFIG_NAME_LOCAL_NAME,     // local node name, for use in XNameAccess etc. ("Item", "Q & A")
        CONFIG_NAME_LOCAL_PATH,     // one-level relative path, for use when building paths etc.  ("Item", "Typ['Q &amp; A']")
        CONFIG_NAME_FULL_PATH,       // full absolute path. ("/org.openoffice.Sample/Group/Item", "/org.openoffice.Sample/Set/Typ['Q &amp; A']")

        CONFIG_NAME_DEFAULT = CONFIG_NAME_LOCAL_PATH // default format
    };

    class ConfigChangeListener_Impl;
    class ConfigManager;

    class UNOTOOLS_DLLPUBLIC ConfigItem : public ConfigurationBroadcaster
    {
            friend class ConfigChangeListener_Impl;
            friend class ConfigManager;

            const OUString              sSubTree;
            com::sun::star::uno::Reference< com::sun::star::container::XHierarchicalNameAccess>
                                        m_xHierarchyAccess;
            com::sun::star::uno::Reference< com::sun::star::util::XChangesListener >
                                        xChangeLstnr;
            sal_Int16 m_nMode;
            bool m_bIsModified;
            bool m_bEnableInternalNotification;
            sal_Int16 m_nInValueChange;

            void                    RemoveChangesListener();
            void                    CallNotify(
                                const com::sun::star::uno::Sequence<OUString>& aPropertyNames);


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
            void impl_packLocalizedProperties   (   const   com::sun::star::uno::Sequence< OUString >&                  lInNames    ,
                                                    const   com::sun::star::uno::Sequence< com::sun::star::uno::Any >&  lInValues   ,
                                                            com::sun::star::uno::Sequence< com::sun::star::uno::Any >&  lOutValues  );
            void impl_unpackLocalizedProperties (   const   com::sun::star::uno::Sequence< OUString >&                  lInNames    ,
                                                    const   com::sun::star::uno::Sequence< com::sun::star::uno::Any >&  lInValues   ,
                                                            com::sun::star::uno::Sequence< OUString >&                  lOutNames   ,
                                                            com::sun::star::uno::Sequence< com::sun::star::uno::Any >&  lOutValues  );

            com::sun::star::uno::Reference< com::sun::star::container::XHierarchicalNameAccess>
                                        GetTree();

        protected:
            explicit ConfigItem(const OUString &rSubTree,
                        sal_Int16 nMode = CONFIG_MODE_DELAYED_UPDATE);

            void                    SetModified  (); // mark item as modified
            void                    ClearModified(); // reset state after commit!

            com::sun::star::uno::Sequence< com::sun::star::uno::Any>
                                    GetProperties(const com::sun::star::uno::Sequence< OUString >& rNames);

            com::sun::star::uno::Sequence< sal_Bool >
                                    GetReadOnlyStates(const com::sun::star::uno::Sequence< OUString >& rNames);

            bool                PutProperties(
                                        const com::sun::star::uno::Sequence< OUString >& rNames,
                                        const com::sun::star::uno::Sequence< com::sun::star::uno::Any>& rValues);

            /** enables notifications about changes on selected sub nodes/values

                Before calling this method a second time for a possibly changed node/value set,
                you must disable the current notifications by calling DisableNotification.

                @see Notify
                @see DisableNotification
            */
            bool                EnableNotification(const com::sun::star::uno::Sequence< OUString >& rNames,
                                        bool bEnableInternalNotification = false);
            /** disables notifications about changes on sub nodes/values, which previosly had
                been enabled with EnableNotification
                @see Notify
                @see EnableNotification
            */
            void                    DisableNotification();
            bool                IsInternalNotification()const {return IsInValueChange();}

            //returns all members of a node in a specific format
            com::sun::star::uno::Sequence< OUString >
                                    GetNodeNames(const OUString& rNode);
            //returns all members of a node in a specific format
            com::sun::star::uno::Sequence< OUString >
                                    GetNodeNames(const OUString& rNode, ConfigNameFormat eFormat);
            // remove all members of a set
            bool                ClearNodeSet(const OUString& rNode);
            // remove selected members of a set
            bool                ClearNodeElements(const OUString& rNode,
                                        com::sun::star::uno::Sequence< OUString >& rElements);
            // change or add members to a set
            bool                SetSetProperties(const OUString& rNode, com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > rValues);
            // remove, change or add members of a set
            bool                ReplaceSetProperties(const OUString& rNode, com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > rValues);
            // add a new node without setting any properties
            bool                AddNode(const OUString& rNode, const OUString& rNewNode);

        public:
            virtual ~ConfigItem();

            /** is called from the ConfigManager before application ends of from the
                PropertyChangeListener if the sub tree broadcasts changes. */
            virtual void            Notify( const com::sun::star::uno::Sequence<OUString>& aPropertyNames)=0;

            const OUString&         GetSubTreeName() const {return sSubTree;}

            bool IsModified() const;

            /** writes the changed values into the sub tree. Always called in the Dtor of the derived class.  */
            virtual void            Commit()=0;

            bool IsInValueChange() const;

            sal_Int16               GetMode() const;
    };
}//namespace utl
#endif // INCLUDED_UNOTOOLS_CONFIGITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
