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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_DLG_CONNECTIONHELPER_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_DLG_CONNECTIONHELPER_HXX

#include "adminpages.hxx"
#include <ucbhelper/content.hxx>
#include "curledit.hxx"
#include <sfx2/filedlghelper.hxx>

namespace dbaui
{

    enum IS_PATH_EXIST
    {
        PATH_NOT_EXIST = 0,
        PATH_EXIST,
        PATH_NOT_KNOWN
    };

    class OConnectionHelper : public OGenericAdministrationPage
    {
        bool            m_bUserGrabFocus : 1;

    public:
        OConnectionHelper( vcl::Window* pParent, const OString& _rId, const OUString& _rUIXMLDescription, const SfxItemSet& _rCoreAttrs);
        virtual ~OConnectionHelper();
        virtual void dispose() override;
        VclPtr<FixedText>           m_pFT_Connection;
        VclPtr<OConnectionURLEdit>  m_pConnectionURL;
        VclPtr<PushButton>          m_pPB_Connection;
        VclPtr<PushButton>          m_pPB_CreateDB;
        OUString     m_eType;          // the type can't be changed in this class, so we hold it as member.

    public:

        // setting/retrieving the current connection URL
        // necessary because for some types, the URL must be decoded for display purposes
        ::dbaccess::ODsnTypeCollection* m_pCollection;  /// the DSN type collection instance
        virtual bool    PreNotify( NotifyEvent& _rNEvt ) override;

        // <method>OGenericAdministrationPage::fillControls</method>
        virtual void    fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) override;
        // <method>OGenericAdministrationPage::fillWindows</method>
        virtual void    fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) override;
        virtual void    implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;

        // setting/retrieving the current connection URL
        // necessary because for some types, the URL must be decoded for display purposes
        //String        getURL( OConnectionURLEdit* _m_pConnection ) const;
        //void      setURL( const OUString& _rURL, OConnectionURLEdit* _m_pConnection );

        OUString    getURLNoPrefix( ) const;
        void        setURLNoPrefix( const OUString& _rURL );

        /** checks if the path is existence
            @param  _rURL
                The URL to check.
        */
        sal_Int32   checkPathExistence(const OUString& _rURL);

        IS_PATH_EXIST   pathExists(const OUString& _rURL, bool bIsFile) const;
        bool        createDirectoryDeep(const OUString& _rPathNormalized);
        bool        commitURL();

        /** opens the FileOpen dialog and asks for a FileName
            @param  _aFileOpen
                Executes the file open dialog, which must be filled from caller.
        */
        void askForFileName(::sfx2::FileDialogHelper& _aFileOpen);

    protected:
        void            setURL( const OUString& _rURL );
        virtual bool    checkTestConnection();

    private:
        DECL_LINK_TYPED(OnBrowseConnections, Button*, void);
        DECL_LINK_TYPED(OnCreateDatabase, Button*, void);
        OUString    impl_getURL( bool _bPrefix ) const;
        void        impl_setURL( const OUString& _rURL, bool _bPrefix );
        void        implUpdateURLDependentStates() const;
    };

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_DLG_CONNECTIONHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
