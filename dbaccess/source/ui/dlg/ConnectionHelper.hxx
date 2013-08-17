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

#ifndef DBAUI_CONNECTIONHELPER_HXX
#define DBAUI_CONNECTIONHELPER_HXX

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
        sal_Bool            m_bUserGrabFocus : 1;

    public:
        OConnectionHelper( Window* pParent, const ResId& _rId, const SfxItemSet& _rCoreAttrs);
        virtual ~OConnectionHelper();
        FixedText           m_aFT_Connection;
        OConnectionURLEdit  m_aConnectionURL;
        PushButton          m_aPB_Connection;
        OUString     m_eType;          // the type can't be changed in this class, so we hold it as member.

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

        IS_PATH_EXIST   pathExists(const OUString& _rURL, sal_Bool bIsFile) const;
        sal_Bool        createDirectoryDeep(const String& _rPathNormalized);
        sal_Bool        commitURL();

        /** opens the FileOpen dialog and asks for a FileName
            @param  _aFileOpen
                Executes the file open dialog, which must be filled from caller.
        */
        void askForFileName(::sfx2::FileDialogHelper& _aFileOpen);

    protected:
        void            setURL( const String& _rURL );
        virtual bool    checkTestConnection();

    private:
        DECL_LINK(OnBrowseConnections, PushButton*);
        String      impl_getURL( sal_Bool _bPrefix ) const;
        void        impl_setURL( const String& _rURL, sal_Bool _bPrefix );
        void        implUpdateURLDependentStates() const;
    };

}   // namespace dbaui

#endif // DBAUI_CONNECTIONHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
