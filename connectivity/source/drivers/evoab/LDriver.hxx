/*************************************************************************
 *
 *  $RCSfile: LDriver.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:01:41 $
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

#ifndef _CONNECTIVITY_EVOAB_LDRIVER_HXX_
#define _CONNECTIVITY_EVOAB_LDRIVER_HXX_

#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include "connectivity/CommonTools.hxx"
#endif
#ifndef _CONNECTIVITY_FILE_ODRIVER_HXX_
#include "file/FDriver.hxx"
#endif
#ifndef _UNOTOOLS_TEMPFILE_HXX
#include <unotools/tempfile.hxx>
#endif
#ifndef _OSL_FILE_H_
#include <osl/file.h>
#endif
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif

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
            ::utl::TempFile m_aTempDir;
            rtl::OUString   m_aFolderListName;
            rtl::OUString   m_aVersionName;
            rtl::OUString   m_aFileExt;
            rtl::OUString   m_aWorkingDirURL;
            rtl::OUString   m_aEvoab_CLI_EffectiveCommand;
            rtl::OUString   m_aEvoab_CLI_FullPathCommand;
            sal_Bool        m_bWorkingDirCreated;
            sal_Bool        fileExists(const ::rtl::OUString& _rURL, sal_Bool _bIsDir=sal_False) const;

        public:
            OEvoabDriver(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory);

            inline rtl::OUString    getEvoFolderListName()          const { return m_aFolderListName;}
            inline rtl::OUString    getFileExt()                    const { return m_aFileExt;}
            inline rtl::OUString    getEvoFolderListFileName()      const { return m_aFolderListName + ::rtl::OUString::createFromAscii(".") + m_aFileExt;}
            inline rtl::OUString    getEvoVersionFileName()         const { return m_aVersionName + ::rtl::OUString::createFromAscii(".") + m_aFileExt;}
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
