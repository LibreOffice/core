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

#ifndef _CONNECTIVITY_EVOAB_LDRIVER_HXX_
#define _CONNECTIVITY_EVOAB_LDRIVER_HXX_

#include <cppuhelper/compbase2.hxx>
#include "connectivity/CommonTools.hxx"
#include "file/FDriver.hxx"
#include <unotools/tempfile.hxx>
#include <osl/file.h>
#include <osl/process.h>

#define EVOAB_EVOLUTION_SCHEMA  "evolution"
#define EVOAB_LDAP_SCHEMA   "ldap"
#define EVOAB_DRIVER_IMPL_NAME  "com.sun.star.comp.sdbc.evoab.OEvoabDriver"

namespace connectivity
{
    namespace evoab
    {

        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL OEvoabDriver_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) throw( ::com::sun::star::uno::Exception );

        class OEvoabDriver : public file::OFileDriver
        {
        private:
            enum MaybeVoidBool
            {
                eUnknown,
                eTrue,
                eFalse
            };
            ::utl::TempFile m_aTempDir;
            rtl::OUString   m_aFolderListName;
            rtl::OUString   m_aVersionName;
            rtl::OUString   m_aFileExt;
            rtl::OUString   m_aWorkingDirURL;
            rtl::OUString   m_aEvoab_CLI_EffectiveCommand;
            rtl::OUString   m_aEvoab_CLI_FullPathCommand;
            sal_Bool        m_bWorkingDirCreated;
            MaybeVoidBool   m_eSupportedEvoVersion;

            sal_Bool        fileExists(const ::rtl::OUString& _rURL, sal_Bool _bIsDir=sal_False) const;

        public:
            OEvoabDriver(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory);

            inline rtl::OUString    getEvoFolderListName()          const { return m_aFolderListName;}
            inline rtl::OUString    getFileExt()                    const { return m_aFileExt;}
            inline rtl::OUString    getEvoFolderListFileName()      const { return m_aFolderListName + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".")) + m_aFileExt;}
            inline rtl::OUString    getEvoVersionFileName()         const { return m_aVersionName + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".")) + m_aFileExt;}
            inline rtl::OUString    getWorkingDirURL()              const { return m_aWorkingDirURL;}
            inline rtl::OUString    getEvoab_CLI_FullPathCommand()  const { return m_aEvoab_CLI_FullPathCommand;}
            inline rtl::OUString    getEvoab_CLI_EffectiveCommand() const { return m_aEvoab_CLI_EffectiveCommand;}
            inline sal_Bool         doesEvoab_CLI_HavePath()        const { return m_aEvoab_CLI_FullPathCommand.lastIndexOf('/')!=-1;}
            const rtl::OUString     getEvoab_CLI_Command() const;
            const rtl::OUString     getEvoab_CLI_Path() const;
            const rtl::OUString     getWorkingDirPath() const;
            const String            getEvoFolderListFileURL() const;
            String                  getTempDirURL() const;

            // XInterface
            ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
            // XDriver
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL connect( const ::rtl::OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL acceptsURL( const ::rtl::OUString& url ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sdbc::DriverPropertyInfo > SAL_CALL getPropertyInfo( const ::rtl::OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);


            // static methods
            static ::rtl::OUString getImplementationName_Static(  ) throw(::com::sun::star::uno::RuntimeException);
            static sal_Bool acceptsURL_Stat( const ::rtl::OUString& url );
            static ::rtl::OUString translateProcessErrorMessage( oslProcessError nProcErr);
            static ::rtl::OUString translateFileErrorMessage( oslFileError nFileErr);
            static const sal_Char*    getSDBC_SCHEME_EVOLUTION();
            static const sal_Char*    getEVOAB_FOLDERLIST_FILE_NAME();
            static const sal_Char*    getEVOAB_VERSION_FILE_NAME();
            static const sal_Char*    getEVOAB_META_FILE_EXT();
            static const sal_Char*    getEVOAB_CLI_FULLPATHCOMMAND();
            static const sal_Char*    getEVOAB_CLI_ARG_LIST_FOLDERS();
            static const sal_Char*    getEVOAB_CLI_ARG_OUTPUT_FILE_PREFIX();
            static const sal_Char*    getEVOAB_CLI_ARG_OUTPUT_FORMAT();
            static const sal_Char*    getEVOAB_CLI_ARG_VERSION();
            static const sal_Char*    getEVOAB_CLI_ARG_OUTPUT_REDIRECT();

        };
    }

}
#endif //_CONNECTIVITY_EVOAB_LDRIVER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
