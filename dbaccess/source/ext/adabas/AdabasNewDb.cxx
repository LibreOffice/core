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

#ifndef adabasui_ADABAS_CREATEDB_HXX
#include "AdabasNewDb.hxx"
#endif
#ifndef adabasui_ADABASNEWDB_HRC
#include "AdabasNewDb.hrc"
#endif

#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _PASSWD_HXX //autogen
#include <sfx2/passwd.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _BIGINT_HXX
#include <tools/bigint.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _EXTENSIONS_COMPONENT_MODULE_HXX_
#include "Acomponentmodule.hxx"
#endif
#ifndef _UNOTOOLS_TEMPFILE_HXX
#include <unotools/tempfile.hxx>
#endif
#ifndef _ADABASUI_RESOURCE_HRC_
#include "adabasui_resource.hrc"
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <unotools/pathoptions.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _UNOTOOLS_TEMPFILE_HXX
#include <unotools/tempfile.hxx>
#endif
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_SQLEXCEPTION_HPP_
#include <com/sun/star/sdbc/SQLException.hpp>
#endif
#ifndef _UNOTOOLS_UCBHELPER_HXX
#include <unotools/ucbhelper.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#include "adabasuistrings.hrc"
#ifndef _THREAD_HXX_
#include <osl/thread.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif


using namespace adabasui;
using namespace ucbhelper;
using namespace utl;
using namespace vos;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::lang;
namespace adabasui
{
    class ODatabaseCreator : public ::osl::Thread
    {
        Link                                m_aTerminateHandler;
        Sequence< PropertyValue>            m_aValues;
        Reference<XCreateCatalog>           m_xCreateCatalog;
        Reference< XMultiServiceFactory >   m_xORB;
        Window*                             m_pErrorWindow;
    public:
        ODatabaseCreator(const Reference<XCreateCatalog>& _rxCreateCatalog
                        ,Window* _pErrorWindow
                        ,const Reference< XMultiServiceFactory >& _xORB
                        ,const Sequence< PropertyValue>& _rValues)
            : m_aValues(_rValues)
            , m_xCreateCatalog(_rxCreateCatalog)
            , m_xORB(_xORB)
            , m_pErrorWindow(_pErrorWindow)
        {}

        void setTerminateHandler(const Link& _aTerminateHandler) { m_aTerminateHandler = _aTerminateHandler; }

    protected:

        /// Working method which should be overridden.
        virtual void SAL_CALL run();
        virtual void SAL_CALL onTerminated();
    };

