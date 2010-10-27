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
        sal_Int32       m_nFixDeltaHeight;
        sal_Bool        m_bCreateForm;

        DECL_LINK(Up_Click,PushButton*);
        DECL_LINK(NewFolder_Click,PushButton*);
        DECL_LINK(Save_Click,PushButton*);
        DECL_LINK(Dbl_Click_FileView,SvtFileView*);

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
