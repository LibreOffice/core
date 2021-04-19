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

#pragma once

#include "adminpages.hxx"
#include <curledit.hxx>
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
        bool            m_bUserGrabFocus;

    public:
        OConnectionHelper(weld::Container* pPage, weld::DialogController* pController, const OUString& _rUIXMLDescription, const OString& _rId, const SfxItemSet& _rCoreAttrs);
        virtual ~OConnectionHelper() override;

        OUString     m_eType;          // the type can't be changed in this class, so we hold it as member.
        // setting/retrieving the current connection URL
        // necessary because for some types, the URL must be decoded for display purposes
        ::dbaccess::ODsnTypeCollection* m_pCollection;  /// the DSN type collection instance

        std::unique_ptr<weld::Label> m_xFT_Connection;
        std::unique_ptr<weld::Button> m_xPB_Connection;
        std::unique_ptr<weld::Button> m_xPB_CreateDB;
        std::unique_ptr<OConnectionURLEdit> m_xConnectionURL;

    public:

        // <method>OGenericAdministrationPage::fillControls</method>
        virtual void    fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;
        // <method>OGenericAdministrationPage::fillWindows</method>
        virtual void    fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;
        virtual void    implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;

        // setting/retrieving the current connection URL
        // necessary because for some types, the URL must be decoded for display purposes
        //String        getURL( OConnectionURLEdit* _m_pConnection ) const;
        //void      setURL( const OUString& _rURL, OConnectionURLEdit* _m_pConnection );

        OUString    getURLNoPrefix( ) const;
        void        setURLNoPrefix( std::u16string_view _rURL );

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
        void            setURL( std::u16string_view _rURL );
        virtual bool    checkTestConnection();

    private:
        DECL_LINK(OnBrowseConnections, weld::Button&, void);
        DECL_LINK(OnCreateDatabase, weld::Button&, void);
        DECL_LINK(GetFocusHdl, weld::Widget&, void);
        DECL_LINK(LoseFocusHdl, weld::Widget&, void);
        OUString    impl_getURL() const;
        void        impl_setURL( std::u16string_view _rURL, bool _bPrefix );
        void        implUpdateURLDependentStates() const;
    };


}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
