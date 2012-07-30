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

#ifndef __FRAMEWORK_SERVICES_PATHSETTINGS_HXX_
#define __FRAMEWORK_SERVICES_PATHSETTINGS_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <stdtypes.h>
#include <properties.h>
#include <stdtypes.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/util/XStringSubstitution.hpp>
#include <com/sun/star/util/XChangesListener.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/weak.hxx>
#include <unotools/configitem.hxx>
#include <comphelper/sequenceasvector.hxx>

/* enable it if you whish to migrate old user settings (using the old cfg schema) on demand ....
   disable it in case only the new schema must be used.
 */

namespace framework
{

class PathSettings : public  css::lang::XTypeProvider             ,
                     public  css::lang::XServiceInfo              ,
                     public  css::util::XChangesListener          , // => XEventListener
                     // base classes
                     // Order is neccessary for right initialization!
                     private ThreadHelpBase                       ,
                     public  ::cppu::OBroadcastHelper             ,
                     public  ::cppu::OPropertySetHelper           , // => XPropertySet / XFastPropertySet / XMultiPropertySet
                     public  ::cppu::OWeakObject                    // => XWeak, XInterface
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
            ::rtl::OUString sPathName;

            /// contains all paths, which are used internaly - but are not visible for the user.
            OUStringList lInternalPaths;

            /// contains all paths configured by the user
            OUStringList lUserPaths;

            /// this special path is used to generate feature depending content there
            ::rtl::OUString sWritePath;

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
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

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
        PathSettings(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);

        /** free all used resources ... if it was not already done. */
        virtual ~PathSettings();

        /** declaration of XInterface, XTypeProvider, XServiceInfo */
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        // css::util::XChangesListener
        virtual void SAL_CALL changesOccurred(const css::util::ChangesEvent& aEvent) throw (css::uno::RuntimeException);

        // css::lang::XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& aSource)
            throw(css::uno::RuntimeException);

        using ::cppu::OPropertySetHelper::disposing;

    // ___________________________________________
    // helper

    private:

        /** read all configured paths and create all needed internal structures. */
        void impl_readAll();

        /** read a path info using the old cfg schema.
            This is needed for "migration on demand" reasons only.
            Can be removed for next major release .-) */
        OUStringList impl_readOldFormat(const ::rtl::OUString& sPath);

        /** read a path info using the new cfg schema. */
        PathSettings::PathInfo impl_readNewFormat(const ::rtl::OUString& sPath);

        /** filter "real user defined paths" from the old configuration schema
            and set it as UserPaths on the new schema.
            Can be removed with new major release ... */

        void impl_mergeOldUserPaths(      PathSettings::PathInfo& rPath,
                                     const OUStringList&           lOld );

        /** reload one path directly from the new configuration schema (because
            it was updated by any external code) */
        PathSettings::EChangeOp impl_updatePath(const ::rtl::OUString& sPath          ,
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


        /** converts our new string list schema to the old ";" seperated schema ... */
        ::rtl::OUString impl_convertPath2OldStyle(const PathSettings::PathInfo& rPath        ) const;
        OUStringList    impl_convertOldStyle2Path(const ::rtl::OUString&        sOldStylePath) const;

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
        sal_Bool impl_isValidPath(const ::rtl::OUString& sPath) const;
        sal_Bool impl_isValidPath(const OUStringList&    lPath) const;

        void impl_storePath(const PathSettings::PathInfo& aPath);

        css::uno::Sequence< sal_Int32 > impl_mapPathName2IDList(const ::rtl::OUString& sPath);

        void impl_notifyPropListener(      PathSettings::EChangeOp eOp     ,
                                           const ::rtl::OUString&        sPath   ,
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
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo              (                                       ) throw(::com::sun::star::uno::RuntimeException);

        /** factory methods to guarantee right (but on demand) initialized members ... */
        css::uno::Reference< css::util::XStringSubstitution > fa_getSubstitution();
        css::uno::Reference< css::container::XNameAccess >    fa_getCfgOld();
        css::uno::Reference< css::container::XNameAccess >    fa_getCfgNew();
};

} // namespace framework

#endif // __FRAMEWORK_SERVICES_PATHSETTINGS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