    void SAL_CALL ODatabaseCreator::run()
    {
        try
        {
            m_xCreateCatalog->createCatalog(m_aValues);
        }
        catch(SQLException& e)
        {
            ::dbtools::showError(::dbtools::SQLExceptionInfo(e),VCLUnoHelper::GetInterface(m_pErrorWindow),m_xORB);
        }
        catch(Exception&)
        {
            OSL_ASSERT(0);
            //  showError(::dbtools::SQLExceptionInfo(e),this,);
        }
    }
    void SAL_CALL ODatabaseCreator::onTerminated()
    {
        if ( m_aTerminateHandler.IsSet() )
            m_aTerminateHandler.Call(NULL);
    }
}
DBG_NAME( OAdabasNewDbDlg )
//========================================================================
OAdabasNewDbDlg::OAdabasNewDbDlg( Window* pParent,
                                 const Reference<XCreateCatalog>& _rxCreateCatalog,
                                 const Reference< XMultiServiceFactory >&   _rxORB,
                                 sal_Bool _bRestore)
    :ModalDialog( pParent, ModuleRes( DLG_ADABAS_NEWDB ))
    ,m_FT_DATABASENAME(         this, ModuleRes( FT_DATABASENAME        ) )
    ,m_ET_DATABASENAME(         this, ModuleRes( ET_DATABASENAME        ) )
    ,m_FL_USER(                 this, ModuleRes( FL_USER                ) )
    ,m_FT_SYSUSR(               this, ModuleRes( FT_SYSUSR              ) )
    ,m_FT_CONUSR(               this, ModuleRes( FT_CONUSR              ) )
    ,m_FT_DOMAIN_USR(           this, ModuleRes( FT_DOMAIN_USR          ) )
    ,m_ET_SYSUSR(               this, ModuleRes( ET_SYSUSR              ) )
    ,m_PB_SYSPWD(               this, ModuleRes( PB_SYSPWD              ) )
    ,m_ET_CONUSR(               this, ModuleRes( ET_CONUSR              ) )
    ,m_PB_CONPWD(               this, ModuleRes( PB_CONPWD              ) )
    ,m_ET_DOMAIN_USR(           this, ModuleRes( ET_DOMAIN_USR          ) )
    ,m_PB_DOMAINPWD(            this, ModuleRes( PB_DOMAINPWD           ) )
    ,m_BMP_CON(                 this, ModuleRes( BMP_CON                ) )
    ,m_BMP_SYS(                 this, ModuleRes( BMP_SYS                ) )
    ,m_BMP_DOMAIN(              this, ModuleRes( BMP_DOMAIN             ) )
    ,m_FT_RESTORE(              this, ModuleRes( FT_RESTORE             ) )
    ,m_ET_RESTORE(              this, ModuleRes( ET_RESTORE             ) )
    ,m_PB_RESTORE(              this, ModuleRes( PB_RESTORE             ) )
    ,m_FL_DBSETTINGS(           this, ModuleRes( FL_DBSETTINGS          ) )
    ,m_FT_SYSDEVSPACE(          this, ModuleRes( FT_SYSDEVSPACE         ) )
    ,m_FT_TRANSACTIONLOG(       this, ModuleRes( FT_TRANSACTIONLOG      ) )
    ,m_FT_DATADEVSPACE(         this, ModuleRes( FT_DATADEVSPACE        ) )
    ,m_ET_SYSDEVSPACE(          this, ModuleRes( ET_SYSDEVSPACE         ) )
    ,m_PB_SYSDEVSPACE(          this, ModuleRes( PB_SYSDEVSPACE         ) )
    ,m_ET_TRANSACTIONLOG(       this, ModuleRes( ET_TRANSACTIONLOG      ) )
    ,m_PB_TRANSACTIONLOG(       this, ModuleRes( PB_TRANSACTIONLOG      ) )
    ,m_ET_DATADEVSPACE(         this, ModuleRes( ET_DATADEVSPACE        ) )
    ,m_PB_DATADEVSPACE(         this, ModuleRes( PB_DATADEVSPACE        ) )
    ,m_FT_TRANSACTIONLOG_SIZE(  this, ModuleRes( FT_TRANSACTIONLOG_SIZE ) )
    ,m_NF_TRANSACTIONLOG_SIZE(  this, ModuleRes( NF_TRANSACTIONLOG_SIZE ) )
    ,m_FT_DATADEVSPACE_SIZE(    this, ModuleRes( FT_DATADEVSPACE_SIZE   ) )
    ,m_NF_DATADEVSPACE_SIZE(    this, ModuleRes( NF_DATADEVSPACE_SIZE   ) )
    ,m_FT_CACHE_SIZE(           this, ModuleRes( FT_CACHE_SIZE          ) )
    ,m_NF_CACHE_SIZE(           this, ModuleRes( NF_CACHE_SIZE          ) )
    ,m_FL_END(                  this, ModuleRes( FL_END                 ) )
    ,m_PB_OK(                   this, ModuleRes( PB_OK                  ) )
    ,m_PB_CANCEL(               this, ModuleRes( PB_CANCEL              ) )
    ,m_xCreateCatalog(_rxCreateCatalog)
    ,m_xORB(_rxORB)
    ,m_bRestore(_bRestore)
    ,m_nSetBitmap(0)
{
    DBG_CTOR( OAdabasNewDbDlg, NULL );
    FreeResource();
    if(m_bRestore)
    {
        m_FL_DBSETTINGS.SetPosPixel(m_FL_DBSETTINGS.GetPosPixel()-LogicToPixel( Point(0,14), MAP_APPFONT ));
        m_FT_SYSDEVSPACE.SetPosPixel(m_FT_SYSDEVSPACE.GetPosPixel()-LogicToPixel( Point(0,14), MAP_APPFONT ));
        m_FT_TRANSACTIONLOG.SetPosPixel(m_FT_TRANSACTIONLOG.GetPosPixel()-LogicToPixel( Point(0,14), MAP_APPFONT ));
        m_FT_DATADEVSPACE.SetPosPixel(m_FT_DATADEVSPACE.GetPosPixel()-LogicToPixel( Point(0,14), MAP_APPFONT ));
        m_ET_SYSDEVSPACE.SetPosPixel(m_ET_SYSDEVSPACE.GetPosPixel()-LogicToPixel( Point(0,14), MAP_APPFONT ));
        m_PB_SYSDEVSPACE.SetPosPixel(m_PB_SYSDEVSPACE.GetPosPixel()-LogicToPixel( Point(0,14), MAP_APPFONT ));
        m_ET_TRANSACTIONLOG.SetPosPixel(m_ET_TRANSACTIONLOG.GetPosPixel()-LogicToPixel( Point(0,14), MAP_APPFONT ));
        m_PB_TRANSACTIONLOG.SetPosPixel(m_PB_TRANSACTIONLOG.GetPosPixel()-LogicToPixel( Point(0,14), MAP_APPFONT ));
        m_ET_DATADEVSPACE.SetPosPixel(m_ET_DATADEVSPACE.GetPosPixel()-LogicToPixel( Point(0,14), MAP_APPFONT ));
        m_PB_DATADEVSPACE.SetPosPixel(m_PB_DATADEVSPACE.GetPosPixel()-LogicToPixel( Point(0,14), MAP_APPFONT ));
        m_FT_TRANSACTIONLOG_SIZE.SetPosPixel(m_FT_TRANSACTIONLOG_SIZE.GetPosPixel()-LogicToPixel( Point(0,14), MAP_APPFONT ));
        m_NF_TRANSACTIONLOG_SIZE.SetPosPixel(m_NF_TRANSACTIONLOG_SIZE.GetPosPixel()-LogicToPixel( Point(0,14), MAP_APPFONT ));
        m_FT_DATADEVSPACE_SIZE.SetPosPixel(m_FT_DATADEVSPACE_SIZE.GetPosPixel()-LogicToPixel( Point(0,14), MAP_APPFONT ));
        m_NF_DATADEVSPACE_SIZE.SetPosPixel(m_NF_DATADEVSPACE_SIZE.GetPosPixel()-LogicToPixel( Point(0,14), MAP_APPFONT ));
        m_FT_CACHE_SIZE.SetPosPixel(m_FT_CACHE_SIZE.GetPosPixel()-LogicToPixel( Point(0,14), MAP_APPFONT ));
        m_NF_CACHE_SIZE.SetPosPixel(m_NF_CACHE_SIZE.GetPosPixel()-LogicToPixel( Point(0,14), MAP_APPFONT ));
        m_PB_OK.SetPosPixel(m_PB_OK.GetPosPixel()-LogicToPixel( Point(0,14), MAP_APPFONT ));
        m_PB_CANCEL.SetPosPixel(m_PB_CANCEL.GetPosPixel()-LogicToPixel( Point(0,14), MAP_APPFONT ));

        Size aSize(GetSizePixel().Width(),GetSizePixel().Height()-LogicToPixel( Size(0,14), MAP_APPFONT ).Height());
        SetSizePixel(aSize);

        m_FT_SYSUSR.Hide();
        m_FT_DOMAIN_USR.Hide();
        m_FL_USER.SetText(ModuleRes(STR_BACKUPFILE_LOAD));
        m_PB_SYSPWD.Hide();
        m_PB_DOMAINPWD.Hide();
        m_ET_SYSUSR.Hide();
        m_ET_DOMAIN_USR.Hide();

        m_ET_SYSUSR.SetText(String::CreateFromAscii("DUMMY"));
        m_BMP_SYS.Hide();
        m_BMP_DOMAIN.Hide();
        m_ET_RESTORE.SetModifyHdl(              LINK(this,OAdabasNewDbDlg,LoseFocusHdl));
        m_ET_RESTORE.SetLoseFocusHdl(           LINK(this,OAdabasNewDbDlg,LoseFocusHdl));
        m_ET_RESTORE.SetMaxTextLen(40);
    }
    else
    {
        m_FT_RESTORE.Hide();
        m_ET_RESTORE.Hide();
        m_PB_RESTORE.Hide();

        m_ET_SYSUSR.SetModifyHdl(               LINK(this,OAdabasNewDbDlg,LoseFocusHdl));
        m_ET_CONUSR.SetModifyHdl(               LINK(this,OAdabasNewDbDlg,LoseFocusHdl));
        m_ET_DOMAIN_USR.Enable(sal_False);

        //  m_ET_SYSUSR.setUpperCase();
        //  m_ET_CONUSR.setUpperCase();
    }


    m_ET_SYSDEVSPACE.SetLoseFocusHdl(       LINK(this,OAdabasNewDbDlg,LoseFocusHdl));
    m_ET_TRANSACTIONLOG.SetLoseFocusHdl(    LINK(this,OAdabasNewDbDlg,LoseFocusHdl));
    m_ET_DATADEVSPACE.SetLoseFocusHdl(      LINK(this,OAdabasNewDbDlg,LoseFocusHdl));

    m_ET_SYSDEVSPACE.SetMaxTextLen(40);
    m_ET_TRANSACTIONLOG.SetMaxTextLen(40);
    m_ET_DATADEVSPACE.SetMaxTextLen(40);

    m_NF_TRANSACTIONLOG_SIZE.SetModifyHdl(  LINK(this,OAdabasNewDbDlg,LoseFocusHdl));
    m_NF_DATADEVSPACE_SIZE.SetModifyHdl(    LINK(this,OAdabasNewDbDlg,LoseFocusHdl));
    m_NF_CACHE_SIZE.SetModifyHdl(           LINK(this,OAdabasNewDbDlg,LoseFocusHdl));

    m_NF_TRANSACTIONLOG_SIZE.SetMin(10);
    m_NF_DATADEVSPACE_SIZE.SetMin(10);
    m_NF_CACHE_SIZE.SetMin(4);

    m_NF_TRANSACTIONLOG_SIZE.SetMax(LONG_MAX);
    m_NF_DATADEVSPACE_SIZE.SetMax(LONG_MAX);
    m_NF_CACHE_SIZE.SetMax(LONG_MAX);

    m_NF_TRANSACTIONLOG_SIZE.SetValue(20);
    m_NF_DATADEVSPACE_SIZE.SetValue(20);
    m_NF_CACHE_SIZE.SetValue(4);

    m_NF_TRANSACTIONLOG_SIZE.SetDecimalDigits(0);
    m_NF_DATADEVSPACE_SIZE.SetDecimalDigits(0);
    m_NF_CACHE_SIZE.SetDecimalDigits(0);

    m_ET_DATABASENAME.SetMaxTextLen( 8 ); //98292

    m_PB_CONPWD.SetClickHdl(            LINK(this,OAdabasNewDbDlg,PwdClickHdl));
    m_PB_SYSPWD.SetClickHdl(            LINK(this,OAdabasNewDbDlg,PwdClickHdl));
    m_PB_DOMAINPWD.SetClickHdl(         LINK(this,OAdabasNewDbDlg,PwdClickHdl));

    m_PB_RESTORE.SetClickHdl(           LINK(this,OAdabasNewDbDlg,LoadButtonClickHdl));

    m_PB_SYSDEVSPACE.SetClickHdl(       LINK(this,OAdabasNewDbDlg,ButtonClickHdl));
    m_PB_TRANSACTIONLOG.SetClickHdl(    LINK(this,OAdabasNewDbDlg,ButtonClickHdl));
    m_PB_DATADEVSPACE.SetClickHdl(      LINK(this,OAdabasNewDbDlg,ButtonClickHdl));

    m_BMP_CON.SetImage(     ModuleRes(UNCHECK_RED) );
    m_BMP_SYS.SetImage(     ModuleRes(UNCHECK_RED) );
    m_BMP_DOMAIN.SetImage(  ModuleRes(UNCHECK_RED) );

    static String s_sRoot(RTL_CONSTASCII_USTRINGPARAM("DBROOT"));
    static String s_sConfig(RTL_CONSTASCII_USTRINGPARAM("DBCONFIG"));
    static String s_sWork(RTL_CONSTASCII_USTRINGPARAM("DBWORK"));

    sal_Bool bError = fillEnvironmentVariable(s_sWork,m_sDbWork);
    if ( bError || !m_sDbWork.Len() )
        bError = fillEnvironmentVariable(s_sRoot,m_sDbWork);

    bError = fillEnvironmentVariable(s_sConfig,m_sDbConfig);
    if ( bError || !m_sDbConfig.Len() )
        bError = fillEnvironmentVariable(s_sRoot,m_sDbConfig);

    if ( m_sDbConfig.Len() && m_sDbWork.Len() )
    {
        m_sDbConfig.AppendAscii("/");
        m_sDbWork.AppendAscii("/");

        String sDefaultName,sExt;
        sDefaultName.AssignAscii("MYDB");
        String sConfig = m_sDbConfig;
        sConfig.AppendAscii("config");
        TempFile aDefaultDatabase(sDefaultName,&sExt,&sConfig);
        aDefaultDatabase.EnableKillingFile();
        INetURLObject aURL(aDefaultDatabase.GetURL());

        m_ET_DATABASENAME.SetText(aURL.GetName());
        m_DBName = m_ET_DATABASENAME.GetText();

        // create temp names for my files
        // here we need only a name the creation is done by the driver
        // so we delete the files after we get a valid name
        // with this we avoiding to mention anything when the user presses cancel

        String aSysExt;
        sal_Bool bValid = sal_True;
        {
            aSysExt.AssignAscii(".sys");
            ::utl::TempFile aTmpFile(m_ET_DATABASENAME.GetText(),&aSysExt,&m_sDbWork);
            aTmpFile.EnableKillingFile();
            bValid = aTmpFile.IsValid();
            if(bValid)
                LocalFileHelper::ConvertURLToPhysicalName(aTmpFile.GetURL(),m_SYSDEV_File);
        }
        {
            aSysExt.AssignAscii(".trs");
            ::utl::TempFile aTmpFile(m_ET_DATABASENAME.GetText(),&aSysExt,&m_sDbWork);
            aTmpFile.EnableKillingFile();
            bValid = !bValid || aTmpFile.IsValid();
            if(aTmpFile.IsValid())
                LocalFileHelper::ConvertURLToPhysicalName(aTmpFile.GetURL(),m_TRANSDEV_File);
        }
        {
            aSysExt.AssignAscii(".dat");
            ::utl::TempFile aTmpFile(m_ET_DATABASENAME.GetText(),&aSysExt,&m_sDbWork);
            aTmpFile.EnableKillingFile();
            bValid = !bValid || aTmpFile.IsValid();
            if(aTmpFile.IsValid())
                LocalFileHelper::ConvertURLToPhysicalName(aTmpFile.GetURL(),m_DATADEV_File);
        }
        if(!bValid) // we found a file which isn't valid so we report an error message
            PostUserEvent(LINK(this, OAdabasNewDbDlg, OnNoAccessRights));

    }

    if(m_SYSDEV_File.Len() < 41 && m_TRANSDEV_File.Len() < 41 && m_DATADEV_File.Len() < 41)
    {
        m_ET_SYSDEVSPACE.SetText(m_SYSDEV_File );
        m_ET_TRANSACTIONLOG.SetText(m_TRANSDEV_File);
        m_ET_DATADEVSPACE.SetText(m_DATADEV_File);
    }
    else
    {
        m_SYSDEV_File = m_TRANSDEV_File = m_DATADEV_File = String();
        PostUserEvent(LINK(this, OAdabasNewDbDlg, OnNoDefaultPath));
    }

    m_ET_CONUSR.SetText(String::CreateFromAscii("CONTROL"));

    m_PB_OK.SetClickHdl(LINK(this,OAdabasNewDbDlg,ImplOKHdl));
    m_PB_OK.Enable(sal_False);

    m_ET_DATABASENAME.SetModifyHdl( LINK(this,OAdabasNewDbDlg,LoseFocusHdl));

}
// -----------------------------------------------------------------------
OAdabasNewDbDlg::~OAdabasNewDbDlg()
{
    DBG_DTOR( OAdabasNewDbDlg, NULL );
}
//------------------------------------------------------------------
IMPL_LINK(OAdabasNewDbDlg, OnError, void*, /*_pArg*/)
{
    String sErrorMsg(ModuleRes(STR_ERROR_IN_ADABAS_INSTALLATION));
    ErrorBox(this, WB_OK, sErrorMsg).Execute();
    EndDialog(RET_CANCEL);
    return 0L;
}
//------------------------------------------------------------------
IMPL_LINK(OAdabasNewDbDlg, OnNoDefaultPath, void*, /*_pArg*/)
{
    String sErrorMsg(ModuleRes(STR_ADABAS_NO_DEFAULTNAME));
    ErrorBox(this, WB_OK, sErrorMsg).Execute();
    return 0L;
}
//------------------------------------------------------------------
IMPL_LINK(OAdabasNewDbDlg, OnNoAccessRights, void*, /*_pArg*/)
{
    String sErrorMsg(ModuleRes(STR_ADABAS_NO_ACCESSRIGHTS));
    ErrorBox(this, WB_OK, sErrorMsg).Execute();
    return 0L;
}

