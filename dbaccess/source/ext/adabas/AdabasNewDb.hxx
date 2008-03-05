/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AdabasNewDb.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-05 08:43:36 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef ADABASUI_ADABAS_CREATEDB_HXX
#define ADABASUI_ADABAS_CREATEDB_HXX


#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif

#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCREATECATALOG_HPP_
#include <com/sun/star/sdbcx/XCreateCatalog.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif
#include <memory>
#include "ASQLNameEdit.hxx"

class WaitObject;

namespace adabasui
{
    class ODatabaseCreator;
    //========================================================================
    // ADABAS for new db; is called from typepage
    //========================================================================
    class OAdabasNewDbDlg : public ModalDialog
    {
    private:
        FixedText           m_FT_DATABASENAME;
        OSQLNameEdit        m_ET_DATABASENAME;

        FixedLine           m_FL_USER;
        FixedText           m_FT_SYSUSR;
        FixedText           m_FT_CONUSR;
        FixedText           m_FT_DOMAIN_USR;

        OSQLNameEdit        m_ET_SYSUSR;
        PushButton          m_PB_SYSPWD;
        OSQLNameEdit        m_ET_CONUSR;
        PushButton          m_PB_CONPWD;
        OSQLNameEdit        m_ET_DOMAIN_USR;
        PushButton          m_PB_DOMAINPWD;

        FixedImage          m_BMP_CON;
        FixedImage          m_BMP_SYS;
        FixedImage          m_BMP_DOMAIN;

        FixedText           m_FT_RESTORE;
        Edit                m_ET_RESTORE;
        PushButton          m_PB_RESTORE;

        FixedLine           m_FL_DBSETTINGS;
        FixedText           m_FT_SYSDEVSPACE;
        FixedText           m_FT_TRANSACTIONLOG;
        FixedText           m_FT_DATADEVSPACE;
        Edit                m_ET_SYSDEVSPACE;
        PushButton          m_PB_SYSDEVSPACE;
        Edit                m_ET_TRANSACTIONLOG;
        PushButton          m_PB_TRANSACTIONLOG;
        Edit                m_ET_DATADEVSPACE;
        PushButton          m_PB_DATADEVSPACE;

        FixedText           m_FT_TRANSACTIONLOG_SIZE;
        NumericField        m_NF_TRANSACTIONLOG_SIZE;
        FixedText           m_FT_DATADEVSPACE_SIZE;
        NumericField        m_NF_DATADEVSPACE_SIZE;
        FixedText           m_FT_CACHE_SIZE;
        NumericField        m_NF_CACHE_SIZE;

        FixedLine           m_FL_END;

        OKButton            m_PB_OK;
        CancelButton        m_PB_CANCEL;

        String              m_SYSDEV_File;
        String              m_TRANSDEV_File;
        String              m_DATADEV_File;
        String              m_Backup_File;
        String              m_ConPwd;
        String              m_SysPwd;
        String              m_DomainPwd;
        String              m_DBName;
        String              m_sDbWork;
        String              m_sDbConfig;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XCreateCatalog>      m_xCreateCatalog;
        com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >    m_xORB;
        BOOL                m_bRestore;
        INT8                m_nSetBitmap;

        ::std::auto_ptr<WaitObject>         m_aCreatorWait;
        ::std::auto_ptr<ODatabaseCreator>   m_aCreatorThread;

        DECL_LINK( LoadButtonClickHdl,  Button      * );
        DECL_LINK( ButtonClickHdl,      Button      * );
        DECL_LINK( PwdClickHdl,         Button      * );
        DECL_LINK( ImplOKHdl,           OKButton    * );
        DECL_LINK( LoseFocusHdl,        Edit        * );
        DECL_LINK( TerminateHdl,        void        * );

        void CheckBitmaps();
        void ShowErrorText(INT32 _nError);

        DECL_LINK(OnError, void*);
        DECL_LINK(OnNoDefaultPath, void*);
        DECL_LINK(OnNoAccessRights, void*);

        sal_Bool fillEnvironmentVariable(const ::rtl::OUString& _sVariableName,String& _rsValue);
    public:
        OAdabasNewDbDlg(Window* pParent,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XCreateCatalog>& _rxCreateCatalog,
                        const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& _rxORB,
                        BOOL _bRestore=FALSE);
        ~OAdabasNewDbDlg();

        ::rtl::OUString GetDatabaseName()       const { return m_ET_DATABASENAME.GetText(); }
        ::rtl::OUString GetControlUser()        const { return m_ET_CONUSR.GetText();       }
        ::rtl::OUString GetControlPassword()    const { return m_ConPwd;                    }
        ::rtl::OUString GetUser()               const { return m_ET_SYSUSR.GetText();       }
        ::rtl::OUString GetUserPassword()       const { return m_SysPwd;                    }
        sal_Int32       GetCacheSize()          const { return static_cast<sal_Int32>(m_NF_CACHE_SIZE.GetValue());  }
    };
}
#endif // adabasui_ADABAS_CREATEDB_HXX

