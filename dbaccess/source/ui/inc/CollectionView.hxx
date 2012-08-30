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

#ifndef DBAUI_COLLECTIONVIEW_HXX
#define DBAUI_COLLECTIONVIEW_HXX

#include <svtools/fileview.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

//.........................................................................
namespace dbaui
{
//.........................................................................
    /* this class allows to browse through the collection of forms and reports
    */
    class OCollectionView : public ModalDialog
    {
        FixedText       m_aFTCurrentPath;
        ImageButton     m_aNewFolder;
        ImageButton     m_aUp;
        SvtFileView     m_aView;
        FixedText       m_aFTName;
        Edit            m_aName;
        FixedLine       m_aFL;
        PushButton      m_aPB_OK;
        CancelButton    m_aPB_CANCEL;
        HelpButton      m_aPB_HELP;
        String          m_sPath;
        ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent>                  m_xContent;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xORB;
        Size            m_aDlgSize;
        Size            m_a6Size;
        sal_Bool        m_bCreateForm;

        DECL_LINK(Up_Click,void*);
        DECL_LINK(NewFolder_Click,void*);
        DECL_LINK(Save_Click,void*);
        DECL_LINK(Dbl_Click_FileView,void*);

        /// sets the fixedtext to the right content
        void initCurrentPath();
    public:
        OCollectionView( Window * pParent
                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent>& _xContent
                        ,const ::rtl::OUString& _sDefaultName
                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xORB);
        virtual ~OCollectionView();

        ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent> getSelectedFolder() const;
        ::rtl::OUString getName() const;
    };
//.........................................................................
}   // namespace dbaui
//.........................................................................


#endif //DBAUI_COLLECTIONVIEW_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