// -----------------------------------------------------------------------
IMPL_LINK( OAdabasNewDbDlg, ImplOKHdl, OKButton*, /*EMPTYARG*/ )
{
    m_SYSDEV_File   = m_ET_SYSDEVSPACE.GetText();
    m_TRANSDEV_File = m_ET_TRANSACTIONLOG.GetText();
    m_DATADEV_File  = m_ET_DATADEVSPACE.GetText();

    m_Backup_File   = m_ET_RESTORE.GetText();
    if(m_bRestore)
    {
        INetURLObject aURL;
        aURL.SetSmartProtocol(INET_PROT_FILE);
        aURL.SetURL(m_Backup_File);
        Content aContent(aURL.GetURLNoPass(),Reference<XCommandEnvironment>());
        try
        {
            if(!aContent.isDocument())
            {
                String aString1 = String(ModuleRes(STR_ADABAS_NO_BACKUP_FILE));
                String aString2 = String(ModuleRes(STR_STAT_WARNING));
                SQLException aSql(aString1,NULL,aString2,1000,Any());
                return 0;
            }

        }
        catch(Exception&)
        {
            String aString1 = String(ModuleRes(STR_ADABAS_NO_BACKUP_FILE));
            String aString2 = String(ModuleRes(STR_STAT_WARNING));
            SQLException aSql(aString1,NULL,aString2,1000,Any());
            throw aSql;
        }
        if(m_Backup_File.Len() > 40)
        {
            InfoBox aInfo(this,ModuleRes(INFO_STR_MAX_FILE_LENGTH));
            String aMsg(aInfo.GetMessText());
            aMsg.SearchAndReplace(String::CreateFromAscii("\'#\'"),String::CreateFromAscii("40"));
            aInfo.SetMessText(aMsg);
            aInfo.Execute();
            return 0;
        }
    }
    else if(m_SYSDEV_File.Len() > 40 || m_TRANSDEV_File.Len() > 40 || m_DATADEV_File.Len() > 40)
    {
        InfoBox aInfo(this,ModuleRes(INFO_STR_MAX_FILE_LENGTH));
        String aMsg(aInfo.GetMessText());
        aMsg.SearchAndReplace(String::CreateFromAscii("\'#\'"),String::CreateFromAscii("40"));
        aInfo.SetMessText(aMsg);
        aInfo.Execute();
        return 0;
    }


    sal_Bool bOk = sal_False;

    {
        // we need url
        String sTemp;
        bOk = LocalFileHelper::ConvertPhysicalNameToURL(m_DATADEV_File,sTemp);
        m_DATADEV_File = sTemp;
        sTemp = String();
        bOk = LocalFileHelper::ConvertPhysicalNameToURL(m_SYSDEV_File,sTemp);
        m_SYSDEV_File = sTemp;
        sTemp = String();
        bOk = LocalFileHelper::ConvertPhysicalNameToURL(m_TRANSDEV_File,sTemp);
        m_TRANSDEV_File = sTemp;
    }
    String aErrorMessage = String(ModuleRes(STR_DEVSPACE_NO_FILE));
    if(!m_SYSDEV_File.Len())
        aErrorMessage.SearchAndReplace(String::CreateFromAscii("$name$"),String::CreateFromAscii("SYSDEVSPACE"));
    else if(!m_TRANSDEV_File.Len())
        aErrorMessage.SearchAndReplace(String::CreateFromAscii("$name$"),String::CreateFromAscii("TRANSACTIONLOG"));
    else if(!m_DATADEV_File.Len())
        aErrorMessage.SearchAndReplace(String::CreateFromAscii("$name$"),String::CreateFromAscii("DATDEVSPACE"));
    else if(m_ET_SYSUSR.GetText() == m_ET_CONUSR.GetText())
        aErrorMessage = String(ModuleRes(STR_NO_SAME_USER));
    else
        bOk = sal_True;


    if (!bOk)
    {
        ErrorBox(this, WB_OK, aErrorMessage).Execute();
        return sal_False;
    }

    sal_Int32 nPos = 0;
    Sequence< PropertyValue> aCreateArgs(m_bRestore ? 14 : 12);
    aCreateArgs[nPos].Name      = PROPERTY_CONTROL_USER;
    aCreateArgs[nPos++].Value  <<= ::rtl::OUString(m_ET_CONUSR.GetText().ToUpperAscii());
    aCreateArgs[nPos].Name      = PROPERTY_CONTROL_PASSWORD;
    aCreateArgs[nPos++].Value  <<= ::rtl::OUString(m_ConPwd);
    aCreateArgs[nPos].Name      = PROPERTY_USER;
    aCreateArgs[nPos++].Value  <<= ::rtl::OUString(m_ET_SYSUSR.GetText().ToUpperAscii());
    aCreateArgs[nPos].Name      = PROPERTY_PASSWORD;
    aCreateArgs[nPos++].Value  <<= ::rtl::OUString(m_SysPwd);
    aCreateArgs[nPos].Name      = PROPERTY_DOMAINPASSWORD;
    aCreateArgs[nPos++].Value  <<= ::rtl::OUString(m_DomainPwd);
    aCreateArgs[nPos].Name      = PROPERTY_CACHESIZE;
    aCreateArgs[nPos++].Value  <<= ::rtl::OUString::valueOf(sal_Int32(m_NF_CACHE_SIZE.GetValue()*256));
    aCreateArgs[nPos].Name      = PROPERTY_DATABASENAME;
    aCreateArgs[nPos++].Value  <<= ::rtl::OUString(m_ET_DATABASENAME.GetText().ToUpperAscii());
    aCreateArgs[nPos].Name      = PROPERTY_DATADEVSPACE;
    aCreateArgs[nPos++].Value  <<= ::rtl::OUString(m_DATADEV_File);
    aCreateArgs[nPos].Name      = PROPERTY_SYSDEVSPACE;
    aCreateArgs[nPos++].Value  <<= ::rtl::OUString(m_SYSDEV_File);
    aCreateArgs[nPos].Name      = PROPERTY_TRANSACTION_LOG;
    aCreateArgs[nPos++].Value  <<= ::rtl::OUString(m_TRANSDEV_File);
    aCreateArgs[nPos].Name      = PROPERTY_DATADEVSIZE;
    aCreateArgs[nPos++].Value  <<= sal_Int32(m_NF_DATADEVSPACE_SIZE.GetValue()*256);
    aCreateArgs[nPos].Name      = PROPERTY_LOGDEVSIZE;
    aCreateArgs[nPos++].Value  <<=  sal_Int32(m_NF_TRANSACTIONLOG_SIZE.GetValue()*256);

    if(m_bRestore)
    {
        aCreateArgs[nPos].Name      = PROPERTY_RESTOREDATABASE;
        aCreateArgs[nPos++].Value  <<= ::cppu::bool2any(sal_True);
        aCreateArgs[nPos].Name      = PROPERTY_BACKUPNAME;
        aCreateArgs[nPos++].Value  <<= ::rtl::OUString(m_Backup_File);
    }

    Disable();

    m_aCreatorWait = ::std::auto_ptr<WaitObject>(new WaitObject(this));
    m_aCreatorThread = ::std::auto_ptr<ODatabaseCreator>(new ODatabaseCreator(m_xCreateCatalog,this,m_xORB,aCreateArgs));
    m_aCreatorThread->setTerminateHandler(LINK(this,OAdabasNewDbDlg,TerminateHdl));
    m_aCreatorThread->create();

    return sal_True;
}
//-------------------------------------------------------------------------
IMPL_LINK( OAdabasNewDbDlg, TerminateHdl, void*, /*NOTUSABLE*/ )
{
    EndDialog(RET_OK);
    return 0;
}
//------------------------------------------------------------------------
IMPL_LINK( OAdabasNewDbDlg, LoseFocusHdl, Edit *, pEdit )
{
    if(&m_ET_SYSDEVSPACE == pEdit)
        m_SYSDEV_File = pEdit->GetText();
    else if(&m_ET_TRANSACTIONLOG == pEdit)
        m_TRANSDEV_File = pEdit->GetText();
    else if(&m_ET_DATADEVSPACE == pEdit)
        m_DATADEV_File = pEdit->GetText();
    else if(&m_ET_RESTORE == pEdit)
        m_Backup_File = pEdit->GetText();
    else if(&m_ET_DATABASENAME == pEdit)
    {// check if the new dbname already exists
        String sNewDbName = pEdit->GetText();
        String sConfig = m_sDbConfig;
        sConfig.AppendAscii("config/");
        sConfig += sNewDbName;
        if(UCBContentHelper::IsDocument(sConfig))
        {
            String sErrorMessage = String(ModuleRes(STR_ADABAS_DB_EXISTS));
            ErrorBox(this, WB_OK, sErrorMessage).Execute();
            m_DBName.Erase();
        }
        else if ( m_DBName.SearchAscii(" ") != STRING_NOTFOUND )
        {
            String sErrorMessage = String(ModuleRes(STR_ADABAS_DB_EXISTS));
            ErrorBox(this, WB_OK, sErrorMessage).Execute();
            m_DBName = m_DBName.GetToken(0,' ');
        }
        else
            m_DBName = sNewDbName;
    }

    CheckBitmaps();
    return 0;
}
//------------------------------------------------------------------------
IMPL_LINK( OAdabasNewDbDlg, LoadButtonClickHdl, Button *, /*pButton*/ )
{
    ::sfx2::FileDialogHelper aLoad(WB_3DLOOK | WB_STDMODAL | WB_OPEN);
    if(!m_Backup_File.Len())
        aLoad.SetDisplayDirectory(SvtPathOptions().GetWorkPath());
    else
        aLoad.SetDisplayDirectory(m_Backup_File);
    if (aLoad.Execute())
    {
        INetURLObject aUrl(aLoad.GetPath());
        m_Backup_File = aUrl.PathToFileName();
        if(m_Backup_File.Len() > 40)
        {
            InfoBox aInfo(this,ModuleRes(INFO_STR_MAX_FILE_LENGTH));
            String aMsg(aInfo.GetMessText());
            aMsg.SearchAndReplace(String::CreateFromAscii("\'#\'"),String::CreateFromAscii("40"));
            aInfo.SetMessText(aMsg);
        }
        m_ET_RESTORE.SetText(m_Backup_File);
    }
    CheckBitmaps();
    return 0;
}
//------------------------------------------------------------------------
IMPL_LINK( OAdabasNewDbDlg, ButtonClickHdl, Button *, pButton )
{
    WinBits nBits(WB_3DLOOK|WB_STDMODAL|WB_SAVEAS);
    ::sfx2::FileDialogHelper aFileDlg( static_cast<sal_uInt32>(nBits) ); // new FileDialog( this, nBits );

    String aPath;
    if(pButton == &m_PB_SYSDEVSPACE)
        aPath = m_SYSDEV_File;
    else if(pButton == &m_PB_TRANSACTIONLOG)
        aPath = m_TRANSDEV_File;
    else if(pButton == &m_PB_DATADEVSPACE)
        aPath = m_DATADEV_File;

    String sUrl;
    ::utl::LocalFileHelper::ConvertPhysicalNameToURL(aPath,sUrl);
    aFileDlg.SetDisplayDirectory(sUrl);



    if (aFileDlg.Execute() == ERRCODE_NONE)
    {
        INetURLObject aUrl(aFileDlg.GetPath());
        if(pButton == &m_PB_SYSDEVSPACE)
        {
            m_SYSDEV_File = aUrl.PathToFileName();
            if(m_SYSDEV_File.Len() > 40)
            {
                InfoBox aInfo(this,ModuleRes(INFO_STR_MAX_FILE_LENGTH));
                String aMsg(aInfo.GetMessText());
                aMsg.SearchAndReplace(String::CreateFromAscii("\'#\'"),String::CreateFromAscii("40"));
                aInfo.SetMessText(aMsg);
                aInfo.Execute();
                return 0;
            }
            m_ET_SYSDEVSPACE.SetText(aUrl.PathToFileName());
        }
        else if(pButton == &m_PB_TRANSACTIONLOG)
        {
            m_TRANSDEV_File = aUrl.PathToFileName();
            if(m_TRANSDEV_File.Len() > 40)
            {
                InfoBox aInfo(this,ModuleRes(INFO_STR_MAX_FILE_LENGTH));
                String aMsg(aInfo.GetMessText());
                aMsg.SearchAndReplace(String::CreateFromAscii("\'#\'"),String::CreateFromAscii("40"));
                aInfo.SetMessText(aMsg);
                aInfo.Execute();
                return 0;
            }
            m_ET_TRANSACTIONLOG.SetText(aUrl.PathToFileName());
        }
        else if(pButton == &m_PB_DATADEVSPACE)
        {
            m_DATADEV_File = aUrl.PathToFileName();
            if(m_DATADEV_File.Len() > 40)
            {
                InfoBox aInfo(this,ModuleRes(INFO_STR_MAX_FILE_LENGTH));
                String aMsg(aInfo.GetMessText());
                aMsg.SearchAndReplace(String::CreateFromAscii("\'#\'"),String::CreateFromAscii("40"));
                aInfo.SetMessText(aMsg);
                aInfo.Execute();
                return 0;
            }
            m_ET_DATADEVSPACE.SetText(aUrl.PathToFileName());
        }
    }

    CheckBitmaps();
    return 0;
}
//------------------------------------------------------------------------
IMPL_LINK( OAdabasNewDbDlg, PwdClickHdl, Button *, pButton )
{
    SfxPasswordDialog aDlg(this);
    aDlg.ShowExtras(SHOWEXTRAS_CONFIRM);
    if(aDlg.Execute())
    {
        String sPwd = aDlg.GetPassword().ToUpperAscii();
        // no space in password allowed
        if ( sPwd.GetTokenCount(' ') == 1 )
        {
            if(pButton == &m_PB_CONPWD)
            {
                m_ConPwd = sPwd;
                m_nSetBitmap &=~2;
            }
            else if(pButton == &m_PB_SYSPWD)
            {
                m_SysPwd = sPwd;
                if(!m_DomainPwd.Len())
                {
                    m_nSetBitmap = 1;
                    m_DomainPwd = m_SysPwd;
                }
                if(!m_ConPwd.Len())
                {
                    m_nSetBitmap |= 2;
                    m_ConPwd = m_SysPwd;
                }

            }
            else if(pButton == &m_PB_DOMAINPWD)
            {
                m_DomainPwd = sPwd;
                m_nSetBitmap &=~1;
            }
        }
        else
        {
            String sErrorMsg(ModuleRes(STR_ADABAS_PASSWORD_ILLEGAL));
            ErrorBox(this, WB_OK, sErrorMsg).Execute();
        }
    }
    CheckBitmaps();
    return 0;
}
//------------------------------------------------------------------------
void OAdabasNewDbDlg::CheckBitmaps()
{
    sal_Int16 i=0;
    if(m_ConPwd.Len() && m_ET_CONUSR.GetText().Len())
            i++,m_BMP_CON.SetImage(ModuleRes(((m_nSetBitmap & 2) == 2) ? DEFAULT_BLUE : CHECK_GREEN));
    else
        m_BMP_CON.SetImage(ModuleRes(UNCHECK_RED));

    if(!m_bRestore)
    {
        if(m_SysPwd.Len() && m_ET_SYSUSR.GetText().Len())
            i++,m_BMP_SYS.SetImage(ModuleRes(CHECK_GREEN));
        else
            m_BMP_SYS.SetImage(ModuleRes(UNCHECK_RED));

        if(m_DomainPwd.Len() )
            i++,m_BMP_DOMAIN.SetImage(ModuleRes(((m_nSetBitmap & 1) == 1) ? DEFAULT_BLUE : CHECK_GREEN));
        else
            m_BMP_DOMAIN.SetImage(ModuleRes(UNCHECK_RED));
    }
    else if(m_Backup_File.Len())
        i+=2;

    if(m_ET_SYSDEVSPACE.GetText().Len() && m_ET_TRANSACTIONLOG.GetText().Len() && m_ET_DATADEVSPACE.GetText().Len() &&
        m_ET_SYSDEVSPACE.GetText() != m_ET_TRANSACTIONLOG.GetText() && m_ET_SYSDEVSPACE.GetText() != m_ET_DATADEVSPACE.GetText() &&
        m_ET_DATADEVSPACE.GetText() != m_ET_TRANSACTIONLOG.GetText())
        i++;

    if(m_DBName.Len())
        i++;

    m_PB_OK.Enable(i == 5);
}
// -----------------------------------------------------------------------------
sal_Bool OAdabasNewDbDlg::fillEnvironmentVariable(const ::rtl::OUString& _sVariableName,String& _rsValue)
{
    rtl_uString* pDbVar = NULL;
    sal_Bool bError = sal_False;
    if(osl_getEnvironment(_sVariableName.pData,&pDbVar) == osl_Process_E_None && pDbVar)
    {
        _rsValue = pDbVar;
        String sTemp;
        LocalFileHelper::ConvertPhysicalNameToURL(_rsValue,sTemp);
        _rsValue = sTemp;
        rtl_uString_release(pDbVar);
        pDbVar = NULL;

        // ensure dir exists
        osl::FileBase::RC rc = osl::Directory::createPath(_rsValue);
        if(rc != osl::FileBase::E_None && rc != osl::FileBase::E_EXIST)
        {
            bError = sal_True;
            PostUserEvent(LINK(this, OAdabasNewDbDlg, OnError));
        }
    }
    return bError;
}
// -----------------------------------------------------------------------------
