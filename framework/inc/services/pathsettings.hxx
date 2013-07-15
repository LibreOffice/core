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

#ifndef __FRAMEWORK_SERVICES_PATHSETTINGS_HXX_
#define __FRAMEWORK_SERVICES_PATHSETTINGS_HXX_

#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <stdtypes.h>
#include <properties.h>

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/util/XStringSubstitution.hpp>
#include <com/sun/star/util/XChangesListener.hpp>
#include <com/sun/star/util/XPathSettings.hpp>

#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/implbase3.hxx>
#include <unotools/configitem.hxx>
#include <comphelper/sequenceasvector.hxx>

/* enable it if you whish to migrate old user settings (using the old cfg schema) on demand ....
   disable it in case only the new schema must be used.
 */

namespace framework
{

typedef ::cppu::WeakImplHelper3<
                         css::lang::XServiceInfo,
                         css::util::XChangesListener,               // => XEventListener
                         css::util::XPathSettings>                  // => XPropertySet
        PathSettings_BASE;

class PathSettings : // base classes
                     // Order is necessary for right initialization!
                     private ThreadHelpBase                       ,
                     public  ::cppu::OBroadcastHelper             ,
                     public  ::cppu::OPropertySetHelper           , // => XPropertySet / XFastPropertySet / XMultiPropertySet
                     public  PathSettings_BASE
{
    struct PathInfo
    {
        public:

            PathInfo()
                : sPathName     ()
                , lInternalPaths()
                , lUserPaths    ()
                , sWritePath    ()
                , bIsSinglePath (sal_False)
                , bIsReadonly   (sal_False)
            {}

            PathInfo(const PathInfo& rCopy)
            {
                takeOver(rCopy);
            }

            void takeOver(const PathInfo& rCopy)
            {
                sPathName      = rCopy.sPathName;
                lInternalPaths = rCopy.lInternalPaths;
                lUserPaths     = rCopy.lUserPaths;
                sWritePath     = rCopy.sWritePath;
                bIsSinglePath  = rCopy.bIsSinglePath;
                bIsReadonly    = rCopy.bIsReadonly;
            }

            /// an internal name describing this path
            OUString sPathName;

            /// contains all paths, which are used internaly - but are not visible for the user.
            OUStringList lInternalPaths;

            /// contains all paths configured by the user
            OUStringList lUserPaths;

            /// this special path is used to generate feature depending content there
            OUString sWritePath;

            /// indicates real single paths, which uses WritePath property only
            sal_Bool bIsSinglePath;

            /// simple handling of finalized/mandatory states ... => we know one state READONLY only .-)
            sal_Bool bIsReadonly;
    };

    typedef BaseHash< PathSettings::PathInfo > PathHash;

    enum EChangeOp
    {
        E_UNDEFINED,
        E_ADDED,
        E_CHANGED,
        E_REMOVED
    };

    // ______________________________________
    // member

    private:

        /** reference to factory, which has create this instance. */
        css::uno::Reference< css::uno::XComponentContext > m_xContext;

        /** list of all path variables and her corresponding values. */
        PathSettings::PathHash m_lPaths;

        /** describes all properties available on our interface.
            Will be generated on demand based on our path list m_lPaths. */
        css::uno::Sequence< css::beans::Property > m_lPropDesc;

        /** helper needed to (re-)substitute all internal save path values. */
        css::uno::Reference< css::util::XStringSubstitution > m_xSubstitution;

        /** provides access to the old configuration schema (which will be migrated on demand). */
        css::uno::Reference< css::container::XNameAccess > m_xCfgOld;

        /** provides access to the new configuration schema. */
        css::uno::Reference< css::container::XNameAccess > m_xCfgNew;

        /** helper to listen for configuration changes without ownership cycle problems */
        css::uno::Reference< css::util::XChangesListener > m_xCfgNewListener;

        ::cppu::OPropertyArrayHelper* m_pPropHelp;

        ::sal_Bool m_bIgnoreEvents;

    // ___________________________________________
    // interface

    public:

        /** initialize a new instance of this class.
            Attention: It's neccessary for right function of this class, that the order of base
            classes is the right one. Because we transfer information from one base to another
            during this ctor runs! */
        PathSettings(const css::uno::Reference< css::uno::XComponentContext >& xContext);

        /** free all used resources ... if it was not already done. */
        virtual ~PathSettings();

        /** declaration of XInterface, XTypeProvider, XServiceInfo */
        DECLARE_XSERVICEINFO

        // XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& type) throw ( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL acquire() throw ()
            { OWeakObject::acquire(); }
        virtual void SAL_CALL release() throw ()
            { OWeakObject::release(); }

        // XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

        // css::util::XChangesListener
        virtual void SAL_CALL changesOccurred(const css::util::ChangesEvent& aEvent) throw (css::uno::RuntimeException);

        // css::lang::XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& aSource)
            throw(css::uno::RuntimeException);

        using ::cppu::OPropertySetHelper::disposing;


        /**
         * XPathSettings attribute methods
         */
        virtual OUString SAL_CALL getAddin() throw (css::uno::RuntimeException)
            { return getStringProperty("Addin"); }
        virtual void SAL_CALL setAddin(const OUString& p1) throw (css::uno::RuntimeException)
            { setStringProperty("Addin", p1); }
        virtual OUString SAL_CALL getAutoCorrect() throw (css::uno::RuntimeException)
            { return getStringProperty("AutoCorrect"); }
        virtual void SAL_CALL setAutoCorrect(const OUString& p1) throw (css::uno::RuntimeException)
            { setStringProperty("AutoCorrect", p1); }
        virtual OUString SAL_CALL getAutoText() throw (css::uno::RuntimeException)
            { return getStringProperty("AutoText"); }
        virtual void SAL_CALL setAutoText(const OUString& p1) throw (css::uno::RuntimeException)
            { setStringProperty("AutoText", p1); }
        virtual OUString SAL_CALL getBackup() throw (css::uno::RuntimeException)
            { return getStringProperty("Backup"); }
        virtual void SAL_CALL setBackup(const OUString& p1) throw (css::uno::RuntimeException)
            { setStringProperty("Backup", p1); }
        virtual OUString SAL_CALL getBasic() throw (css::uno::RuntimeException)
            { return getStringProperty("Basic"); }
        virtual void SAL_CALL setBasic(const OUString& p1) throw (css::uno::RuntimeException)
            { setStringProperty("Basic", p1); }
        virtual OUString SAL_CALL getBitmap() throw (css::uno::RuntimeException)
            { return getStringProperty("Bitmap"); }
        virtual void SAL_CALL setBitmap(const OUString& p1) throw (css::uno::RuntimeException)
            { setStringProperty("Bitmap", p1); }
        virtual OUString SAL_CALL getConfig() throw (css::uno::RuntimeException)
            { return getStringProperty("Config"); }
        virtual void SAL_CALL setConfig(const OUString& p1) throw (css::uno::RuntimeException)
            { setStringProperty("Config", p1); }
        virtual OUString SAL_CALL getDictionary() throw (css::uno::RuntimeException)
            { return getStringProperty("Dictionary"); }
        virtual void SAL_CALL setDictionary(const OUString& p1) throw (css::uno::RuntimeException)
            { setStringProperty("Dictionary", p1); }
        virtual OUString SAL_CALL getFavorite() throw (css::uno::RuntimeException)
            { return getStringProperty("Favorite"); }
        virtual void SAL_CALL setFavorite(const OUString& p1) throw (css::uno::RuntimeException)
            { setStringProperty("Favorite", p1); }
        virtual OUString SAL_CALL getFilter() throw (css::uno::RuntimeException)
            { return getStringProperty("Filter"); }
        virtual void SAL_CALL setFilter(const OUString& p1) throw (css::uno::RuntimeException)
            { setStringProperty("Filter", p1); }
        virtual OUString SAL_CALL getGallery() throw (css::uno::RuntimeException)
            { return getStringProperty("Gallery"); }
        virtual void SAL_CALL setGallery(const OUString& p1) throw (css::uno::RuntimeException)
            { setStringProperty("Gallery", p1); }
        virtual OUString SAL_CALL getGraphic() throw (css::uno::RuntimeException)
            { return getStringProperty("Graphic"); }
        virtual void SAL_CALL setGraphic(const OUString& p1) throw (css::uno::RuntimeException)
            { setStringProperty("Graphic", p1); }
        virtual OUString SAL_CALL getHelp() throw (css::uno::RuntimeException)
            { return getStringProperty("Help"); }
        virtual void SAL_CALL setHelp(const OUString& p1) throw (css::uno::RuntimeException)
            { setStringProperty("Help", p1); }
        virtual OUString SAL_CALL getLinguistic() throw (css::uno::RuntimeException)
            { return getStringProperty("Linguistic"); }
        virtual void SAL_CALL setLinguistic(const OUString& p1) throw (css::uno::RuntimeException)
            { setStringProperty("Linguistic", p1); }
        virtual OUString SAL_CALL getModule() throw (css::uno::RuntimeException)
            { return getStringProperty("Module"); }
        virtual void SAL_CALL setModule(const OUString& p1) throw (css::uno::RuntimeException)
            { setStringProperty("Module", p1); }
        virtual OUString SAL_CALL getPalette() throw (css::uno::RuntimeException)
            { return getStringProperty("Palette"); }
        virtual void SAL_CALL setPalette(const OUString& p1) throw (css::uno::RuntimeException)
            { setStringProperty("Palette", p1); }
        virtual OUString SAL_CALL getPlugin() throw (css::uno::RuntimeException)
            { return getStringProperty("Plugin"); }
        virtual void SAL_CALL setPlugin(const OUString& p1) throw (css::uno::RuntimeException)
            { setStringProperty("Plugin", p1); }
        virtual OUString SAL_CALL getStorage() throw (css::uno::RuntimeException)
            { return getStringProperty("Storage"); }
        virtual void SAL_CALL setStorage(const OUString& p1) throw (css::uno::RuntimeException)
            { setStringProperty("Storage", p1); }
        virtual OUString SAL_CALL getTemp() throw (css::uno::RuntimeException)
            { return getStringProperty("Temp"); }
        virtual void SAL_CALL setTemp(const OUString& p1) throw (css::uno::RuntimeException)
            { setStringProperty("Temp", p1); }
        virtual OUString SAL_CALL getTemplate() throw (css::uno::RuntimeException)
            { return getStringProperty("Template"); }
        virtual void SAL_CALL setTemplate(const OUString& p1) throw (css::uno::RuntimeException)
            { setStringProperty("Template", p1); }
        virtual OUString SAL_CALL getUIConfig() throw (css::uno::RuntimeException)
            { return getStringProperty("UIConfig"); }
        virtual void SAL_CALL setUIConfig(const OUString& p1) throw (css::uno::RuntimeException)
            { setStringProperty("UIConfig", p1); }
        virtual OUString SAL_CALL getUserConfig() throw (css::uno::RuntimeException)
            { return getStringProperty("UserConfig"); }
        virtual void SAL_CALL setUserConfig(const OUString& p1) throw (css::uno::RuntimeException)
            { setStringProperty("UserConfig", p1); }
        virtual OUString SAL_CALL getUserDictionary() throw (css::uno::RuntimeException)
            { return getStringProperty("UserDictionary"); }
        virtual void SAL_CALL setUserDictionary(const OUString& p1) throw (css::uno::RuntimeException)
            { setStringProperty("UserDictionary", p1); }
        virtual OUString SAL_CALL getWork() throw (css::uno::RuntimeException)
            { return getStringProperty("Work"); }
        virtual void SAL_CALL setWork(const OUString& p1) throw (css::uno::RuntimeException)
            { setStringProperty("Work", p1); }
        virtual OUString SAL_CALL getBasePathShareLayer() throw (css::uno::RuntimeException)
            { return getStringProperty("UIConfig"); }
        virtual void SAL_CALL setBasePathShareLayer(const OUString& p1) throw (css::uno::RuntimeException)
            { setStringProperty("UIConfig", p1); }
        virtual OUString SAL_CALL getBasePathUserLayer() throw (css::uno::RuntimeException)
            { return getStringProperty("UserConfig"); }
        virtual void SAL_CALL setBasePathUserLayer(const OUString& p1) throw (css::uno::RuntimeException)
            { setStringProperty("UserConfig", p1); }



        /**
         * overrides to resolve inheritance ambiguity
         */
        virtual void SAL_CALL setPropertyValue(const OUString& p1, const css::uno::Any& p2)
            throw (css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException)
            { ::cppu::OPropertySetHelper::setPropertyValue(p1, p2); }
        virtual css::uno::Any SAL_CALL getPropertyValue(const OUString& p1)
            throw (com::sun::star::beans::UnknownPropertyException, com::sun::star::lang::WrappedTargetException, com::sun::star::uno::RuntimeException)
            { return ::cppu::OPropertySetHelper::getPropertyValue(p1); }
        virtual void SAL_CALL addPropertyChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XPropertyChangeListener>& p2)
            throw (com::sun::star::beans::UnknownPropertyException, com::sun::star::lang::WrappedTargetException, com::sun::star::uno::RuntimeException)
            { ::cppu::OPropertySetHelper::addPropertyChangeListener(p1, p2); }
        virtual void SAL_CALL removePropertyChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XPropertyChangeListener>& p2)
            throw (com::sun::star::beans::UnknownPropertyException, com::sun::star::lang::WrappedTargetException, com::sun::star::uno::RuntimeException)
            { ::cppu::OPropertySetHelper::removePropertyChangeListener(p1, p2); }
        virtual void SAL_CALL addVetoableChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XVetoableChangeListener>& p2)
            throw (com::sun::star::beans::UnknownPropertyException, com::sun::star::lang::WrappedTargetException, com::sun::star::uno::RuntimeException)
            { ::cppu::OPropertySetHelper::addVetoableChangeListener(p1, p2); }
        virtual void SAL_CALL removeVetoableChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XVetoableChangeListener>& p2)
            throw (com::sun::star::beans::UnknownPropertyException, com::sun::star::lang::WrappedTargetException, com::sun::star::uno::RuntimeException)
            { ::cppu::OPropertySetHelper::removeVetoableChangeListener(p1, p2); }


    // ___________________________________________
    // helper

    private:

        OUString getStringProperty(const OUString& p1)
            throw(css::uno::RuntimeException);

        void setStringProperty(const OUString& p1, const OUString& p2)
            throw(css::uno::RuntimeException);

        /** read all configured paths and create all needed internal structures. */
        void impl_readAll();

        /** read a path info using the old cfg schema.
            This is needed for "migration on demand" reasons only.
            Can be removed for next major release .-) */
        OUStringList impl_readOldFormat(const OUString& sPath);

        /** read a path info using the new cfg schema. */
        PathSettings::PathInfo impl_readNewFormat(const OUString& sPath);

        /** filter "real user defined paths" from the old configuration schema
            and set it as UserPaths on the new schema.
            Can be removed with new major release ... */

        void impl_mergeOldUserPaths(      PathSettings::PathInfo& rPath,
                                     const OUStringList&           lOld );

        /** reload one path directly from the new configuration schema (because
            it was updated by any external code) */
        PathSettings::EChangeOp impl_updatePath(const OUString& sPath          ,
                                                      sal_Bool         bNotifyListener);

        /** replace all might existing placeholder variables inside the given path ...
            or check if the given path value uses paths, which can be replaced with predefined
            placeholder variables ...
         */
        void impl_subst(      OUStringList&                                          lVals   ,
                        const css::uno::Reference< css::util::XStringSubstitution >& xSubst  ,
                              sal_Bool                                               bReSubst);

        void impl_subst(PathSettings::PathInfo& aPath   ,
                        sal_Bool                bReSubst);


        /** converts our new string list schema to the old ";" separated schema ... */
        OUString impl_convertPath2OldStyle(const PathSettings::PathInfo& rPath        ) const;
        OUStringList    impl_convertOldStyle2Path(const OUString&        sOldStylePath) const;

        /** remove still known paths from the given lList argument.
            So real user defined paths can be extracted from the list of
            fix internal paths !
         */
        void impl_purgeKnownPaths(const PathSettings::PathInfo& rPath,
                                         OUStringList&           lList);

        /** rebuild the member m_lPropDesc using the path list m_lPaths. */
        void impl_rebuildPropertyDescriptor();

        /** provides direct access to the list of path values
            using it's internal property id.
         */
        css::uno::Any impl_getPathValue(      sal_Int32      nID ) const;
        void          impl_setPathValue(      sal_Int32      nID ,
                                        const css::uno::Any& aVal);

        /** check the given handle and return the corresponding PathInfo reference.
            These reference can be used then directly to manipulate these path. */
              PathSettings::PathInfo* impl_getPathAccess     (sal_Int32 nHandle);
        const PathSettings::PathInfo* impl_getPathAccessConst(sal_Int32 nHandle) const;

        /** it checks, if the given path value seams to be a valid URL or system path. */
        sal_Bool impl_isValidPath(const OUString& sPath) const;
        sal_Bool impl_isValidPath(const OUStringList&    lPath) const;

        void impl_storePath(const PathSettings::PathInfo& aPath);

        css::uno::Sequence< sal_Int32 > impl_mapPathName2IDList(const OUString& sPath);

        void impl_notifyPropListener(      PathSettings::EChangeOp eOp     ,
                                           const OUString&        sPath   ,
                                           const PathSettings::PathInfo* pPathOld,
                                           const PathSettings::PathInfo* pPathNew);


        //  OPropertySetHelper
        virtual sal_Bool                                            SAL_CALL convertFastPropertyValue        (       css::uno::Any&  aConvertedValue ,
                                                                                                                     css::uno::Any&  aOldValue       ,
                                                                                                                     sal_Int32       nHandle         ,
                                                                                                               const css::uno::Any&  aValue          ) throw(css::lang::IllegalArgumentException);
        virtual void                                                SAL_CALL setFastPropertyValue_NoBroadcast(       sal_Int32       nHandle         ,
                                                                                                               const css::uno::Any&  aValue          ) throw(css::uno::Exception);
        using cppu::OPropertySetHelper::getFastPropertyValue;
        virtual void                                                SAL_CALL getFastPropertyValue            (       css::uno::Any&  aValue          ,
                                                                                                                     sal_Int32       nHandle         ) const;
        virtual ::cppu::IPropertyArrayHelper&                       SAL_CALL getInfoHelper                   (                                       );
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo              (                                       ) throw(::css::uno::RuntimeException);

        /** factory methods to guarantee right (but on demand) initialized members ... */
        css::uno::Reference< css::util::XStringSubstitution > fa_getSubstitution();
        css::uno::Reference< css::container::XNameAccess >    fa_getCfgOld();
        css::uno::Reference< css::container::XNameAccess >    fa_getCfgNew();
};

} // namespace framework

#endif // __FRAMEWORK_SERVICES_PATHSETTINGS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
