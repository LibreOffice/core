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
#include <o3tl/typed_flags_set.hxx>

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

enum class ConfigItemMode
{
    ImmediateUpdate    = 0x00,
    DelayedUpdate      = 0x01,
    AllLocales         = 0x02,
    ReleaseTree        = 0x04,
};
namespace o3tl
{
    template<> struct typed_flags<ConfigItemMode> : is_typed_flags<ConfigItemMode, 0x07> {};
}

namespace utl
{

    enum class ConfigNameFormat
    {
        LocalNode,     // local node name, for use in XNameAccess etc. ("Item", "Q & A")
        LocalPath,     // one-level relative path, for use when building paths etc.  ("Item", "Typ['Q &amp; A']")
    };

    class ConfigChangeListener_Impl;
    class ConfigManager;

    class UNOTOOLS_DLLPUBLIC ConfigItem : public ConfigurationBroadcaster
    {
            friend class ConfigChangeListener_Impl;
            friend class ConfigManager;

            const OUString              sSubTree;
            css::uno::Reference< css::container::XHierarchicalNameAccess>
                                        m_xHierarchyAccess;
            css::uno::Reference< css::util::XChangesListener >
                                        xChangeLstnr;
            ConfigItemMode              m_nMode;
            bool                        m_bIsModified;
            bool                        m_bEnableInternalNotification;
            sal_Int16                   m_nInValueChange;

            void                    RemoveChangesListener();
            void                    CallNotify(
                                const css::uno::Sequence<OUString>& aPropertyNames);

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
            void impl_packLocalizedProperties   (   const   css::uno::Sequence< OUString >&                  lInNames    ,
                                                    const   css::uno::Sequence< css::uno::Any >&  lInValues   ,
                                                            css::uno::Sequence< css::uno::Any >&  lOutValues  );
            void impl_unpackLocalizedProperties (   const   css::uno::Sequence< OUString >&                  lInNames    ,
                                                    const   css::uno::Sequence< css::uno::Any >&  lInValues   ,
                                                            css::uno::Sequence< OUString >&                  lOutNames   ,
                                                            css::uno::Sequence< css::uno::Any >&  lOutValues  );

            css::uno::Reference< css::container::XHierarchicalNameAccess>
                                        GetTree();
            /** writes the changed values into the sub tree.
                Private and only called from non-virtual public Commit().  */
            virtual void            ImplCommit() = 0;

        protected:
            explicit ConfigItem(const OUString &rSubTree,
                        ConfigItemMode nMode = ConfigItemMode::DelayedUpdate);

            void                    SetModified  (); // mark item as modified
            void                    ClearModified(); // reset state after commit!

            css::uno::Sequence< css::uno::Any>
                                    GetProperties(const css::uno::Sequence< OUString >& rNames);

            css::uno::Sequence< sal_Bool >
                                    GetReadOnlyStates(const css::uno::Sequence< OUString >& rNames);

            bool                PutProperties(
                                        const css::uno::Sequence< OUString >& rNames,
                                        const css::uno::Sequence< css::uno::Any>& rValues);

            /** enables notifications about changes on selected sub nodes/values

                Before calling this method a second time for a possibly changed node/value set,
                you must disable the current notifications by calling DisableNotification.

                @see Notify
                @see DisableNotification
            */
            bool                EnableNotification(const css::uno::Sequence< OUString >& rNames,
                                        bool bEnableInternalNotification = false);
            /** disables notifications about changes on sub nodes/values, which previously had
                been enabled with EnableNotification
                @see Notify
                @see EnableNotification
            */
            void                    DisableNotification();

            //returns all members of a node in a specific format
            css::uno::Sequence< OUString >
                                    GetNodeNames(const OUString& rNode);
            //returns all members of a node in a specific format
            css::uno::Sequence< OUString >
                                    GetNodeNames(const OUString& rNode, ConfigNameFormat eFormat);
            // remove all members of a set
            bool                ClearNodeSet(const OUString& rNode);
            // remove selected members of a set
            bool                ClearNodeElements(const OUString& rNode,
                                        css::uno::Sequence< OUString > const & rElements);
            // change or add members to a set
            bool                SetSetProperties(const OUString& rNode, const css::uno::Sequence< css::beans::PropertyValue >& rValues);
            // remove, change or add members of a set
            bool                ReplaceSetProperties(const OUString& rNode, const css::uno::Sequence< css::beans::PropertyValue >& rValues);
            // add a new node without setting any properties
            bool                AddNode(const OUString& rNode, const OUString& rNewNode);

        public:
            virtual ~ConfigItem() override;

            ConfigItem(ConfigItem const &) = default;
            ConfigItem(ConfigItem &&) = default;
            ConfigItem & operator =(ConfigItem const &) = default;
            ConfigItem & operator =(ConfigItem &&) = default;

            /** is called from the ConfigManager before application ends of from the
                PropertyChangeListener if the sub tree broadcasts changes. */
            virtual void            Notify( const css::uno::Sequence<OUString>& aPropertyNames)=0;

            const OUString&         GetSubTreeName() const {return sSubTree;}

            bool IsModified() const { return m_bIsModified;}

            void                    Commit();

            ConfigItemMode GetMode() const { return m_nMode;}
    };
}//namespace utl
#endif // INCLUDED_UNOTOOLS_CONFIGITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
