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

#ifndef DBAUI_CONNECTIONHELPER_HXX
#define DBAUI_CONNECTIONHELPER_HXX

#include "adminpages.hxx"
#include <ucbhelper/content.hxx>
#include "curledit.hxx"
#include <sfx2/filedlghelper.hxx>


#define FILL_STRING_ITEM(editcontrol, itemset, itemid, modifiedflag)    \
    if (editcontrol.GetText() != editcontrol.GetSavedValue())           \
    {                                                                   \
        itemset.Put(SfxStringItem(itemid, editcontrol.GetText()));      \
        modifiedflag = sal_True;                                        \
    }

//.........................................................................
namespace dbaui
{
//.........................................................................

    enum IS_PATH_EXIST
    {
        PATH_NOT_EXIST = 0,
        PATH_EXIST,
        PATH_NOT_KNOWN
    };

    class IDatabaseSettingsDialog;

    class OConnectionHelper : public OGenericAdministrationPage
    {
        sal_Bool            m_bUserGrabFocus : 1;

    public:
        OConnectionHelper( Window* pParent, const ResId& _rId, const SfxItemSet& _rCoreAttrs);
        virtual ~OConnectionHelper();
        FixedText           m_aFT_Connection;
        OConnectionURLEdit  m_aConnectionURL;
        PushButton          m_aPB_Connection;
        ::rtl::OUString     m_eType;          // the type can't be changed in this class, so we hold it as member.

    public:

        // setting/retrieving the current connection URL
        // necessary because for some types, the URL must be decoded for display purposes
        ::dbaccess::ODsnTypeCollection* m_pCollection;  /// the DSN type collection instance
        virtual long    PreNotify( NotifyEvent& _rNEvt );

        // <method>OGenericAdministrationPage::fillControls</method>
        virtual void    fillControls(::std::vector< ISaveValueWrapper* >& _rControlList);
        // <method>OGenericAdministrationPage::fillWindows</method>
        virtual void    fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList);
        virtual void    implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);


        // setting/retrieving the current connection URL
        // necessary because for some types, the URL must be decoded for display purposes
        //String        getURL( OConnectionURLEdit* _m_pConnection ) const;
        //void      setURL( const String& _rURL, OConnectionURLEdit* _m_pConnection );

        String      getURLNoPrefix( ) const;
        void        setURLNoPrefix( const String& _rURL );

        /** checks if the path is existence
            @param  _rURL
                The URL to check.
        */
        sal_Int32   checkPathExistence(const String& _rURL);


        IS_PATH_EXIST   pathExists(const ::rtl::OUString& _rURL, sal_Bool bIsFile) const;
        sal_Bool        createDirectoryDeep(const String& _rPathNormalized);
        sal_Bool        commitURL();

        /** opens the FileOpen dialog and asks for a FileName
            @param  _aFileOpen
                Executes the file open dialog, which must be filled from caller.
        */
        void askForFileName(::sfx2::FileDialogHelper& _aFileOpen);

           virtual void SetServiceFactory(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > _rxORB)
        {
            OGenericAdministrationPage::SetServiceFactory(_rxORB);
        }

    protected:
        void            setURL( const String& _rURL );
        virtual bool    checkTestConnection();

    private:
        DECL_LINK(OnBrowseConnections, PushButton*);
        StringBag   getInstalledAdabasDBDirs(const String &_rPath,const ::ucbhelper::ResultSetInclude& _reResultSetInclude);
        StringBag   getInstalledAdabasDBs(const String &_rConfigDir,const String &_rWorkDir);
        String      impl_getURL( sal_Bool _bPrefix ) const;
        void        impl_setURL( const String& _rURL, sal_Bool _bPrefix );
        void        implUpdateURLDependentStates() const;
    };

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // DBAUI_CONNECTIONHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
